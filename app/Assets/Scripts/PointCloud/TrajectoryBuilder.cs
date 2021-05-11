namespace Reconstruction4D.PointCloud
{
    using System.Collections.Generic;
    using UnityEngine;

    public struct TrajectoryBuilder
    {
        private List<Pose> trajectory;

        //-----------------------------------------------------------------------
        public TrajectoryBuilder(int _unused)
        {
            trajectory = new List<Pose>();
        }

        //-----------------------------------------------------------------------
        public int Size()
        {
            return trajectory.Count;
        }

        //-----------------------------------------------------------------------
        public void Update(Pose pose)
        {
            // Pose is a structure of Unity3D
            trajectory.Add(pose);
        }

        //-----------------------------------------------------------------------
        public List<Pose> GetTrajectory()
        {
            return trajectory;
        }

        //-----------------------------------------------------------------------
        public Pose GetLastPose()
        {
            return trajectory[trajectory.Count - 1];
        }
    }
}
