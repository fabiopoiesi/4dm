namespace Reconstruction4D.Settings
{
    /// <summary>
    /// SettingsSaveFormat rappresent the saving format of the settings, on this class all field that need to be saved was publicly available
    /// </summary>
    class SettingsSaveFormat
    {
        /// <summary>
        /// This constructor copy from a Settings object all parameters need to be saved
        /// </summary>
        /// <param name="settings">parameters origin</param>
        public SettingsSaveFormat(Settings settings)
        {
            relayAddress = settings.RelayAddress;
            elaborationServerAddress = settings.ElaborationServerAddress;
            elaborationServerPort = settings.ElaborationServerPort;
            performanceServerPort = settings.PerformanceServerPort;
            type = settings.Type;
            fps = settings.FPS;
            autoStopRecordingAfterNSeconds = settings.AutoStopRecordingAfterNSeconds;
            pingCompesationActive = settings.PingCompesationActive;
            reservedBandwidth = settings.ReservedBandwidth;
            startDelay = settings.StartDelay;
        }

        public string relayAddress = "192.168.43.69";

        public string elaborationServerAddress = "192.168.43.69";

        public uint elaborationServerPort = 5960;

        public uint performanceServerPort = 5959;

        public Settings.ConnectionType type = Settings.ConnectionType.MLAPI_Relay;

        public float fps = 0.1f;

        public float autoStopRecordingAfterNSeconds = 0f;

        public bool pingCompesationActive = true;

        public int reservedBandwidth = 25;

        public int startDelay = 0;

        /// <summary>
        /// Copy all parameter to a specify Settings class
        /// </summary>
        /// <param name="settings">parameters destination</param>
        public void Decapsulate(Settings settings)
        {
            settings.RelayAddress = relayAddress;
            settings.ElaborationServerAddress = elaborationServerAddress;
            settings.ElaborationServerPort = elaborationServerPort;
            settings.PerformanceServerPort = performanceServerPort;
            settings.Type = type;
            settings.FPS = 1 / fps;
            settings.AutoStopRecordingAfterNSeconds = autoStopRecordingAfterNSeconds;
            settings.PingCompesationActive = pingCompesationActive;
            settings.ReservedBandwidth = reservedBandwidth;
            settings.StartDelay = startDelay;
        }
    }
}
