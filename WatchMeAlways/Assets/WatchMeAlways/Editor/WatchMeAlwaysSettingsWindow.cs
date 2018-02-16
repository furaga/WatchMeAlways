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

        readonly DesktopRecorder.CppRecorder.RecordingQuality[] qualityPresets = new[] {
            DesktopRecorder.CppRecorder.RecordingQuality.FASTER,
            DesktopRecorder.CppRecorder.RecordingQuality.MEDIUM,
            DesktopRecorder.CppRecorder.RecordingQuality.SLOWER,
        };

        CaptureTarget monitor = CaptureTarget.EditorWindow;

        string[] getMonitorTexts()
        {
            int count = DesktopRecorder.CppRecorder.GetMonitorCount();
            List<string> texts = new List<string>();
            for (int i = 0; i < count; i++)
            {
                var monitor = DesktopRecorder.CppRecorder.GetMonitor(i);
                if (monitor != null)
                {
                    string t = string.Format(
                        "Monitor{0} ({1}x{2}{3})",
                        (i + 1),
                        monitor.Width,
                        monitor.Height,
                        monitor.IsPrimary ? ", PRIMARY" : "");
                    texts.Add(t);
                }
            }
            return texts.ToArray();
        }

        void OnGUI()
        {
            // restore current parameters
            var prevParams = WatchMeAlwaysMenuEditor.RecordingParameters as DesktopRecorder.RecordingParameters;
            int monitor = prevParams.Monitor;
            float recordLength = prevParams.RecordLength;
            float fps = prevParams.Fps;
            DesktopRecorder.CppRecorder.RecordingQuality quality = prevParams.Quality;

            // draw GUI
            GUILayout.Label("Recording Settings", EditorStyles.boldLabel);
            recordLength = EditorGUILayout.Slider("Record Length (seconds)", recordLength, 10, 300);
            monitor = EditorGUILayout.Popup("Monitor", monitor, getMonitorTexts());
            fps = EditorGUILayout.Slider("FPS", fps, 1, 120);
            int qualityIndex = EditorGUILayout.Popup("Quality", quality2index(quality), new string[] { "Low", "Medium", "High", });
            quality = index2quality(qualityIndex);

            // reset button
            if (GUILayout.Button("Reset"))
            {
                WatchMeAlwaysMenuEditor.RecordingParameters = DefaultParameters;
            }

            if (fps != prevParams.Fps ||
                recordLength != prevParams.RecordLength ||
                quality != prevParams.Quality)
            {
                WatchMeAlwaysMenuEditor.RecordingParameters = new DesktopRecorder.RecordingParameters()
                {
                    Monitor = monitor,
                    Fps = fps,
                    Quality = quality,
                    RecordLength = recordLength,
                };
            }
        }

        DesktopRecorder.RecordingParameters DefaultParameters
        {
            get
            {
                return new DesktopRecorder.RecordingParameters()
                {
                    Monitor = 0,
                    RecordLength = 120.0f,
                    Fps = 30.0f,
                    Quality = DesktopRecorder.CppRecorder.RecordingQuality.MEDIUM,
                };
            }
        }

        int quality2index(DesktopRecorder.CppRecorder.RecordingQuality quality)
        {
            int index = Array.IndexOf(qualityPresets, quality);
            return index;
        }

        DesktopRecorder.CppRecorder.RecordingQuality index2quality(int index)
        {
            if (0 <= index && index < qualityPresets.Length)
            {
                return qualityPresets[index];
            }
            return DesktopRecorder.CppRecorder.RecordingQuality.MEDIUM;
        }

    }
}
