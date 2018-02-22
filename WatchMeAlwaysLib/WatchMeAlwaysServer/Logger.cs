using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WatchMeAlwaysServer
{
    public class Logger
    {
        const string Prefix = ""; // "WatchMeAlways: ";

        public static void Info(string format, params object[] args)
        {
            Console.WriteLine(Prefix + format, args);
        }

        public static void Warn(string format, params object[] args)
        {
            Console.WriteLine(Prefix + format, args);
        }

        public static void Error(string format, params object[] args)
        {
            Console.WriteLine(Prefix + format, args);
        }
    }
}
