using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

namespace WatchMeAlways
{
    public class DesktopRecorder : IRecorder
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

            public class Frame
            {
                public byte[] Bytes;
                public int Width;
                public int Height;
            }

            [DllImport("WatchMeAlwaysLib")]
            public static extern int StartRecording(int width, int height, float maxSeconds, float fps, RecordingQuality quality);
            [DllImport("WatchMeAlwaysLib")]
            public static extern int AddFrame(byte[] pixels, int imgWidth, int imgHeight, float timeStamp);
            [DllImport("WatchMeAlwaysLib", CharSet = CharSet.Ansi)]
            public static extern int FinishRecording(string filepath);
            [DllImport("WatchMeAlwaysLib")]
            public static extern Frame CaptureDesktopImage();
        }

        public class RecordingParameters : IRecordingParameters
        {
            public float ReplayLength { get; set; }
            public float Fps { get; set; }
            public CppRecorder.RecordingQuality Quality { get; set; }
        }
        
        Queue<Frame> framesToEncode_ = new Queue<Frame>();
        State state_ = State.NotStarted;
        int frameCount_ = 0;
        int frameWidth_ = 0;
        int frameHeight_ = 0;
        System.Threading.Thread captureThread_ = null;
        System.Threading.Thread encodeThread_ = null;
        bool quitEncodeFramesIfQueueIsEmpty_ = false;
        System.Diagnostics.Stopwatch recordingTimer_ = new System.Diagnostics.Stopwatch();

        void Start()
        {
            framesToEncode_ = new Queue<Frame>();
            state_ = State.NotStarted;
            frameCount_ = 0;
            frameWidth_ = 0;
            frameHeight_ = 0;
            encodeThread_ = null;
            quitEncodeFramesIfQueueIsEmpty_ = false;
        }

        public void StartRecording(IRecordingParameters parameters)
        {
            if (state_ != State.Running)
            {
                frameWidth_ = Screen.width / 2 * 2;
                frameHeight_ = Screen.height / 2 * 2;

                var param = parameters as RecordingParameters;

                int res = CppRecorder.StartRecording(frameWidth_, frameHeight_, param.ReplayLength, param.Fps, param.Quality);
                state_ = State.Running;

                recordingTimer_.Reset(); // need?
                recordingTimer_.Start();

                startFrameCaptureThread();
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
                stopFrameCaptureThread();
                stopFrameEncodeThread();
                int res = CppRecorder.FinishRecording(saveVideoPath);
                Debug.Log("FinishRecording: " + res);
            }
        }

        // thread for recording
        void startFrameCaptureThread()
        {
            captureThread_ = new System.Threading.Thread(new System.Threading.ThreadStart(encoding));
            captureThread_.Start();
        }

        void stopFrameCaptureThread()
        {
            if (captureThread_ != null)
            {
                captureThread_.Abort();
            }
        }

        void capturing()
        {
            while (true)
            {
                // TODO: FPS control
                System.Threading.Thread.Sleep(10);

                if (frameWidth_ > 0 && frameHeight_ > 0)
                {
                    // bottle-neck!: take 20ms - 30ms
                    var frame = CppRecorder.CaptureDesktopImage();
                    framesToEncode_.Enqueue(new Frame(frame.Bytes, frame.Width, frame.Height, recordingTimer_.ElapsedMilliseconds));
                }
            }
        }

        // thread for encoding
        void startFrameEncodeThread()
        {
            quitEncodeFramesIfQueueIsEmpty_ = false;
            encodeThread_ = new System.Threading.Thread(new System.Threading.ThreadStart(encoding));
            encodeThread_.Start();
        }

        void stopFrameEncodeThread()
        {
            if (encodeThread_ != null)
            {
                // finish after flushing
                quitEncodeFramesIfQueueIsEmpty_ = true;
                encodeThread_.Join();
            }
        }

        void encoding()
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

        public void TakeScreenshot(string filepath)
        {
            // capture image
            var frame = CppRecorder.CaptureDesktopImage();
            var tex = new Texture2D(frame.Width, frame.Height);
            tex.LoadRawTextureData(frame.Bytes);

            // encode to png
            var pngData = tex.EncodeToPNG();

            // save
            filepath = System.IO.Path.ChangeExtension(filepath, "png");
            System.IO.File.WriteAllBytes(filepath, pngData);
        }
    }
}