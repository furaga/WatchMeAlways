// cf) https://stackoverflow.com/questions/43732825/use-debug-log-from-c

using AOT;
using System;
using System.Runtime.InteropServices;
using UnityEngine;

public class UnityDebugCpp : MonoBehaviour
{
    [DllImport("WatchMeAlwaysLib", CallingConvention = CallingConvention.Cdecl)]
    static extern void RegisterUnityPrintLogFn(UnityPrintLogFn cb);

    [MonoPInvokeCallback(typeof(UnityPrintLogFn))]
    static void OnDebugCallback(IntPtr request, int color, int size)
    {
        //Ptr to string
        string debug_string = Marshal.PtrToStringAnsi(request, size);

        //Add Specified Color
        debug_string =
            String.Format("{0}{1}{2}{3}{4}",
            "<color=",
            ((Color)color).ToString(),
            ">",
            debug_string,
            "</color>"
            );

        UnityEngine.Debug.Log(debug_string);
    }

    delegate void UnityPrintLogFn(IntPtr request, int color, int size);
    enum Color { red, green, blue, black, white, yellow, orange };
    void OnEnable()
    {
        RegisterUnityPrintLogFn(OnDebugCallback);
    }
}
