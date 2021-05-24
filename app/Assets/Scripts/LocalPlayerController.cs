//-----------------------------------------------------------------------
// <copyright file="LocalPlayerController.cs" company="Google">
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
    using MLAPI;
    using MLAPI.Messaging;
    using System;
    using System.Collections.Generic;
    using System.IO;
    using UnityEngine;
#if PLATFORM_ANDROID
    using UnityEngine.Android;
#endif

    /// <summary>
    /// Local player controller. Handles the spawning of the networked Game Objects.
    /// </summary>
    public class LocalPlayerController : NetworkedBehaviour
    {
        /// <summary>
        /// The Star model that will represent networked objects in the scene.
        /// </summary>
        public GameObject StarPrefab;

        /// <summary>
        /// The Anchor model that will represent the anchor in the scene.
        /// </summary>
        public GameObject AnchorPrefab;

        /// <summary>
        /// The Anchor model that will represent the anchor in the scene.
        /// </summary>
        public GameObject RecordingControllerPrefab;

        public static Dictionary<ulong, GameObject> anchorsPlaced;

        /// <summary>
        /// The Unity Start() method.
        /// </summary>
        public void Start()
        {
            gameObject.name = "LocalPlayer";
            anchorsPlaced = new Dictionary<ulong, GameObject>();
            // Require permission for write file
#if PLATFORM_ANDROID
            if (!Permission.HasUserAuthorizedPermission(Permission.ExternalStorageWrite))
            {
                Permission.RequestUserPermission(Permission.ExternalStorageWrite);
            }
#endif
        }

        /// <summary>
        /// Will spawn the origin anchor and host the Cloud Anchor. Must be called by the host.
        /// </summary>
        /// <param name="position">Position of the object to be instantiated.</param>
        /// <param name="rotation">Rotation of the object to be instantiated.</param>
        /// <param name="anchor">The ARCore Anchor to be hosted.</param>
        public void SpawnAnchor(Vector3 position, Quaternion rotation, Component anchor, string creatorClientId)
        {
            // With the anchor we need to instance also the recording controller

            // Instantiate Recording Controller model at the hit pose.
            GameObject recordingControllerObject = Instantiate(RecordingControllerPrefab, Vector3.zero, Quaternion.identity);

            // Host can spawn directly without using a Command because the server is running in this instance.
            recordingControllerObject.GetComponent<NetworkedObject>().Spawn();

            // After recording controller we can instance the anchor as usual

            // Instantiate Anchor model at the hit pose.
            GameObject anchorObject = Instantiate(AnchorPrefab, position, rotation);

            anchorObject.GetComponent<MultiplatformMeshSelector>().creatorClientId.Value = creatorClientId;
            anchorObject.GetComponent<MultiplatformMeshSelector>().isAnchor = true;
            anchorObject.GetComponent<MultiplatformMeshSelector>().initDateTime = DateTime.Now;

            // Anchor must be hosted in the device.
            anchorObject.GetComponent<AnchorController>().HostLastPlacedAnchor(anchor);

            // Host can spawn directly without using a Command because the server is running in this instance.
            anchorObject.GetComponent<NetworkedObject>().Spawn();

            // Add to the map of anchors
            anchorsPlaced.Add(anchorObject.GetComponent<MultiplatformMeshSelector>().NetworkId, anchorObject);
        }

        public void SpawnStar(Vector3 position, Quaternion rotation, string creatorClientId, long initDateTimeTicks)
        {
            InvokeServerRpc(CmdSpawnStar, position, rotation, creatorClientId, initDateTimeTicks);
        }

        /// <summary>
        /// A command run on the server that will spawn the Star prefab in all clients.
        /// </summary>
        /// <param name="position">Position of the object to be instantiated.</param>
        /// <param name="rotation">Rotation of the object to be instantiated.</param>
        [ServerRPC(RequireOwnership = false)]
        public void CmdSpawnStar(Vector3 position, Quaternion rotation, string creatorClientId, long initDateTimeTicks)
        {
            // Instantiate Star model at the hit pose.
            GameObject starObject = Instantiate(StarPrefab, position, rotation);
            starObject.GetComponent<MultiplatformMeshSelector>().creatorClientId.Value = creatorClientId;
            starObject.GetComponent<MultiplatformMeshSelector>().initDateTime = new DateTime(initDateTimeTicks);
            starObject.GetComponent<MultiplatformMeshSelector>().isAnchor = false;

            // Spawn the object in all clients.
            starObject.GetComponent<NetworkedObject>().Spawn();

            // Add to the map of anchors / stars
            anchorsPlaced.Add(starObject.GetComponent<MultiplatformMeshSelector>().NetworkId, starObject);
        }

        public void ReceiveStar(ulong gameObjectNetId)
        {
            InvokeServerRpc(CmdStarReceived, gameObjectNetId);
        }

        [ServerRPC(RequireOwnership = false)]
        public void CmdStarReceived(ulong gameObjectNetId)
        {
            if (anchorsPlaced.ContainsKey(gameObjectNetId))
            {
                GameObject anchorOrStarPlaced = anchorsPlaced[gameObjectNetId];
                DateTime finishDateTime = DateTime.Now;
                TimeSpan duration = finishDateTime.Subtract(anchorOrStarPlaced.GetComponent<MultiplatformMeshSelector>().initDateTime);
                FileLogger.AppendText(Path.Combine(Application.persistentDataPath, "duration.jl"), "{\"latency\":" + duration.TotalMilliseconds + ",\"hostingSmartphonePlaceStar\":" + (anchorOrStarPlaced.GetComponent<MultiplatformMeshSelector>().creatorClientId.Value == SystemInfo.deviceUniqueIdentifier ? true : false) + ",\"isAnchor\":" + anchorOrStarPlaced.GetComponent<MultiplatformMeshSelector>().isAnchor + "}");
                anchorsPlaced.Remove(gameObjectNetId);
            }
        }
    }
}
