using CherryApp.Classes.Memory;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using static CherryApp.Classes.Memory.Natives;

/*
ModuleInfo

[0] - dll version
[1] - injector version
[2] - module download link
[3] - injector download link
*/

/* This Code Makes Me Wanna Kms If You Want Some Better Code Just Lmk ~ Immune */
namespace CherryApp.Classes
{
    public class CherryAPI
    {
        private bool Initialized = false;
        private string ModuleData = "https://raw.githubusercontent.com/auxopen/-e7f362a5-bf34-45ba-8acb-f87ca3fed9ba-/main/ebe0d2c8-1b3c-415f-af2c-54cfbe4c672b";
        private string PipeName = "Cherry";
        private bool is_pid = false;

        public enum InjectionStatus
        {
            Failed = -1, 
            Success = 0,
            AlreadyInjected = 1,
        }

        public async Task InitializeModule()
        {
            if (Initialized)
                return;
            /* variables */
            string[] ModuleInfo = ModuleData.Get().Split('\n'); /* split by new line */
            string BinDirectory = $"{AppContext.BaseDirectory}\\bin";
            string DVerLoc = $"{BinDirectory}\\dver", IVerLoc = $"{BinDirectory}\\iver";
            string DllDown = ModuleInfo[2], InjDown = ModuleInfo[3];


            /* folders */
            Directory.CreateDirectory(BinDirectory);

            /* files (doing file checks cause I don't wanna overwrite just yet) */
            if (!File.Exists(DVerLoc))
            {
                await DllDown.DownloadFile("bin\\exploit-main.dll"); // $"{BinDirectory}\\"
                await DVerLoc.WriteToFile(ModuleInfo[0]);
            }

            if (!File.Exists(IVerLoc))
            {
                await InjDown.DownloadFile("bin\\CherryInjector.exe"); // $"{BinDirectory}\\"
                await IVerLoc.WriteToFile(ModuleInfo[1]);
            }

            using (StreamReader Reader = new StreamReader(DVerLoc))
            {
                string dver = await Reader.ReadToEndAsync();

                if (dver != ModuleInfo[0])
                {
                    await DllDown.DownloadFile("bin\\exploit-main.dll");
                    await DVerLoc.WriteToFile(ModuleInfo[0]);
                }

                Reader.Close();
            }

            using (StreamReader Reader = new StreamReader(IVerLoc))
            {
                string iver = await Reader.ReadToEndAsync();

                if (iver != ModuleInfo[1])
                {
                    await InjDown.DownloadFile("bin\\CherryInjector.exe");
                    await IVerLoc.WriteToFile(ModuleInfo[1]);
                }

                Reader.Close();
            }

            if (!File.Exists($"{BinDirectory}\\exploit-main.dll"))
                await DllDown.DownloadFile("bin\\exploit-main.dll");

            if (!File.Exists($"{BinDirectory}\\CherryInjector.exe"))
                await InjDown.DownloadFile("bin\\CherryInjector.exe");

            Initialized = true;
        }

        public InjectionStatus Inject()
        {
            new Process
            {                
                StartInfo = new ProcessStartInfo
                {
                    FileName = "Launcher.exe"
                }

            }.Start();
            return InjectionStatus.Success;
            //string FullPath = Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "bin\\CherryDLL.dll"));
            //IntPtr Kernel = GetModuleHandle("kernel32.dll");

            //if (Kernel == IntPtr.Zero)
            //    return InjectionStatus.Failed;

            //IntPtr ProcAddress = GetProcAddress(Kernel, "LoadLibraryA");

            //if (ProcAddress == IntPtr.Zero)
            //    return InjectionStatus.Failed;

            //foreach (Process Proci in Process.GetProcessesByName("RobloxPlayerBeta"))
            //{
            //    if (string.IsNullOrEmpty(Proci.MainWindowTitle))
            //        Proci.Kill();
            //    else
            //    {
            //        if (IsInjected(Proci.Id) == false)
            //        {
            //            IntPtr num1 = OpenProcess(1082, false, Proci.Id);
            //            IntPtr num2 = VirtualAllocEx(num1, IntPtr.Zero, (uint)((FullPath.Length + 1) * Marshal.SizeOf(typeof(char))), 12288U, 4U);
            //            WriteProcessMemory(num1, num2, Encoding.Default.GetBytes(FullPath), (uint)((FullPath.Length + 1) * Marshal.SizeOf(typeof(char))), out UIntPtr _);
            //            CreateRemoteThread(num1, IntPtr.Zero, 0U, ProcAddress, num2, 0U, IntPtr.Zero);
            //            CloseHandle(num1);
            //        }
            //    }
            //}

            //return InjectionStatus.Success;
        }

        public async Task<bool> ExecuteScript(string Script)
        {
            foreach (string Pipe in Directory.GetFiles(@"\\.\pipe\")) {
                if (Pipe.Contains(PipeName)) {
                    using (NamedPipeClientStream PipeStream = new NamedPipeClientStream(".", Pipe.Replace(@"\\.\pipe\", string.Empty), PipeDirection.InOut)) {
                        await PipeStream.ConnectAsync();

                        using (StreamWriter Writer = new StreamWriter(PipeStream, Encoding.Default, 999999))
                            await Writer.WriteAsync(Script);
                    }
                }
            }
            //foreach (Process Proc in Process.GetProcessesByName("RobloxPlayerBeta"))
            //{
            //    if (!IsInjected(Proc.Id))
            //    {
            //        Console.WriteLine("Skipped");
            //        continue;
            //    }

            //    using (NamedPipeClientStream PipeStream = new NamedPipeClientStream(".", $"{PipeName}{ (is_pid == true ? Proc.Id : "") }", PipeDirection.InOut))
            //    {
            //        await PipeStream.ConnectAsync();

            //        using (StreamWriter Writer = new StreamWriter(PipeStream, Encoding.Default, 999999))
            //            await Writer.WriteAsync(Script);
            //    }
            //}

            return true;
        }

        public bool IsInjected(int PID = 0)
        {
            foreach (string Pipe in Directory.GetFiles(@"\\.\pipe\")) {
                if (Pipe.Contains(PipeName)) {
                    return true;
                }
            }

            return false;
        }

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern bool WaitNamedPipe(string name, int timeout);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern int CloseHandle(IntPtr hObject);

        [DllImport("kernel32.dll")]
        private static extern IntPtr OpenProcess(
          int dwDesiredAccess,
          bool bInheritHandle,
          int dwProcessId);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        private static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32", CharSet = CharSet.Ansi, SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr VirtualAllocEx(
          IntPtr hProcess,
          IntPtr lpAddress,
          uint dwSize,
          uint flAllocationType,
          uint flProtect);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool WriteProcessMemory(
          IntPtr hProcess,
          IntPtr lpBaseAddress,
          byte[] lpBuffer,
          uint nSize,
          out UIntPtr lpNumberOfBytesWritten);

        [DllImport("kernel32.dll")]
        private static extern IntPtr CreateRemoteThread(
          IntPtr hProcess,
          IntPtr lpThreadAttributes,
          uint dwStackSize,
          IntPtr lpStartAddress,
          IntPtr lpParameter,
          uint dwCreationFlags,
          IntPtr lpThreadId);
    }
}