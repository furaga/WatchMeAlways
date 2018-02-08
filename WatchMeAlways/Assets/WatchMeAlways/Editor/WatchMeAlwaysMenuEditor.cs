using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.Runtime.InteropServices;
using System.IO;
using System;

namespace WatchMeAlways
{
    public class WatchMeAlwaysMenuEditor : MonoBehaviour
    {
        static bool enableInstantReplay_ = false;
        static bool recording_ = false;

        // SaveDir is fullpath
        static string SaveDir
        {
            get
            {
                string documentRoot = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
                string path = System.IO.Path.Combine(documentRoot, "WatchMeAlways");
                path = System.IO.Path.Combine(path, "Gallery");
                return path;
            }
        }

        // createDirectoryIfNotExists returns if the directory was created or not.
        static bool createDirectoryIfNotExists(string path)
        {
            if (System.IO.Directory.Exists(path))
            {
                return false;
            }
            System.IO.Directory.CreateDirectory(path);
            return true;
        }

        //
        // Instant Replay
        //

        [MenuItem("WatchMeAlways/Save Instant Replay %F10", false, 10)]
        private static void SaveInstantReplay()
        {
            Debug.Log("SaveInstantReplay");
        }

        [MenuItem("WatchMeAlways/Save Instant Replay %F10", true, 10)]
        private static bool ValidateSaveInstantReplay()
        {
            return enableInstantReplay_;
        }

        [MenuItem("WatchMeAlways/Enable Instant Replay", false, 10)]
        private static void EnableInstantReplay()
        {
            enableInstantReplay_ = true;
            Debug.Log("Instant Replay: ON");
        }

        [MenuItem("WatchMeAlways/Enable Instant Replay", true)]
        private static bool ValidateEnableInstantReplay()
        {
            return !enableInstantReplay_;
        }

        [MenuItem("WatchMeAlways/Disable Instant Replay", false, 10)]
        private static void DisableInstantReplay()
        {
            enableInstantReplay_ = false;
            Debug.Log("Instant Replay: OFF");
        }

        [MenuItem("WatchMeAlways/Disable Instant Replay", true)]
        private static bool ValidateDisableInstantReplay()
        {
            return enableInstantReplay_;
        }

        //
        // Recording
        //

        [MenuItem("WatchMeAlways/Start Recording %F9", false, 50)]
        private static void StartRecording()
        {
            recording_ = true;
            Debug.Log("Recording: STARTED");
        }

        [MenuItem("WatchMeAlways/Start Recording %F9", true)]
        private static bool ValidateStartRecording()
        {
            return !recording_;
        }

        [MenuItem("WatchMeAlways/Finish Recording %F9", false, 50)]
        private static void FinishRecording()
        {
            recording_ = false;
            Debug.Log("Recording: FINISHED. The recorded video was saved in " + SaveDir);
        }

        [MenuItem("WatchMeAlways/Finish Recording %F9", true)]
        private static bool ValidateFinishRecording()
        {
            return recording_;
        }

        //
        // Screenshot
        //

        [MenuItem("WatchMeAlways/Take Screenshot %F1", false, 70)]
        private static void TakeScreenshot()
        {
            Debug.Log("Screenshot: a new screenshot was saved in " + SaveDir);
        }

        [MenuItem("WatchMeAlways/Open Gallery in Explorer", false, 90)]
        private static void OpenGalleryInExplorer()
        {
            createDirectoryIfNotExists(SaveDir);
            System.Diagnostics.Process.Start(SaveDir);
            Debug.Log("Open gallery folder " + SaveDir + " in Explorer");
        }

        [MenuItem("WatchMeAlways/Config", false, 110)]
        private static void Config()
        {
            EditorWindow.GetWindow(typeof(WatchMeAlwaysConfigWindow));
        }
    }
}