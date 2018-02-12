using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

namespace WatchMeAlways
{
    public class VideoRecorder : MonoBehaviour, IRecorder
    {
        public class CppRecorder
        {
            public enum RecordingQuality
            {
                ULTRAFAST = 0,
                SUPERFAST,
                VERYFAST,
                FASTER,
                FAST,
                MEDIUM, // default
                SLOW,
                SLOWER,
                VERYSLOW,
            };

            [DllImport("WatchMeAlwaysLib")]
            public static extern int StartRecording(int width, int height, float maxSeconds, float fps, RecordingQuality quality);
            [DllImport("WatchMeAlwaysLib")]
            public static extern int AddFrame(byte[] pixels, int imgWidth, int imgHeight, float timeStamp);
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
            public int Width { get; private set; }
            public int Height { get; private set; }

            public Frame(byte[] pixels, int width, int height)
            {
                this.Pixels = pixels;
                this.Width = width;
                this.Height = height;
            }
        }

        public class DefaultRecordingParameters
        {
            public float ReplayLength { get; set; }
            public float Fps { get; set; }
            public CppRecorder.RecordingQuality Quality { get; set; }
        }

        public static DefaultRecordingParameters DefaultParameters = new DefaultRecordingParameters()
        {
            ReplayLength = 120.0f,
            Fps = 30.0f,
            Quality = CppRecorder.RecordingQuality.MEDIUM,
        };

        Queue<Frame> framesToEncode_ = new Queue<Frame>();
        State state_ = State.NotStarted;
        int frameCount_ = 0;
        int frameWidth_ = 0;
        int frameHeight_ = 0;
        System.Threading.Thread frameEncodeThread_ = null;
        Coroutine takeScreenshotCoroutine_ = null;
        Coroutine singleScreenshotCoroutine_ = null;
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

        public void StartRecording(IRecordingParameters parameters)
        {
            if (state_ != State.Running)
            {
                frameWidth_ = Screen.width / 2 * 2;
                frameHeight_ = Screen.height / 2 * 2;
                
                int res = CppRecorder.StartRecording(frameWidth_, frameHeight_, DefaultParameters.ReplayLength, DefaultParameters.Fps, DefaultParameters.Quality);
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
                stopSingleScreenshotCoroutine();
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
                    int res = CppRecorder.AddFrame(frame.Pixels, frame.Width, frame.Height, frameCount_++);
                    frameCount_++;
                    Debug.Log("AddFrame: " + (res == 0 ? "OK" : "NG"));
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
            takeScreenshotCoroutine_ = StartCoroutine(periodicalScreenshot());
        }

        void stopScreenshotCoroutine()
        {
            if (takeScreenshotCoroutine_ != null)
            {
                StopCoroutine(takeScreenshotCoroutine_);
            }
        }

        IEnumerator periodicalScreenshot()
        {
            while (true)
            {
                // TODO: FPS control
                yield return new WaitForEndOfFrame();

                if (frameWidth_ > 0 && frameHeight_ > 0)
                {
                    var tex = new Texture2D(Screen.width, Screen.height, TextureFormat.RGB24, false);

                    // bottle-neck!: down fps 90fps -> 65fps
                    tex.ReadPixels(new Rect(0, 0, tex.width, tex.height), 0, 0);
                    tex.Apply();

                    var bytes = tex.GetRawTextureData();
                    framesToEncode_.Enqueue(new Frame(bytes, tex.width, tex.height));
                }
            }
        }

        public void TakeScreenshot(string filepath)
        {
            startSingleScreenshotCoroutine(filepath);
        }

        // coroutine for taking screenshot
        void startSingleScreenshotCoroutine(string filepath)
        {
            singleScreenshotCoroutine_ = StartCoroutine(singleScreenshot(filepath));
        }

        void stopSingleScreenshotCoroutine()
        {
            if (singleScreenshotCoroutine_ != null)
            {
                StopCoroutine(singleScreenshotCoroutine_);
            }
        }

        IEnumerator singleScreenshot(string filepath)
        {
            yield return new WaitForEndOfFrame();
            int w = Screen.width / 2 * 2;
            int h = Screen.height / 2 * 2;
            var tex = new Texture2D(w, h, TextureFormat.RGB24, false);
            tex.ReadPixels(new Rect(0, 0, tex.width, tex.height), 0, 0);
            tex.Apply();
            System.IO.File.WriteAllBytes(filepath, tex.EncodeToPNG());
            Debug.Log("Saved screenshot in " + filepath);
        }
    }
}