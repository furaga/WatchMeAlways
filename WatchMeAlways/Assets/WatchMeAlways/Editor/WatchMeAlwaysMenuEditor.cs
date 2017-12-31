using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.Runtime.InteropServices;

namespace WatchMeAlways
{
    public class WatchMeAlwaysMenuEditor : MonoBehaviour
    {
        [DllImport("WatchMeAlways_Lib")]
        static extern float FooPluginFunction();

        [MenuItem("WatchMeAlways/GameObject/VideoRecorder", false, 10)]
        private static void CreateVideoRecorderObject(MenuCommand menuCommand)
        {
            var res = FooPluginFunction();
            Debug.Log("FooPluginFunctino() = " + res);

            //GameObject videoRecorderPrefab = PrefabUtility.InstantiatePrefab(Resources.Load("Prefabs/VideoRecorder")) as GameObject;
            //videoRecorderPrefab.name = "VideoRecorder";
            //PrefabUtility.DisconnectPrefabInstance(videoRecorderPrefab);
            //GameObjectUtility.SetParentAndAlign(videoRecorderPrefab, menuCommand.context as GameObject);
            //Undo.RegisterCreatedObjectUndo(videoRecorderPrefab, "Create " + videoRecorderPrefab.name);
            //Selection.activeObject = videoRecorderPrefab;
        }
    }
}