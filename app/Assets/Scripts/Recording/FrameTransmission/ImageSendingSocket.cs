using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using UnityEngine;

namespace Reconstruction4D.Recording.FrameTransmission
{
    class ImageSendingSocket
    {
        /// <summary>
        /// This delegate is call when a frame is not send correctly
        /// </summary>
        /// <param name="error">The error for send frame</param>
        public delegate void OnFrameSendingError(string error);

        public delegate void OnSocketClose(long totalFrameSend);

        // Singleton references
        private static ImageSendingSocket instance = null;
        private static readonly object padlock = new object();

        ImageSendingSocket() {}

        /// <summary>
        /// This application require at max one instance of the ImageSendingSocket you can obtain his reference from this singleton
        /// </summary>
        public static ImageSendingSocket Instance
        {
            get
            {
                lock (padlock)
                {
                    if (instance == null)
                    {
                        instance = new ImageSendingSocket();
                    }
                    return instance;
                }
            }
        }

        private bool active = false;
        private uint connectionId = 0;

        private OnFrameSendingError callbackOnFrameSendingError;
        private OnSocketClose callbackOnClose;

        private ConcurrentQueue<FrameNode> requests;
        private bool endElaboration = false;
        private Thread encodingThread = null;
        private Thread sendingThread = null;

        private long enqueueFrame = 0;

        private long totalFrame = 0;

        // Debug Fields
        string encodeSuccessFrameFilename = "";
        private List<long> encodeSuccessFramesList = new List<long>(10);

        /// <summary>
        /// Create the structure for a socket where to send the data
        /// </summary>
        /// <param name="maximumRate">the maximum usable bandwidth by the socket</param>
        /// <returns>the socket structure identifier</returns>
        [DllImport("image_sending", CharSet = CharSet.Ansi)]
        private static extern uint initSendingImage(ulong maximumRate, [MarshalAs(UnmanagedType.LPStr)] string debugFolder);

        /// <summary>
        /// Init the base common field for the connection
        /// </summary>
        /// <param name="maximumRate">the maximum usable bandwidth by the socket</param>
        private void initLibrary(ulong maximumRate)
        {
            encodeSuccessFrameFilename = Path.Combine(Application.persistentDataPath, "EncodedFrame" + DateTime.Now.ToString("yyyy-MM-dd-hh-mm-ss") + ".txt");
            this.connectionId = initSendingImage(maximumRate, Application.persistentDataPath);
            this.enqueueFrame = 0;
        }

        /// <summary>
        /// This function set how many client the server must expect inside the session
        /// </summary>
        /// <param name="position">the socket ID</param>
        /// <param name="clients">number of clients</param>
        [DllImport("image_sending")]
        private static extern void setSessionClient(uint position, uint clients);

        /// <summary>
        /// This function return the streamID of the session create initSendingImage call
        /// </summary>
        /// <param name="position">the socket ID</param>
        /// <returns>streamID</returns>
        [DllImport("image_sending", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.LPStr)] private static extern string getStreamId(uint position);

        /// <summary>
        /// Init a connection to the server
        /// </summary>
        /// <param name="serverIpAddress">IP address</param>
        /// <param name="serverPort">Elaboration Server Port</param>
        /// <param name="phoneId">A UUID of the phone that make the connection</param>
        /// <param name="maximumRate">the maximum usable bandwidth by the socket</param>
        /// <param name="numberOfClients">number of clients for the session</param>
        /// <param name="callbackOnError">function to call on error</param>
        /// <param name="callbackSocketClose">function to call when socket is close</param>
        /// <returns>the UUID of the session created</returns>
        public string InitConnectionAsHost(string serverIpAddress, uint serverPort, string phoneId, ulong maximumRate, uint numberOfClients, OnFrameSendingError callbackOnError, OnSocketClose callbackSocketClose)
        {
            initLibrary(maximumRate);
            setSessionClient(connectionId, numberOfClients);
            if(startSocket(serverIpAddress, serverPort, phoneId, callbackOnError, callbackSocketClose))
            {
                return getStreamId(connectionId);
            }
            return "";
        }

        /// <summary>
        /// Set the session ID of the session that client need to join
        /// </summary>
        /// <param name="position">the socket ID</param>
        /// <param name="streamId">the ID of the stream</param>
        [DllImport("image_sending", CharSet = CharSet.Ansi)]
        private static extern void setSessionIdentifier(uint position, [MarshalAs(UnmanagedType.LPStr)] string streamId);

        /// <summary>
        /// Set the session ID of the session that client need to join
        /// </summary>
        /// <param name="position">the socket ID</param>
        /// <param name="streamId">the ID of the stream</param>
        public bool InitConnectionAsClient(string serverIpAddress, uint serverPort, string phoneId, ulong maximumRate, string streamId, OnFrameSendingError callbackOnError, OnSocketClose callbackSocketClose)
        {
            initLibrary(maximumRate);
            setSessionIdentifier(connectionId, streamId);
            return startSocket(serverIpAddress, serverPort, phoneId, callbackOnError, callbackSocketClose);
        }

        /// <summary>
        /// Connect to the server and init the section for the given structure
        /// </summary>
        /// <param name="position">the socket ID</param>
        /// <param name="serverIpAddress">IP address</param>
        /// <param name="serverPort">Elaboration Server Port</param>
        /// <param name="phoneId">A UUID of the phone that make the connection</param>
        /// <returns>true if connection success, else false</returns>
        [DllImport("image_sending", CharSet = CharSet.Ansi)]
        private static extern bool createConnection(uint position, [MarshalAs(UnmanagedType.LPStr)] string serverIpAddress, uint serverPort, [MarshalAs(UnmanagedType.LPStr)] string phoneId);

        /// <summary>
        /// Start the connection and if success the threads to elaborate and send the frames
        /// </summary>
        /// <param name="serverIpAddress">IP address</param>
        /// <param name="serverPort">Elaboration Server Port</param>
        /// <param name="phoneId">A UUID of the phone that make the connection</param>
        /// <param name="callbackOnError">function to call on error</param>
        /// <param name="callbackSocketClose">function to call when socket is close</param>
        /// <returns>true if the connection success, else false</returns>
        private bool startSocket(string serverIpAddress, uint serverPort, string phoneId, OnFrameSendingError callbackOnError, OnSocketClose callbackSocketClose)
        {
            bool result = createConnection(connectionId, serverIpAddress, serverPort, phoneId);
            if(result == false)
            {
                return false;
            }
            active = true;
            Debug.Log("Image Sending init correctly");
            StartThreads(callbackOnError, callbackSocketClose);
            return true;
        }

        /// <summary>
        /// Enqueue a frames into the list to elaborate it
        /// </summary>
        /// <param name="node">the frame to enqueue</param>
        public void EnquequeFrame(ref FrameNode node)
        {
            requests.Enqueue(node);
            Debug.Log("Queue size: " + requests.Count);
        }

        /// <summary>
        /// Stop abrutally the frame elaboration and the sending thread, don't do this if not absolutely neccessary
        /// </summary>
        public void ForceStopElaboration()
        {
            EndElaboration(enqueueFrame);
            if (encodingThread == null) return;
            if (encodingThread.ThreadState != System.Threading.ThreadState.Aborted && encodingThread.ThreadState != System.Threading.ThreadState.AbortRequested)
            {
                encodingThread.Abort();
            }

            if (sendingThread == null) return;
            if (sendingThread.ThreadState != System.Threading.ThreadState.Aborted && sendingThread.ThreadState != System.Threading.ThreadState.AbortRequested)
            {
                sendingThread.Abort();
            }
        }

        /// <summary>
        /// Start the thread for elaborate the images and sending it
        /// </summary>
        /// <param name="callbackOnError">function to call on error</param>
        /// <param name="callbackSocketClose">function to call when socket is close</param>
        private void StartThreads(OnFrameSendingError callbackOnError, OnSocketClose callbackSocketClose)
        {
            endElaboration = false;
            requests = new ConcurrentQueue<FrameNode>();
            Debug.Log("Parameter set");
            encodingThread = new Thread(JpegEncoding);
            encodingThread.Start();
            sendingThread = new Thread(ImageSending);
            sendingThread.Start();
            
            this.callbackOnFrameSendingError = callbackOnError;
            this.callbackOnClose = callbackSocketClose;
            Debug.Log("Thread start");
        }

        /// <summary>
        /// Encode and enqueue a frame
        /// </summary>
        /// <param name="position">the socket ID</param>
        /// <param name="frameId">the incremental identifier of the frame inside the sequence of frame capture</param>
        /// <param name="y_buffer">the reference to the Y part of a YVU image</param>
        /// <param name="vu_buffer">the reference to the V part of a VU image</param>
        /// <param name="width">The width of the image</param>
        /// <param name="height">The height of the image</param>
        /// <param name="additionalData">The additionalData to bind with the image</param>
        /// <param name="additionalDataLenght">The lenght of the additionalData array/param>
        /// <returns></returns>
        [DllImport("image_sending")]
        private static extern bool sendImage(uint position, ulong frameId, byte[] y_buffer, byte[] vu_buffer, int width, int height, byte[] additionalData, uint additionalDataLenght);

        /// <summary>
        /// That function run inside a thread and continue to extract elements from the elaboration queue and send to the sender below, until the elaboration end and there's no more frame to send
        /// </summary>
        private void JpegEncoding()
        {
            while (endElaboration == false)
            {
                Thread.Sleep(10);
                while (requests.TryDequeue(out FrameNode item))
                {
                    try
                    {
                        AdditionalDataSend additionalDataContainer = new AdditionalDataSend()
                        {
                            position = item.position,
                            rotation = item.rotation,
                            positionOri = item.positionOri,
                            rotationOri = item.rotationOri,
                            focalLength = item.focalLength,
                            principalPoint = item.principalPoint,
                        };
                        byte[] additionalData = Encoding.UTF8.GetBytes(JsonUtility.ToJson(additionalDataContainer));

                        bool result = false;
                        int limit = 0;
                        do
                        {
                            result = sendImage(connectionId, (ulong)item.frameId, item.yBuffer, item.vuBuffer, item.imageWidth, item.imageHeight, additionalData, (uint)additionalData.Length);
                            limit++;
                        }
                        while (result == false && limit < 10);

                        if(result == true)
                        {
                            encodeSuccessFramesList.Add(item.frameId);
                            if(encodeSuccessFramesList.Capacity - 1 >= encodeSuccessFramesList.Count)
                            {
                                StringBuilder logToSave = new StringBuilder();
                                foreach (long element in encodeSuccessFramesList)
                                {
                                    logToSave.AppendLine(element.ToString());
                                }
                                FileLogger.AppendText(this.encodeSuccessFrameFilename, logToSave.ToString());
                                encodeSuccessFramesList.Clear();
                            }
                        }

                        enqueueFrame++;
                        Debug.Log("Elaboration complete, queue size: " + requests.Count);
                        continue;
                    }
                    catch (Exception e)
                    {
                        Debug.LogError("Error during image sending, exception message" + e.ToString());
                        callbackOnFrameSendingError("Error during image sending, exception message" + e.ToString());
                        continue;
                    }
                }
            }
            sendingComplete(connectionId, (ulong)totalFrame);
        }

        /// <summary>
        /// This function is thinked to run inside a thread (managed by the caller) and continue to send the encoded frame until the queue is empty and elaboration finish
        /// </summary>
        /// <param name="position">the socket ID</param>
        [DllImport("image_sending")]
        private static extern void sendingImageThreadFuction(uint position);

        /// <summary>
        /// The function map the lib function to the C# function and also when the below function finish to send it call the callback to execute on socket closing
        /// </summary>
        private void ImageSending()
        {
            sendingImageThreadFuction(connectionId);
            this.callbackOnClose?.Invoke(enqueueFrame);
        }

        /// <summary>
        /// This function say to the sendingImageThreadFuction to stop wait other frame when it send all the remain
        /// </summary>
        /// <param name="position">the socket ID</param>
        /// <param name="totalFrame">Number of frame host send during this period</param>
        [DllImport("image_sending")]
        private static extern void sendingComplete(uint position, ulong totalFrame);

        /// <summary>
        /// This function say to the sendingImageThreadFuction to stop wait other frame when it send all the remain
        /// </summary>
        /// <param name="position">the socket ID</param>
        /// <param name="totalFrame">Number of frame host send during this period</param>
        [DllImport("image_sending")]
        private static extern void setMaximumRate(uint position, ulong maximumRate);

        /// <summary>
        /// Stop the sending of all the frame
        /// </summary>
        /// <param name="totalFrame">Number of frame host send during this period</param>
        public void EndElaboration(long totalFrame)
        {
            endElaboration = true;
            this.totalFrame = totalFrame;
            setMaximumRate(connectionId, 0);
        }
    }
}
