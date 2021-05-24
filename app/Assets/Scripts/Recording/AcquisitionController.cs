
using GoogleARCore;
using MLAPI;
using Reconstruction4D.Recording.FrameTransmission;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;
using UnityEngine.Networking;

namespace Reconstruction4D.Recording
{
    class AcquisitionController : MonoBehaviour 
    {
        private long nextFrameId = long.MinValue;

        private Matrix4x4 trsmatrix;

        private Pose worldPose;

        private Transform cameraTransform;

        private List<long> frameAcquired = new List<long>();

        private string streamID;

        /// <summary>
        /// This variable contain the reference to the job instance that consume the elaboration queue
        /// </summary>
        private ImageSendingSocket sendingSocket = null;

        /// <summary>
        /// Callback to call when sending socket close
        /// </summary>
        private ImageSendingSocket.OnSocketClose callbackOnSocketClose;

        public void AcquireNextAvailableFrame(long frameId)
        {
            nextFrameId = frameId;
        }

        /// <summary>
        /// This function is call before init to recording and prepare the system to upload the image on the web server, also it prepare the reference to the object use during acquisition
        /// </summary>
        /// <param name="streamId">the UUID of the stream given by the data elaboration server</param>
        /// <param name="callbackOnFrameSendError">Fucntion call when frame send encounter in a error</param>
        public string InitAcquisition(string streamId, ImageSendingSocket.OnFrameSendingError callbackOnFrameSendError, ImageSendingSocket.OnSocketClose callbackOnSocketClose)
        {
            this.streamID = streamId;
            cameraTransform = Camera.main.GetComponent<Transform>();
            worldPose = FindObjectOfType<ARCoreWorldOriginHelper>().WorldPose;
            trsmatrix = Matrix4x4.TRS(worldPose.position, worldPose.rotation, new Vector3(1, 1, 1));
            frameAcquired.Clear();

            if (sendingSocket == null)
            {
                this.callbackOnSocketClose = callbackOnSocketClose;
                sendingSocket = ImageSendingSocket.Instance;
                ulong bandwidthUsage = 0;
                if (Settings.Settings.Instance.ReservedBandwidth != 0)
                {
                    bandwidthUsage = Convert.ToUInt64(NetworkPerformance.Bandwidth * (1 - ((float)Settings.Settings.Instance.ReservedBandwidth / 100)));
                }
                
                Debug.Log("Bandwidth used during image sending: " + bandwidthUsage + " - Total bandwidth measure: " + NetworkPerformance.Bandwidth);
                if (streamId == "")
                {
                    string createStreamID = sendingSocket.InitConnectionAsHost(Settings.Settings.Instance.ElaborationServerAddress, Settings.Settings.Instance.ElaborationServerPort, SystemInfo.deviceUniqueIdentifier, bandwidthUsage, (uint)NetworkingManager.Singleton.ConnectedClientsList.Count, callbackOnFrameSendError, OnSocketClose);
                    return createStreamID;
                }
                else
                {
                    
                    sendingSocket.InitConnectionAsClient(Settings.Settings.Instance.ElaborationServerAddress, Settings.Settings.Instance.ElaborationServerPort, SystemInfo.deviceUniqueIdentifier, bandwidthUsage, streamId, callbackOnFrameSendError, OnSocketClose);
                    return streamId;
                }
            }
            return "";
        }

        /// <summary>
        ///  Callback for the socket, reset all the reference to the socket
        /// </summary>
        /// <param name="totalFrameSend">How many frame socket send during the activity</param>
        private void OnSocketClose(long totalFrameSend)
        {
            sendingSocket = null;
            cameraTransform = null;
            this.callbackOnSocketClose?.Invoke(totalFrameSend);
        }

        /// <summary>
        /// This is the standard Unity function called at every frame, in this particular case it acquire an image if neccesary, and enqueque inside 
        /// </summary>
        public void Update()
        {
            if (nextFrameId == long.MinValue) return;

            // Method 1 (some problem on Xiaomi, resolve on 1.8 version)
            using (CameraImageBytes image = Frame.CameraImage.AcquireCameraImageBytes())
            {
                if (!image.IsAvailable) return;

                Debug.Log("Acquired frame nextFrameId: " + nextFrameId);

                byte[] yBuffer = new byte[image.Width * image.Height];

                byte[] vuBuffer = new byte[image.Width * (image.Height / 2)];

                Marshal.Copy(image.Y, yBuffer, 0, yBuffer.Length);

                Marshal.Copy(image.V, vuBuffer, 0, vuBuffer.Length);

                Vector3 position = trsmatrix.MultiplyPoint3x4(Frame.Pose.position);

                Quaternion rotation = Frame.Pose.rotation * Quaternion.LookRotation(
                trsmatrix.GetColumn(2), trsmatrix.GetColumn(1));
                Quaternion rotationNoEdit = new Quaternion(rotation.x, rotation.y, rotation.z, rotation.w);

                Vector3 positionOri = cameraTransform.position;
                Quaternion rotationOri = cameraTransform.rotation;

                FrameNode frame = new FrameNode
                {
                    frameId = nextFrameId,
                    imageHeight = image.Height,
                    imageWidth = image.Width,
                    focalLength = Frame.CameraImage.ImageIntrinsics.FocalLength,
                    principalPoint = Frame.CameraImage.ImageIntrinsics.PrincipalPoint,
                    position = position,
                    positionOri = positionOri,
                    rotation = rotation,
                    rotationOri = rotationOri,
                    vuBuffer = vuBuffer,
                    yBuffer = yBuffer
                };

                sendingSocket.EnquequeFrame(ref frame);

                frameAcquired.Add(nextFrameId);

                nextFrameId = long.MinValue;
            }
        }

        /// <summary>
        /// Send command to close the system for frame sending
        /// </summary>
        /// <param name="totalFrame">How many frame you send</param>
        public void StopAcquisition(long totalFrame)
        {
            sendingSocket.EndElaboration(totalFrame);
            
            FileLogger.AppendText(Path.Combine(Application.persistentDataPath, "CaptureFrame" + DateTime.UtcNow.ToString("yyyy-MM-dd-hh-mm-ss") + ".txt"), String.Join("\n", frameAcquired));

            SendJson("http://" + Settings.Settings.Instance.ElaborationServerAddress + ":3000/captureFrame/", "{\"sessionID\":\"" + this.streamID + "\",\"phoneID\":\"" + SystemInfo.deviceUniqueIdentifier + "\",\"captureFrame\":[" + String.Join(",", frameAcquired) + "]}");
        }



        private void SendJson(string url, string json)
        {
            StartCoroutine(PostRequestCoroutine(url, json));
        }

        private IEnumerator PostRequestCoroutine(string url, string json)
        {
            var jsonBinary = System.Text.Encoding.UTF8.GetBytes(json);

            DownloadHandlerBuffer downloadHandlerBuffer = new DownloadHandlerBuffer();

            UploadHandlerRaw uploadHandlerRaw = new UploadHandlerRaw(jsonBinary);
            uploadHandlerRaw.contentType = "application/json";

            UnityWebRequest www =
                new UnityWebRequest(url, "POST", downloadHandlerBuffer, uploadHandlerRaw);

            yield return www.SendWebRequest();

            if (www.isNetworkError)
                Debug.LogError(string.Format("{0}: {1}", www.url, www.error));
            else
                Debug.Log(string.Format("Response: {0}", www.downloadHandler.text));
        }
    }
}
