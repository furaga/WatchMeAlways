using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEditor;
using System.Runtime.InteropServices;
using System.IO;
using System;

namespace WatchMeAlways
{
    public class WatchMeAlwaysSettingsWindow : EditorWindow
    {
        static bool modified = false;
        static InstantReplayConfig config_ = null;

        [MenuItem("WatchMeAlways/Settings", false, 110)]
        static void Open()
        {
            modified = false;
            config_ = InstantReplay.Instance.GetConfig();
            GetWindow<WatchMeAlwaysSettingsWindow>();
        }

        void OnEnable()
        {
            modified = false;
            config_ = InstantReplay.Instance.GetConfig();
        }

        void OnGUI()
        {
            if (config_ == null)
            {
                config_ = InstantReplayConfig.Create();
            }

            // reset button
            if (GUILayout.Button("Reset"))
            {
                config_ = InstantReplayConfig.Create();
                modified = true;
            }

            // draw GUI
            GUILayout.Label("Recording Settings", EditorStyles.boldLabel);
            bool autoStart = EditorGUILayout.Toggle("Start on load", config_.AutoStart);
            int monitor = EditorGUILayout.Popup("Monitor", config_.Monitor, monitorLabels());
            float replayLength = EditorGUILayout.IntSlider("ReplayLength (seconds)", (int)config_.ReplayLength, 5, 300);
            float fps = EditorGUILayout.IntSlider("FPS", (int)config_.Fps, 1, 60);
            var quality = idx2qty(EditorGUILayout.Popup("Quality", qty2idx(config_.Quality), qtxTexts()));

            if (
                autoStart != config_.AutoStart ||
                monitor != config_.Monitor ||
                fps != config_.Fps ||
                replayLength != config_.ReplayLength ||
                quality != config_.Quality)
            {
                // apply button
                config_.AutoStart = autoStart;
                config_.Monitor = monitor;
                config_.Fps = fps;
                config_.Quality = quality;
                config_.ReplayLength = replayLength;
                modified = true;
            }

            // apply button
            GUI.enabled = modified;
            if (GUILayout.Button("Apply"))
            {
                InstantReplay.Instance.ApplyConfig(config_);
                modified = false;
            }
            GUI.enabled = true;
        }

        string[] monitorLabels()
        {
            int count = NativeRecorder.GetMonitorCount();
            List<string> texts = new List<string>();
            for (int i = 0; i < count; i++)
            {
                var monitor = NativeRecorder.GetMonitor(i);
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


        class QtyLabel
        {
            public InstantReplay.RecordingQuality Qty { get; private set; }
            public string Label { get; private set; }
            public QtyLabel(InstantReplay.RecordingQuality q, string l)
            {
                Qty = q;
                Label = l;
            }
        }


        List<QtyLabel> qty2label = new List<QtyLabel>()
        {
            new QtyLabel(InstantReplay.RecordingQuality.FASTER, "Low"),
            new QtyLabel(InstantReplay.RecordingQuality.MEDIUM, "Medium (Default)"),
            new QtyLabel(InstantReplay.RecordingQuality.SLOWER, "High"),
        };

        string[] qtxTexts()
        {
            return qty2label.Select(ql => ql.Label).ToArray();
        }

        InstantReplay.RecordingQuality idx2qty(int index)
        {
            if (0 <= index && index < qty2label.Count)
            {
                return qty2label[index].Qty;
            }
            return InstantReplay.RecordingQuality.MEDIUM;
        }

        int qty2idx(InstantReplay.RecordingQuality q)
        {
            for (int i = 0; i < qty2label.Count; i++)
            {
                if (qty2label[i].Qty == q)
                {
                    return i;
                }
            }

            // search by default quality
            q = idx2qty(-1);
            for (int i = 0; i < qty2label.Count; i++)
            {
                if (qty2label[i].Qty == q)
                {
                    return i;
                }
            }

            return 0;
        }


        public class NativeRecorder
        {
            [StructLayout(LayoutKind.Sequential)]
            public class Monitor
            {
                public int Left = 0;
                public int Top = 0;
                public int Width = 0;
                public int Height = 0;
                public bool IsPrimary = false;
            };

            [DllImport("WatchMeAlwaysLib")]
            public static extern int GetMonitorCount();

            [DllImport("WatchMeAlwaysLib")]
            public static extern int GetMonitor(int n, [Out] Monitor monitor);

            public static Monitor GetMonitor(int n)
            {
                Monitor monitor = new Monitor();
                int err = GetMonitor(n, monitor);
                if (err != 0)
                {
                    return null;
                }
                return monitor;
            }
        }
    }
}
