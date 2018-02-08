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
        enum CaptureTarget: int
        {
            GamePanel = 0,
            EditorWindow,
            Desktop,
        }

        enum Quality : int
        {
            Low = 0, Medium, High, Custom,
        }
        
        int replaySeconds = 60;
        CaptureTarget captureTarget = CaptureTarget.EditorWindow;
        Quality quality = Quality.Low;
        float fps = 30;
        int mbps= 22;

        void OnGUI()
        {
            // Instant Replay Settings
            GUILayout.Label("Instant Replay Settings", EditorStyles.boldLabel);
            replaySeconds = EditorGUILayout.IntSlider("Replay Length (seconds)", replaySeconds, 10, 300);

            // Recording Settings
            GUILayout.Label("Recording Settings", EditorStyles.boldLabel);
            captureTarget = (CaptureTarget)EditorGUILayout.Popup("Capture Target", (int)captureTarget, new string[] {
                "\"Game\" Panel",
                "Unity Editor Window",
                "Desktop",
            });

            quality = (Quality)EditorGUILayout.Popup("Quality", (int)quality, new string[] {
                "Low (30FPS, 15Mbps)",
                "Medium (30FPS, 22Mbps)",
                "High (30FPS, 50Mbps)",
                "Custom",
            });

            switch (quality)
            {
                case Quality.Low:
                    fps = 30;
                    mbps = 15;
                    break;
                case Quality.High:
                    fps = 30;
                    mbps = 50;
                    break;
                case Quality.Medium:
                    fps = 30;
                    mbps = 22;
                    break;
                case Quality.Custom:
                    break;
                default:
                    Debug.LogErrorFormat("Unexpected quality type: {0}", quality);
                    break;
            }

            customQualityGUI(quality == Quality.Custom);
        }

        void customQualityGUI(bool isCustomMode)
        {
            EditorGUI.BeginDisabledGroup(false == isCustomMode);
            fps = EditorGUILayout.Slider("FPS", fps, 0.1f, 120.0f);
            mbps = EditorGUILayout.IntSlider("Bit Rate (Mbps)", mbps, 10, 130);
            EditorGUI.EndDisabledGroup();
        }
    }
}
