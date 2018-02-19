using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections;
using System.Runtime.InteropServices;

namespace WatchMeAlwaysServer
{
    class Debug
    {
        internal static void Log(string msg)
        {
            // Console.WriteLine(msg);
        }
        internal static void LogErrorFormat(string msg, params object[] args)
        {
            // Console.WriteLine(string.Format(msg, args));
        }
    }

    class DesktopRecorder : Singleton<DesktopRecorder>
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
                public Rect Rect
                {
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
            public static extern int CaptureDesktopFrame(Rect rect, [Out] Frame frame);

            [DllImport("WatchMeAlwaysLib")]
            public static extern int EncodeDesktopFrame(int data, float timeStamp);

            [DllImport("WatchMeAlwaysLib")]
            public static extern int GetMonitorCount();

            [DllImport("WatchMeAlwaysLib")]
            public static extern int GetMonitor(int n, [Out] Monitor monitor);

            public static Monitor GetMonitor(int n)
            {
                Monitor monitor = new Monitor();
                int err = GetMonitor(n, monitor);
                if (err != 0)
                {
                    return null;
                }
                return monitor;
            }
        }

        class Frame
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

        enum State
        {
            NotStarted,
            Running,
            Stopped,
        }

        public class RecordingParameters
        {
            public int Monitor { get; set; }
            public float RecordLength { get; set; }
            public float Fps { get; set; }
            public CppRecorder.RecordingQuality Quality { get; set; }
        }

        Queue<Frame> framesToEncode_ = new Queue<Frame>();
        State state_ = State.NotStarted;
        int frameCount_ = 0;
        int monitorNumber_ = 0;
        System.Threading.Thread captureThread_ = null;
        System.Threading.Thread encodeThread_ = null;
        bool quitEncodeFramesIfQueueIsEmpty_ = false;
        System.Diagnostics.Stopwatch recordingTimer_ = new System.Diagnostics.Stopwatch();

        void Initialize()
        {
            framesToEncode_ = new Queue<Frame>();
            state_ = State.NotStarted;
            frameCount_ = 0;
            encodeThread_ = null;
            quitEncodeFramesIfQueueIsEmpty_ = false;
        }

        public void StartRecording(RecordingParameters parameters)
        {
            if (state_ != State.Running)
            {
                var param = parameters as RecordingParameters;
                var monitor = CppRecorder.GetMonitor(param.Monitor);
                if (monitor == null || monitor.Width <= 0 || monitor.Height <= 0)
                {
                    Debug.LogErrorFormat("Failed to get monitor ({0}) informaiton", param.Monitor);
                    return;
                }

                monitorNumber_ = param.Monitor;

                int res = CppRecorder.StartRecording(monitor.Width, monitor.Height, param.RecordLength, param.Fps, param.Quality);
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

                var monitor = CppRecorder.GetMonitor(monitorNumber_); // todo:
                if (monitor == null)
                {
                    Debug.LogErrorFormat("Failed to get monitor ({0}) informaiton", monitorNumber_);
                    continue;
                }

                var frame = new CppRecorder.Frame();
                int err = CppRecorder.CaptureDesktopFrame(monitor.Rect, frame);
                if (err != 0)
                {
                    Debug.LogErrorFormat("Failed to capture desktop frame");
                    continue;
                }

                framesToEncode_.Enqueue(new Frame(frame.Data, frame.Width, frame.Height, recordingTimer_.ElapsedMilliseconds));
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
                    int res = CppRecorder.EncodeDesktopFrame(frame.Data, frame.TimeMilliSeconds);
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
    }
}