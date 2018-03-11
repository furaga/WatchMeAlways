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
        internal static string LogFilePath { get; set; } = "watchmealways_server.log";
        internal static void Log(string msg)
        {
            if (!string.IsNullOrWhiteSpace(LogFilePath))
            {
                System.IO.File.AppendAllText(LogFilePath, "[" + DateTime.Now.ToString() + "]" + msg + "\n");
            }
        }
        internal static void LogErrorFormat(string msg, params object[] args)
        {
            if (!string.IsNullOrWhiteSpace(LogFilePath))
            {
                System.IO.File.AppendAllText(LogFilePath, "[" + DateTime.Now.ToString() + "]" + string.Format(msg + "\n", args));
            }
        }
    }

    class DesktopRecorder : Singleton<DesktopRecorder>
    {
        public class NativeRecorder
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

            [DllImport("WatchMeAlwaysLib", CharSet = CharSet.Ansi)]
            public static extern int SetLogPath(string filepath);

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

        public enum State
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
            public NativeRecorder.RecordingQuality Quality { get; set; }
        }

        Queue<Frame> framesToEncode_ = new Queue<Frame>();
        State state_ = State.NotStarted;
        int monitorNumber_ = 0;
        float fps_ = 0.0f;
        System.Threading.Thread captureThread_ = null;
        System.Threading.Thread encodeThread_ = null;

        public State RecordingState
        {
            get
            {
                return state_;
            }
        }
        public bool IsCaptureThreadWorking
        {
            get
            {
                return captureThread_.IsAlive;
            }
        }

        public bool IsEncodeThreadWorking
        {
            get
            {
                return captureThread_.IsAlive;
            }
        }

        bool quitCaptureThread_ = false;
        bool quitEncodeThread_ = false;

        System.Diagnostics.Stopwatch recordingTimer_ = new System.Diagnostics.Stopwatch();

        Queue<long> msList = new Queue<long>();

        void Initialize()
        {
            framesToEncode_ = new Queue<Frame>();
            state_ = State.NotStarted;
            encodeThread_ = null;
            quitCaptureThread_ = false;
            quitEncodeThread_ = false;
        }

        public void SetLogPath(string filepath)
        {
            Debug.LogFilePath = filepath;
            NativeRecorder.SetLogPath(filepath);
            return;
        }

        public void StartRecording(RecordingParameters parameters)
        {
            if (state_ != State.Running)
            {
                var param = parameters as RecordingParameters;
                var monitor = NativeRecorder.GetMonitor(param.Monitor);
                if (monitor == null || monitor.Width <= 0 || monitor.Height <= 0)
                {
                    Debug.LogErrorFormat("Failed to get monitor ({0}) informaiton", param.Monitor);
                    return;
                }

                monitorNumber_ = param.Monitor;

                int res = NativeRecorder.StartRecording(monitor.Width, monitor.Height, param.RecordLength, param.Fps, param.Quality);
                state_ = State.Running;

                recordingTimer_.Reset(); // need?
                recordingTimer_.Start();

                fps_ = param.Fps;
                msList.Clear();
                msList.Enqueue(recordingTimer_.ElapsedMilliseconds);

                startFrameCaptureThread();
                startFrameEncodeThread();
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
                int res = NativeRecorder.FinishRecording(saveVideoPath);
                Debug.Log("FinishRecording: " + res);
            }
        }

        // thread for recording
        void startFrameCaptureThread()
        {
            quitCaptureThread_ = false;
            captureThread_ = new System.Threading.Thread(new System.Threading.ThreadStart(capturing));
            captureThread_.Start();
        }

        void stopFrameCaptureThread()
        {
            if (captureThread_ != null)
            {
                quitCaptureThread_ = true;
                captureThread_.Join();
            }
        }

        // thread for encoding
        void startFrameEncodeThread()
        {
            quitEncodeThread_ = false;
            encodeThread_ = new System.Threading.Thread(new System.Threading.ThreadStart(encoding));
            encodeThread_.Start();
        }

        void stopFrameEncodeThread()
        {
            if (encodeThread_ != null)
            {
                // finish after flushing
                quitEncodeThread_ = true;
                encodeThread_.Join();
            }
        }

        float controlAndMeasureFPS()
        {
            long prevMs = msList.Last();

            // record current time
            msList.Enqueue(recordingTimer_.ElapsedMilliseconds);
            while (msList.Count >= 30)
            {
                msList.Dequeue();
            }

            // wait
            float totalMs = msList.Last() - msList.First();
            if (fps_ <= 0)
            {
                System.Threading.Thread.Sleep(1);
            }
            else
            {
                float totalTargetMs = 1000.0f / fps_ * (msList.Count - 1);
                if (totalTargetMs - totalMs > 0)
                {
                    System.Threading.Thread.Sleep((int)(totalTargetMs - totalMs));
                }
            }

            // measure fps
            float currentFPS = 0.0f;
            float dt = totalMs / (msList.Count - 1);
            if (dt > 0)
            {
                currentFPS = 1000.0f / dt;
            }
            if (msList.Last() / 1000 != prevMs / 1000)
            {
                // print every 10 seconds
                //               Console.WriteLine((msList.Last() / 1000) + ": FPS = " + currentFPS);
            }

            return currentFPS;
        }

        enum APIResult : int
        {
            OK = 0,
            NG = 1,
            Fatal = 2,
        }

        void capturing()
        {
            while (!quitCaptureThread_)
            {
                controlAndMeasureFPS();

                var monitor = NativeRecorder.GetMonitor(monitorNumber_); // todo:
                if (monitor == null)
                {
                    Debug.LogErrorFormat("Failed to get monitor ({0}) informaiton", monitorNumber_);
                    continue;
                }

                var frame = new NativeRecorder.Frame();
                int res = NativeRecorder.CaptureDesktopFrame(monitor.Rect, frame);
                if (res == (int)APIResult.Fatal)
                {
                    Debug.LogErrorFormat("Fatal error occurred when capturing desktop frame");
                    break;
                }
                else if (res != 0)
                {
                    Debug.LogErrorFormat("Failed to capture desktop frame");
                    continue;
                }

                long ms = recordingTimer_.ElapsedMilliseconds;
                framesToEncode_.Enqueue(new Frame(frame.Data, frame.Width, frame.Height, ms));
            }
        }

        void encoding()
        {
            while (!quitEncodeThread_)
            {
                if (framesToEncode_.Count >= 1)
                {
                    var frame = framesToEncode_.Dequeue();
                    int res = NativeRecorder.EncodeDesktopFrame(frame.Data, frame.TimeMilliSeconds * 0.001f);
                    if (res == (int)APIResult.Fatal)
                    {
                        Debug.LogErrorFormat("Fatal error occurred when encoding");
                        break;
                    }
                    else if (res != 0)
                    {
                        Debug.LogErrorFormat("Failed to encode");
                        continue;
                    }
                }
                else
                {
                    System.Threading.Thread.Sleep(100); // sleep 100ms if there is no frame to encode.
                }
            }

            // flush
            while (framesToEncode_.Count >= 1)
            {
                var frame = framesToEncode_.Dequeue();
                int res = NativeRecorder.EncodeDesktopFrame(frame.Data, frame.TimeMilliSeconds * 0.001f);
                if (res == (int)APIResult.Fatal)
                {
                    Debug.LogErrorFormat("Fatal error occurred when encoding (Flush)");
                    break;
                }
                else if (res != (int)APIResult.OK)
                {
                    Debug.LogErrorFormat("Failed to encode (Flush)");
                    continue;
                }
            }
        }
    }
}