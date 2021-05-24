//-----------------------------------------------------------------------
// <copyright file="Reconstruction4DController.cs" company="Google">
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
    using Reconstruction4D.Recording;
    using Reconstruction4D.UI;
    using System;
    using UnityEngine;
    using Reconstruction4D.PointCloud;
#if UNITY_EDITOR
    // NOTE:
    // - InstantPreviewInput does not support `deltaPosition`.
    // - InstantPreviewInput does not support input from
    //   multiple simultaneous screen touches.
    // - InstantPreviewInput might miss frames. A steady stream
    //   of touch events across frames while holding your finger
    //   on the screen is not guaranteed.
    // - InstantPreviewInput does not generate Unity UI event system
    //   events from device touches. Use mouse/keyboard in the editor
    //   instead.
    using Input = GoogleARCore.InstantPreviewInput;
#endif


    /// <summary>
    /// Controller for the Cloud Anchors Example. Handles the ARCore lifecycle.
    /// </summary>
    public class Reconstruction4DController : MonoBehaviour
    {
        [Header("ARCore")]

        /// <summary>
        /// The UI Controller.
        /// </summary>
        public NetworkManagerUIController UIController;

        /// <summary>
        /// The root for ARCore-specific GameObjects in the scene.
        /// </summary>
        public GameObject ARCoreRoot;

        /// <summary>
        /// The helper that will calculate the World Origin offset when performing a raycast or generating planes.
        /// </summary>
        public ARCoreWorldOriginHelper ARCoreWorldOriginHelper;

        [Header("ARKit")]

        /// <summary>
        /// The root for ARKit-specific GameObjects in the scene.
        /// </summary>
        public GameObject ARKitRoot;

        /// <summary>
        /// The first-person camera used to render the AR background texture for ARKit.
        /// </summary>
        public Camera ARKitFirstPersonCamera;

        [Header("Common")]

        /// <summary>
        /// A helper object to ARKit functionality.
        /// </summary>
        private ARKitHelper m_ARKit = new ARKitHelper();

        /// <summary>
        /// Indicates whether the Origin of the new World Coordinate System, i.e. the Cloud Anchor, was placed.
        /// </summary>
        private bool m_IsOriginPlaced = false;

        /// <summary>
        /// Indicates whether the Anchor was already instantiated.
        /// </summary>
        private bool m_AnchorAlreadyInstantiated = false;

        /// <summary>
        /// Indicates whether the Cloud Anchor finished hosting.
        /// </summary>
        private bool m_AnchorFinishedHosting = false;

        /// <summary>
        /// True if the app is in the process of quitting due to an ARCore connection error, otherwise false.
        /// </summary>
        private bool m_IsQuitting = false;

        /// <summary>
        /// The last placed anchor.
        /// </summary>
        private Component m_LastPlacedAnchor = null;

        /// <summary>
        /// The current cloud anchor mode.
        /// </summary>
        private ApplicationMode m_CurrentMode = ApplicationMode.Ready;

        private PointCloudBuilder pointCloudBuilder;

        private TrajectoryBuilder trajectoryBuilder;

        /// <summary>
        /// Enumerates modes the example application can be in.
        /// </summary>
        public enum ApplicationMode
        {
            Ready,
            Hosting,
            Resolving,
        }

        /// <summary>
        /// The Unity Start() method.
        /// </summary>
        public void Start()
        {
            Debug.LogWarning("Unique ID: " + SystemInfo.deviceUniqueIdentifier);

            // A Name is provided to the Game Object so it can be found by other Scripts instantiated as prefabs in the
            // scene.
            gameObject.name = "Reconstruction4DController";
            ARCoreRoot.SetActive(false);
            ARKitRoot.SetActive(false);
            _ResetStatus();

            pointCloudBuilder = new PointCloudBuilder(10);
            trajectoryBuilder = new TrajectoryBuilder(10);
        }

        /// <summary>
        /// The Unity Update() method.
        /// </summary>
        public void Update()
        {
            _UpdateApplicationLifecycle();

            if (Frame.PointCloud.IsUpdatedThisFrame)
            {
                _UpdatePointCloud();
                _UpdateTrajectoryBuilder();
            }

            // If we are neither in hosting nor resolving mode then the update is complete.
            if (m_CurrentMode != ApplicationMode.Hosting && m_CurrentMode != ApplicationMode.Resolving)
            {
                return;
            }

            // If the origin anchor has not been placed yet, then update in resolving mode is complete.
            if (m_CurrentMode == ApplicationMode.Resolving && !m_IsOriginPlaced)
            {
                return;
            }

            if (_CanPlaceStars())
            {
                return;
            }

            // If the player has not touched the screen then the update is complete.
            Touch touch;
            if (Input.touchCount < 1 || (touch = Input.GetTouch(0)).phase != TouchPhase.Began)
            {
                return;
            }

            if (RecordingButtonController.singleton != null)
            {
                Debug.Log("[Reconstruction4DController] RecordingButtonController Exist");
                if (RecordingButtonController.singleton.TouchIsInsideRecordingButton(Input.GetTouch(0)))
                {
                    return;
                }
            }

            // Raycast against the location the player touched to search for planes.
            if (Application.platform != RuntimePlatform.IPhonePlayer)
            {
                TrackableHit hit;
                if (ARCoreWorldOriginHelper.Raycast(touch.position.x, touch.position.y,
                        TrackableHitFlags.PlaneWithinPolygon, out hit))
                {
                    m_LastPlacedAnchor = hit.Trackable.CreateAnchor(hit.Pose);
                }
            }
            else
            {
                Pose hitPose;
                if (m_ARKit.RaycastPlane(ARKitFirstPersonCamera, touch.position.x, touch.position.y, out hitPose))
                {
                    m_LastPlacedAnchor = m_ARKit.CreateAnchor(hitPose);
                }
            }

            // If there was an anchor placed, then instantiate the corresponding object.
            if (m_LastPlacedAnchor != null)
            {
                // The first touch on the Hosting mode will instantiate the origin anchor. Any subsequent touch will
                // instantiate a star, both in Hosting and Resolving modes.
                if (_CanPlaceStars())
                {
                    _InstantiateStar();
                }
                else if (!m_IsOriginPlaced && m_CurrentMode == ApplicationMode.Hosting)
                {
                    SetWorldOrigin(m_LastPlacedAnchor.transform);
                    _InstantiateAnchor();
                    OnAnchorInstantiated(true);
                }
            }
        }

        private void _UpdatePointCloud()
        {
#if !(UNITY_EDITOR)
            for (int p = 0; p < Frame.PointCloud.PointCount; p++)
            {
                pointCloudBuilder.Update(Frame.PointCloud.GetPointAsStruct(p));
            }
#endif
        }

        private void _UpdateTrajectoryBuilder()
        {
#if !(UNITY_EDITOR)
            trajectoryBuilder.Update(Frame.Pose);
#endif
        }

        public void SavePointCloud(string prefix = "")
        {
            Debug.Log("Path: " + System.IO.Path.Combine(Application.persistentDataPath, prefix + SystemInfo.deviceUniqueIdentifier + "_" + DateTime.UtcNow.ToString("yyyyMMddHHmmss") + ".ply"));
            Utilities.SavePointCloudOnFile(System.IO.Path.Combine(Application.persistentDataPath, prefix + SystemInfo.deviceUniqueIdentifier + "_" + DateTime.UtcNow.ToString("yyyyMMddHHmmss") + ".ply"), pointCloudBuilder.getPointCloud(), trajectoryBuilder.GetTrajectory());
        }

        /// <summary>
        /// Sets the apparent world origin so that the Origin of Unity's World Coordinate System coincides with the
        /// Anchor. This function needs to be called once the Cloud Anchor is either hosted or resolved.
        /// </summary>
        /// <param name="anchorTransform">Transform of the Cloud Anchor.</param>
        public void SetWorldOrigin(Transform anchorTransform)
        {
            if (m_IsOriginPlaced)
            {
                Debug.LogWarning("The World Origin can be set only once.");
                return;
            }

            m_IsOriginPlaced = true;

            if (Application.platform != RuntimePlatform.IPhonePlayer)
            {
                ARCoreWorldOriginHelper.SetWorldOrigin(anchorTransform);
            }
            else
            {
                m_ARKit.SetWorldOrigin(anchorTransform);
            }

            OnWorldOriginSet();

        }

        private void SetScreenPosition(bool freePosition)
        {
            /*
            Debug.Log("Change Lobby visibility " + freePosition);
            Screen.autorotateToLandscapeLeft = true;
            Screen.autorotateToLandscapeRight = freePosition;
            Screen.autorotateToPortrait = freePosition;
            Screen.autorotateToPortraitUpsideDown = freePosition;
            Screen.orientation = freePosition ? ScreenOrientation.AutoRotation : ScreenOrientation.LandscapeLeft;
            Debug.Log("Orientation " + Screen.orientation.ToString());
            */
        }

        /// <summary>
        /// Handles user intent to enter a mode where they can place an anchor to host or to exit this mode if
        /// already in it.
        /// </summary>
        public void OnEnterHostingModeClick()
        {
            if (m_CurrentMode == ApplicationMode.Hosting)
            {
                Debug.Log("Application is in mode " + m_CurrentMode.ToString());
                m_CurrentMode = ApplicationMode.Ready;
                _ResetStatus();
            }

            Debug.Log("Application go to Hosting mode");
            m_CurrentMode = ApplicationMode.Hosting;
            _SetPlatformActive();
            SetScreenPosition(false);
        }

        /// <summary>
        /// Handles a user intent to enter a mode where they can input an anchor to be resolved or exit this mode if
        /// already in it.
        /// </summary>
        public void OnEnterResolvingModeClick()
        {
            if (m_CurrentMode == ApplicationMode.Resolving)
            {
                m_CurrentMode = ApplicationMode.Ready;
                _ResetStatus();
            }

            m_CurrentMode = ApplicationMode.Resolving;
            _SetPlatformActive();
            SetScreenPosition(false);
        }

        /// <summary>
        /// Callback indicating that the Cloud Anchor was instantiated and the host request was made.
        /// </summary>
        /// <param name="isHost">Indicates whether this player is the host.</param>
        public void OnAnchorInstantiated(bool isHost)
        {
            if (m_AnchorAlreadyInstantiated)
            {
                return;
            }

            m_AnchorAlreadyInstantiated = true;
            UIController.OnAnchorInstantiated(isHost);
        }

        /// <summary>
        /// Callback indicating that the Cloud Anchor was hosted.
        /// </summary>
        /// <param name="success">If set to <c>true</c> indicates the Cloud Anchor was hosted successfully.</param>
        /// <param name="response">The response string received.</param>
        public void OnAnchorHosted(bool success, string response)
        {
            m_AnchorFinishedHosting = success;
            UIController.OnAnchorHosted(success, response);
            RecordingController.singleton.CanStartRecording();
        }

        /// <summary>
        /// Callback indicating that the Cloud Anchor was resolved.
        /// </summary>
        /// <param name="success">If set to <c>true</c> indicates the Cloud Anchor was resolved successfully.</param>
        /// <param name="response">The response string received.</param>
        public void OnAnchorResolved(bool success, string response)
        {
            UIController.OnAnchorResolved(success, response);
        }

        public void OnWorldOriginSet()
        {
            SavePointCloud("onanchor_");
        }

        /// <summary>
        /// Instantiates the anchor object at the pose of the m_LastPlacedAnchor Anchor. This will host the Cloud
        /// Anchor.
        /// </summary>
        private void _InstantiateAnchor()
        {
            // The anchor will be spawned by the host, so no networking Command is needed.
            GameObject.Find("LocalPlayer").GetComponent<LocalPlayerController>()
                      .SpawnAnchor(Vector3.zero, Quaternion.identity, m_LastPlacedAnchor, SystemInfo.deviceUniqueIdentifier);
        }

        /// <summary>
        /// Instantiates a star object that will be synchronized over the network to other clients.
        /// </summary>
        private void _InstantiateStar()
        {
            // Star must be spawned in the server so a networking Command is used.
            GameObject.Find("LocalPlayer").GetComponent<LocalPlayerController>().SpawnStar(m_LastPlacedAnchor.transform.position, m_LastPlacedAnchor.transform.rotation, SystemInfo.deviceUniqueIdentifier, DateTime.Now.Ticks);
        }

        /// <summary>
        /// Sets the corresponding platform active.
        /// </summary>
        private void _SetPlatformActive()
        {
            if (Application.platform != RuntimePlatform.IPhonePlayer)
            {
                ARCoreRoot.SetActive(true);
                ARKitRoot.SetActive(false);
            }
            else
            {
                ARCoreRoot.SetActive(false);
                ARKitRoot.SetActive(true);
            }
        }

        /// <summary>
        /// Indicates whether a star can be placed.
        /// </summary>
        /// <returns><c>true</c>, if stars can be placed, <c>false</c> otherwise.</returns>
        private bool _CanPlaceStars()
        {
            if (m_CurrentMode == ApplicationMode.Resolving)
            {
                return m_IsOriginPlaced;
            }

            if (m_CurrentMode == ApplicationMode.Hosting)
            {
                return m_IsOriginPlaced && m_AnchorFinishedHosting;
            }

            return false;
        }

        /// <summary>
        /// Resets the internal status.
        /// </summary>
        private void _ResetStatus()
        {
            // Reset internal status.
            m_CurrentMode = ApplicationMode.Ready;
            if (m_LastPlacedAnchor != null)
            {
                Destroy(m_LastPlacedAnchor.gameObject);
            }
            SetScreenPosition(true);

            m_LastPlacedAnchor = null;
        }

        /// <summary>
        /// Check and update the application lifecycle.
        /// </summary>
        private void _UpdateApplicationLifecycle()
        {
            // Exit the app when the 'back' button is pressed.
            if (Input.GetKey(KeyCode.Escape))
            {
                Application.Quit();
            }

            var sleepTimeout = SleepTimeout.NeverSleep;

#if !UNITY_IOS
            // Only allow the screen to sleep when not tracking.
            if (Session.Status != SessionStatus.Tracking)
            {
                const int lostTrackingSleepTimeout = 50;
                sleepTimeout = lostTrackingSleepTimeout;
            }
#endif

            if (m_IsQuitting)
            {
                return;
            }

            // Quit if ARCore was unable to connect and give Unity some time for the toast to appear.
            if (Session.Status == SessionStatus.ErrorPermissionNotGranted)
            {
                Toast.ShowAndroidToastMessage("Camera permission is needed to run this application.");
                m_IsQuitting = true;
                Invoke("_DoQuit", 0.5f);
            }
            else if (Session.Status.IsError())
            {
                Toast.ShowAndroidToastMessage("ARCore encountered a problem connecting.  Please start the app again.");
                m_IsQuitting = true;
                Invoke("_DoQuit", 0.5f);
            }
        }

        /// <summary>
        /// Actually quit the application.
        /// </summary>
        private void _DoQuit()
        {
            Application.Quit();
        }
    }
}
