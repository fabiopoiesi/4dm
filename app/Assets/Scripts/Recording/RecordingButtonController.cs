using MLAPI;
using System;
using UnityEngine;
using UnityEngine.EventSystems;

namespace Reconstruction4D.Recording
{
    public class RecordingButtonController : MonoBehaviour
    {
        public static RecordingButtonController singleton;

        public void Awake()
        {
            singleton = this;
        }

        public GameObject startSymbol;

        public GameObject stopSymbol;

        public GameObject restartSymbol;

        public delegate void ButtonPressedDelegate();

        public ButtonPressedDelegate OnStart = null;

        public ButtonPressedDelegate OnStop = null;

        public ButtonPressedDelegate OnRestart = null;

        public enum RecordingButtonMode
        {
            Start,
            Stop,
            Restart
        }

        private RecordingButtonMode mode = RecordingButtonMode.Start;

        public RecordingButtonMode Mode
        {
            get
            {
                return mode;
            }
            set
            {
                if(value == RecordingButtonMode.Start)
                {
                    startSymbol.SetActive(true);
                    stopSymbol.SetActive(false);
                    restartSymbol.SetActive(false);
                } else if(value == RecordingButtonMode.Stop)
                {
                    startSymbol.SetActive(false);
                    stopSymbol.SetActive(true);
                    restartSymbol.SetActive(false);
                } else
                {
                    startSymbol.SetActive(false);
                    stopSymbol.SetActive(false);
                    restartSymbol.SetActive(true);
                }
                mode = value;
            }
        }

        public bool IsVisible
        {
            get
            {
                return this.gameObject.activeSelf;
            }
            set
            {
                if (value == true)
                {
                    this.gameObject.SetActive(true);
                }
                else
                {
                    this.gameObject.SetActive(false);
                }
            }
        }

        public void OnRecordingButtonClick()
        {
            if(Mode == RecordingButtonMode.Start)
            {
                Debug.Log("Button start pressed");
                Mode = RecordingButtonMode.Stop;
                OnStart?.Invoke();
            } else if(Mode == RecordingButtonMode.Stop)
            {
                Debug.Log("Button stop pressed");
                Mode = RecordingButtonMode.Restart;
                OnStop?.Invoke();
            } else
            {
                Debug.Log("Button restart pressed");
                Mode = RecordingButtonMode.Start;
                OnRestart?.Invoke();
            }
        }

        public bool TouchIsInsideRecordingButton(Touch touch)
        {
            int id = touch.fingerId;
            if (EventSystem.current.IsPointerOverGameObject(id))
            {
                return true;
            }
            return false;
        }
    }
}
