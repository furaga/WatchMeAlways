using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WatchMeAlwaysConsole
{
    class Program
    {
        static void Main(string[] args)
        {
            DesktopRecorder.Instance.StartRecording(new DesktopRecorder.RecordingParameters()
            {
                Monitor = 0,
                Fps = 30,
                Quality = DesktopRecorder.CppRecorder.RecordingQuality.MEDIUM,
                RecordLength = 120,
            });


            Console.WriteLine("Press Any key to stop");
            Console.ReadKey();

            DesktopRecorder.Instance.FinishRecording("desktop.h264");
            Console.WriteLine("Finished");
        }
    }
}
