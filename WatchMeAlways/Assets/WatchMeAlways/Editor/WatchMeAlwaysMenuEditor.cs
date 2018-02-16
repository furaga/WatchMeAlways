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
        static bool enableVideoRecorder_ = false;
        static bool recording_ = false;

        public static IRecordingParameters RecordingParameters = new DesktopRecorder.RecordingParameters()
        {
            RecordLength = 120.0f,
            Fps = 30.0f,
            Quality = DesktopRecorder.CppRecorder.RecordingQuality.MEDIUM,
        };

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
        [MenuItem("WatchMeAlways/Enable Instant Replay", false, 10)]
        private static void EnableVideoRecorder(MenuCommand menuCommand)
        {
            enableVideoRecorder_ = true;
            DesktopRecorder.Instance.StartRecording(RecordingParameters);
            Debug.Log("Instant Replay: ON");
        }

        [MenuItem("WatchMeAlways/Enable Instant Replay", true)]
        private static bool ValidateEnableVideoRecorder()
        {
            return !enableVideoRecorder_;
        }

        [MenuItem("WatchMeAlways/Disable Instant Replay", false, 10)]
        private static void DisableVideoRecorder(MenuCommand menuCommand)
        {
            enableVideoRecorder_ = false;
            DesktopRecorder.Instance.FinishRecording(System.IO.Path.Combine(SaveDir, "video.h264"));
            Debug.Log("Instant Replay: OFF");
        }

        [MenuItem("WatchMeAlways/Disable Instant Replay", true)]
        private static bool ValidateDisableVideoRecorder()
        {
            return enableVideoRecorder_;
        }

        [MenuItem("WatchMeAlways/Save Instant Replay %F10", false, 10)]
        private static void SaveVideoRecorder(MenuCommand menuCommand)
        {
            DesktopRecorder.Instance.FinishRecording(System.IO.Path.Combine(SaveDir, "video.h264"));
            DesktopRecorder.Instance.StartRecording(RecordingParameters);
            Debug.Log("Instant Replay: OFF");
        }

        [MenuItem("WatchMeAlways/Save Instant Replay %F10", true, 10)]
        private static bool ValidateSaveVideoRecorder()
        {
            return enableVideoRecorder_;
        }

        //
        // Recording
        //

        [MenuItem("WatchMeAlways/Start Recording %F9", false, 50)]
        private static void StartRecording(MenuCommand menuCommand)
        {
            recording_ = true;
            DesktopRecorder.Instance.StartRecording(RecordingParameters);
            Debug.Log("Recording: STARTED");

        }

        [MenuItem("WatchMeAlways/Start Recording %F9", true)]
        private static bool ValidateStartRecording()
        {
            return !recording_;
        }

        [MenuItem("WatchMeAlways/Finish Recording %#F9", false, 50)]
        private static void FinishRecording(MenuCommand menuCommand)
        {
            recording_ = false;
            DesktopRecorder.Instance.FinishRecording(System.IO.Path.Combine(SaveDir, "video.h264"));
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

        // [MenuItem("WatchMeAlways/Take Screenshot %F1", false, 70)]
        private static void TakeScreenshot(MenuCommand menuCommand)
        {
            if (DesktopRecorder.Instance != null)
            {
                DesktopRecorder.Instance.TakeScreenshot(System.IO.Path.Combine(SaveDir, "screenshot.png"));
                Debug.Log("Screenshot: a new screenshot was saved in " + SaveDir);
            }
        }

        //
        // Explorer
        // 
        [MenuItem("WatchMeAlways/Open Gallery in Explorer", false, 90)]
        private static void OpenGalleryInExplorer()
        {
            createDirectoryIfNotExists(SaveDir);
            System.Diagnostics.Process.Start(SaveDir);
            Debug.Log("Open gallery folder " + SaveDir + " in Explorer");
        }

        //
        // Config
        // 
        [MenuItem("WatchMeAlways/Settings", false, 110)]
        private static void Settings()
        {
            EditorWindow.GetWindow(typeof(WatchMeAlwaysSettingsWindow));
        }

    }
}