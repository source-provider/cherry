using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace Extensions
{
    public static class Extension
    {
        public static Color ToColor(this Brush MainBrush)
        {
            return ((SolidColorBrush)MainBrush).Color;
        }

    }
}

namespace CherryApp
{
    public partial class App : Application
    {
        /* Should Use This More Often It's Useful */

        public App()
        {
            Startup += (s, e) =>
            {
                return;
            };

            Exit += (s, e) =>
            {
                return;
            };
        }
    }
}
