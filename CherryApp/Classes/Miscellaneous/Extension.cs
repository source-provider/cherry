using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace CherryApp.Classes
{
    public static class Extension
    {
        public static Process GetProcessByName(this Process[] Processes, string Name)
        {
            foreach (Process Proc in Processes)
            {
                if (Proc.MainWindowTitle == Name)
                    return Proc;
            }

            return null;
        }

        public static string Get(this string url)
        {
            Uri URI = new Uri(url);
            using (WebClient Client = new WebClient{ Proxy = null })
                return Client.DownloadString(URI);
        }

        public static async Task DownloadFile(this string url, string file) 
        {
            using (WebClient Client = new WebClient { Proxy = null })
                await Client.DownloadFileTaskAsync(new Uri(url), file);
        }

        /* Why Have Async DownloadFile But Not DownloadString?? ~ Immune */
        public static async Task<string> GetAsync(this string Url)
        {
            using (WebClient Http = new WebClient { Proxy = null })
                return await Http.DownloadStringTaskAsync(Url);
        }
        
        public static async Task WriteToFile(this string file, string data)
        {
            if (File.Exists(file))
                File.Delete(file);

            using (StreamWriter Writer = File.CreateText(file))
            {
                await Writer.WriteAsync(data);
                Writer.Close();
            }
        }
    }
}
