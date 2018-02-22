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
    [InitializeOnLoad]
    public class WatchMeAlwaysMenuEditor : MonoBehaviour
    {
        static WatchMeAlwaysMenuEditor()
        {
            // auto start on load
            var config = InstantReplay.Instance.GetConfig();
            if (config.AutoStart)
            {
                EnableVideoRecorder();
            }
        }

        [MenuItem("WatchMeAlways/Enable Instant Replay", false, 10)]
        private static void EnableVideoRecorder()
        {
            InstantReplay.Instance.Start();
        }

        [MenuItem("WatchMeAlways/Enable Instant Replay", true)]
        private static bool ValidateEnableVideoRecorder()
        {
            return !InstantReplay.Instance.IsRecording();
        }

        [MenuItem("WatchMeAlways/Disable Instant Replay", false, 10)]
        private static void DisableVideoRecorder()
        {
            InstantReplay.Instance.Stop();
        }

        [MenuItem("WatchMeAlways/Disable Instant Replay", true)]
        private static bool ValidateDisableVideoRecorder()
        {
            return InstantReplay.Instance.IsRecording();
        }

        [MenuItem("WatchMeAlways/Save Instant Replay %F10", false, 10)]
        private static void SaveVideoRecorder()
        {
            InstantReplay.Instance.Save();
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
            Utils.CreateDirectoryIfNotExists(InstantReplay.GalleryDicrectory);
            System.Diagnostics.Process.Start(InstantReplay.GalleryDicrectory);
            Logger.Info("Open gallery folder " + InstantReplay.GalleryDicrectory + " in Explorer");
        }
    }
}