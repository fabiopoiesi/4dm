//-----------------------------------------------------------------------
// <copyright file="AnchorController.cs" company="Google">
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
    using GoogleARCore;
    using GoogleARCore.CrossPlatform;
    using UnityEngine;
    using MLAPI;
    using MLAPI.Messaging;
    using MLAPI.NetworkedVar;

    /// <summary>
    /// A Controller for the Anchor object that handles hosting and resolving the Cloud Anchor.
    /// </summary>
    public class AnchorController : NetworkedBehaviour
    {
        /// <summary>
        /// The Cloud Anchor ID that will be used to host and resolve the Cloud Anchor. This variable will be
        /// syncrhonized over all clients.
        /// </summary>
        private NetworkedVar<string> cloudAnchorId = new NetworkedVar<string>(string.Empty);

        /// <summary>
        /// Indicates whether this script is running in the Host.
        /// </summary>
        private bool m_IsHost = false;

        /// <summary>
        /// Indicates whether an attempt to resolve the Cloud Anchor should be made.
        /// </summary>
        private bool m_ShouldResolve = false;

        /// <summary>
        /// The Cloud Anchors example controller.
        /// </summary>
        private Reconstruction4DController m_CloudAnchorsExampleController;

        /// <summary>
        /// The Unity Start() method.
        /// </summary>
        public void Start()
        {
            cloudAnchorId.OnValueChanged = OnChangeId;
            m_CloudAnchorsExampleController = GameObject.Find("Reconstruction4DController")
                                                        .GetComponent<Reconstruction4DController>();
        }

        /// <summary>
        /// The Unity Update() method.
        /// </summary>
        public void Update()
        {
            if (m_ShouldResolve)
            {
                _ResolveAnchorFromId(cloudAnchorId.Value);
            }
        }

        /// <summary>
        /// Command run on the server to set the Cloud Anchor Id.
        /// </summary>
        /// <param name="cloudAnchorId">The new Cloud Anchor Id.</param>
        [ServerRPC(RequireOwnership = false)]
        public void CmdSetCloudAnchorId(string cloudAnchorId)
        {
            this.cloudAnchorId.Value = cloudAnchorId;
        }

        /// <summary>
        /// Gets the Cloud Anchor Id.
        /// </summary>
        /// <returns>The Cloud Anchor Id.</returns>
        public string GetCloudAnchorId()
        {
            return cloudAnchorId.Value;
        }

        /// <summary>
        /// Hosts the user placed cloud anchor and associates the resulting Id with this object.
        /// </summary>
        /// <param name="lastPlacedAnchor">The last placed anchor.</param>
        public void HostLastPlacedAnchor(Component lastPlacedAnchor)
        {
            m_IsHost = true;

#if !UNITY_IOS
            var anchor = (Anchor)lastPlacedAnchor;
#elif ARCORE_IOS_SUPPORT
            var anchor = (UnityEngine.XR.iOS.UnityARUserAnchorComponent)lastPlacedAnchor;
#endif

#if !UNITY_IOS || ARCORE_IOS_SUPPORT
            XPSession.CreateCloudAnchor(anchor).ThenAction(result =>
            {
                if (result.Response != CloudServiceResponse.Success)
                {
                    Debug.Log(string.Format("Failed to host Cloud Anchor: {0}", result.Response));

                    m_CloudAnchorsExampleController.OnAnchorHosted(false, result.Response.ToString());
                    return;
                }

                Debug.Log(string.Format("Cloud Anchor {0} was created and saved.", result.Anchor.CloudId));

                InvokeServerRpc(CmdSetCloudAnchorId, result.Anchor.CloudId);

                m_CloudAnchorsExampleController.OnAnchorHosted(true, result.Response.ToString());
            });
#endif
        }

        /// <summary>
        /// Resolves an anchor id and instantiates an Anchor prefab on it.
        /// </summary>
        /// <param name="cloudAnchorId">Cloud anchor id to be resolved.</param>
        private void _ResolveAnchorFromId(string cloudAnchorId)
        {
            m_CloudAnchorsExampleController.OnAnchorInstantiated(false);

            // If device is not tracking, let's wait to try to resolve the anchor.
            if (Session.Status != SessionStatus.Tracking)
            {
                return;
            }

            m_ShouldResolve = false;

            XPSession.ResolveCloudAnchor(cloudAnchorId).ThenAction((System.Action<CloudAnchorResult>)(result =>
            {
                if (result.Response != CloudServiceResponse.Success)
                {
                    Debug.LogError(string.Format("Client could not resolve Cloud Anchor {0}: {1}",
                                                 cloudAnchorId, result.Response));

                    m_CloudAnchorsExampleController.OnAnchorResolved(false, result.Response.ToString());
                    m_ShouldResolve = true;
                    return;
                }

                Debug.Log(string.Format("Client successfully resolved Cloud Anchor {0}.",
                                        cloudAnchorId));

                m_CloudAnchorsExampleController.OnAnchorResolved(true, result.Response.ToString());
                _OnResolved(result.Anchor.transform);
            }));
        }

        /// <summary>
        /// Callback invoked once the Cloud Anchor is resolved.
        /// </summary>
        /// <param name="anchorTransform">Transform of the resolved Cloud Anchor.</param>
        private void _OnResolved(Transform anchorTransform)
        {
            var cloudAnchorController = GameObject.Find("Reconstruction4DController")
                                                  .GetComponent<Reconstruction4DController>();
            cloudAnchorController.SetWorldOrigin(anchorTransform);
        }

        /// <summary>
        /// Callback invoked once the Cloud Anchor Id changes.
        /// </summary>
        /// <param name="newValue">New identifier.</param>
        private void OnChangeId(string previousValue, string newValue)
        {
            if (!m_IsHost && newValue != string.Empty)
            {
                m_ShouldResolve = true;
            }
        }
    }
}
