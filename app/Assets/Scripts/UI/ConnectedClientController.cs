using MLAPI;
using MLAPI.Spawning;
using System;
using UnityEngine;
using UnityEngine.UI;
using static MLAPI.NetworkingManager;

namespace Reconstruction4D.UI
{
    public class ConnectedClientController : MonoBehaviour
    {
        public Text textClientConnect;

        public GameObject playerPrefab;

        private bool newClientAccepted = true;

        public void InitConnectedClient()
        {
            this.gameObject.SetActive(true);
            SetConnectedClient();
        }

        public void HideUIElement()
        {
            this.gameObject.SetActive(false);
        }

        public void OnclientDisconnectCallback(ulong clientId)
        {
            SetConnectedClient();
        }

        public void OnClientConnected(ulong clientId)
        {
            SetConnectedClient();
        }

        public void SetConnectedClient()
        {
            textClientConnect.text = Convert.ToString(NetworkingManager.Singleton.ConnectedClients.Count - 1 < 0 ? 0 : NetworkingManager.Singleton.ConnectedClients.Count - 1);
        }

        public void BlockNewClient()
        {
            this.newClientAccepted = false;
            this.gameObject.GetComponent<Image>().color = new Color(0, 1, 0, 0.3921569f);
        }

        public void ApprovalCheck(byte[] connectionData, ulong clientId, ConnectionApprovedDelegate callback)
        {
            ulong? prefabHash = SpawnManager.GetPrefabHashFromGenerator(null);

            callback(clientId, prefabHash, newClientAccepted, Vector3.zero, Quaternion.identity);
        }
    }
}
