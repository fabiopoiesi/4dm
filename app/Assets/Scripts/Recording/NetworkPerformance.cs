using MLAPI;
using MLAPI.Messaging;
using Reconstruction4D.Settings;
using System.Runtime.InteropServices;
using System.Threading;
using UnityEngine;

/// <summary>
/// This class measure the network performance to the edge server to limit bandwidth.
/// All clients execute the measures at the same time so it consider the problems that can derive by sharing the same channel
/// </summary>
public class NetworkPerformance : NetworkedBehaviour
{
    private static ulong bandwidth = ulong.MaxValue;

    /// <summary>
    /// Bandwith measure during the last measurement session
    /// </summary>
    public static ulong Bandwidth { get => bandwidth; }

    public delegate void NetworkPerformanceResult();

    /// <summary>
    /// Count how many client finish
    /// </summary>
    private int clientFinish = 0;

    /// <summary>
    /// This is set to true when performance measure end
    /// </summary>
    private bool triggerFinishMeasure = false;

    /// <summary>
    /// the reference to the function to call when all clients finish
    /// </summary>
    private NetworkPerformanceResult responseDelegate;

    /// <summary>
    /// Start on all the client the performance measurement
    /// </summary>
    /// <param name="result">The function to call when result are ready</param>
    public void InitNetworkPerformanceMeasurement(NetworkPerformanceResult result)
    {
        Debug.Log("Measure network performance send 1");
        responseDelegate = result;
        this.clientFinish = 0;
        InvokeClientRpcOnEveryone(InitPerformanceMeasure);
        Debug.Log("Measure network performance send");
    }

    /// <summary>
    ///  Each client start to measure performance
    /// </summary>
    [ClientRPC]
    private void InitPerformanceMeasure()
    {
        Debug.Log("InitPerformanceMeasure call");
        Thread performanceThread = new Thread(PerformanceMeasureThread);
        performanceThread.Start();
        Debug.Log("performance thread call");
    }

    /// <summary>
    /// Check if performance measure success
    /// </summary>
    /// <returns>function result</returns>
    [DllImport("image_sending", CharSet = CharSet.Ansi)]
    private static extern bool getStatus();

    /// <summary>
    /// Return a Description of why performance measure don't work
    /// </summary>
    /// <returns>Description of why performance measure don't work</returns>
    [DllImport("image_sending", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall)]
    [return: MarshalAs(UnmanagedType.LPStr)] private static extern string getErrorMessage();

    /// <summary>
    /// If performance measure success return the average rate
    /// </summary>
    /// <returns>Bandwith in bytes/sec</returns>
    [DllImport("image_sending", CharSet = CharSet.Ansi)]
    private static extern ulong getAvgRate();

    /// <summary>
    /// If performance measure success return the total byte send during measure
    /// </summary>
    /// <returns>how many bytes was sent</returns>
    [DllImport("image_sending", CharSet = CharSet.Ansi)]
    private static extern ulong getTotalByteSend();

    /// <summary>
    /// Measure the performance to the receiving server
    /// </summary>
    /// <param name="serverIpAddress">The Ip address of the server</param>
    /// <param name="serverPort">The server port</param>
    [DllImport("image_sending", CharSet = CharSet.Ansi)]
    private static extern void performanceTesting([MarshalAs(UnmanagedType.LPStr)] string serverIpAddress, uint serverPort);

    /// <summary>
    /// This function is executed as thread to measure bandwith to the server
    /// </summary>
    private void PerformanceMeasureThread()
    {
        Debug.Log("Measure bandwidth");
        performanceTesting(Settings.Instance.ElaborationServerAddress, Settings.Instance.PerformanceServerPort);
        if(getStatus() == true)
        {
            NetworkPerformance.bandwidth = getAvgRate();
            Debug.Log("Average bandwith: " + NetworkPerformance.bandwidth);
        } else
        {
            string message = getErrorMessage();
            Debug.Log("Performance measure error: " + message);
        }
        triggerFinishMeasure = true;
        Debug.Log("Performance measure thread finish");
    }

    /// <summary>
    /// Update the data
    /// </summary>
    private void Update()
    {
        if(triggerFinishMeasure == true)
        {
            Debug.Log("Trigger finish measure");
            triggerFinishMeasure = false;
            InvokeServerRpc(SendFinished);
        }
    }

    /// <summary>
    /// When all the client finish, it call the callback set during init
    /// </summary>
    [ServerRPC(RequireOwnership = false)]
    private void SendFinished()
    {
        clientFinish++;
        if(clientFinish >= NetworkingManager.Singleton.ConnectedClients.Count)
        {
            Debug.Log("Max bandwith: " + NetworkPerformance.bandwidth);
            this.responseDelegate?.Invoke();
        }
    }
}
