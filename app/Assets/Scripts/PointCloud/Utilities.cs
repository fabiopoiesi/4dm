namespace Reconstruction4D.PointCloud
{
    using GoogleARCore;
    using System;
    using System.IO;
    using System.Collections.Generic;
    using UnityEngine;

    using Debug = UnityEngine.Debug;

    public class Utilities
    {
        //-----------------------------------------------------------------------
        static public void SavePointCloudOnFile(string fileName, Dictionary<int, PointCloudPoint> pointCloud, List<Pose> trajectory = null)
        {
            string fileFormat = Path.GetExtension(fileName).ToUpper();
            Debug.Log("File format: \"" + fileFormat + "\"");
            if (fileFormat == ".PLY")
            {
                int totalNumPoints = pointCloud.Count;

                if (trajectory != null)
                {
                    totalNumPoints += trajectory.Count;
                }

                string[] header = { "ply",
                                    "format ascii 1.0",
                                    "comment Fabio Poiesi generated",
                                    string.Format("element vertex {0}", totalNumPoints),
                                    "property float x",
                                    "property float y",
                                    "property float z",
                                    "property uchar red",
                                    "property uchar green",
                                    "property uchar blue",
                                    "property float confidence",
                                    "end_header"};

                Pose worldPose = UnityEngine.Object.FindObjectOfType<ARCoreWorldOriginHelper>().WorldPose;

                using (StreamWriter fileNoEdit = new StreamWriter(fileName + ".noedit"))
                {
                    using (StreamWriter file = new StreamWriter(fileName))
                    {
                        // write normal header
                        foreach (string line in header)
                        {
                            file.WriteLine(line);
                        }

                        header[2] = "comment transformation: " + JsonUtility.ToJson(worldPose);
                        foreach (string line in header)
                        {
                            fileNoEdit.WriteLine(line);
                        }

                        // Translate point cloud
                        Matrix4x4 trsmatrix = Matrix4x4.TRS(worldPose.position, worldPose.rotation, new Vector3(1, 1, 1));

                        // write point cloud
                        List<int> ptIdList = new List<int>(pointCloud.Keys);
                        foreach (int ptId in ptIdList)
                        {
                            Vector3 globalPosition = trsmatrix.MultiplyPoint3x4(pointCloud[ptId].Position);

                            file.WriteLine(string.Format("{0} {1} {2} 255 255 255 {3}",
                                                                    globalPosition.x,
                                                                    globalPosition.y,
                                                                    globalPosition.z,
                                                                    pointCloud[ptId].Confidence));
                            fileNoEdit.WriteLine(string.Format("{0} {1} {2} 255 255 255 {3}",
                                                                    pointCloud[ptId].Position.x,
                                                                    pointCloud[ptId].Position.y,
                                                                    pointCloud[ptId].Position.z,
                                                                    pointCloud[ptId].Confidence));
                        }

                        // write trajectory
                        //
                        if (trajectory != null)
                        {
                            for (int t = 0; t < trajectory.Count; t++)
                            {
                                Vector3 globalPosition = trsmatrix.MultiplyPoint3x4(trajectory[t].position);

                                file.WriteLine(string.Format("{0} {1} {2} 0 255 0 1",
                                                                    globalPosition.x,
                                                                    globalPosition.y,
                                                                    globalPosition.z));
                                fileNoEdit.WriteLine(string.Format("{0} {1} {2} 0 255 0 1",
                                                                    trajectory[t].position.x,
                                                                    trajectory[t].position.y,
                                                                    trajectory[t].position.z));
                            }
                        }
                    }
                }

                Debug.Log("Point cloud stored -> PLY");
            }
            else if (fileFormat == ".CAM")
            {
                // TODO add to the file: timestamp of when the pose was recorded, the focal lenght and the distortion param
                string header = "CAM_V1";
            
                using (System.IO.StreamWriter file = new System.IO.StreamWriter(fileName))
                {
                    file.WriteLine(header);
                    file.WriteLine("");
                    file.WriteLine(trajectory.Count);

                    for (int t = 0; t < trajectory.Count; t++)
                    {
                        string data = string.Format("{0} {1} {2} {3} {4} {5} {6} {7} {8} {9}",
                            "ts",
                            "fl",
                            trajectory[t].position.x,
                            trajectory[t].position.y,
                            trajectory[t].position.z,
                            trajectory[t].rotation.w,
                            trajectory[t].rotation.x,
                            trajectory[t].rotation.y,
                            trajectory[t].rotation.z,
                            "dist"
                            );

                        file.WriteLine(data);
                    }

                    Debug.Log("Cameras stored -> CAM");
                }
            }
            else
            {
                throw new NotSupportedException("Given format not supported");
            }
        }
    }
}
