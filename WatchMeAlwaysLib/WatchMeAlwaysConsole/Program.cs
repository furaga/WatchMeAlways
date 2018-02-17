using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WatchMeAlwaysConsole
{
    class Program
    {
        class Parameter
        {
            public DesktopRecorder.RecordingParameters RecordingParameters = new DesktopRecorder.RecordingParameters();
            public string OutputPath = "";
        }

       static Parameter parseArguments(string[] args)
        {
            // set default
            var param = new Parameter() {
                RecordingParameters = new DesktopRecorder.RecordingParameters()
                {
                    Monitor = 0,
                    Fps = 30,
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
                    case "--output":
                        param.OutputPath= args[i + 1];
                        break;
                }
            }

            return param;
        }

        static void Main(string[] args)
        {
            var param = parseArguments(args);

            DesktopRecorder.Instance.StartRecording(param.RecordingParameters);

            Console.WriteLine("Press Any key to stop");
            Console.ReadKey();

            DesktopRecorder.Instance.FinishRecording(param.OutputPath);
            Console.WriteLine("Finished: " + param.OutputPath);
            Console.ReadKey();
        }
    }
}
