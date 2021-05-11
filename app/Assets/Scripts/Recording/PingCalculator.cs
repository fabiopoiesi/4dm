using MLAPI;
using MLAPI.Connection;
using MLAPI.Messaging;
using MLAPI.Serialization.Pooled;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEngine;

namespace Reconstruction4D.Recording
{
    using AcquisitionDelays = KeyValuePair<ulong, double>;

    /// <summary>
    /// Ping calculator measure for 50 times the latency between each client and the host after it calculate the average
    /// </summary>
    public class PingCalculator : NetworkedBehaviour
    {
        private const int pingRound = 50;

        private int currentPingRound = 0;

        private PingResult responseDelegate;

        public delegate void PingResult(List<AcquisitionDelays> delay);

        private Dictionary<ulong, long> sendTimes;

        private Dictionary<ulong, List<long>> pingTimes;

        private System.Diagnostics.Stopwatch stopwatch;

        public void InitPingMeasurement(PingResult result)
        {
            responseDelegate = result;
            if (Settings.Settings.Instance.PingCompesationActive)
            {
                InitPingSystem();
            } else
            {
                ReturnADisabledLikePingSystem();
            }
        }

        private void ReturnADisabledLikePingSystem()
        {
            Debug.Log("Disable ping compensation");
            List<AcquisitionDelays> delays = new List<AcquisitionDelays>();
            foreach (NetworkedClient client in NetworkingManager.Singleton.ConnectedClientsList)
            {
                if (client.ClientId != NetworkingManager.Singleton.ServerClientId)
                {
                    delays.Add(new AcquisitionDelays(client.ClientId, 0.005f));
                }
            }
            delays.Add(new AcquisitionDelays(NetworkingManager.Singleton.ServerClientId, 0.005f));
            responseDelegate(delays);
        }

        private void InitPingSystem()
        {
            if (NetworkingManager.Singleton.ConnectedClients.Count <= 1)
            {
                List<AcquisitionDelays> delays = new List<AcquisitionDelays>()
                {
                    new AcquisitionDelays(NetworkingManager.Singleton.ServerClientId, 0)
                };
                responseDelegate(delays);
            }
            else
            {
                stopwatch = System.Diagnostics.Stopwatch.StartNew();
                pingTimes = new Dictionary<ulong, List<long>>();
                foreach (NetworkedClient client in NetworkingManager.Singleton.ConnectedClientsList)
                {
                    if (client.ClientId != NetworkingManager.Singleton.ServerClientId)
                    {
                        pingTimes.Add(client.ClientId, new List<long>());
                    }
                }
                sendTimes = new Dictionary<ulong, long>();
                ExecutePing();
            }
        }

        private void ExecutePing()
        {
            foreach (NetworkedClient client in NetworkingManager.Singleton.ConnectedClientsList)
            {
                if (client.ClientId != NetworkingManager.Singleton.ServerClientId)
                {
                    using (PooledBitStream stream = PooledBitStream.Get())
                    {
                        using (PooledBitWriter writer = PooledBitWriter.Get(stream))
                        {
                            writer.WriteUInt64Packed(client.ClientId);
                            InvokeClientRpcOnClientPerformance(PongPerformance, client.ClientId, stream);
                        }
                    }
                    sendTimes.Add(client.ClientId, stopwatch.ElapsedMilliseconds);
                    
                    // InvokeClientRpcOnClient(Pong, client.ClientId, client.ClientId);
                }
            }
            currentPingRound++;
        }

        [ClientRPC]
        public void Pong(ulong clientId)
        {
            InvokeServerRpc(ServerRPC, clientId);
        }

        [ClientRPC]
        public void PongPerformance(ulong clientId, Stream stream)
        {
            ulong serverClientId = 0;
            using (PooledBitReader reader = PooledBitReader.Get(stream))
            {
                serverClientId = reader.ReadUInt64Packed();
            }
            using (PooledBitStream responseStream = PooledBitStream.Get())
            {
                using (PooledBitWriter writer = PooledBitWriter.Get(responseStream))
                {
                    writer.WriteUInt64Packed(serverClientId);
                    InvokeServerRpcPerformance(ServerRPCPerformance, responseStream);
                }
            }
            Debug.Log("serverClientId: " + serverClientId + " clientId: " + clientId);
            // InvokeServerRpc(ServerRPC, clientId);
        }

        [ServerRPC(RequireOwnership = false)]
        public void ServerRPCPerformance(ulong clientId, Stream stream)
        {
            ulong packetClientId = 0;
            using (PooledBitReader reader = PooledBitReader.Get(stream))
            {
                packetClientId = reader.ReadUInt64Packed();
            }
            Debug.Log("packetClientId: " + packetClientId + " clientId: " + clientId);
            long pingTime = stopwatch.ElapsedMilliseconds - sendTimes[packetClientId];
            sendTimes.Remove(packetClientId);
            pingTimes[packetClientId].Add(pingTime / 2);
            if (sendTimes.Count == 0)
            {
                if (currentPingRound > pingRound)
                {
                    CalculateResult();
                }
                else
                {
                    ExecutePing();
                }
            }
        }

        [ServerRPC(RequireOwnership = false)]
        public void ServerRPC(ulong clientId)
        {
            long pingTime = stopwatch.ElapsedMilliseconds - sendTimes[clientId];
            sendTimes.Remove(clientId);
            pingTimes[clientId].Add(pingTime / 2);
            if(sendTimes.Count == 0)
            {
                if (currentPingRound > pingRound)
                {
                    CalculateResult();
                }
                else
                {
                    ExecutePing();
                }
            }
        }

        public void CalculateResult()
        {
            stopwatch.Stop();
            stopwatch = null;
            Dictionary<ulong, double> averages = new Dictionary<ulong, double>();
            double maxAverage = double.MinValue;
            foreach(KeyValuePair<ulong, List<long>> pingSequence in pingTimes)
            {
                double average = pingSequence.Value.Average();
                averages.Add(pingSequence.Key, average);
                if(average >= maxAverage)
                {
                    maxAverage = average;
                }
            }
            List<AcquisitionDelays> delays = new List<AcquisitionDelays>();
            foreach (KeyValuePair<ulong, double> average in averages)
            {
                delays.Add(new AcquisitionDelays(average.Key, (maxAverage - average.Value) / 1000));
            }
            delays.Add(new AcquisitionDelays(NetworkingManager.Singleton.ServerClientId, maxAverage / 1000));
            responseDelegate(delays);
        }
    }
}
