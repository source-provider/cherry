#pragma once
#ifndef CHERRY_OFFSETS
#define CHERRY_OFFSETS
#include <Windows.h>
#include <cstdint>
#include <string>
#include <string_view>
#include <winternl.h>

namespace cherry {
	enum offsets : std::intptr_t {
		/* TaskScheduler */
		schedulerJobName			 = 16,
		schedulerFps				 = 280,
		schedulerJobStart			 = 308,
		schedulerJobEnd				 = 312,
									 
		/* ScriptJob */			 
		scriptJobDataModel			 = 40,
		scriptJobScriptContext		 = 304,
									 
		/* PropertyDescriptor */	 
		propertyDescriptorScriptable = 32,
									 
		/* Script */				 
		moduleScriptBytecode		 = 276, 
		localScriptBytecode			 = 304,
									 
		/* Signals */				 
		signalNext					 = 16,
		signalRefIdx				 = 12,
		signalState					 = 20,
		signalRef0					 = 28,
		signalRef1					 = 100,
	};
};
#endif

#pragma region structs
typedef struct _TEB_ACTIVE_FRAME_CONTEXT
{
    ULONG Flags;
    CHAR* FrameName;
} TEB_ACTIVE_FRAME_CONTEXT, * PTEB_ACTIVE_FRAME_CONTEXT;

typedef struct _TEB_ACTIVE_FRAME
{
    ULONG Flags;
    _TEB_ACTIVE_FRAME* Previous;
    PTEB_ACTIVE_FRAME_CONTEXT Context;
} TEB_ACTIVE_FRAME, * PTEB_ACTIVE_FRAME;

typedef struct _GDI_TEB_BATCH
{
    ULONG Offset;
    ULONG HDC;
    ULONG Buffer[310];
} GDI_TEB_BATCH, * PGDI_TEB_BATCH;


typedef struct _RTL_ACTIVATION_CONTEXT_STACK_FRAME
{
    _RTL_ACTIVATION_CONTEXT_STACK_FRAME* Previous;
    _ACTIVATION_CONTEXT* ActivationContext;
    ULONG Flags;
} RTL_ACTIVATION_CONTEXT_STACK_FRAME, * PRTL_ACTIVATION_CONTEXT_STACK_FRAME;

typedef struct _ACTIVATION_CONTEXT_STACK
{
    PRTL_ACTIVATION_CONTEXT_STACK_FRAME ActiveFrame;
    LIST_ENTRY FrameListCache;
    ULONG Flags;
    ULONG NextCookieSequenceNumber;
    ULONG StackId;
} ACTIVATION_CONTEXT_STACK, * PACTIVATION_CONTEXT_STACK;

typedef struct _TTEB
{
    NT_TIB NtTib;
    PVOID EnvironmentPointer;
    CLIENT_ID ClientId;
    PVOID ActiveRpcHandle;
    PVOID ThreadLocalStoragePointer;
    PPEB ProcessEnvironmentBlock;
    ULONG LastErrorValue;
    ULONG CountOfOwnedCriticalSections;
    PVOID CsrClientThread;
    PVOID Win32ThreadInfo;
    ULONG User32Reserved[26];
    ULONG UserReserved[5];
    PVOID WOW32Reserved;
    ULONG CurrentLocale;
    ULONG FpSoftwareStatusRegister;
    VOID* SystemReserved1[54];
    LONG ExceptionCode;
    PACTIVATION_CONTEXT_STACK ActivationContextStackPointer;
    UCHAR SpareBytes1[36];
    ULONG TxFsContext;
    GDI_TEB_BATCH GdiTebBatch;
    CLIENT_ID RealClientId;
    PVOID GdiCachedProcessHandle;
    ULONG GdiClientPID;
    ULONG GdiClientTID;
    PVOID GdiThreadLocalInfo;
    ULONG Win32ClientInfo[62];
    VOID* glDispatchTable[233];
    ULONG glReserved1[29];
    PVOID glReserved2;
    PVOID glSectionInfo;
    PVOID glSection;
    PVOID glTable;
    PVOID glCurrentRC;
    PVOID glContext;
    ULONG LastStatusValue;
    UNICODE_STRING StaticUnicodeString;
    WCHAR StaticUnicodeBuffer[261];
    PVOID DeallocationStack;
    VOID* TlsSlots[64];
    LIST_ENTRY TlsLinks;
    PVOID Vdm;
    PVOID ReservedForNtRpc;
    VOID* DbgSsReserved[2];
    ULONG HardErrorMode;
    VOID* Instrumentation[9];
    GUID ActivityId;
    PVOID SubProcessTag;
    PVOID EtwLocalData;
    PVOID EtwTraceData;
    PVOID WinSockData;
    ULONG GdiBatchCount;
    UCHAR SpareBool0;
    UCHAR SpareBool1;
    UCHAR SpareBool2;
    UCHAR IdealProcessor;
    ULONG GuaranteedStackBytes;
    PVOID ReservedForPerf;
    PVOID ReservedForOle;
    ULONG WaitingOnLoaderLock;
    PVOID SavedPriorityState;
    ULONG SoftPatchPtr1;
    PVOID ThreadPoolData;
    VOID** TlsExpansionSlots;
    ULONG ImpersonationLocale;
    ULONG IsImpersonating;
    PVOID NlsCache;
    PVOID pShimData;
    ULONG HeapVirtualAffinity;
    PVOID CurrentTransactionHandle;
    PTEB_ACTIVE_FRAME ActiveFrame;
    PVOID FlsData;
    PVOID PreferredLanguages;
    PVOID UserPrefLanguages;
    PVOID MergedPrefLanguages;
    ULONG MuiImpersonation;
    WORD CrossTebFlags;
    ULONG SpareCrossTebBits : 16;
    WORD SameTebFlags;
    ULONG DbgSafeThunkCall : 1;
    ULONG DbgInDebugPrint : 1;
    ULONG DbgHasFiberData : 1;
    ULONG DbgSkipThreadAttach : 1;
    ULONG DbgWerInShipAssertCode : 1;
    ULONG DbgRanProcessInit : 1;
    ULONG DbgClonedThread : 1;
    ULONG DbgSuppressDebugMsg : 1;
    ULONG SpareSameTebBits : 8;
    PVOID TxnScopeEnterCallback;
    PVOID TxnScopeExitCallback;
    PVOID TxnScopeContext;
    ULONG LockCount;
    ULONG ProcessRundown;
    UINT64 LastSwitchTime;
    UINT64 TotalSwitchOutTime;
    LARGE_INTEGER WaitReasonBitMap;
} TTEB, * PPTEB;
#pragma endregion