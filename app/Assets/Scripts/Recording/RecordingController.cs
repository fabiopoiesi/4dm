using Reconstruction4D.UI;
using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Reconstruction4D.Recording
{
    using AcquisitionDelays = KeyValuePair<ulong, double>;

    [RequireComponent(typeof(PingCalculator))]
    [RequireComponent(typeof(NetworkPerformance))]
    public class RecordingController : MonoBehaviour
    {
        internal static RecordingController singleton;

        private RecordingButtonController RecordingButton;

        public VideoStreamingController videoStreamingController;

        public Text SnackbarText;

        private List<AcquisitionDelays> delayInit;

        private bool autoStopActive = false;
        private float autoStopTime = 0;
        private float initTime = 0f;

        private bool delayedStartActive = false;
        private float startTime = 0;

        public void Start()
        {
            gameObject.name = "GlobalRecordingController";
            InitReference();
            singleton = this;
        }

        private void InitReference()
        {
            RecordingButtonController[] recordingButtonArray = Resources.FindObjectsOfTypeAll<RecordingButtonController>();
            if(recordingButtonArray.Length > 0)
            {
                RecordingButton = recordingButtonArray[0];
                RecordingButton.OnStart = OnStartButtonPressed;
                RecordingButton.OnStop = OnStopButtonPressed;
                RecordingButton.OnRestart = OnRestartButtonPressed;
            }
            SnackbarText = GameObject.Find("Snackbar").GetComponentInChildren<Text>();
        }

        public void CanStartRecording()
        {
            this.GetComponent<PingCalculator>().InitPingMeasurement(OnPingResult);
        }

        private void OnPingResult(List<AcquisitionDelays> delays)
        {
            delayInit = delays;

            GetComponent<NetworkPerformance>().InitNetworkPerformanceMeasurement(OnPerformanceMeasureResult);
            Debug.Log("init performance measure");
            Debug.Log("Network performance is null: " + (GetComponent<NetworkPerformance>() == null));
        }
        
        private void OnPerformanceMeasureResult()
        {
            Debug.Log("Performance result set");
            AsyncOnPingResult();
        }

        private bool AsyncOnPingResult()
        {
            // TODO: make this in a separate thread
            
            Debug.Log("Ping results: " + delayInit.ToString());

            string result = videoStreamingController.InitHost();
            if (result == String.Empty)
            {
                OnStreamIdObtainError();
                return false;
            }
            else
            {
                OnStreamIdObtain(result);
                return true;
            }
        }

        private void OnStreamIdObtain(string streamId)
        {
            Debug.Log("StreamId is: " + streamId);
            videoStreamingController.InitVideoStreamingController(delayInit, streamId);
            Debug.Log("InitVideoStreamingController");
            EnableRecordingView();
        }

        private void OnStreamIdObtainError()
        {
            Toast.ShowAndroidToastMessage("Error during streaming ID acquisition, please restart");
        }

        public void EnableRecordingView()
        {
            SnackbarText.text = "When ready, touch the red button to start...";
            RecordingButton.IsVisible = true;
        }

        private void OnStopButtonPressed()
        {
            // End recording
            videoStreamingController.StopSendingImage();
            RecordingButton.IsVisible = false;
            SnackbarText.text = "Recording stop, last operations in progress";
        }

        private void OnStartButtonPressed()
        {
            RecordingButton.IsVisible = false;
            if (Settings.Settings.Instance.StartDelay == 0)
            {
                StartRecording();
            } else
            {
                startTime = Time.time + Settings.Settings.Instance.StartDelay;
                delayedStartActive = true;
            }
        }

        private void StartRecording()
        {
            // Start recording
            videoStreamingController.StartSendingImage(OnStreamingError, OnAllFrameSend);
            SnackbarText.text = "Recording in progress...";
            RecordingButton.IsVisible = true;
            if (!(Settings.Settings.Instance.AutoStopRecordingAfterNSeconds < float.Epsilon && Settings.Settings.Instance.AutoStopRecordingAfterNSeconds > -float.Epsilon))
            {
                Debug.Log("Auto Stop active");
                autoStopActive = true;
                this.autoStopTime = Settings.Settings.Instance.AutoStopRecordingAfterNSeconds;
                initTime = Time.time;
            }
        }

        private void OnAllFrameSend(long totalFrameSend)
        {
            Debug.Log("OnAllFrameSend called");
            // Toast.ShowAndroidToastMessage("Total frame send: " + totalFrameSend);
            // Debug.Log("Android toast show");
            RecordingButton.IsVisible = true;
        }

        private void OnRestartButtonPressed()
        {
            Debug.Log("Request restart");
            RecordingButton.IsVisible = false;
            CanStartRecording();
        }

        private void OnStreamingError(string message)
        {
            Debug.LogError("Error during sending, message: " + message);
            // videoStreamingController.StopSendingImage();
            // RecordingButton.Mode = RecordingButtonController.RecordingButtonMode.Start;
            // RecordingButton.IsVisible = false;
            // SnackbarText.text = "Recording stop";
        }

        public void Update()
        {
            DelayedStartUpdate();
            AutoStopUpdate();
        }

        public void DelayedStartUpdate()
        {
            if (delayedStartActive == false) return;

            if (startTime <= Time.time)
            {
                StartRecording();
                delayedStartActive = false;
            }
        }

        public void AutoStopUpdate()
        {
            if (autoStopActive == false) return;

            if (initTime + autoStopTime <= Time.time)
            {
                Debug.Log("Auto Stop => Stop recording");
                RecordingButton.OnRecordingButtonClick();
                autoStopActive = false;
                initTime = 0f;
            }
        }
    }
}
