//-----------------------------------------------------------------------
// <copyright file="NetworkManagerUIController.cs" company="Google">
//
// Copyright 2018 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// </copyright>
//-----------------------------------------------------------------------

namespace Reconstruction4D
{
    using System.Collections.Generic;
    using UnityEngine;
    using MLAPI;
    using UnityEngine.Networking;
    using UnityEngine.Networking.Match;
    using UnityEngine.UI;
    using System.Net;
    using System.Net.Sockets;
    using System;
    using Reconstruction4D.UI;
    using Reconstruction4D.Transport;
    using System.IO;
    using MLAPI.Transports.UNET;
    using System.Text;

    /// <summary>
    /// Controller managing UI for joining and creating rooms.
    /// </summary>
    [RequireComponent(typeof(NetworkingManager))]
    [RequireComponent(typeof(NetworkManager))]
    public class NetworkManagerUIController : MonoBehaviour
    {
        /// <summary>
        /// The Lobby Screen to see Available Rooms or create a new one.
        /// </summary>
        public Canvas LobbyScreen;

        /// <summary>
        /// The snackbar text.
        /// </summary>
        public Text SnackbarText;

        /// <summary>
        /// The Label showing the current active room.
        /// </summary>
        public GameObject CurrentRoomLabel;

        /// <summary>
        /// The Cloud Anchors Example Controller.
        /// </summary>
        public Reconstruction4DController Reconstruction4DController;

        /// <summary>
        /// The Panel containing the list of available rooms to join.
        /// </summary>
        public GameObject RoomListPanel;

        /// <summary>
        /// Text indicating that no previous rooms exist.
        /// </summary>
        public Text NoPreviousRoomsText;

        /// <summary>
        /// The prefab for a row in the available rooms list.
        /// </summary>
        public GameObject JoinRoomListRowPrefab;

        /// <summary>
        /// This is a link to the dialog bog that show connection detail
        /// </summary>
        public ConnectionInfoDisplay ConnectionInfoPanel;

        /// <summary>
        /// This is a link to the UI element that show how many client are connected
        /// </summary>
        public ConnectedClientController ClientConnectedUI;

        /// <summary>
        /// The number of matches that will be shown.
        /// </summary>
        private const int k_MatchPageSize = 5;

        /// <summary>
        /// The Network Manager.
        /// </summary>
        private NetworkManager m_Manager;

        /// <summary>
        /// The current room number.
        /// </summary>
        private string m_CurrentRoomNumber;

        /// <summary>
        /// The Join Room buttons.
        /// </summary>
        private List<GameObject> m_JoinRoomButtonsPool = new List<GameObject>();

        /**
         ** Util functions
         **/

        /// <summary>
        /// Function to recovery client LAN IP
        /// </summary>
        private static string ExternalIPAddressAsString()
        {
            try
            {
                Socket clientSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                byte[] receiveBuffer = new byte[20];
                IPAddress elaborationServerAddress = IPAddress.Parse(Settings.Settings.Instance.RelayAddress);
                int IPDiscoveryPort = 8889;
                Debug.Log("ElaborationServerAddress: " + elaborationServerAddress.ToString() + " - IP Discovery Port: " + IPDiscoveryPort);
                clientSocket.Connect(new IPEndPoint(elaborationServerAddress, IPDiscoveryPort));
                Debug.Log("Client connect");
                int k = clientSocket.Receive(receiveBuffer);
                Debug.Log("Client receive " + k + " bytes");
                string address = Encoding.UTF8.GetString(receiveBuffer, 0, k);
                Debug.Log("Given address: " + address);
                return address;
            } catch(SocketException e)
            {
                Debug.LogError("SocketException: " + e.ToString());
                return "0.0.0.0";
            } catch(Exception e)
            {
                Debug.LogError("General Exception: " + e.ToString());
                return "0.0.0.0";
            }
        }

        private static IPAddress LocalIpAddress()
        {
            IPHostEntry host;
            host = Dns.GetHostEntry(Dns.GetHostName());
            foreach (IPAddress ip in host.AddressList)
            {
                if (ip.AddressFamily == AddressFamily.InterNetwork)
                {
                    return ip;
                }
            }
            return null;
        }

        /// <summary>
        /// The Unity Awake() method.
        /// </summary>
        public void Awake()
        {
            // Initialize the pool of Join Room buttons.
            for (int i = 0; i < k_MatchPageSize; i++)
            {
                GameObject button = Instantiate(JoinRoomListRowPrefab);
                button.transform.SetParent(RoomListPanel.transform, false);
                button.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, -100 - (200 * i));
                button.SetActive(true);
                button.GetComponentInChildren<Text>().text = string.Empty;
                m_JoinRoomButtonsPool.Add(button);
            }
            m_Manager = GetComponent<NetworkManager>();
            m_Manager.StartMatchMaker();
            m_Manager.matchMaker.ListMatches(
                startPageNumber: 0,
                resultPageSize: k_MatchPageSize,
                matchNameFilter: string.Empty,
                filterOutPrivateMatchesFromResults: false,
                eloScoreTarget: 0,
                requestDomain: 0,
                callback: _OnMatchList);

            _ChangeLobbyUIVisibility(true);
        }

        /// <summary>
        /// Handles the user intent to create a new room.
        /// </summary>
        public void OnCreateRoomClicked()
        {
            string ipAddress = ExternalIPAddressAsString();
            int port = this.GetComponent<CustomRelayedTransport>().ConnectPort;

            m_Manager.matchMaker.CreateMatch(m_Manager.matchName + "@" + ipAddress + ":" + port, m_Manager.matchSize,
                                            true, string.Empty, string.Empty, string.Empty,
                                            0, 0, _OnMatchCreate);
        }

        /// <summary>
        /// Handles the user intent to refresh the room list.
        /// </summary>
        public void OnRefhreshRoomListClicked()
        {
            m_Manager.matchMaker.ListMatches(
                startPageNumber: 0,
                resultPageSize: k_MatchPageSize,
                matchNameFilter: string.Empty,
                filterOutPrivateMatchesFromResults: false,
                eloScoreTarget: 0,
                requestDomain: 0,
                callback: _OnMatchList);
        }

        /// <summary>
        /// Callback indicating that the Cloud Anchor was instantiated and the host request was made.
        /// </summary>
        /// <param name="isHost">Indicates whether this player is the host.</param>
        public void OnAnchorInstantiated(bool isHost)
        {
            this.ClientConnectedUI.BlockNewClient();
            if (isHost)
            {
                SnackbarText.text = "Hosting Cloud Anchor...";
            }
            else
            {
                SnackbarText.text = "Cloud Anchor added to session! Attempting to resolve anchor...";
            }
        }

        /// <summary>
        /// Callback indicating that the Cloud Anchor was hosted.
        /// </summary>
        /// <param name="success">If set to <c>true</c> indicates the Cloud Anchor was hosted successfully.</param>
        /// <param name="response">The response string received.</param>
        public void OnAnchorHosted(bool success, string response)
        {
            if (success)
            {
                SnackbarText.text = "Cloud Anchor successfully hosted! Wait while we prepare the client";
            }
            else
            {
                SnackbarText.text = "Cloud Anchor could not be hosted. " + response;
            }
        }

        /// <summary>
        /// Callback indicating that the Cloud Anchor was resolved.
        /// </summary>
        /// <param name="success">If set to <c>true</c> indicates the Cloud Anchor was resolved successfully.</param>
        /// <param name="response">The response string received.</param>
        public void OnAnchorResolved(bool success, string response)
        {
            if (success)
            {
                SnackbarText.text = "Cloud Anchor successfully resolved! Capture the subject...";
            }
            else
            {
                SnackbarText.text = "Cloud Anchor could not be resolved. Will attempt again. " + response;
            }
        }

        /// <summary>
        /// Handles the user intent to join the room associated with the button clicked.
        /// </summary>
        /// <param name="match">The information about the match that the user intents to join.</param>
        private void _OnJoinRoomClicked(MatchInfoSnapshot match)
        {
            m_Manager.matchName = match.name;
            m_Manager.matchMaker.JoinMatch(match.networkId, string.Empty, string.Empty,
                                         string.Empty, 0, 0, _OnMatchJoined);
            Reconstruction4DController.OnEnterResolvingModeClick();
        }

        /// <summary>
        /// Callback that happens when a <see cref="T:NetworkMatch.ListMatches"/> request has been processed on the
        /// server.
        /// </summary>
        /// <param name="success">Indicates if the request succeeded.</param>
        /// <param name="extendedInfo">A text description for the error if success is false.</param>
        /// <param name="matches">A list of matches corresponding to the filters set in the initial list
        /// request.</param>
        private void _OnMatchList(bool success, string extendedInfo, List<MatchInfoSnapshot> matches)
        {
            m_Manager.OnMatchList(success, extendedInfo, matches);
            if (!success)
            {
                SnackbarText.text = "Could not list matches: " + extendedInfo;
                return;
            }

            if (m_Manager.matches != null)
            {
                // Reset all buttons in the pool.
                foreach (GameObject button in m_JoinRoomButtonsPool)
                {
                    button.SetActive(false);
                    button.GetComponentInChildren<Button>().onClick.RemoveAllListeners();
                    button.GetComponentInChildren<Text>().text = string.Empty;
                }

                NoPreviousRoomsText.gameObject.SetActive(m_Manager.matches.Count == 0);

                // Add buttons for each existing match.
                int i = 0;
                foreach (var match in m_Manager.matches)
                {
                    if (i >= k_MatchPageSize)
                    {
                        break;
                    }

                    var text = "Room " + _GeetRoomNumberFromNetworkId(match.networkId);
                    GameObject button = m_JoinRoomButtonsPool[i++];
                    button.GetComponentInChildren<Text>().text = text;
                    button.GetComponentInChildren<Button>().onClick.AddListener(() => _OnJoinRoomClicked(match));
                    button.SetActive(true);
                }
            }
        }

        private void ShowConnectionInformation()
        {
            ConnectionInfoPanel.SetDetail(true, MLAPI.Transports.UNET.RelayTransport.RelayAddress, MLAPI.Transports.UNET.RelayTransport.RelayPort);
        }

        /// <summary>
        /// Callback that happens when a <see cref="T:NetworkMatch.CreateMatch"/> request has been processed on the
        /// server.
        /// </summary>
        /// <param name="success">Indicates if the request succeeded.</param>
        /// <param name="extendedInfo">A text description for the error if success is false.</param>
        /// <param name="matchInfo">The information about the newly created match.</param>
        private void _OnMatchCreate(bool success, string extendedInfo, MatchInfo matchInfo)
        {
            if (!success)
            {
                SnackbarText.text = "Could not create match: " + extendedInfo;
                return;
            }

            SetTransportSettings();

            // Init connected element UI and client connection firewall
            ClientConnectedUI.InitConnectedClient();
            NetworkingManager.Singleton.OnClientConnectedCallback = ClientConnectedUI.OnClientConnected;
            NetworkingManager.Singleton.OnClientDisconnectCallback = ClientConnectedUI.OnclientDisconnectCallback;
            NetworkingManager.Singleton.ConnectionApprovalCallback = ClientConnectedUI.ApprovalCheck;

            // Setting the custom transport that change parameter to adapt settings to 4G network

            NetworkingManager.Singleton.StartHost();

            ShowConnectionInformation();

            m_CurrentRoomNumber = _GeetRoomNumberFromNetworkId(matchInfo.networkId);
            _ChangeLobbyUIVisibility(false);
            SnackbarText.text = "Wait all clients connect and pointing on same plane, when ready touch the plane...";
            CurrentRoomLabel.GetComponentInChildren<Text>().text = "Room: " + m_CurrentRoomNumber;
        }

        /// <summary>
        /// Callback that happens when a <see cref="T:NetworkMatch.JoinMatch"/> request has been processed on the
        /// server.
        /// </summary>
        /// <param name="success">Indicates if the request succeeded.</param>
        /// <param name="extendedInfo">A text description for the error if success is false.</param>
        /// <param name="matchInfo">The info for the newly joined match.</param>
        private void _OnMatchJoined(bool success, string extendedInfo, MatchInfo matchInfo)
        {
            if (!success)
            {
                SnackbarText.text = "Could not join to match: " + extendedInfo;
                return;
            }

            string ipAddress = "";
            int port = 0;
            bool parameterRecovered = true;
            try
            {
                string connectionURL = m_Manager.matchName.Substring(m_Manager.matchName.IndexOf('@') + 1);
                ipAddress = connectionURL.Substring(0, connectionURL.IndexOf(':'));
                port = Convert.ToInt32(connectionURL.Substring(connectionURL.IndexOf(':') + 1));
            }
            catch (Exception)
            {
                parameterRecovered = false;
            }
            if (parameterRecovered == false)
            {
                return;
            }

            // Init connected element UI
            ClientConnectedUI.HideUIElement();

            SetTransportSettings();

            CustomRelayedTransport customRelayedTransport = GetComponent<CustomRelayedTransport>();
            customRelayedTransport.ConnectAddress = ipAddress;
            customRelayedTransport.ConnectPort = port;

            // Settings client IP address

            NetworkingManager.Singleton.StartClient();

            ShowConnectionInformation();

            m_CurrentRoomNumber = _GeetRoomNumberFromNetworkId(matchInfo.networkId);
            _ChangeLobbyUIVisibility(false);
            SnackbarText.text = "Point the same plane of hosting smartphone and wait...";
            CurrentRoomLabel.GetComponentInChildren<Text>().text = "Room: " + m_CurrentRoomNumber;
        }

        private void SetTransportSettings()
        {
            // Setting the custom transport that change parameter to adapt settings to 4G network
            CustomRelayedTransport customRelayedTransport = GetComponent<CustomRelayedTransport>();
            if (Settings.Settings.Instance.Type == Settings.Settings.ConnectionType.MLAPI_Relay)
            {
                // Set dynamically Relay Server address
                customRelayedTransport.UseMLAPIRelay = true;
                customRelayedTransport.MLAPIRelayAddress = Settings.Settings.Instance.RelayAddress;
                customRelayedTransport.MLAPIRelayPort = 8888;
            }
            else
            {
                // Unused
                customRelayedTransport.UseMLAPIRelay = false;
            }
        }

        /// <summary>
        /// Changes the lobby UI Visibility by showing or hiding the buttons.
        /// </summary>
        /// <param name="visible">If set to <c>true</c> the lobby UI will be visible. It will be hidden
        /// otherwise.</param>
        private void _ChangeLobbyUIVisibility(bool visible)
        {
            LobbyScreen.gameObject.SetActive(visible);
            CurrentRoomLabel.gameObject.SetActive(!visible);
            foreach (GameObject button in m_JoinRoomButtonsPool)
            {
                bool active = visible && button.GetComponentInChildren<Text>().text != string.Empty;
                button.SetActive(active);
            }

            Debug.Log("Change Lobby visibility " + visible);
            Screen.autorotateToLandscapeLeft = true;
            Screen.autorotateToLandscapeRight = visible;
            Screen.autorotateToPortrait = visible;
            Screen.autorotateToPortraitUpsideDown = visible;
            Screen.orientation = visible ? ScreenOrientation.AutoRotation : ScreenOrientation.LandscapeLeft;
            Debug.Log("Orientation " + Screen.orientation.ToString());
        }

        private string _GeetRoomNumberFromNetworkId(UnityEngine.Networking.Types.NetworkID networkID)
        {
            return (System.Convert.ToInt64(networkID.ToString()) % 10000).ToString();
        }
    }
}
