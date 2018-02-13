﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WatchMeAlways
{
    internal enum State
    {
        NotStarted,
        Running,
        Stopped,
    }

    internal class Frame
    {
        public byte[] Pixels { get; private set; }
        public int Width { get; private set; }
        public int Height { get; private set; }
        public long TimeMilliSeconds { get; private set; }

        public Frame(byte[] pixels, int width, int height, long time = 0)
        {
            this.Pixels = pixels;
            this.Width = width;
            this.Height = height;
            this.TimeMilliSeconds = time;
        }
    }

    public class IRecordingParameters
    {
    }

    public interface IRecorder
    {
        void StartRecording(IRecordingParameters parameters);
        void FinishRecording(string filepath);
        void TakeScreenshot(string filepath);
    }
}