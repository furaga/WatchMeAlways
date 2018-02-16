using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace WatchMeAlways
{
    public class PrintTime : MonoBehaviour
    {
        System.Diagnostics.Stopwatch stopWatch = new System.Diagnostics.Stopwatch();

        // Use this for initialization
        void Start()
        {
            stopWatch.Start();
        }

        // Update is called once per frame
        void Update()
        {

        }

        void OnGUI()
        {
            GUI.Label(new Rect(20, 20, 300, 40), string.Format("経過時間: {0:   0.00}秒", stopWatch.Elapsed.TotalSeconds));
        }
    }
}