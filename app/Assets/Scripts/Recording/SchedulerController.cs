using Reconstruction4D.Recording.FrameTransmission;
using System.Collections.Generic;
using UnityEngine;

namespace Reconstruction4D.Recording
{
    [RequireComponent(typeof(AcquisitionController))]
    class SchedulerController : MonoBehaviour
    {
        private struct FrameToAcquire
        {
            public double acquireTime;
            public long frameId;
        }

        /// <summary>
        /// This contain the queue of the frame to acquire
        /// </summary>
        private Queue<FrameToAcquire> acquisitionQueue;

        /// <summary>
        /// the delay between when the send command are trigger and when acquisition need to be executed (unique for each client)
        /// </summary>
        private double imageDelayAcquisition = 0.0f;

        /// <summary>
        /// reference to the controller that acquire and send the frame
        /// </summary>
        private AcquisitionController acquisitionController;

        /// <summary>
        /// flag to know when we're waiting the sending of all the frame
        /// </summary>
        private bool stopAcquisition = false;

        public string InitHost(ImageSendingSocket.OnSocketClose callbackOnSocketClose)
        {
            acquisitionController = this.GetComponent<AcquisitionController>();
            return acquisitionController.InitAcquisition("", OnFrameSendError, callbackOnSocketClose);
        }

        public void InitAcquisition(double acquisitionDelay, string streamId, ImageSendingSocket.OnSocketClose callbackOnSocketClose)
        {
            acquisitionQueue = new Queue<FrameToAcquire>();
            imageDelayAcquisition = acquisitionDelay;
            Debug.Log("Acquisition Delay: " + imageDelayAcquisition);
            if (stopAcquisition) acquisitionController.StopAcquisition(0);
            acquisitionController = this.GetComponent<AcquisitionController>();
            acquisitionController.InitAcquisition(streamId, OnFrameSendError, callbackOnSocketClose);
            stopAcquisition = false;
        }

        public void StopAcquisition()
        {
            stopAcquisition = true;
            Invoke("StopAcquisitionOnAcquisitionController", Settings.Settings.Instance.FPS * 3);
        }

        // This is neccesary to permit last frame to be captured
        public void StopAcquisitionOnAcquisitionController()
        {
            acquisitionController.StopAcquisition(0);
            FindObjectOfType<Reconstruction4DController>().SavePointCloud();
            Debug.Log("Request stop elaboration thread");
        }

        public void Update()
        {
            if (acquisitionQueue == null) return;

            long nextFrameId = RecoveryFrameIdToAcquire();
            if (nextFrameId == long.MinValue) return;

            acquisitionController.AcquireNextAvailableFrame(nextFrameId);
        }

        private long RecoveryFrameIdToAcquire()
        {
            if (acquisitionQueue.Count == 0) return long.MinValue;
            if (acquisitionQueue.Peek().acquireTime > Time.time) return long.MinValue;

            long extractFrameId;
            while(true)
            {
                extractFrameId = acquisitionQueue.Dequeue().frameId;
                if(acquisitionQueue.Count == 0)
                {
                    return extractFrameId;
                }
                else if(acquisitionQueue.Peek().acquireTime > Time.time)
                {
                    return extractFrameId;
                }
                else
                {
                    Debug.LogError("Error during extraction of the next frame");
                }
            }
        }

        public void ScheduleAcquisition(long frameId)
        {
            acquisitionQueue.Enqueue(new FrameToAcquire()
            {
                acquireTime = Time.time + this.imageDelayAcquisition,
                frameId = frameId
            });
        }

        private void OnFrameSendError(string error)
        {
            Debug.Log("Error during transmission of a frame");
        }
    }
}
