using MLAPI;
using MLAPI.Messaging;
using MLAPI.Serialization.Pooled;
using Reconstruction4D.UI;
using System;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.UI;

namespace Reconstruction4D.Recording
{
    using AcquisitionDelays = System.Collections.Generic.KeyValuePair<ulong, double>;

    [RequireComponent(typeof(SchedulerController))]
    public class VideoStreamingController : NetworkedBehaviour
    {
        // Debug Information

        private Text snackbarText;

        /// <summary>
        /// Callback for when there is an error during streaming
        /// </summary>
        public delegate void ErrorDuringStreaming(string message);

        /// <summary>
        /// This is the callback to call in case of error during streaming
        /// </summary>
        private ErrorDuringStreaming callbackStreamingError;

        /// <summary>
        /// Callback for when there is an error during streaming
        /// </summary>
        public delegate void AllFrameSend(long totalFrameSend);

        /// <summary>
        /// This is the callback to call when frame sending is complete
        /// </summary>
        private AllFrameSend callbackAllFrameSend;

        // Shared Variable

        /// <summary>
        /// This variable contain the id of the stream
        /// </summary>
        private string streamId = "";

        /// <summary>
        /// Contain the index of the frame into the server side
        /// </summary>
        private long frameCounter = 0;

        /// <summary>
        /// If this variable is set to true test for recording are executed else they aren't executed
        /// </summary>
        private bool acquiringInProgress = false;

        /// <summary>
        /// Reference to the scheduler of frame acquisition
        /// </summary>
        private SchedulerController schedulerController;

        // Server Side Variable

        /// <summary>
        /// timeLastSendingCommand contains the last time a acquisition command are send 
        /// </summary>
        private float timeInitAcquisition = -1f;

        /// <summary>
        /// This contain the fps we need to acquire
        /// </summary>
        private float secBetweenFrame = 0.1f;

        /// <summary>
        /// List of all phoneId that have complete sending
        /// </summary>
        private HashSet<string> phoneIdCompleteSending;

        /// <summary>
        /// This variable if different than long.MinValue say that we need to execute the finish procedure because all the frame was sent
        /// </summary>
        private long totalSendedFrame = long.MinValue;

        // Client variables

        private long lastRPCframeID = long.MinValue;

        private List<long> listLostFrameId;

        public string InitHost()
        {
            schedulerController = GetComponent<SchedulerController>();
            return schedulerController.InitHost(OnHostSocketClose);
        }

        /// <summary>
        /// [2nd step] This function is called for second when anchor is placed 
        /// </summary>
        public void InitVideoStreamingController(List<AcquisitionDelays> acquisitionDelays, string streamId)
        {
            Debug.Log("InitVideoStreamingController called");
            try
            {
                // debug information
                this.snackbarText = GameObject.Find("Snackbar").GetComponentInChildren<Text>();

                // Init code
                this.streamId = String.Copy(streamId);
                this.secBetweenFrame = Settings.Settings.Instance.FPS;
                this.phoneIdCompleteSending = new HashSet<string>();
                this.frameCounter = 0;
                this.acquiringInProgress = false;

                foreach (AcquisitionDelays delay in acquisitionDelays)
                {
                    Debug.Log("Key = " + delay.Key + ", Value = " + delay.Value);
                }
                foreach (MLAPI.Connection.NetworkedClient client in NetworkingManager.Singleton.ConnectedClientsList)
                {
                    Debug.Log("Client ID: " + client.ClientId);
                }
                Debug.Log("StreamID: " + this.streamId);

                foreach (AcquisitionDelays delay in acquisitionDelays)
                {
                    Debug.Log("Invoke remote RPC on: " + delay.Key + " delay send: " + delay.Value);

                    using (PooledBitStream stream = PooledBitStream.Get())
                    {
                        using (PooledBitWriter writer = PooledBitWriter.Get(stream))
                        {
                            writer.WriteString(this.streamId);
                            writer.WriteDouble(delay.Value);

                            InvokeClientRpcOnClientPerformance(InitVideoStreamingPerformance, delay.Key, stream);
                        }
                    }

                        
                    // InvokeClientRpcOnClient(InitVideoStreaming, delay.Key, this.streamId, delay.Value);
                    Debug.Log("RPC client invoked");
                }
            }
            catch (Exception e)
            {
                Debug.LogError("Exception InitVideoStreaming: " + e.ToString());
            }
        }

        /// <summary>
        /// Function call on each client, and set the acquisition delay to streamId
        /// </summary>
        /// <param name="streamId"></param>
        /// <param name="acquisitionDelay"></param>
        [ClientRPC]
        private void InitVideoStreamingPerformance(ulong clientId, Stream stream)
        {
            string streamId = "";
            double acquisitionDelay = 0d;

            using (PooledBitReader reader = PooledBitReader.Get(stream))
            {
                streamId = reader.ReadString().ToString();
                acquisitionDelay = reader.ReadDouble();
            }

            Debug.Log("InitVideoStreaming on client");

            totalSendedFrame = long.MinValue;
            this.lastRPCframeID = long.MinValue;

            schedulerController = GetComponent<SchedulerController>();
            schedulerController.InitAcquisition(acquisitionDelay, streamId, OnClientClose);

            this.listLostFrameId = new List<long>();
        }

        /// <summary>
        /// [3th step] This function are call when recording button is pressed and we can start to send frame to server
        /// </summary>
        public void StartSendingImage(ErrorDuringStreaming callbackStreamingError, AllFrameSend callbackAllFrameSend)
        {
            acquiringInProgress = true;
            timeInitAcquisition = Time.time;
            this.callbackStreamingError = callbackStreamingError;
            this.callbackAllFrameSend = callbackAllFrameSend;

            InvokeClientRpcOnEveryone(StartAcquisitionCommand);
            StartAcquisitionCommand();
        }

        /// <summary>
        /// [3.5th step] This function run on the client and prepare to acquire frame
        /// </summary>
        [ClientRPC]
        public void StartAcquisitionCommand()
        {
            this.snackbarText = GameObject.Find("Snackbar").GetComponentInChildren<Text>();
            Transform anchorTransform = GameObject.Find("Anchor(Clone)").GetComponent<Transform>();
            Toast.ShowAndroidToastMessage("P:[" + anchorTransform.position.x + "," + anchorTransform.position.y + "," + anchorTransform.position.z + "] R:[" + anchorTransform.rotation.w + "," + anchorTransform.rotation.x + "," + anchorTransform.rotation.y + "," + anchorTransform.rotation.z + "]");
            /*
            // Method 2
            if(textureRender.enabled == false)
            {
                Debug.Log("Texture Render enable");
                textureRender.enabled = true;
                // textureRender.gameObject.SetActive(true);
                textureRender.OnImageAvailableCallback += OnGPUImageAvailable;
                textureRender.Apply();
            }
            */
        }

        /// <summary>
        /// [4th step] This function are the standard Unity update function call when recording button is pressed and we can start to send frame to server, it's active only when step 4 had executed
        /// </summary>
        public void Update()
        {
            if (this.totalSendedFrame != long.MinValue)
            {
                if(this.IsHost || this.IsServer)
                {
                    callbackAllFrameSend?.Invoke(this.totalSendedFrame);
                    this.totalSendedFrame = long.MinValue;
                }
                else
                {
                    OnClientCloseCallbackMainThread(this.totalSendedFrame);
                }
            }

            if (acquiringInProgress != true) return;

            // [START] debug information
            Transform cameraTransform = Camera.main.GetComponent<Transform>();
            Pose cameraPose = new Pose(cameraTransform.position, cameraTransform.rotation);
            this.snackbarText.text = "P:[" + cameraPose.position.x + "," + cameraPose.position.y + "," + cameraPose.position.z + "] R:[" + cameraPose.rotation.w + "," + cameraPose.rotation.x + "," + cameraPose.rotation.y + "," + cameraPose.rotation.z + "]";
            // [END] debug information

            if (this.IsServer || this.IsHost)
            {
                // Functions execute only server side
                AcquireIfNeccessary();
            }
        }

        private byte[] frameSendingBytes = new byte[9];

        /// <summary>
        /// [6th step] This function run only on the macchine hosting the game, and check if we need to acquire a new frame, if true it send a broadcast command
        /// </summary>
        private void AcquireIfNeccessary()
        {
            long currentFrame = (long)Math.Floor((Time.time - timeInitAcquisition) / secBetweenFrame);
            if (currentFrame <= frameCounter) return;

            Debug.Log("Time remain expired, schedule frame: " + frameCounter);

            for (int c = 0; c < 1; c++)
            {
                using (PooledBitStream stream = PooledBitStream.Get())
                {
                    using (PooledBitWriter writer = PooledBitWriter.Get(stream))
                    {
                        writer.WriteInt64Packed(frameCounter);

                        InvokeClientRpcOnEveryonePerformance(ScheduleAcquisitionPerformance, stream, "MLAPI_TIME_SYNC");
                    }
                }
            }
            
            frameCounter++;
        }

        [ClientRPC]
        private void ScheduleAcquisitionPerformance(ulong clientId, Stream stream)
        {
            using (PooledBitReader reader = PooledBitReader.Get(stream))
            {
                long frameId = reader.ReadInt64Packed();
                if (this.lastRPCframeID >= frameId)
                {
                    listLostFrameId.Add(frameId);
                    return;
                }

                lastRPCframeID = frameId;
                Debug.Log("Try to schedule frame: " + frameId);
                schedulerController.ScheduleAcquisition(frameId);
            }
        }

        private void CustomMessageHandler(ulong clientId, Stream stream)
        {
            Debug.Log("Custom message received");
            try
            {
                using (PooledBitReader reader = PooledBitReader.Get(stream))
                {
                    int messageType = reader.ReadByte();
                    if (messageType == (int)CustomMessageType.FrameTrigger)
                    {
                        long frameId = 0;
                        frameId = reader.ReadInt64();

                        if (this.lastRPCframeID >= frameId)
                        {
                            listLostFrameId.Add(frameId);
                            return;
                        }

                        lastRPCframeID = frameId;
                        Debug.Log("Try to schedule frame: " + frameId);
                        schedulerController.ScheduleAcquisition(frameId);
                    }
                    else
                    {
                        Debug.Log("Unknown message type");
                    }
                }
            }
            catch(Exception e)
            {
                Debug.LogError("CustomMessageHandler: " + e.ToString());
            }
            
        }

        /*
        // Unused
        private void OnGPUImageAvailable(TextureReaderApi.ImageFormatType format, int width, int height, IntPtr pixelBuffer, int bufferSize)
        {
            Debug.Log("GPU Image Available");
            if (acquireFrame == false) return;

            acquireFrame = false;

            Debug.Log("Need to acquire frame");

            Debug.Log("Acquired frame nextFrameId: " + nextFrameId);

            // TODO: load into OpenCV matrix

            // TODO: save as JPEG

            // TODO: start sending coroutine

            // Requests.UploadImageRequest(exampleImage, nextFrameId, streamId.Value, Frame.Pose.position, Frame.Pose.rotation, OnFrameSend, OnFrameSendingSuccess, OnFrameSendWithError).ConfigureAwait(false);
        }
        */
        
        /// <summary>
        /// [10th step] This is called server side to stop the recording
        /// </summary>
        public void StopSendingImage()
        {
            Debug.Log("Stop sending image");
            acquiringInProgress = false;

            if (NetworkingManager.Singleton.ConnectedClients.Count <= 1)
            {
                CloseSession();
            } else
            {
                InvokeClientRpcOnEveryone(StopAcquisition);
            }
        }

        /// <summary>
        /// [11th step] This stop the recording of the images
        /// </summary>
        [ClientRPC]
        public void StopAcquisition()
        {
            /*
            // Method 2
            if (textureRender.enabled == true)
            {
                Debug.Log("Texture Render disable");
                textureRender.enabled = false;
                // textureRender.gameObject.SetActive(false);
                textureRender.OnImageAvailableCallback -= OnGPUImageAvailable;
                textureRender.Apply();
            }
            */
            if(!(this.IsHost || this.IsServer))
            {
                CloseSession();
            }
        }

        private void PrintReorderedFrame()
        {
            File.WriteAllText(Path.Combine(Application.persistentDataPath, "reordered" + DateTime.UtcNow.ToString().Replace(' ', '_').Replace(':','_').Replace('/','_') + ".txt"), string.Join("\n", this.listLostFrameId));
        }

        // TODO: change signature to include the number of frame encoded (ulong totalSendedFrame)
        public void OnClientClose(long totalSendedFrame)
        {
            this.totalSendedFrame = totalSendedFrame;
        }

        private void OnClientCloseCallbackMainThread(long totalSendedFrame)
        {
            try
            {
                InvokeServerRpc(ClientFinishSendingPacket, SystemInfo.deviceUniqueIdentifier);
                this.totalSendedFrame = long.MinValue;
            }
            catch (Exception e)
            {
                Debug.Log("Exception during RPC call: " + e.ToString());
            }
        }

        /// <summary>
        /// [13th step] This function is executed only inside the server
        /// </summary>
        [ServerRPC(RequireOwnership = false)]
        public void ClientFinishSendingPacket(string phoneId)
        {
            Debug.Log("Server RPC received, " + phoneId);
            phoneIdCompleteSending.Add(phoneId);
            Debug.Log("phoneIdCompleteSending: " + phoneIdCompleteSending.Count + " ConnectedClients: " + NetworkingManager.Singleton.ConnectedClients.Count);

            if(phoneIdCompleteSending.Count >= NetworkingManager.Singleton.ConnectedClients.Count - 1)
            {
                // Now we can close the session also on the host
                CloseSession();
            }
        }

        public void CloseSession()
        {
            Debug.Log("Host stop sending");
            schedulerController.StopAcquisition();
            PrintReorderedFrame();
        }

        public void OnHostSocketClose(long enqueueFrame)
        {
            this.totalSendedFrame = enqueueFrame;
        }
    }
}
