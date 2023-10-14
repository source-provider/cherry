#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <winternl.h>

#include "console.hpp"
#include "configuration.hpp"
#include "utilities.hpp"
#include "exception_handler.hpp"
#include <zstd.h>
#include <xxhash.h>
#include "fnv.hpp"
extern "C" {
    #include <lz4.h>
}
//#include "memory.hpp"

#define BASE_0 0x0
#define BASE_4 0x400000
#define __usercall __fastcall

#define vmvalue0_t noneenc_t
#define vmvalue1_t addenc_t
#define vmvalue2_t sub2enc_t
#define vmvalue3_t xorenc_t
#define vmvalue4_t sub1enc_t

#define vmvalue0 noneenc_t
#define vmvalue1 addenc_t
#define vmvalue2 sub2enc_t
#define vmvalue3 xorenc_t
#define vmvalue4 sub1enc_t

#define vm_value0 noneenc_t
#define vm_value1 addenc_t
#define vm_value2 sub2enc_t
#define vm_value3 xorenc_t
#define vm_value4 sub1enc_t

#define LUAVM_SHUFFLE_COMMA ,

#ifndef DADA
#define DADA
#define CHERRYRVM_EXTERN extern __forceinline
#endif // !DADA


#define dereference_pointer(ptr) (*(int*)ptr)

enum class ENC
{
    NONE,
    ADD,
    SUB1,
    SUB2,
    XOR
};

struct shared_string_t {
    std::uint8_t pad_0[0x10];
    std::string str;
};

namespace cherry 
{
    namespace encryptions
    {
        static __forceinline auto rebase(int x, int base = 0) -> int {
            return (x - base + (uintptr_t)GetModuleHandleA(NULL));
        }

        static __forceinline auto unbase(int x, int base = 0) -> int {
            return (x + base - (uintptr_t)GetModuleHandleA(NULL));
        }

        static __forceinline std::string compress_bytecode(const std::string& data)
        {
            std::string output = "RSB1";
            std::size_t dataSize = data.size();
            std::size_t maxSize = ZSTD_compressBound(dataSize);
            std::vector<char> zaazaa(maxSize);
            std::size_t compSize = ZSTD_compress(&zaazaa[0], maxSize, data.c_str(), dataSize, ZSTD_maxCLevel());

            output.append(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
            output.append(&zaazaa[0], compSize);

            std::uint32_t first_hash = XXH32(&output[0], output.size(), 42U);
            std::uint8_t hashed_bytes[4];

            std::memcpy(hashed_bytes, &first_hash, sizeof(first_hash));

            for (std::size_t i = 0; i < output.size(); ++i)
                output[i] ^= hashed_bytes[i % 4] + i * 41U;

            return output;
        }

        static __forceinline std::string decompress_bytecode(std::string data)
        {
            const char bytecode_magic[] = "RSB1";

            uint8_t hash_bytes[4];
            memcpy(hash_bytes, &data[0], 4);

            for (auto i = 0u; i < 4; ++i)
            {
                hash_bytes[i] ^= bytecode_magic[i];
                hash_bytes[i] -= i * 41;
            }

            for (size_t i = 0; i < data.size(); ++i)
                data[i] ^= hash_bytes[i % 4] + i * 41;

            XXH32(&data[0], data.size(), 42);

            uint32_t data_size;
            memcpy(&data_size, &data[4], 4);

            std::vector<uint8_t> zaazaa(data_size);
            ZSTD_decompress(&zaazaa[0], data_size, &data[8], data.size() - 8);

            return std::string((char*)(&zaazaa[0]), data_size);
        }

        template<typename return_type>
        class offset_t {
        private:
            uintptr_t off = 0;
            ENC enc = ENC::NONE;
        public:
            offset_t(const uintptr_t off, const ENC enc = ENC::NONE) {
                this->off = off;
                this->enc = enc;
            }

            __forceinline auto get(uintptr_t val) -> return_type {
                uintptr_t res = 0;

                switch (this->enc)
                {
                case ENC::ADD:
                    res = *reinterpret_cast<uintptr_t*>(val + this->off) - (val + this->off); break;
                case ENC::SUB1:
                    res = *reinterpret_cast<uintptr_t*>(val + this->off) + (val + this->off); break;
                case ENC::SUB2:
                    res = (val + this->off) - *reinterpret_cast<uintptr_t*>(val + this->off); break;
                case ENC::XOR:
                    res = *reinterpret_cast<uintptr_t*>(val + this->off) ^ (val + this->off); break;
                case ENC::NONE:
                    return *reinterpret_cast<return_type*>(val + this->off); break;
                }

                return (return_type)(res);
            };

            __forceinline auto set(uintptr_t val, return_type data) -> void {
                int size = sizeof(return_type);

                if (size == 8)
                {
#define ttypee double
                }
                else
                {
#define ttypee int
                }

                switch (this->enc)
                {
                case ENC::ADD:
                    *reinterpret_cast<ttypee*>((ttypee)val + this->off) = (ttypee)data + (val + this->off);
                    break;
                case ENC::SUB1:
                    *reinterpret_cast<ttypee*>((ttypee)val + this->off) = (ttypee)data - (val + this->off);
                    break;
                case ENC::SUB2:
                    *reinterpret_cast<ttypee*>((ttypee)val + this->off) = (val + this->off) - (ttypee)data;
                    break;
                case ENC::XOR:
                    *reinterpret_cast<ttypee*>((ttypee)val + this->off) = (ttypee)data ^ (val + this->off);
                    break;
                case ENC::NONE:
                    *reinterpret_cast<ttypee*>((ttypee)val + this->off) = (ttypee)data;
                    break;
                }

#undef ttypee
            };

            __declspec(noinline) auto get_offset() -> int {
                return this->off;
            };

            __declspec(noinline) auto get_enc() -> ENC {
                return this->enc;
            };
        };
    }

}

template <typename T> struct noneenc_t {
private:
    T storage;
public:
    __forceinline operator const T() const {
        return storage;
    }

    __forceinline void operator=(const T& value) {
        storage = value;
    }

    __forceinline const T operator->() const {
        return operator const T();
    }
};

template <typename T> struct addenc_t {
private:
    T storage;
public:
    __forceinline operator const T() const {
        return (T)((uintptr_t)storage - (uintptr_t)this);
    }

    __forceinline void operator=(const T& value) {
        storage = (T)((uintptr_t)value + (uintptr_t)this);
    }

    __forceinline const T operator->() const {
        return operator const T();
    }
};

template <typename T> struct sub2enc_t {
private:
    T storage;
public:
    __forceinline operator const T() const {
        return (T)((uintptr_t)this - (uintptr_t)storage);
    }

    __forceinline void operator=(const T& value) {
        storage = (T)((uintptr_t)this - (uintptr_t)value);
    }

    __forceinline const T operator->() const {
        return operator const T();
    }
};

template <typename T> struct xorenc_t {
private:
    T storage;
public:
    __forceinline operator const T() const {
        return (T)((uintptr_t)this ^ (uintptr_t)storage);
    }

    __forceinline void operator=(const T& value) {
        storage = (T)((uintptr_t)value ^ (uintptr_t)this);
    }

    __forceinline const T operator->() const {
        return operator const T();
    }
};

template <typename T> struct sub1enc_t {
private:
    T storage;
public:
    __forceinline operator const T() const {
        return (T)((uintptr_t)this + (uintptr_t)storage);
    }

    __forceinline void operator=(const T& value) {
        storage = (T)((uintptr_t)value - (uintptr_t)this);
    }

    __forceinline const T operator->() const {
        return operator const T();
    }
};

#pragma region structs
typedef struct _TEB_ACTIVE_FRAME_CONTEXT {
    ULONG Flags;
    CHAR* FrameName;
} TEB_ACTIVE_FRAME_CONTEXT, * PTEB_ACTIVE_FRAME_CONTEXT;

typedef struct _TEB_ACTIVE_FRAME {
    ULONG Flags;
    _TEB_ACTIVE_FRAME* Previous;
    PTEB_ACTIVE_FRAME_CONTEXT Context;
} TEB_ACTIVE_FRAME, * PTEB_ACTIVE_FRAME;

typedef struct _GDI_TEB_BATCH {
    ULONG Offset;
    ULONG HDC;
    ULONG Buffer[310];
} GDI_TEB_BATCH, * PGDI_TEB_BATCH;


typedef struct _RTL_ACTIVATION_CONTEXT_STACK_FRAME {
    _RTL_ACTIVATION_CONTEXT_STACK_FRAME* Previous;
    _ACTIVATION_CONTEXT* ActivationContext;
    ULONG Flags;
} RTL_ACTIVATION_CONTEXT_STACK_FRAME, * PRTL_ACTIVATION_CONTEXT_STACK_FRAME;

typedef struct _ACTIVATION_CONTEXT_STACK {
    PRTL_ACTIVATION_CONTEXT_STACK_FRAME ActiveFrame;
    LIST_ENTRY FrameListCache;
    ULONG Flags;
    ULONG NextCookieSequenceNumber;
    ULONG StackId;
} ACTIVATION_CONTEXT_STACK, * PACTIVATION_CONTEXT_STACK;

typedef struct _TTEB {
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