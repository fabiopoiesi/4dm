using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Text;

namespace Server.Core
{
    public class IPAddressDiscoveryServer
    {
        private Thread serverThread;
        private TcpListener server;

        public void Start(int port)
        {
            this.serverThread = new Thread(() => ServerRunner(port))
            {
                IsBackground = true
            };
            this.serverThread.Start();
        }

        public void Stop()
        {
            this.serverThread.Interrupt();
            Console.WriteLine("[INFO] IP discovery server stop");
        }

        public void ServerRunner(int port)
        {
            this.server = new TcpListener(IPAddress.Any, port);
            // we set our IP address as server's address, and we also set the port: 9999

            this.server.Start();  // this will start the server

            Console.WriteLine("[INFO] IP discovery server available at port " + port);

            while (true)   //we wait for a connection
            {
                TcpClient client = this.server.AcceptTcpClient();  //if a connection exists, the server will accept it

                NetworkStream ns = client.GetStream(); //networkstream is used to send/receive messages

                string address = ((IPEndPoint)client.Client.RemoteEndPoint).Address.ToString();
                byte[] addressByte = Encoding.UTF8.GetBytes(address);

                ns.Write(addressByte, 0, addressByte.Length);     //sending the message

                ns.Flush();
                ns.Close();
                client.Close();
            }
        }
    }
}
