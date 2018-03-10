using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WatchMeAlwaysServer
{
    class Program
    {
        class Parameter
        {
            public DesktopRecorder.RecordingParameters RecordingParameters = new DesktopRecorder.RecordingParameters();
            public string MessagePath = "msg.txt";
            public string OutputPath = "";
            public int ParentProcessId = -1;
        }

        static bool finishedWatching_ = true;
        static Parameter param = new Parameter();

        static void Main(string[] args)
        {
            param = parseArguments(args);
            DesktopRecorder.Instance.StartRecording(param.RecordingParameters);

            System.IO.FileSystemWatcher watcher = null;
            startWatching(watcher);

            System.Diagnostics.Process parent = null;
            if (param.ParentProcessId > 0)
            {
                parent = System.Diagnostics.Process.GetProcessById(param.ParentProcessId);
            }
            
            while (!finishedWatching_)
            {
                System.Threading.Thread.Sleep(500);

                bool isRunning = DesktopRecorder.Instance.RecordingState == DesktopRecorder.State.Running;
                bool allThreadWorking = DesktopRecorder.Instance.IsEncodeThreadWorking && DesktopRecorder.Instance.IsCaptureThreadWorking;
                if (isRunning && !allThreadWorking)
                {
                    Console.Error.WriteLine("Fatal error occurred in a thread of recorder. Try to restart");
                    DesktopRecorder.Instance.FinishRecording("");
                    DesktopRecorder.Instance.StartRecording(param.RecordingParameters);
                }

                // if parent process is dead, this process will die.
                if (parent != null && parent.HasExited)
                {
                    DesktopRecorder.Instance.FinishRecording("");
                    break;
                }
            }
        }

        static Parameter parseArguments(string[] args)
        {
            // set default
            var param = new Parameter()
            {
                RecordingParameters = new DesktopRecorder.RecordingParameters()
                {
                    Monitor = 0,
                    Fps = 30.0f,
                    Quality = DesktopRecorder.CppRecorder.RecordingQuality.MEDIUM,
                    RecordLength = 120,
                },
                OutputPath = "movie.h264",
                ParentProcessId = -1,
            };

            // parse
            for (int i = 0; i < args.Length - 1; i++)
            {
                switch (args[i].ToLower())
                {
                    case "--monitor":
                        param.RecordingParameters.Monitor = int.Parse(args[i + 1]);
                        break;
                    case "--length":
                        param.RecordingParameters.RecordLength = float.Parse(args[i + 1]);
                        break;
                    case "--fps":
                        param.RecordingParameters.Fps = float.Parse(args[i + 1]);
                        break;
                    case "--quality":
                        var t = typeof(DesktopRecorder.CppRecorder.RecordingQuality);
                        param.RecordingParameters.Quality = (DesktopRecorder.CppRecorder.RecordingQuality)Enum.Parse(t, args[i + 1]);
                        break;
                    case "--msgpath":
                        param.MessagePath = args[i + 1];
                        break;
                    case "--parentpid":
                        param.ParentProcessId = int.Parse(args[i + 1]);
                        break;
                }
            }

            param.MessagePath = System.IO.Path.GetFullPath(param.MessagePath);
            return param;
        }

        static void startWatching(System.IO.FileSystemWatcher watcher)
        {
            if (watcher != null) return;
            if (string.IsNullOrWhiteSpace(param.MessagePath)) return;

            watcher = new System.IO.FileSystemWatcher();
            watcher.Path = System.IO.Path.GetDirectoryName(param.MessagePath);
            watcher.NotifyFilter = System.IO.NotifyFilters.LastWrite;
            watcher.Filter = System.IO.Path.GetFileName(param.MessagePath);

            watcher.Changed += new System.IO.FileSystemEventHandler(watcher_Changed);
            watcher.Created += new System.IO.FileSystemEventHandler(watcher_Changed);

            watcher.EnableRaisingEvents = true;
            finishedWatching_ = false;
            Console.Error.WriteLine("Start watching: " + param.MessagePath);
        }

        static void watcher_Changed(System.Object source, System.IO.FileSystemEventArgs e)
        {
            try
            {
                switch (e.ChangeType)
                {
                    case System.IO.WatcherChangeTypes.Changed:
                        try
                        {
                            string line = System.IO.File.ReadAllText(param.MessagePath);
                            if (line == null)
                            {
                                break;
                            }
                            var tokens = line.Split(' ').Where(t => string.IsNullOrWhiteSpace(t) == false).ToArray();
                            if (tokens.Length >= 0 && tokens[0].Contains('@'))
                            {
                                switch (tokens[0].Substring(tokens[0].IndexOf('@')))
                                {
                                    case "@save":
                                        if (tokens.Length >= 3)
                                        {
                                            if (false == save(tokens[1]))
                                            {
                                                break;
                                            }
                                            Console.Error.WriteLine("Saved in " + tokens[1]);
                                            Console.Error.WriteLine();
                                            System.IO.File.WriteAllText(tokens[2], "");
                                        }
                                        break;
                                    case "@quit":
                                        finishedWatching_ = true;
                                        break;
                                }
                            }
                        }
                        catch (Exception ex)
                        {
                            Console.Error.WriteLine(ex.ToString() + "\n" + ex.StackTrace);
                        }
                        break;
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine(ex.ToString() + "\n" + ex.StackTrace);
            }
        }

        static bool save(string videopath)
        {
            if (System.IO.File.Exists(videopath))
            {
                return false;
            }
            DesktopRecorder.Instance.FinishRecording(videopath);
            DesktopRecorder.Instance.StartRecording(param.RecordingParameters);
            return true;
        }
    }
}