using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.Runtime.InteropServices;
using System.IO;
using System;
using System.Linq;
using System.Text;

namespace WatchMeAlways
{
    public class WatchMeAlwaysMenuEditor : MonoBehaviour
    {
        //
        // Instant Replay
        //
        [MenuItem("WatchMeAlways/Enable Instant Replay", false, 10)]
        private static void EnableVideoRecorder(MenuCommand menuCommand)
        {
            InstantReplay.Instance.Start();
            Debug.Log("Instant Replay: ON");
        }

        [MenuItem("WatchMeAlways/Enable Instant Replay", true)]
        private static bool ValidateEnableVideoRecorder()
        {
            return !InstantReplay.Instance.IsRecording();
        }

        [MenuItem("WatchMeAlways/Disable Instant Replay", false, 10)]
        private static void DisableVideoRecorder(MenuCommand menuCommand)
        {
            InstantReplay.Instance.Stop();
            Debug.Log("Instant Replay: OFF");
        }

        [MenuItem("WatchMeAlways/Disable Instant Replay", true)]
        private static bool ValidateDisableVideoRecorder()
        {
            return InstantReplay.Instance.IsRecording();
        }

        [MenuItem("WatchMeAlways/Save Instant Replay %F10", false, 10)]
        private static void SaveVideoRecorder(MenuCommand menuCommand)
        {
            InstantReplay.Instance.Save();
            Debug.Log("Instant Replay: OFF");
        }

        [MenuItem("WatchMeAlways/Save Instant Replay %F10", true, 10)]
        private static bool ValidateSaveVideoRecorder()
        {
            return InstantReplay.Instance.IsRecording();
        }

        //
        // Explorer
        // 
        [MenuItem("WatchMeAlways/Open Gallery in Explorer", false, 90)]
        private static void OpenGalleryInExplorer()
        {
            createDirectoryIfNotExists(InstantReplay.GalleryDicrectory);
            System.Diagnostics.Process.Start(InstantReplay.GalleryDicrectory);
            Debug.Log("Open gallery folder " + InstantReplay.GalleryDicrectory + " in Explorer");
        }

        static bool createDirectoryIfNotExists(string path)
        {
            if (System.IO.Directory.Exists(path))
            {
                return false;
            }
            System.IO.Directory.CreateDirectory(path);
            return true;
        }
        
    }
}