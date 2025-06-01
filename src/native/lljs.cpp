#include "headers/lljs.h"

/**
 * Initialize the LLJS native module
 * @param env - N-API environment
 * @param exports - Module exports object
 * @returns Initialized module exports
 */
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    // Memory operations
    exports.Set("allocateBuffer", Napi::Function::New(env, LLJS::Memory::AllocateBuffer));
    exports.Set("freeBuffer", Napi::Function::New(env, LLJS::Memory::FreeBuffer));
    exports.Set("copyMemory", Napi::Function::New(env, LLJS::Memory::CopyMemory));
    exports.Set("setMemory", Napi::Function::New(env, LLJS::Memory::SetMemory));
    exports.Set("compareMemory", Napi::Function::New(env, LLJS::Memory::CompareMemory));
    exports.Set("getMemoryUsage", Napi::Function::New(env, LLJS::Memory::GetMemoryUsage));
    exports.Set("alignedAlloc", Napi::Function::New(env, LLJS::Memory::AlignedAlloc));
    exports.Set("getPointerValue", Napi::Function::New(env, LLJS::Memory::GetPointerValue));
    exports.Set("setPointerValue", Napi::Function::New(env, LLJS::Memory::SetPointerValue));

    // CPU operations
    exports.Set("getCPUInfo", Napi::Function::New(env, LLJS::CPU::GetCPUInfo));
    exports.Set("getCoreCount", Napi::Function::New(env, LLJS::CPU::GetCoreCount));
    exports.Set("getCacheInfo", Napi::Function::New(env, LLJS::CPU::GetCacheInfo));
    exports.Set("executeAssembly", Napi::Function::New(env, LLJS::CPU::ExecuteAssembly));
    exports.Set("getCPUUsage", Napi::Function::New(env, LLJS::CPU::GetCPUUsage));
    exports.Set("setCPUAffinity", Napi::Function::New(env, LLJS::CPU::SetCPUAffinity));
    exports.Set("getRegisters", Napi::Function::New(env, LLJS::CPU::GetRegisters));
    exports.Set("prefetchMemory", Napi::Function::New(env, LLJS::CPU::PrefetchMemory));
    exports.Set("getCPUTemperature", Napi::Function::New(env, LLJS::CPU::GetCPUTemperature));
    exports.Set("getCPUFrequency", Napi::Function::New(env, LLJS::CPU::GetCPUFrequency));

    // System operations
    exports.Set("getSystemInfo", Napi::Function::New(env, LLJS::System::GetSystemInfo));
    exports.Set("executeSystemCall", Napi::Function::New(env, LLJS::System::ExecuteSystemCall));
    exports.Set("getEnvironmentVariable", Napi::Function::New(env, LLJS::System::GetEnvironmentVariable));
    exports.Set("setEnvironmentVariable", Napi::Function::New(env, LLJS::System::SetEnvironmentVariable));
    exports.Set("getProcessId", Napi::Function::New(env, LLJS::System::GetProcessId));
    exports.Set("killProcess", Napi::Function::New(env, LLJS::System::KillProcess));
    exports.Set("createProcess", Napi::Function::New(env, LLJS::System::CreateProcess));
    exports.Set("getProcessList", Napi::Function::New(env, LLJS::System::GetProcessList));

    // I/O operations
    exports.Set("readFile", Napi::Function::New(env, LLJS::IO::ReadFile));
    exports.Set("writeFile", Napi::Function::New(env, LLJS::IO::WriteFile));
    exports.Set("openFile", Napi::Function::New(env, LLJS::IO::OpenFile));
    exports.Set("closeFile", Napi::Function::New(env, LLJS::IO::CloseFile));
    exports.Set("seekFile", Napi::Function::New(env, LLJS::IO::SeekFile));
    exports.Set("flushFile", Napi::Function::New(env, LLJS::IO::FlushFile));
    exports.Set("getFileInfo", Napi::Function::New(env, LLJS::IO::GetFileInfo));
    exports.Set("directoryOperations", Napi::Function::New(env, LLJS::IO::DirectoryOperations));

    // Threading operations
    exports.Set("createThread", Napi::Function::New(env, LLJS::Threading::CreateThread));
    exports.Set("joinThread", Napi::Function::New(env, LLJS::Threading::JoinThread));
    exports.Set("detachThread", Napi::Function::New(env, LLJS::Threading::DetachThread));
    exports.Set("getThreadId", Napi::Function::New(env, LLJS::Threading::GetThreadId));
    exports.Set("createMutex", Napi::Function::New(env, LLJS::Threading::CreateMutex));
    exports.Set("lockMutex", Napi::Function::New(env, LLJS::Threading::LockMutex));
    exports.Set("unlockMutex", Napi::Function::New(env, LLJS::Threading::UnlockMutex));
    exports.Set("createSemaphore", Napi::Function::New(env, LLJS::Threading::CreateSemaphore));
    exports.Set("waitSemaphore", Napi::Function::New(env, LLJS::Threading::WaitSemaphore));
    exports.Set("signalSemaphore", Napi::Function::New(env, LLJS::Threading::SignalSemaphore));

    // Time operations
    exports.Set("getHighResTime", Napi::Function::New(env, LLJS::Time::GetHighResTime));
    exports.Set("sleep", Napi::Function::New(env, LLJS::Time::Sleep));
    exports.Set("sleepMicroseconds", Napi::Function::New(env, LLJS::Time::SleepMicroseconds));
    exports.Set("getTimestamp", Napi::Function::New(env, LLJS::Time::GetTimestamp));
    exports.Set("createTimer", Napi::Function::New(env, LLJS::Time::CreateTimer));
    exports.Set("destroyTimer", Napi::Function::New(env, LLJS::Time::DestroyTimer));
    exports.Set("getCPUTime", Napi::Function::New(env, LLJS::Time::GetCPUTime));
    exports.Set("getThreadCPUTime", Napi::Function::New(env, LLJS::Time::GetThreadCPUTime));
    exports.Set("getMonotonicTime", Napi::Function::New(env, LLJS::Time::GetMonotonicTime));
    exports.Set("measureElapsed", Napi::Function::New(env, LLJS::Time::MeasureElapsed));
    exports.Set("getTimeZoneInfo", Napi::Function::New(env, LLJS::Time::GetTimeZoneInfo));

    // Math operations
    exports.Set("fastSqrt", Napi::Function::New(env, LLJS::Math::FastSqrt));
    exports.Set("fastInvSqrt", Napi::Function::New(env, LLJS::Math::FastInvSqrt));
    exports.Set("vectorOperations", Napi::Function::New(env, LLJS::Math::VectorOperations));
    exports.Set("matrixOperations", Napi::Function::New(env, LLJS::Math::MatrixOperations));
    exports.Set("bitwiseOperations", Napi::Function::New(env, LLJS::Math::BitwiseOperations));
    exports.Set("randomNumbers", Napi::Function::New(env, LLJS::Math::RandomNumbers));
    exports.Set("fastFourierTransform", Napi::Function::New(env, LLJS::Math::FastFourierTransform));

    // String operations
    exports.Set("fastStringCompare", Napi::Function::New(env, LLJS::String::FastStringCompare));
    exports.Set("stringLength", Napi::Function::New(env, LLJS::String::StringLength));
    exports.Set("stringCopy", Napi::Function::New(env, LLJS::String::StringCopy));
    exports.Set("stringConcat", Napi::Function::New(env, LLJS::String::StringConcat));
    exports.Set("stringSearch", Napi::Function::New(env, LLJS::String::StringSearch));
    exports.Set("stringHash", Napi::Function::New(env, LLJS::String::StringHash));
    exports.Set("stringValidate", Napi::Function::New(env, LLJS::String::StringValidate));
    exports.Set("stringReplace", Napi::Function::New(env, LLJS::String::StringReplace));

    return exports;
}

NODE_API_MODULE(lljs, Init)