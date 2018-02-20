﻿using System;
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
        }

        static bool finishedWatching_ = true;
        static Parameter param = new Parameter();

        static void Main(string[] args)
        {
            param = parseArguments(args);
            DesktopRecorder.Instance.StartRecording(param.RecordingParameters);

            System.IO.FileSystemWatcher watcher = null;
            startWatching(watcher);

            while (!finishedWatching_)
            {
                System.Threading.Thread.Sleep(500);
            }

            Console.WriteLine("Quit");
        }
        
        static Parameter parseArguments(string[] args)
        {
            // set default
            var param = new Parameter() {
                RecordingParameters = new DesktopRecorder.RecordingParameters()
                {
                    Monitor = 0,
                    Fps = 30.0f,
                    Quality = DesktopRecorder.CppRecorder.RecordingQuality.MEDIUM,
                    RecordLength = 120,
                },
                OutputPath = "movie.h264",
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
                                        if (System.IO.File.Exists(tokens[1]))
                                        {
                                            break;
                                        }
                                        DesktopRecorder.Instance.FinishRecording(tokens[1]);
                                        DesktopRecorder.Instance.StartRecording(param.RecordingParameters);
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
                        break;
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine(ex.ToString() + "\n" + ex.StackTrace);
            }
        }
    }
}