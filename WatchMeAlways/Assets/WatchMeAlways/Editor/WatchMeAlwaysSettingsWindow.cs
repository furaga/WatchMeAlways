using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.Runtime.InteropServices;
using System.IO;
using System;

namespace WatchMeAlways
{
    public class WatchMeAlwaysSettingsWindow : EditorWindow
    {
        enum CaptureTarget : int
        {
            GamePanel = 0,
            EditorWindow,
            Desktop,
        }

        readonly VideoRecorder.CppRecorder.RecordingQuality[] qualityPresets = new[] {
            VideoRecorder.CppRecorder.RecordingQuality.FASTER,
            VideoRecorder.CppRecorder.RecordingQuality.MEDIUM,
            VideoRecorder.CppRecorder.RecordingQuality.SLOWER,
        };

        CaptureTarget captureTarget = CaptureTarget.EditorWindow;
        void OnGUI()
        {
            var oldParams = VideoRecorder.DefaultParameters;
            float fps = oldParams.Fps;
            float replaySeconds = oldParams.ReplayLength;
            VideoRecorder.CppRecorder.RecordingQuality quality = oldParams.Quality;

            GUILayout.Label("Instant Replay Settings", EditorStyles.boldLabel);

            // replay length
            replaySeconds = EditorGUILayout.Slider("Replay Length (seconds)", replaySeconds, 10, 300);

            GUILayout.Label("Recording Settings", EditorStyles.boldLabel);

            // target
            captureTarget = (CaptureTarget)EditorGUILayout.Popup("Capture Target", (int)captureTarget, new string[] {
                "\"Game\" Panel",
                "Unity Editor Window",
                "Desktop",
            });

            // fps
            fps = EditorGUILayout.Slider("FPS", fps, 1, 120);

            // quality
            int index = EditorGUILayout.Popup("Quality", quality2index(quality), new string[] { "Low", "Medium", "High", });
            quality = index2quality(index);


            if (GUILayout.Button("Reset"))
            {
                VideoRecorder.DefaultParameters = new VideoRecorder.DefaultRecordingParameters()
                {
                    ReplayLength = 120.0f,
                    Fps = 30.0f,
                    Quality = VideoRecorder.CppRecorder.RecordingQuality.MEDIUM,
                };
            }

            if (fps != oldParams.Fps ||
                replaySeconds != oldParams.ReplayLength ||
                quality != oldParams.Quality)
            {
                VideoRecorder.DefaultParameters = new VideoRecorder.DefaultRecordingParameters()
                {
                    Fps = fps,
                    Quality = quality,
                    ReplayLength = replaySeconds,
                };
            }
        }

        int quality2index(VideoRecorder.CppRecorder.RecordingQuality quality)
        {
            int index = Array.IndexOf(qualityPresets, quality);
            return index;
        }

        VideoRecorder.CppRecorder.RecordingQuality index2quality(int index)
        {
            if (0 <= index && index < qualityPresets.Length)
            {
                return qualityPresets[index];
            }
            return VideoRecorder.CppRecorder.RecordingQuality.MEDIUM;
        }

    }
}
