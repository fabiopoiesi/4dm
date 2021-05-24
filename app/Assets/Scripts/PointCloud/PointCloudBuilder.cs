namespace Reconstruction4D.PointCloud
{
    using GoogleARCore;
    using System.Collections.Generic;

    public struct PointCloudBuilder
    {

        Dictionary<int, PointCloudPoint> pointCloud;

        //-----------------------------------------------------------------------
        public PointCloudBuilder(int _unused)
        {
            pointCloud = new Dictionary<int, PointCloudPoint>();
        }

        //-----------------------------------------------------------------------
        public int Size()
        {
            return pointCloud.Count;
        }

        //-----------------------------------------------------------------------
        public void Update(PointCloudPoint pt)
        {
            pointCloud[pt.Id] = pt;
        }

        //-----------------------------------------------------------------------
        public Dictionary<int, PointCloudPoint> getPointCloud()
        {
            return pointCloud;
        }
    }
}