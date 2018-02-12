using System;
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

        public Frame(byte[] pixels, int width, int height)
        {
            this.Pixels = pixels;
            this.Width = width;
            this.Height = height;
        }
    }

    public class IRecordingParameters
    {
    }

    interface IRecorder
    {
        void StartRecording(IRecordingParameters parameters);
        void FinishRecording(string filepath);
        void EncodeFrames();
        void TakeScreenshot(string filepath);
    }
}
