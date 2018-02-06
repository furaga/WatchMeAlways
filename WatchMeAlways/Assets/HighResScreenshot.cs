using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System.Collections.Generic;

public class HighResScreenshot : MonoBehaviour
{
    [DllImport("WatchMeAlwaysLib")]
    static extern int StartRecording(int width, int height);
    [DllImport("WatchMeAlwaysLib")]
    static extern int AddFrame(byte[] pixels, float timeStamp, int imgWidth, int imgHeight);
    [DllImport("WatchMeAlwaysLib")]
    static extern int FinishRecording();

    enum State
    {
        NotStarted,
        Running,
        Stopped,
    }

    class Frame
    {
        public byte[] Pixels { get; private set; }
        public Frame(byte[] pixels)
        {
            this.Pixels = pixels;
        }
    }

    Queue<Frame> framesToEncode = new Queue<Frame>();
    State state = State.NotStarted;
    int frameCount = 0;
    int frameWidth_ = 0;
    int frameHeight_ = 0;

    void Start()
    {
        state = State.NotStarted;
        frameCount = 0;
    }

    void LateUpdate()
    {
        if (state == State.NotStarted)
        {
            frameWidth_ = Screen.width / 2 * 2;
            frameHeight_ = Screen.height / 2 * 2;

            int res = StartRecording(frameWidth_, frameHeight_);
            state = State.Running;
            startScreenshotCoroutine();
            startFrameEncodeThread();
            frameCount = 0;

            Debug.Log("StartRecording: " + res);
        }
    }

    void OnApplicationQuit()
    {
        if (state == State.Running)
        {
            state = State.Stopped;
            stopFrameEncodeThread();
            stopScreenshotCoroutine();
            int res = FinishRecording();
            Debug.Log("FinishRecording: " + res);
        }
    }

    // thread for encoding
    System.Threading.Thread frameEncodeThread = null;
    public void startFrameEncodeThread()
    {
        frameEncodeThread = new System.Threading.Thread(new System.Threading.ThreadStart(EncodeFrames));
    }

    public void stopFrameEncodeThread()
    {
        if (frameEncodeThread != null)
        {
            frameEncodeThread.Abort(); // how to stop thread?
        }
    }

    void EncodeFrames()
    {
        while (true)
        {
            if (framesToEncode.Count >= 1)
            {
                var frame = framesToEncode.Dequeue();
                int res = AddFrame(frame.Pixels, frameCount++, frameWidth_, frameHeight_);
                frameCount++;
                Debug.Log("AddFrame: " + res);
            }
            else
            {
                System.Threading.Thread.Sleep(100); // sleep 100ms if there is no frame to encode.
            }
        }
    }

    // coroutine for taking screenshot
    Coroutine takeScreenshotCoroutine = null;
    public void startScreenshotCoroutine()
    {
        takeScreenshotCoroutine = StartCoroutine(TakeScreenShots());
    }

    public void stopScreenshotCoroutine()
    {
        if (takeScreenshotCoroutine != null)
        {
            StopCoroutine(takeScreenshotCoroutine);
        }
    }

    IEnumerator TakeScreenShots()
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
                framesToEncode.Enqueue(new Frame(bytes));
                Debug.Log("# of bytes: " + bytes.Length);
            }
        }
    }


}