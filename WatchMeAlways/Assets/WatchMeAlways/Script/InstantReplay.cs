using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;
using System.Runtime.InteropServices;

namespace WatchMeAlways
{
    public class InstantReplay : Singleton<InstantReplay>
    {
        readonly string serverPath = System.IO.Path.GetFullPath("./Assets/WatchMeAlways/Plugins/x86_64/WatchMeAlwaysServer.exe");
        readonly string ffmpegPath = System.IO.Path.GetFullPath("./Assets/WatchMeAlways/Plugins/x86_64/ffmpeg.exe");

        public static string GalleryDicrectory
        {
            // The full path of folder where recorded video will be saved.
            get
            {
                string documentRoot = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
                string path = System.IO.Path.Combine(documentRoot, "WatchMeAlways");
                path = System.IO.Path.Combine(path, "Gallery");
                return path;
            }
        }

        public static string TmpDicrectory
        {
            get
            {
                string documentRoot = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
                string path = System.IO.Path.Combine(documentRoot, "WatchMeAlways");
                path = System.IO.Path.Combine(path, "tmp");
                return path;
            }
        }

        public static string MessageFile
        {
            get
            {
                string path = System.IO.Path.Combine(TmpDicrectory, "msg.txt");
                return path;
            }
        }

        InstantReplayConfig config_ { get; set; }

        public void Start()
        {
            createDirectoryIfNotExists(TmpDicrectory);
            foreach (var f in System.IO.Directory.GetFiles(TmpDicrectory))
            {
                System.IO.File.Delete(f);
            }

            killAll(serverPath);

            string arg = "";
            if (config_ != null)
            {
                arg += " --monitor " + config_.Monitor;
                arg += " --length " + config_.ReplayLength;
                arg += " --fps " + config_.Fps;
                arg += " --quality " + config_.Quality.ToString();
            }
            arg += " --msgpath " + MessageFile;

            runServer(arg);
        }

        public void Stop()
        {
            killAll(serverPath);
        }

        public void Save()
        {
            createDirectoryIfNotExists(GalleryDicrectory);
            createDirectoryIfNotExists(TmpDicrectory);

            string basepath = System.IO.Path.Combine(GalleryDicrectory, DateTime.Now.ToString("yyyyMMdd-HHmmss"));
            string h264path = basepath + ".h264";
            sendMessage("@save " + h264path);

            string mp4path = basepath + ".mp4";
            runFFmpeg(string.Format("-i {0} -c:v copy -f mp4 -y {1}", h264path, mp4path));
        }

        bool createDirectoryIfNotExists(string path)
        {
            if (System.IO.Directory.Exists(path))
            {
                return false;
            }
            System.IO.Directory.CreateDirectory(path);
            return true;
        }

        public InstantReplayConfig GetConfig()
        {
            if (config_ == null)
            {
                config_ = InstantReplayConfig.Load();
            }

            var config = InstantReplayConfig.Create();
            if (config_ != null)
            {
                config.Monitor= config_.Monitor;
                config.ReplayLength = config_.ReplayLength;
                config.Fps = config_.Fps;
                config.Quality = config_.Quality;
            }
            return config;
        }

        public void ApplyConfig(InstantReplayConfig newConfig)
        {
            if (config_ == null)
            {
                config_ = InstantReplayConfig.Create();
            }

            if (newConfig != null)
            {
                config_.Monitor = newConfig.Monitor;
                config_.ReplayLength = newConfig.ReplayLength;
                config_.Fps = newConfig.Fps;
                config_.Quality = newConfig.Quality;
                config_.Save();

                // restart
                Stop();
                Start();
            }
        }


        public bool IsRecording()
        {
            return search(serverPath).Count >= 1;
        }

        public void GetMonitors()
        {
            List<Monitor> monitors = new List<Monitor>();
            int count = GetMonitorCount();
            for (int i = 0; i < count; i++)
            {
                var m = new Monitor();
                int err = GetMonitor(i, m);
                if (err != 0)
                {
                    Debug.LogWarning("Failed to get information of monitor " + i);
                    continue;
                }
                monitors.Add(m);
            }
        }

        void killAll(string path)
        {
            var processes = search(path);
            foreach (var p in processes)
            {
                kill(p);
            }
        }

        List<System.Diagnostics.Process> search(string path)
        {
            try
            {
                string processName = System.IO.Path.GetFileNameWithoutExtension(path).ToLower();
                var allProcesses = System.Diagnostics.Process.GetProcesses();
                var processes = allProcesses.Where(p =>
                {
                    return p.ProcessName.ToLower() == processName;
                }).ToList();
                return processes;
            }
            catch (Exception ex)
            {
                Debug.LogError("search: " + ex.ToString() + "\n" + ex.StackTrace);
            }
            return new List<System.Diagnostics.Process>();
        }

        System.Diagnostics.Process runServer(string arguments)
        {
            try
            {
                Debug.Log(arguments);
                var process = System.Diagnostics.Process.Start(serverPath, arguments);
                return process;
            }
            catch (Exception ex)
            {
                Debug.LogError("run: " + ex.ToString() + "\n" + ex.StackTrace);
            }
            return null;
        }

        System.Diagnostics.Process runFFmpeg(string arguments)
        {
            try
            {
                var startInfo = new System.Diagnostics.ProcessStartInfo(ffmpegPath)
                {
                    CreateNoWindow = true,
                    RedirectStandardInput = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                    Arguments = arguments,
                };

                var process = new System.Diagnostics.Process();
                process.StartInfo = startInfo;
                process.Start();
                hookProcessOutput(process);
                return process;
            }
            catch (Exception ex)
            {
                Debug.LogError("run: " + ex.ToString() + "\n" + ex.StackTrace);
            }

            return null;
        }

        void hookProcessOutput(System.Diagnostics.Process process)
        {
            process.OutputDataReceived += (obj, args) => Debug.Log(args.Data);
            process.ErrorDataReceived += (obj, args) => Debug.LogWarning(args.Data);
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
        }

        void kill(System.Diagnostics.Process process)
        {
            try
            {
                if (process != null && process.HasExited == false)
                {
                    process.Kill();
                }
            }
            catch (Exception ex)
            {
                Debug.LogError("kill: " + ex.ToString() + "\n" + ex.StackTrace);
            }
        }

        void sendMessage(string msg)
        {
            string tokenPath = System.IO.Path.Combine(TmpDicrectory, Guid.NewGuid().ToString());
            System.IO.File.WriteAllText(MessageFile, msg + " " + tokenPath);
            while (true)
            {
                bool ok = System.IO.File.Exists(tokenPath);
                if (ok) break;
                System.Threading.Thread.Sleep(1);
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        public class Monitor
        {
            public int Left = 0;
            public int Top = 0;
            public int Width = 0;
            public int Height = 0;
            public bool IsPrimary = false;
        };

        [DllImport("WatchMeAlwaysLib")]
        public static extern int GetMonitorCount();

        [DllImport("WatchMeAlwaysLib")]
        public static extern int GetMonitor(int n, [Out] Monitor monitor);

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
    }
}
