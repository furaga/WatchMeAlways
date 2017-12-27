using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace WatchMeAlways
{
    public class WatchMeAlwaysMenuEditor : MonoBehaviour
    {
        [MenuItem("WatchMeAlways/GameObject/VideoRecorder", false, 10)]
        private static void CreateVideoRecorderObject(MenuCommand menuCommand)
        {
            GameObject videoRecorderPrefab = PrefabUtility.InstantiatePrefab(Resources.Load("Prefabs/VideoRecorder")) as GameObject;
            videoRecorderPrefab.name = "VideoRecorder";
            PrefabUtility.DisconnectPrefabInstance(videoRecorderPrefab);
            GameObjectUtility.SetParentAndAlign(videoRecorderPrefab, menuCommand.context as GameObject);
            Undo.RegisterCreatedObjectUndo(videoRecorderPrefab, "Create " + videoRecorderPrefab.name);
            Selection.activeObject = videoRecorderPrefab;
        }
    }
}