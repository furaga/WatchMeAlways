using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

namespace WatchMeAlways
{
    public class InstantReplay : MonoBehaviour
    {
        class CppRecorder
        {
            [DllImport("WatchMeAlwaysLib")]
            public static extern int StartRecording(int width, int height);
            [DllImport("WatchMeAlwaysLib")]
            public static extern int AddFrame(byte[] pixels, float timeStamp, int imgWidth, int imgHeight);
            [DllImport("WatchMeAlwaysLib", CharSet = CharSet.Ansi)]
            public static extern int FinishRecording(string filepath);
        }

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

        Queue<Frame> framesToEncode_ = new Queue<Frame>();
        State state_ = State.NotStarted;
        int frameCount_ = 0;
        int frameWidth_ = 0;
        int frameHeight_ = 0;
        System.Threading.Thread frameEncodeThread_ = null;
        Coroutine takeScreenshotCoroutine_ = null;
        bool quitEncodeFramesIfQueueIsEmpty_ = false;

        void Start()
        {
            framesToEncode_ = new Queue<Frame>();
            state_ = State.NotStarted;
            frameCount_ = 0;
            frameWidth_ = 0;
            frameHeight_ = 0;
            frameEncodeThread_ = null;
            takeScreenshotCoroutine_ = null;
            quitEncodeFramesIfQueueIsEmpty_ = false;
        }

        public void StartRecording()
        {
            if (state_ == State.NotStarted)
            {
                frameWidth_ = Screen.width / 2 * 2;
                frameHeight_ = Screen.height / 2 * 2;

                int res = CppRecorder.StartRecording(frameWidth_, frameHeight_);
                state_ = State.Running;

                startScreenshotCoroutine();

                startFrameEncodeThread();
                frameCount_ = 0;

                Debug.Log("StartRecording: " + res);
            }
        }

        public void FinishRecording(string saveVideoPath)
        {
            if (state_ == State.Running)
            {
                state_ = State.Stopped;
                stopScreenshotCoroutine();
                stopFrameEncodeThread();
                int res = CppRecorder.FinishRecording(saveVideoPath);
                Debug.Log("FinishRecording: " + res);
            }
        }

        // thread for encoding
        void startFrameEncodeThread()
        {
            quitEncodeFramesIfQueueIsEmpty_ = false;
            frameEncodeThread_ = new System.Threading.Thread(new System.Threading.ThreadStart(EncodeFrames));
            frameEncodeThread_.Start();
        }

        void stopFrameEncodeThread()
        {
            if (frameEncodeThread_ != null)
            {
                // finish after flushing
                quitEncodeFramesIfQueueIsEmpty_ = true;
                frameEncodeThread_.Join(); 
            }
        }

        void EncodeFrames()
        {
            while (true)
            {
                if (framesToEncode_.Count >= 1)
                {
                    var frame = framesToEncode_.Dequeue();
                    int res = CppRecorder.AddFrame(frame.Pixels, frameCount_++, frameWidth_, frameHeight_);
                    frameCount_++;
                    Debug.Log("AddFrame: " + res);
                }
                else
                {
                    if (quitEncodeFramesIfQueueIsEmpty_)
                    {
                        break;
                    }
                    System.Threading.Thread.Sleep(100); // sleep 100ms if there is no frame to encode.
                }
            }
        }

        // coroutine for taking screenshot
        void startScreenshotCoroutine()
        {
            takeScreenshotCoroutine_ = StartCoroutine(PeriodicalScreenshot());
        }

        void stopScreenshotCoroutine()
        {
            if (takeScreenshotCoroutine_ != null)
            {
                StopCoroutine(takeScreenshotCoroutine_);
            }
        }

        IEnumerator PeriodicalScreenshot()
        {
            while (true)
            {
                // TODO: FPS control
//                yield return new WaitForEndOfFrame();
                yield return new WaitForSeconds(0.1f);

                if (frameWidth_ > 0 && frameHeight_ > 0)
                {
                    var tex = new Texture2D(frameWidth_, frameHeight_, TextureFormat.RGB24, false);

                    // bottle-nec!: down fps 90fps -> 65fps
                    tex.ReadPixels(new Rect(0, 0, frameWidth_, frameHeight_), 0, 0);
                    tex.Apply();

                    var bytes = tex.GetRawTextureData();
                    framesToEncode_.Enqueue(new Frame(bytes));
                    Debug.Log("# of bytes: " + bytes.Length);
                }
            }
        }

    }
}