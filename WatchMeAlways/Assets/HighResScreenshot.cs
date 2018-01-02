using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

public class HighResScreenshot : MonoBehaviour
{
    [DllImport("WatchMeAlwaysLib")]
    static extern int StartRecording(int width, int height);
    [DllImport("WatchMeAlwaysLib")]
    static extern int AddFrame(byte[] pixels, float timeStamp, int lineSize);
    [DllImport("WatchMeAlwaysLib")]
    static extern int FinishRecording();

    enum State
    {
        NotStarted,
        Running,
        Stopped,
    }

    State state = State.NotStarted;
    int count = 0;

    void Start()
    {
        state = State.NotStarted;
        count = 0;
    }

    int frameWidth_ = 0;
    int frameHeight_ = 0;

    void LateUpdate()
    {
        if (state == State.NotStarted)
        {
            frameWidth_ = Screen.width / 2 * 2;
            frameHeight_ = Screen.height / 2 * 2;
            int res = StartRecording(frameWidth_, frameHeight_);
            Debug.Log("StartRecording: " + res);
            state = State.Running;
            myScreenShot();
            count = 0;
        }

        //if (state == State.Running && count >= 25)
        //{
        //    state = State.Stopped;
        //    if (takeScreenshotCoroutine != null)
        //    {
        //        StopCoroutine(takeScreenshotCoroutine);
        //    }
        //    int res = FinishRecording();
        //    Debug.Log("FinishRecording: " + res);
        //}
    }
    void OnApplicationQuit()
    {
        if (state == State.Running)
        {
            state = State.Stopped;
            if (takeScreenshotCoroutine != null)
            {
                StopCoroutine(takeScreenshotCoroutine);
            }
            int res = FinishRecording();
            Debug.Log("FinishRecording: " + res);
        }
    }

    Coroutine takeScreenshotCoroutine = null;
    public void myScreenShot()
    {
        takeScreenshotCoroutine = StartCoroutine(TakeScreenShot());
    }

    IEnumerator TakeScreenShot()
    {
        while (true)
        {
            yield return new WaitForEndOfFrame();

            if (frameWidth_ > 0 && frameHeight_ > 0)
            {
                var tex = new Texture2D(frameWidth_, frameHeight_, TextureFormat.RGB24, false);

                tex.ReadPixels(new Rect(0, 0, frameWidth_, frameHeight_), 0, 0);
                tex.Apply();

                var bytes = tex.GetRawTextureData();
                Debug.Log("# of bytes: " + bytes.Length);
                int res = AddFrame(bytes, count++, frameWidth_ * 3 /* REALLY? */);
                Debug.Log("AddFrame: " + res);
                count++;
            }
        }
    }
}