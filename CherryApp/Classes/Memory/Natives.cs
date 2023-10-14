using System;
using System.Runtime.InteropServices;
using System.Text;

namespace CherryApp.Classes.Memory
{
    public static class Natives
    {
        public const nint Success = 0;
        public const nint InvalidHandle = -1;

        [DllImport("psapi.dll")]
        public static extern IntPtr GetModuleBaseName(
          IntPtr W,
          IntPtr Hm,
          StringBuilder lpBaseName,
          uint nSize);

        [DllImport("user32.dll")]
        public static extern IntPtr FindWindowA(string lpClassName, string lpWindowName);

        [DllImport("kernel32.dll")]
        public static extern bool ReadProcessMemory(
          IntPtr hProcess,
          IntPtr lpBaseAddress,
          byte[] lpBuffer,
          int dwSize,
          ref int lpNumberOfBytesRead);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr VirtualAllocEx(
          IntPtr hProcess,
          IntPtr lpAddress,
          int dwSize,
          Natives.AllocationType flAllocationType,
          Natives.MemoryProtection flProtect);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool WriteProcessMemory(
          IntPtr hProcess,
          IntPtr lpBaseAddress,
          byte[] lpBuffer,
          int nSize,
          out UIntPtr lpNumberOfBytesWritten);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern bool WaitNamedPipe(string Name, int Timeout);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern uint WaitForSingleObject(IntPtr hHandle, uint Milliseconds);

        [DllImport("kernel32.dll")]
        public static extern IntPtr OpenProcess(
          Natives.ProcessAccess dwDesiredAccess,
          [MarshalAs(UnmanagedType.Bool)] bool bInheritHandle,
          int dwProcessId);

        [DllImport("kernel32", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern IntPtr LoadLibraryA(string lpFileName);

        [DllImport("kernel32", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern UIntPtr GetProcAddress(IntPtr hModule, string ProcName);

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool FreeLibrary(IntPtr hModule);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern bool CloseHandle(IntPtr hThread);

        [DllImport("kernel32.dll")]
        public static extern IntPtr CreateRemoteThread(
          IntPtr hProcess,
          IntPtr lpThreadAttributes,
          uint dwStackSize,
          UIntPtr lpStartAddress,
          IntPtr lpParameter,
          uint dwCreationFlags,
          out IntPtr lpThreadId);

        [DllImport("kernel32.dll")]
        public static extern bool GetExitCodeThread(IntPtr hThread, out uint lpExitCode);

        [DllImport("kernel32.dll")]
        public static extern bool VirtualFreeEx(
          IntPtr hProcess,
          IntPtr lpAddress,
          uint Size,
          Natives.FreeType Type);

        [DllImport("kernel32.dll")]
        public static extern bool VirtualProtectEx(
          IntPtr hProcess,
          IntPtr lpAddress,
          UIntPtr Size,
          Natives.MemoryProtection lpNewProtect,
          out Natives.MemoryProtection lpOldProtect);

        [DllImport("kernel32.dll", EntryPoint = "GetModuleHandleW", SetLastError = true)]
        public static extern IntPtr GetModuleHandle(string ModuleName);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern IntPtr OpenThread(
          Natives.ThreadAccess dwDesiredAccess,
          bool bInheritHandle,
          int dwThreadId);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern IntPtr SuspendThread(IntPtr hThread);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern IntPtr ResumeThread(IntPtr hThread);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern bool GetThreadContext(IntPtr hThread, ref Natives.ThreadContext lpContext);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern bool SetThreadContext(IntPtr hThread, ref Natives.ThreadContext lpContext);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern int CreateToolhelp32Snapshot(Natives.SnapRules dwFlags, int th32ProcessID);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern bool Thread32First(IntPtr hSnapshot, ref Natives.ThreadEntry lpte);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern bool Thread32Next(IntPtr hSnapshot, ref Natives.ThreadEntry lpte);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, SetLastError = true)]
        public static extern IntPtr GetThreadId(IntPtr Thread);

        [DllImport("Kernel32.dll", SetLastError = true, ExactSpelling = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool CheckRemoteDebuggerPresent(IntPtr hProcess, [MarshalAs(UnmanagedType.Bool)] ref bool isDebuggerPresent);

        [Flags]
        public enum AllocationType : uint
        {
            Commit = 4096, // 0x00001000
            Reserve = 8192, // 0x00002000
            Decommit = 16384, // 0x00004000
            Release = 32768, // 0x00008000
            Reset = 524288, // 0x00080000
            Physical = 4194304, // 0x00400000
            TopDown = 1048576, // 0x00100000
            WriteWatch = 2097152, // 0x00200000
            LargePages = 536870912, // 0x20000000
        }

        [Flags]
        public enum ContextFlags : uint
        {
            I386 = 65536, // 0x00010000
            Control = 65537, // 0x00010001
            Integer = 65538, // 0x00010002
            Segments = 65540, // 0x00010004
            FloatingPoint = 65544, // 0x00010008
            DebugRegisters = 65552, // 0x00010010
            ExtendedRegisers = 65568, // 0x00010020
            Full = 65543, // 0x00010007
            All = 65599, // 0x0001003F
        }

        [Flags]
        public enum FreeType : uint
        {
            Decommit = 16384, // 0x00004000
            Release = 32768, // 0x00008000
        }

        [Flags]
        public enum MemoryProtection : uint
        {
            Execute = 16, // 0x00000010
            ReadExecute = 32, // 0x00000020
            ReadWriteExecute = 64, // 0x00000040
            WriteCopyExecute = 128, // 0x00000080
            NoAccess = 1,
            ReadOnly = 2,
            ReadWrite = 4,
            WriteCopy = 8,
            Guard = 256, // 0x00000100
            NoCache = 512, // 0x00000200
            WriteCombine = 1024, // 0x00000400
        }

        [Flags]
        public enum ProcessAccess : uint
        {
            All = 2035711, // 0x001F0FFF
            Terminate = 1,
            CreateThread = 2,
            VirtualMemoryOperation = 8,
            VirtualMemoryRead = 16, // 0x00000010
            VirtualMemoryWrite = 32, // 0x00000020
            DuplicateHandle = 64, // 0x00000040
            CreateProcess = 128, // 0x00000080
            SetQuota = 256, // 0x00000100
            SetInformation = 512, // 0x00000200
            QueryInformation = 1024, // 0x00000400
            QueryLimitedInformation = 4096, // 0x00001000
            Synchronize = 1048576, // 0x00100000
        }

        [Flags]
        public enum SnapRules : uint
        {
            HeapList = 1,
            Process = 2,
            Thread = 4,
            Module = 8,
            Module32 = 16, // 0x00000010
            Inherit = 2147483648, // 0x80000000
            SnapAll = Module | Thread | Process | HeapList, // 0x0000000F
        }

        [Flags]
        public enum ThreadAccess : uint
        {
            Terminate = 1,
            SuspendResume = 2,
            GetContext = 8,
            SetContext = 16, // 0x00000010
            SetInfo = 32, // 0x00000020
            QueryInfo = 64, // 0x00000040
            SetToken = 128, // 0x00000080
            Impersonate = 256, // 0x00000100
            DirectImpersonate = 512, // 0x00000200
            All = Terminate | SuspendResume | GetContext | SetContext | SetInfo | QueryInfo | SetToken | Impersonate | DirectImpersonate,
        }

        public struct ThreadContext
        {
            public Natives.ContextFlags Flags;
            public uint Dr0;
            public uint Dr1;
            public uint Dr2;
            public uint Dr3;
            public uint Dr6;
            public uint Dr7;
            public Natives.XmmSave Xmm;
            public uint SegGs;
            public uint SegFs;
            public uint SegEs;
            public uint SegDs;
            public uint Edi;
            public uint Esi;
            public uint Ebx;
            public uint Edx;
            public uint Ecx;
            public uint Eax;
            public uint Ebp;
            public uint Eip;
            public uint SegCs;
            public uint EFlags;
            public uint Esp;
            public uint SegSs;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 512)]
            public byte[] ExtendedRegisters;
        }

        public struct ThreadEntry
        {
            public int Size;
            public uint UsageCount;
            public int ThreadId;
            public int ProcessId;
            public int KeBasePriority;
            public int DeltaPriority;
            public uint Flags;
        }

        public struct XmmSave
        {
            public int ControlWord;
            public int StatusWord;
            public int TagWord;
            public int ErrorOffset;
            public int ErrorSelector;
            public int DataOffset;
            public int DataSelector;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 80)]
            public byte[] RegisterArea;
            public int Cr0NpxState;
        }
    }
}