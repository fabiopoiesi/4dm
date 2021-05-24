using UnityEngine;
using UnityEngine.UI;

namespace Reconstruction4D
{
    public class ConnectionInfoDisplay : MonoBehaviour
    {
        /// <summary>
        /// The connection type label
        /// </summary>
        public GameObject ConnectionTypeLabel;

        /// <summary>
        /// The server IP label
        /// </summary>
        public GameObject serverIPLabel;

        public void SetDetail(bool useRelay, string connectionIpAddress, int port)
        {
            gameObject.SetActive(true);
            gameObject.GetComponent<Image>().color = useRelay ? Color.red : Color.green;
            ConnectionTypeLabel.GetComponent<Text>().text = useRelay ? "Relay Connection" : "Direct Connection";
            serverIPLabel.GetComponent<Text>().text = "Server IP: " + connectionIpAddress + ":" + port;
        }
    }
}
