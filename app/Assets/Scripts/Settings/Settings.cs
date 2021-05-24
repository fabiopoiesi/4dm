namespace Reconstruction4D.Settings
{
    using System;
    using System.IO;
    using UnityEngine;
#if PLATFORM_ANDROID
    using UnityEngine.Android;
#endif

    /// <summary>
    /// Controller of the a settings windows, this class mantain trace of the configuration parameters save it on apply button and load it on loading
    /// </summary>
    class Settings : MonoBehaviour
    {
        /// <summary>
        /// Contain the available connection method, UNET is actually not implemented
        /// </summary>
        public enum ConnectionType {
            MLAPI_Relay = 0,
            UNET = 1
        }

        private static Settings _instance;

        /// <summary>
        /// Singleton reference
        /// </summary>
        public static Settings Instance { get { return _instance; } }

        /// <summary>
        /// Default Unity method, load the settings from JSON file if exist, also init the singleton
        /// </summary>
        private void Awake()
        {
            if (_instance != null && _instance != this)
            {
                Destroy(this.gameObject);
            }
            else
            {
#if PLATFORM_ANDROID
                if (!Permission.HasUserAuthorizedPermission(Permission.ExternalStorageRead))
                {
                    Permission.RequestUserPermission(Permission.ExternalStorageRead);
                }
#endif
                _instance = this;
                string settingsPath = Path.Combine(Application.persistentDataPath, "settings.json");
                if (File.Exists(settingsPath))
                {
                    try
                    {
                        string jsonSettings = File.ReadAllText(settingsPath);
                        SettingsSaveFormat fileSettings = JsonUtility.FromJson<SettingsSaveFormat>(jsonSettings);
                        fileSettings.Decapsulate(this);
                    } catch(Exception e)
                    {
                        Debug.LogError("Exception during config file reading, message: " + e.Message);
                    }
                }
            }
        }

        /// <summary>
        /// Relay Address is the address (IP or hostname) where communicate with relay service (Only if communication method is set to MLAPI)
        /// </summary>
        public string RelayAddress { get => relayAddress; set => relayAddress = value; }

        private string relayAddress = "192.168.43.69";

        /// <summary>
        /// Elaboration Server Address is the address (IP or hostname) where communicate with data manager service and performance measure service
        /// </summary>
        public string ElaborationServerAddress { get => elaborationServerAddress; set => elaborationServerAddress = value; }

        private string elaborationServerAddress = "192.168.43.69";

        /// <summary>
        /// Performance Server Port is the port where contact performance measure service
        /// </summary>
        public uint PerformanceServerPort { get => performanceServerPort; set => performanceServerPort = value; }

        private uint performanceServerPort = 5959; 

        /// <summary>
        /// Elaboration Server Port is the port where contact data manager service
        /// </summary>
        public uint ElaborationServerPort { get => elaborationServerPort; set => elaborationServerPort = value; }

        private uint elaborationServerPort = 5960;

        /// <summary>
        /// This variable contain the system of connection to use
        /// </summary>
        public ConnectionType Type { get => type; set => type = value; }

        private ConnectionType type = ConnectionType.MLAPI_Relay;

        /// <summary>
        /// This variable set if the system need to automatically stop recording after n second from start
        /// </summary>
        /// <remarks>
        /// Note that this value is save as second between each frame (1/N with N on canonical format) and is returned as is saved
        /// </remarks>
        public float FPS { get => fps; set => fps = 1 / value; }

        private float fps = 0.1f;

        /// <summary>
        /// This variable set if the system need to automatically stop recording after n second
        /// </summary>
        public float AutoStopRecordingAfterNSeconds { get => autoStopRecordingAfterNSeconds; set => autoStopRecordingAfterNSeconds = value; }

        private float autoStopRecordingAfterNSeconds = 0f;

        /// <summary>
        /// This variable set if the network latency system is active or disable during registration
        /// </summary>
        public bool PingCompesationActive { get => pingCompesationActive; set => pingCompesationActive = value; }

        private bool pingCompesationActive = true;

        /// <summary>
        /// This variable set how many bandwidth must be reserved to trigger and other usage
        /// </summary>
        public int ReservedBandwidth { get => reservedBandwidth; set => reservedBandwidth = value; }

        private int reservedBandwidth;

        /// <summary>
        /// This variable set how many bandwidth must be reserved to trigger and other usage
        /// </summary>
        public int StartDelay { get => startDelay; set => startDelay = value; }

        private int startDelay;

        /// <summary>
        /// This function save the settings into a json file
        /// </summary>
        public void SaveConfig()
        {
#if PLATFORM_ANDROID
            if (!Permission.HasUserAuthorizedPermission(Permission.ExternalStorageWrite))
            {
                Permission.RequestUserPermission(Permission.ExternalStorageWrite);
            }
#endif
            try
            {
                SettingsSaveFormat fileSettings = new SettingsSaveFormat(this);
                string jsonSettings = JsonUtility.ToJson(fileSettings);
                File.WriteAllText(Path.Combine(Application.persistentDataPath, "settings.json"), jsonSettings);
            }
            catch (Exception e)
            {
                Debug.LogError("Exception during config file writing, message: " + e.Message);
            }
        }
    }
}
