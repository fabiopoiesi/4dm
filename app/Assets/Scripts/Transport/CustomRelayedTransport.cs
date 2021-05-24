#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member
using System;
using System.Collections.Generic;
using MLAPI.Logging;
using UnityEngine;
using UnityEngine.Networking;
using System.IO;
using MLAPI.Transports.UNET;

namespace Reconstruction4D.Transport
{
    public class CustomRelayedTransport : MLAPI.Transports.Transport
    {
        // Inspector / settings
        public int MessageBufferSize = 1024 * 5;
        public int MaxConnections = 100;

        public string ConnectAddress = "127.0.0.1";
        public int ConnectPort = 7777;
        public int ServerListenPort = 7777;
        public int ServerWebsocketListenPort = 8887;
        public bool SupportWebsocket = false;
        public List<MLAPI.Transports.TransportChannel> Channels = new List<MLAPI.Transports.TransportChannel>();


        // Relay
        public bool UseMLAPIRelay = false;
        public string MLAPIRelayAddress = "184.72.104.138";
        public int MLAPIRelayPort = 8888;

        // Runtime / state
        private byte[] messageBuffer;
        private WeakReference temporaryBufferReference;

        // Lookup / translation
        private readonly Dictionary<string, int> channelNameToId = new Dictionary<string, int>();
        private readonly Dictionary<int, string> channelIdToName = new Dictionary<int, string>();
        private int serverConnectionId;
        private int serverHostId;

        public override ulong ServerClientId => GetMLAPIClientId(0, 0, true);

        public override void Send(ulong clientId, ArraySegment<byte> data, string channelName, bool skipQueue)
        {
            GetUnetConnectionDetails(clientId, out byte hostId, out ushort connectionId);

            int channelId = channelNameToId[channelName];

            byte[] buffer;

            if (data.Offset > 0)
            {
                // UNET cant handle this, do a copy

                if (messageBuffer.Length >= data.Count)
                {
                    buffer = messageBuffer;
                }
                else
                {
                    if (temporaryBufferReference != null && temporaryBufferReference.IsAlive && ((byte[])temporaryBufferReference.Target).Length >= data.Count)
                    {
                        buffer = (byte[])temporaryBufferReference.Target;
                    }
                    else
                    {
                        buffer = new byte[data.Count];
                        temporaryBufferReference = new WeakReference(buffer);
                    }
                }

                Buffer.BlockCopy(data.Array, data.Offset, buffer, 0, data.Count);
            }
            else
            {
                buffer = data.Array;
            }

            if (skipQueue)
            {
                RelayTransport.Send(hostId, connectionId, channelId, buffer, data.Count, out byte error);
            }
            else
            {
                RelayTransport.QueueMessageForSending(hostId, connectionId, channelId, buffer, data.Count, out byte error);
            }
        }

        public override void FlushSendQueue(ulong clientId)
        {
            GetUnetConnectionDetails(clientId, out byte hostId, out ushort connectionId);

            RelayTransport.SendQueuedMessages(hostId, connectionId, out byte error);
        }

        public override MLAPI.Transports.NetEventType PollEvent(out ulong clientId, out string channelName, out ArraySegment<byte> payload)
        {
            NetworkEventType eventType = RelayTransport.Receive(out int hostId, out int connectionId, out int channelId, messageBuffer, messageBuffer.Length, out int receivedSize, out byte error);

            clientId = GetMLAPIClientId((byte)hostId, (ushort)connectionId, false);

            NetworkError networkError = (NetworkError)error;

            if (networkError == NetworkError.MessageToLong)
            {
                byte[] tempBuffer;

                if (temporaryBufferReference != null && temporaryBufferReference.IsAlive && ((byte[])temporaryBufferReference.Target).Length >= receivedSize)
                {
                    tempBuffer = (byte[])temporaryBufferReference.Target;
                }
                else
                {
                    tempBuffer = new byte[receivedSize];
                    temporaryBufferReference = new WeakReference(tempBuffer);
                }

                eventType = RelayTransport.Receive(out hostId, out connectionId, out channelId, tempBuffer, tempBuffer.Length, out receivedSize, out error);
                payload = new ArraySegment<byte>(tempBuffer, 0, receivedSize);
            }
            else
            {
                payload = new ArraySegment<byte>(messageBuffer, 0, receivedSize);
            }

            channelName = channelIdToName[channelId];

            if (networkError == NetworkError.Timeout)
            {
                // In UNET. Timeouts are not disconnects. We have to translate that here.
                eventType = NetworkEventType.DisconnectEvent;
            }

            // Translate NetworkEventType to NetEventType
            switch (eventType)
            {
                case NetworkEventType.DataEvent:
                    return MLAPI.Transports.NetEventType.Data;
                case NetworkEventType.ConnectEvent:
                    return MLAPI.Transports.NetEventType.Connect;
                case NetworkEventType.DisconnectEvent:
                    return MLAPI.Transports.NetEventType.Disconnect;
                case NetworkEventType.Nothing:
                    return MLAPI.Transports.NetEventType.Nothing;
                case NetworkEventType.BroadcastEvent:
                    return MLAPI.Transports.NetEventType.Nothing;
            }

            return MLAPI.Transports.NetEventType.Nothing;
        }

        public override void StartClient()
        {
            serverHostId = RelayTransport.AddHost(new HostTopology(GetConfig(), 1), false);

            serverConnectionId = RelayTransport.Connect(serverHostId, ConnectAddress, ConnectPort, 0, out byte error);
        }

        public override void StartServer()
        {
            HostTopology topology = new HostTopology(GetConfig(), MaxConnections);

            if (SupportWebsocket)
            {
                if (!UseMLAPIRelay)
                {
                    int websocketHostId = NetworkTransport.AddWebsocketHost(topology, ServerWebsocketListenPort);
                }
                else
                {
                    if (LogHelper.CurrentLogLevel <= LogLevel.Error) LogHelper.LogError("Cannot create websocket host when using MLAPI relay");
                }

            }

            int normalHostId = RelayTransport.AddHost(topology, ServerListenPort, true);
        }

        public override void DisconnectRemoteClient(ulong clientId)
        {
            GetUnetConnectionDetails(clientId, out byte hostId, out ushort connectionId);

            RelayTransport.Disconnect((int)hostId, (int)connectionId, out byte error);
        }

        public override void DisconnectLocalClient()
        {
            RelayTransport.Disconnect(serverHostId, serverConnectionId, out byte error);
        }

        public override ulong GetCurrentRtt(ulong clientId)
        {
            GetUnetConnectionDetails(clientId, out byte hostId, out ushort connectionId);

            if (UseMLAPIRelay)
            {
                return 0;
            }
            else
            {
                return (ulong)NetworkTransport.GetCurrentRTT((int)hostId, (int)connectionId, out byte error);
            }
        }

        public override void Shutdown()
        {
            channelIdToName.Clear();
            channelNameToId.Clear();
            NetworkTransport.Shutdown();
        }

        public override void Init()
        {
            UpdateRelay();

            messageBuffer = new byte[MessageBufferSize];

            NetworkTransport.Init();
        }

        public ulong GetMLAPIClientId(byte hostId, ushort connectionId, bool isServer)
        {
            if (isServer)
            {
                return 0;
            }
            else
            {
                return ((ulong)connectionId | (ulong)hostId << 16) + 1;
            }
        }

        public void GetUnetConnectionDetails(ulong clientId, out byte hostId, out ushort connectionId)
        {
            if (clientId == 0)
            {
                hostId = (byte)serverHostId;
                connectionId = (ushort)serverConnectionId;
            }
            else
            {
                hostId = (byte)((clientId - 1) >> 16);
                connectionId = (ushort)((clientId - 1));
            }
        }

        public ConnectionConfig GetConfig()
        {
            ConnectionConfig config = new ConnectionConfig();

            for (int i = 0; i < MLAPI_CHANNELS.Length; i++)
            {
                int channelId = AddChannel(MLAPI_CHANNELS[i].Type, config);

                channelIdToName.Add(channelId, MLAPI_CHANNELS[i].Name);
                channelNameToId.Add(MLAPI_CHANNELS[i].Name, channelId);
            }

            for (int i = 0; i < Channels.Count; i++)
            {
                int channelId = AddChannel(Channels[i].Type, config);

                channelIdToName.Add(channelId, Channels[i].Name);
                channelNameToId.Add(Channels[i].Name, channelId);
            }

            string filename = Path.Combine(Application.persistentDataPath, "config.txt");
            if (File.Exists(filename))
            {
                string[] lines = File.ReadAllLines(filename);
                config.ResendTimeout = Convert.ToUInt32(lines[0]);
                config.ConnectTimeout = Convert.ToUInt32(lines[1]);
                config.DisconnectTimeout = Convert.ToUInt32(lines[2]);
                config.PingTimeout = Convert.ToUInt32(lines[3]);
                config.ReducedPingTimeout = Convert.ToUInt32(lines[4]);
                config.AllCostTimeout = Convert.ToUInt32(lines[5]);
                config.SendDelay = Convert.ToUInt32(lines[6]);
            }
            else
            {
                config.ResendTimeout = 1200;
                config.ConnectTimeout = 2000;
                config.DisconnectTimeout = 20000;
                config.PingTimeout = 1000;
                config.ReducedPingTimeout = 200;
                config.AllCostTimeout = 100;
                config.SendDelay = 0;
            }

            Debug.Log("ConnectionConfig Detail:");
            Debug.Log("PacketSize: " + config.PacketSize);
            Debug.Log("FragmentSize: " + config.FragmentSize);
            Debug.Log("ResendTimeout: " + config.ResendTimeout);
            Debug.Log("DisconnectTimeout: " + config.DisconnectTimeout);
            Debug.Log("ConnectTimeout: " + config.ConnectTimeout);
            Debug.Log("MinUpdateTimeout: " + config.MinUpdateTimeout);
            Debug.Log("PingTimeout: " + config.PingTimeout);
            Debug.Log("ReducedPingTimeout: " + config.ReducedPingTimeout);
            Debug.Log("AllCostTimeout: " + config.AllCostTimeout);
            Debug.Log("NetworkDropThreshold: " + config.NetworkDropThreshold);
            Debug.Log("OverflowDropThreshold: " + config.OverflowDropThreshold);
            Debug.Log("MaxConnectionAttempt: " + config.MaxConnectionAttempt);
            Debug.Log("AckDelay: " + config.AckDelay);
            Debug.Log("SendDelay: " + config.SendDelay);
            Debug.Log("MaxCombinedReliableMessageSize: " + config.MaxCombinedReliableMessageSize);
            Debug.Log("MaxCombinedReliableMessageCount: " + config.MaxCombinedReliableMessageCount);
            Debug.Log("MaxSentMessageQueueSize: " + config.MaxSentMessageQueueSize);
            Debug.Log("AcksType: " + config.AcksType);
            Debug.Log("InitialBandwidth: " + config.InitialBandwidth);
            Debug.Log("BandwidthPeakFactor: " + config.BandwidthPeakFactor);
            Debug.Log("WebSocketReceiveBufferMaxSize: " + config.WebSocketReceiveBufferMaxSize);
            Debug.Log("UdpSocketReceiveBufferMaxSize: " + config.UdpSocketReceiveBufferMaxSize);
            Debug.Log("SSLCertFilePath: " + config.SSLCertFilePath);
            Debug.Log("SSLPrivateKeyFilePath: " + config.SSLPrivateKeyFilePath);
            Debug.Log("SSLCAFilePath: " + config.SSLCAFilePath);
            Debug.Log("Channels:" + config.ChannelCount);
            int id = 0;
            foreach (ChannelQOS channel in config.Channels)
            {
                Debug.Log("[" + id + "] => " + channel.QOS.ToString());
                id++;
            }

            Debug.Log("Contact MLAPI server: " + this.UseMLAPIRelay);
            Debug.Log("MLAPI Server IP: " + this.MLAPIRelayAddress);
            Debug.Log("MLAPI Server Port: " + this.MLAPIRelayPort);

            return config;
        }

        public int AddChannel(MLAPI.Transports.ChannelType type, ConnectionConfig config)
        {
            switch (type)
            {
                case MLAPI.Transports.ChannelType.Unreliable:
                    return config.AddChannel(QosType.Unreliable);
                case MLAPI.Transports.ChannelType.UnreliableFragmented:
                    return config.AddChannel(QosType.UnreliableFragmented);
                case MLAPI.Transports.ChannelType.UnreliableSequenced:
                    return config.AddChannel(QosType.UnreliableSequenced);
                case MLAPI.Transports.ChannelType.Reliable:
                    return config.AddChannel(QosType.Reliable);
                case MLAPI.Transports.ChannelType.ReliableFragmented:
                    return config.AddChannel(QosType.ReliableFragmented);
                case MLAPI.Transports.ChannelType.ReliableSequenced:
                    return config.AddChannel(QosType.ReliableSequenced);
                case MLAPI.Transports.ChannelType.StateUpdate:
                    return config.AddChannel(QosType.StateUpdate);
                case MLAPI.Transports.ChannelType.ReliableStateUpdate:
                    return config.AddChannel(QosType.ReliableStateUpdate);
                case MLAPI.Transports.ChannelType.AllCostDelivery:
                    return config.AddChannel(QosType.AllCostDelivery);
                case MLAPI.Transports.ChannelType.UnreliableFragmentedSequenced:
                    return config.AddChannel(QosType.UnreliableFragmentedSequenced);
                case MLAPI.Transports.ChannelType.ReliableFragmentedSequenced:
                    return config.AddChannel(QosType.ReliableFragmentedSequenced);
            }
            return 0;
        }

        private void UpdateRelay()
        {
            RelayTransport.Enabled = UseMLAPIRelay;
            RelayTransport.RelayAddress = MLAPIRelayAddress;
            RelayTransport.RelayPort = (ushort)MLAPIRelayPort;
        }
    }
}
#pragma warning restore CS1591 // Missing XML comment for publicly visible type or member
