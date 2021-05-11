using System;
using System.IO;
using UnityEngine;

namespace Reconstruction4D
{
    class FileLogger : MonoBehaviour
    {
        public FileLogger()
        {

        }

        public static void AppendText(string filepath, string text)
        {
            try
            {
                Directory.CreateDirectory(Path.GetDirectoryName(filepath));
                File.AppendAllText(filepath, text + "\n");
            }
            catch (Exception e)
            {
                Debug.Log("Exception: " + e.Message);
            }
        }
    }
}