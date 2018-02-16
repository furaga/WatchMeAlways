using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

namespace WatchMeAlways
{
    public class DesktopRecorder : Singleton<DesktopRecorder>, IRecorder
    {
        internal class Frame
        {
            public int Data { get; private set; }
            public int Width { get; private set; }
            public int Height { get; private set; }
            public long TimeMilliSeconds { get; private set; }

            public Frame(int data, int width, int height, long time = 0)
            {
                this.Data = data;
                this.Width = width;
                this.Height = height;
                this.TimeMilliSeconds = time;
            }
        }

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

            [StructLayout(LayoutKind.Sequential)]
            public class Rect
            {
                public int Left = 0;
                public int Top = 0;
                public int Width = 0;
                public int Height = 0;
            };

            [StructLayout(LayoutKind.Sequential)]
            public class Monitor
            {
                public int Left = 0;
                public int Top = 0;
                public int Width = 0;
                public int Height = 0;
                public bool IsPrimary = false;
                public Rect Rect {
                    get
                    {
                        return new Rect
                        {
                            Left = Left,
                            Top = Top,
                            Width = Width,
                            Height = Height,
                        };
                    }
                }
            };


            [StructLayout(LayoutKind.Sequential)]
            public class Frame
            {
                public int Width = 0;
                public int Height = 0;
                public int Data = 0;
            }

            [DllImport("WatchMeAlwaysLib")]
            public static extern int StartRecording(int width, int height, float maxSeconds, float fps, RecordingQuality quality);

            [DllImport("WatchMeAlwaysLib", CharSet = CharSet.Ansi)]
            public static extern int FinishRecording(string filepath);

            [DllImport("WatchMeAlwaysLib", CallingConvention = CallingConvention.Cdecl)]
            public static extern int CaptureDesktop(Rect rect, [Out] Frame frame);

            [DllImport("WatchMeAlwaysLib")]
            public static extern int AddCapturedDesktopFrame(int data, float timeStamp);

            [DllImport("WatchMeAlwaysLib")]
            public static extern int GetMonitorCount();

            [DllImport("WatchMeAlwaysLib")]
            public static extern int GetMonitor(int n, [Out] Monitor monitor);
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
                frameWidth_ = 1920;// Screen.width / 2 * 2;
                frameHeight_ = 1080; // Screen.height / 2 * 2;

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


                string ffpmegPath = System.IO.Path.GetFullPath("./Assets/WatchMeAlways/Plugins/x86_64/ffmpeg.exe");
                System.Diagnostics.Process.Start(
                    ffpmegPath,
                    string.Format(
                        "-i {0} -c:v copy -f mp4 -y {1}",
                        saveVideoPath,
                        System.IO.Path.ChangeExtension(saveVideoPath, "mp4")
                    )
                );
                Debug.Log("FinishRecording: " + res);
            }
        }

        // thread for recording
        void startFrameCaptureThread()
        {
            captureThread_ = new System.Threading.Thread(new System.Threading.ThreadStart(capturing));
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
                    int monitorCount = CppRecorder.GetMonitorCount();
                    if (monitorCount <= 0)
                    {
                        continue;
                    }


                    var monitor = new CppRecorder.Monitor();
                    int err = CppRecorder.GetMonitor(monitorCount - 1, monitor); // todo:
                    if (err != 0) {
                        continue;
                    }

                    var frame = new CppRecorder.Frame();
                    err = CppRecorder.CaptureDesktop(monitor.Rect, frame);
                    if (err != 0)
                    {
                        continue;
                    }

                    framesToEncode_.Enqueue(new Frame(frame.Data, frame.Width, frame.Height, recordingTimer_.ElapsedMilliseconds));
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
                    int res = CppRecorder.AddCapturedDesktopFrame(frame.Data, frame.TimeMilliSeconds);
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
            //// capture image
            //var frame = CppRecorder.CaptureDesktopImage();
            //var tex = new Texture2D(frame.Width, frame.Height);

            //tex.LoadRawTextureData(frame.Bytes);

            //// encode to png
            //var pngData = tex.EncodeToPNG();

            //// save
            //filepath = System.IO.Path.ChangeExtension(filepath, "png");
            //System.IO.File.WriteAllBytes(filepath, pngData);
        }
    }
}