using UnityEngine;
using UnityEngine.Networking;

namespace Reconstruction4D.Recording.FrameTransmission
{
    class FrameNode
    {
        public byte[] yBuffer;

        public byte[] vuBuffer;

        public int imageHeight;

        public int imageWidth;

        public long frameId;

        public Vector3 position;

        public Quaternion rotation;

        public Vector3 positionOri;

        public Quaternion rotationOri;

        public Vector2 focalLength;

        public Vector2 principalPoint;

        public byte[] jpegBuffer;
    }
}
