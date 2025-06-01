#include "headers/lljs.h"
#include <cstdlib>
#include <cstring>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
// Undefine problematic Windows macros
#ifdef CopyMemory
#undef CopyMemory
#endif
#ifdef SetMemory
#undef SetMemory
#endif
#else
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace LLJS::Memory {

/**
 * Allocates a raw memory buffer
 * @param info - CallbackInfo containing size parameter
 * @returns Buffer object or null on failure
 */
Napi::Value AllocateBuffer(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Size parameter required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    size_t size = info[0].As<Napi::Number>().Uint32Value();
    
    void* ptr = malloc(size);
    if (!ptr) {
        Napi::Error::New(env, "Memory allocation failed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    return Napi::Buffer<uint8_t>::New(env, static_cast<uint8_t*>(ptr), size,
        [](Napi::Env env, uint8_t* data) {
            free(data);
        });
}

/**
 * Frees a memory buffer
 * @param info - CallbackInfo containing buffer parameter
 * @returns Boolean indicating success
 */
Napi::Value FreeBuffer(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Buffer parameter required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    // Note: Node.js manages buffer memory automatically
    // This function exists for API completeness
    return Napi::Boolean::New(env, true);
}

/**
 * Copies memory from source to destination
 * @param info - CallbackInfo containing dest, src, size parameters
 * @returns Boolean indicating success
 */
Napi::Value CopyMemory(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 3 || !info[0].IsBuffer() || !info[1].IsBuffer() || !info[2].IsNumber()) {
        Napi::TypeError::New(env, "Invalid parameters: dest, src, size required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    Napi::Buffer<uint8_t> dest = info[0].As<Napi::Buffer<uint8_t>>();
    Napi::Buffer<uint8_t> src = info[1].As<Napi::Buffer<uint8_t>>();
    size_t size = info[2].As<Napi::Number>().Uint32Value();
    
    if (size > dest.Length() || size > src.Length()) {
        Napi::RangeError::New(env, "Size exceeds buffer length").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    std::memcpy(dest.Data(), src.Data(), size);
    return Napi::Boolean::New(env, true);
}

/**
 * Sets memory to a specific value
 * @param info - CallbackInfo containing buffer, value, size parameters
 * @returns Boolean indicating success
 */
Napi::Value SetMemory(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 3 || !info[0].IsBuffer() || !info[1].IsNumber() || !info[2].IsNumber()) {
        Napi::TypeError::New(env, "Invalid parameters: buffer, value, size required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
    int value = info[1].As<Napi::Number>().Int32Value();
    size_t size = info[2].As<Napi::Number>().Uint32Value();
    
    if (size > buffer.Length()) {
        Napi::RangeError::New(env, "Size exceeds buffer length").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    std::memset(buffer.Data(), value, size);
    return Napi::Boolean::New(env, true);
}

/**
 * Compares two memory regions
 * @param info - CallbackInfo containing buffer1, buffer2, size parameters
 * @returns Number indicating comparison result (-1, 0, 1)
 */
Napi::Value CompareMemory(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 3 || !info[0].IsBuffer() || !info[1].IsBuffer() || !info[2].IsNumber()) {
        Napi::TypeError::New(env, "Invalid parameters: buffer1, buffer2, size required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Buffer<uint8_t> buffer1 = info[0].As<Napi::Buffer<uint8_t>>();
    Napi::Buffer<uint8_t> buffer2 = info[1].As<Napi::Buffer<uint8_t>>();
    size_t size = info[2].As<Napi::Number>().Uint32Value();
    
    if (size > buffer1.Length() || size > buffer2.Length()) {
        Napi::RangeError::New(env, "Size exceeds buffer length").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    int result = std::memcmp(buffer1.Data(), buffer2.Data(), size);
    return Napi::Number::New(env, result);
}

/**
 * Gets current memory usage statistics
 * @param info - CallbackInfo (no parameters required)
 * @returns Object containing memory usage statistics
 */
Napi::Value GetMemoryUsage(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);
    
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        result.Set("rss", Napi::Number::New(env, pmc.WorkingSetSize));
        result.Set("peak", Napi::Number::New(env, pmc.PeakWorkingSetSize));
        result.Set("pageFaults", Napi::Number::New(env, pmc.PageFaultCount));
    }
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        result.Set("rss", Napi::Number::New(env, usage.ru_maxrss * 1024)); // Convert KB to bytes
        result.Set("userTime", Napi::Number::New(env, usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec));
        result.Set("systemTime", Napi::Number::New(env, usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec));
        result.Set("pageFaults", Napi::Number::New(env, usage.ru_majflt));
    }
#endif
    
    return result;
}

/**
 * Allocates aligned memory
 * @param info - CallbackInfo containing size and alignment parameters
 * @returns Buffer object or null on failure
 */
Napi::Value AlignedAlloc(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Size and alignment parameters required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    size_t size = info[0].As<Napi::Number>().Uint32Value();
    size_t alignment = info[1].As<Napi::Number>().Uint32Value();
    
#ifdef _WIN32
    void* ptr = _aligned_malloc(size, alignment);
#else
    void* ptr = aligned_alloc(alignment, size);
#endif
    
    if (!ptr) {
        Napi::Error::New(env, "Aligned memory allocation failed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    return Napi::Buffer<uint8_t>::New(env, static_cast<uint8_t*>(ptr), size,
        [](Napi::Env env, uint8_t* data) {
#ifdef _WIN32
            _aligned_free(data);
#else
            free(data);
#endif
        });
}

/**
 * Gets value at pointer address (unsafe operation)
 * @param info - CallbackInfo containing pointer and type parameters
 * @returns Value at the pointer address
 */
Napi::Value GetPointerValue(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsString()) {
        Napi::TypeError::New(env, "Pointer address and type required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uintptr_t addr = info[0].As<Napi::Number>().Uint32Value();
    std::string type = info[1].As<Napi::String>();
    
    // WARNING: This is extremely unsafe and should only be used with great care
    if (type == "int32") {
        return Napi::Number::New(env, *reinterpret_cast<int32_t*>(addr));
    } else if (type == "uint32") {
        return Napi::Number::New(env, *reinterpret_cast<uint32_t*>(addr));
    } else if (type == "float") {
        return Napi::Number::New(env, *reinterpret_cast<float*>(addr));
    } else if (type == "double") {
        return Napi::Number::New(env, *reinterpret_cast<double*>(addr));
    }
    
    Napi::TypeError::New(env, "Unsupported type").ThrowAsJavaScriptException();
    return env.Null();
}

/**
 * Sets value at pointer address (unsafe operation)
 * @param info - CallbackInfo containing pointer, type, and value parameters
 * @returns Boolean indicating success
 */
Napi::Value SetPointerValue(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 3 || !info[0].IsNumber() || !info[1].IsString() || !info[2].IsNumber()) {
        Napi::TypeError::New(env, "Pointer address, type, and value required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    uintptr_t addr = info[0].As<Napi::Number>().Uint32Value();
    std::string type = info[1].As<Napi::String>();
    double value = info[2].As<Napi::Number>().DoubleValue();
    
    // WARNING: This is extremely unsafe and should only be used with great care
    try {
        if (type == "int32") {
            *reinterpret_cast<int32_t*>(addr) = static_cast<int32_t>(value);
        } else if (type == "uint32") {
            *reinterpret_cast<uint32_t*>(addr) = static_cast<uint32_t>(value);
        } else if (type == "float") {
            *reinterpret_cast<float*>(addr) = static_cast<float>(value);
        } else if (type == "double") {
            *reinterpret_cast<double*>(addr) = value;
        } else {
            Napi::TypeError::New(env, "Unsupported type").ThrowAsJavaScriptException();
            return Napi::Boolean::New(env, false);
        }
        return Napi::Boolean::New(env, true);
    } catch (...) {
        return Napi::Boolean::New(env, false);
    }
}

}