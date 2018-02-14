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
