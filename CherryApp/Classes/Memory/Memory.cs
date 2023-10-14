using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Web.SessionState;
using Newtonsoft.Json.Linq;

using static CherryApp.Classes.Native;

namespace CherryApp.Classes.Memory
{
   public class RtModule : IEquatable<RtModule>
    {
        public RtTarget Target { get; }
        
        public readonly IntPtr Base;
        public readonly int Size;

        internal RtModule(RtTarget Target, IntPtr Base, int Size)
        {
            this.Target = Target;
            this.Base = Base;
            this.Size = Size;
        }

        public string Path { get; internal set; }
        
        public string Name => Path != null ? System.IO.Path.GetFileName(Path) : null;

        public bool Equals(RtModule Other) =>
            Other != null && Other.Base == Base && Other.Target == Target;
    }

    public class RtTarget : IDisposable, IEquatable<RtTarget>
    {
        internal IntPtr Handle;

        internal RtTarget(int Id) =>
            this.Id = Id;

        public int Id { get; internal set; }
        public bool IsOpen { get; private set; }

        public RtTarget Open()
        {
            if (IsOpen == true)
                return this;

            Handle = OpenProcess(
                ProcessAccess.All,
                false,
                Id);

            IsOpen = Handle != Success;

            return IsOpen ? this : null;
        }

        public RtModule MainModule { get; internal set; }

        public string Path => MainModule.Path;
        public string Name => MainModule.Name;

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        public bool Equals(RtTarget Other) =>
            Other != null && Id == Other.Id;

        public bool IsDebuggerPresent
        {
            get
            {
                bool Flag = false;
                CheckRemoteDebuggerPresent(Handle, ref Flag);

                return Flag;
            }
        }

        protected virtual void Dispose(bool Disposing)
        {
            bool Disposed = false;
            if (Disposed)
                return;

            Disposed = true;

            if (Disposing)
                CloseHandle(Handle);
        }
        
        ~RtTarget()
        {
            Dispose(false);
        }
    }

    public class RtThread : IDisposable, IEquatable<RtThread>
    {
        public readonly int Id;
        public readonly RtTarget Target;

        bool Disposed;
        IntPtr Handle;

        internal RtThread(RtTarget Target, int ThreadId)
        {
            this.Target = Target;
            this.Id = ThreadId;
        }
        
        public bool IsOpen { get; private set; }

        public bool IsFrozen
        {
            get
            {
                if (SuspendThread(Handle) != Success ||
                    ResumeThread(Handle) != Success)
                    return false;

                return true;
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        public bool Equals(RtThread Other) =>
            Other != null && Other.Id == Id && Other.Target == Target;

        public RtThread Open()
        {
            if (IsOpen)
                return this;

            Handle = OpenThread(
                ThreadAccess.All,
                false,
                Id);

            IsOpen = Handle != Success;

            return IsOpen ? this : null;
        }

        public bool Close() =>
            !IsOpen || CloseHandle(Handle) == true;

        public bool Freeze() =>
            SuspendThread(Handle) == Success;

        public ThreadContext GetContext()
        {
            ThreadContext Ctx = new ThreadContext { Flags = ContextFlags.All };

            if (GetThreadContext(Handle, ref Ctx) != true)
                return new ThreadContext { Flags = ContextFlags.All };

            return Ctx;
        }

        public bool SetCtx(ThreadContext Ctx) =>
            SetThreadContext(Handle, ref Ctx) == true;

        public bool SetEip(IntPtr Eip)
        {
            ThreadContext Ctx = GetContext();
            Ctx.Eip = (uint)Eip;
            return SetCtx(Ctx);
        }

        protected virtual void Dispose(bool Disposing)
        {
            if (Disposed)
                return;

            Disposed = true;

            if (Disposing && IsOpen)
                Close();
        }

        ~RtThread()
        {
            Dispose(false);
        }
    }

    public static class Memory
    {
        public static uint Rebase(RtModule Module, IntPtr Address, uint? Base = null) =>
            (uint)Module.Base + (uint)Address - (Base ?? 0);

        public static IntPtr Allocate(RtTarget Proc, int Size, MemoryProtection Protection = MemoryProtection.ReadWriteExecute)
        {
            IntPtr Address = VirtualAllocEx(
                Proc.Handle,
                IntPtr.Zero,
                Size,
                AllocationType.Commit | AllocationType.Reserve,
                Protection);
            
            if (Address != Success)
                return IntPtr.Zero;

            return Address;
        }
        
        public static bool Free(RtTarget Target, IntPtr Address) =>
            VirtualFreeEx(Target.Handle, Address, 0, FreeType.Release) == true;

        public static MemoryProtection Protect(RtTarget Target, IntPtr Address, int Size, MemoryProtection Protection)
        {
            if (VirtualProtectEx(
                Target.Handle,
                Address,
                Size,
                Protection,
                out MemoryProtection Old
                ) != true) return MemoryProtection.NoAccess;

            return Old;
        }

        /* <- Generic Read/Write -> */
        
        public static byte[] Read(RtTarget Target, IntPtr Address, int Length)
        {
            byte[] Buffer = new byte[Length];

            int BytesRead = 0;
            if (ReadProcessMemory(
                Target.Handle,
                Address,
                Buffer,
                Length,
                ref BytesRead) != true)
                return null;

            return Buffer;
        }

        public static bool Write(RtTarget Target, IntPtr Address, byte[] Buffer) =>
            WriteProcessMemory(
                Target.Handle,
                Address,
                Buffer,
                Buffer.Length,
                out _);

        /* ------------------ */

        /* <- T Read/Write -> */

        public static T Read<T>(RtTarget Target, IntPtr Address) where T : unmanaged
        {
            int Size = Marshal.SizeOf(typeof(T));
            byte[] Buffer = new byte[Size];

            int BytesRead = 0;
            ReadProcessMemory(
                Target.Handle,
                Address,
                Buffer,
                Size,
                ref BytesRead);

            return ByteArrayToStructure<T>(Buffer);
        }

        public static bool Write<T>(RtTarget Target, IntPtr Address, T Buffer) where T : unmanaged =>
            WriteProcessMemory(
                Target.Handle,
                Address,
                StructureToByteArray(Buffer),
                Marshal.SizeOf(typeof(T)),
                out _);
        
        /* ------------------ */

        private static T ByteArrayToStructure<T>(byte[] Bytes) where T : struct
        {
            GCHandle Handle = GCHandle.Alloc(Bytes, GCHandleType.Pinned);
            T Pointer = (T)Marshal.PtrToStructure(Handle.AddrOfPinnedObject(), typeof(T));

            Handle.Free();
            return Pointer;
        }

        private static byte[] StructureToByteArray(object Object)
        {
            int Length = Marshal.SizeOf(Object);
            byte[] Bytes = new byte[Length];

            IntPtr Pointer = Marshal.AllocHGlobal(Length);

            Marshal.StructureToPtr(Object, Pointer, true);
            Marshal.Copy(Pointer, Bytes, 0, Length);
            Marshal.FreeHGlobal(Pointer);

            return Bytes;
        }
    }
}
