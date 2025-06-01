#ifndef LLJS_H
#define LLJS_H

#include <napi.h>
#include <memory>
#include <vector>
#include <string>
#include <cstdint>
#include <thread>
#include <chrono>
#include <cmath>
#include <cstring>

namespace LLJS {
    // Memory management functions
    namespace Memory {
        Napi::Value AllocateBuffer(const Napi::CallbackInfo& info);
        Napi::Value FreeBuffer(const Napi::CallbackInfo& info);
        Napi::Value CopyMemory(const Napi::CallbackInfo& info);
        Napi::Value SetMemory(const Napi::CallbackInfo& info);
        Napi::Value CompareMemory(const Napi::CallbackInfo& info);
        Napi::Value GetMemoryUsage(const Napi::CallbackInfo& info);
        Napi::Value AlignedAlloc(const Napi::CallbackInfo& info);
        Napi::Value GetPointerValue(const Napi::CallbackInfo& info);
        Napi::Value SetPointerValue(const Napi::CallbackInfo& info);
    }

    // CPU operations
    namespace CPU {
        Napi::Value GetCPUInfo(const Napi::CallbackInfo& info);
        Napi::Value GetCoreCount(const Napi::CallbackInfo& info);
        Napi::Value GetCacheInfo(const Napi::CallbackInfo& info);
        Napi::Value ExecuteAssembly(const Napi::CallbackInfo& info);
        Napi::Value GetCPUUsage(const Napi::CallbackInfo& info);
        Napi::Value SetCPUAffinity(const Napi::CallbackInfo& info);
        Napi::Value GetRegisters(const Napi::CallbackInfo& info);
        Napi::Value PrefetchMemory(const Napi::CallbackInfo& info);
        Napi::Value GetCPUTemperature(const Napi::CallbackInfo& info);
        Napi::Value GetCPUFrequency(const Napi::CallbackInfo& info);
    }

    // System calls and operations
    namespace System {
        Napi::Value GetSystemInfo(const Napi::CallbackInfo& info);
        Napi::Value ExecuteSystemCall(const Napi::CallbackInfo& info);
        Napi::Value GetEnvironmentVariable(const Napi::CallbackInfo& info);
        Napi::Value SetEnvironmentVariable(const Napi::CallbackInfo& info);
        Napi::Value GetProcessId(const Napi::CallbackInfo& info);
        Napi::Value KillProcess(const Napi::CallbackInfo& info);
        Napi::Value CreateProcess(const Napi::CallbackInfo& info);
        Napi::Value GetProcessList(const Napi::CallbackInfo& info);
    }

    // I/O operations
    namespace IO {
        Napi::Value ReadFile(const Napi::CallbackInfo& info);
        Napi::Value WriteFile(const Napi::CallbackInfo& info);
        Napi::Value OpenFile(const Napi::CallbackInfo& info);
        Napi::Value CloseFile(const Napi::CallbackInfo& info);
        Napi::Value SeekFile(const Napi::CallbackInfo& info);
        Napi::Value FlushFile(const Napi::CallbackInfo& info);
        Napi::Value GetFileInfo(const Napi::CallbackInfo& info);
        Napi::Value DirectoryOperations(const Napi::CallbackInfo& info);
    }

    // Threading operations
    namespace Threading {
        Napi::Value CreateThread(const Napi::CallbackInfo& info);
        Napi::Value JoinThread(const Napi::CallbackInfo& info);
        Napi::Value DetachThread(const Napi::CallbackInfo& info);
        Napi::Value GetThreadId(const Napi::CallbackInfo& info);
        Napi::Value CreateMutex(const Napi::CallbackInfo& info);
        Napi::Value LockMutex(const Napi::CallbackInfo& info);
        Napi::Value UnlockMutex(const Napi::CallbackInfo& info);
        Napi::Value CreateSemaphore(const Napi::CallbackInfo& info);
        Napi::Value WaitSemaphore(const Napi::CallbackInfo& info);
        Napi::Value SignalSemaphore(const Napi::CallbackInfo& info);
    }

    // Time operations
    namespace Time {
        Napi::Value GetHighResTime(const Napi::CallbackInfo& info);
        Napi::Value Sleep(const Napi::CallbackInfo& info);
        Napi::Value SleepMicroseconds(const Napi::CallbackInfo& info);
        Napi::Value GetTimestamp(const Napi::CallbackInfo& info);
        Napi::Value CreateTimer(const Napi::CallbackInfo& info);
        Napi::Value DestroyTimer(const Napi::CallbackInfo& info);
        Napi::Value GetCPUTime(const Napi::CallbackInfo& info);
        Napi::Value GetThreadCPUTime(const Napi::CallbackInfo& info);
        Napi::Value GetMonotonicTime(const Napi::CallbackInfo& info);
        Napi::Value MeasureElapsed(const Napi::CallbackInfo& info);
        Napi::Value GetTimeZoneInfo(const Napi::CallbackInfo& info);
    }

    // Math operations
    namespace Math {
        Napi::Value FastSqrt(const Napi::CallbackInfo& info);
        Napi::Value FastInvSqrt(const Napi::CallbackInfo& info);
        Napi::Value VectorOperations(const Napi::CallbackInfo& info);
        Napi::Value MatrixOperations(const Napi::CallbackInfo& info);
        Napi::Value BitwiseOperations(const Napi::CallbackInfo& info);
        Napi::Value RandomNumbers(const Napi::CallbackInfo& info);
        Napi::Value FastFourierTransform(const Napi::CallbackInfo& info);
    }

    // String operations
    namespace String {
        Napi::Value FastStringCompare(const Napi::CallbackInfo& info);
        Napi::Value StringLength(const Napi::CallbackInfo& info);
        Napi::Value StringCopy(const Napi::CallbackInfo& info);
        Napi::Value StringConcat(const Napi::CallbackInfo& info);
        Napi::Value StringSearch(const Napi::CallbackInfo& info);
        Napi::Value StringHash(const Napi::CallbackInfo& info);
        Napi::Value StringValidate(const Napi::CallbackInfo& info);
        Napi::Value StringReplace(const Napi::CallbackInfo& info);
    }
}

#endif // LLJS_H