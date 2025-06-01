#include "headers/lljs.h"
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <map>
#include <memory>
#include <functional>
#ifdef _WIN32
#include <windows.h>
#include <profileapi.h>
#else
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#endif

namespace LLJS::Time {

// Timer management
struct TimerInfo {
    std::unique_ptr<std::thread> timerThread;
    std::atomic<bool> running{true};
    uint64_t interval; // microseconds
    std::function<void()> callback;
    uint64_t id;
};

static std::atomic<uint64_t> timerCounter{0};
static std::map<uint64_t, std::unique_ptr<TimerInfo>> timers;
static std::mutex timerMutex;

/**
 * Gets high-resolution time in nanoseconds
 * @param info - CallbackInfo (no parameters required)
 * @returns Time in nanoseconds since epoch
 */
Napi::Value GetHighResTime(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    auto now = std::chrono::high_resolution_clock::now();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    
    return Napi::Number::New(env, static_cast<double>(nanoseconds));
}

/**
 * Sleeps for specified milliseconds
 * @param info - CallbackInfo containing milliseconds parameter
 */
Napi::Value Sleep(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Sleep duration in milliseconds required").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    
    int ms = info[0].As<Napi::Number>().Int32Value();
    
    if (ms < 0) {
        Napi::TypeError::New(env, "Sleep duration must be non-negative").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return env.Undefined();
}

/**
 * Sleeps for specified microseconds
 * @param info - CallbackInfo containing microseconds parameter
 */
Napi::Value SleepMicroseconds(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Sleep duration in microseconds required").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    
    int us = info[0].As<Napi::Number>().Int32Value();
    
    if (us < 0) {
        Napi::TypeError::New(env, "Sleep duration must be non-negative").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    
#ifdef _WIN32
    // Windows high-precision sleep
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    
    double targetTicks = static_cast<double>(us) * frequency.QuadPart / 1000000.0;
    
    do {
        QueryPerformanceCounter(&end);
    } while ((end.QuadPart - start.QuadPart) < targetTicks);
#else
    std::this_thread::sleep_for(std::chrono::microseconds(us));
#endif
    
    return env.Undefined();
}

/**
 * Gets current timestamp in various formats
 * @param info - CallbackInfo containing format parameter
 * @returns Timestamp in requested format
 */
Napi::Value GetTimestamp(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    std::string format = "unix";
    if (info.Length() > 0 && info[0].IsString()) {
        format = info[0].As<Napi::String>();
    }
    
    auto now = std::chrono::system_clock::now();
    
    if (format == "unix") {
        auto unix_timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        return Napi::Number::New(env, static_cast<double>(unix_timestamp));
    } else if (format == "unix-ms") {
        auto unix_timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return Napi::Number::New(env, static_cast<double>(unix_timestamp_ms));
    } else if (format == "unix-us") {
        auto unix_timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
        return Napi::Number::New(env, static_cast<double>(unix_timestamp_us));
    } else if (format == "unix-ns") {
        auto unix_timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
        return Napi::Number::New(env, static_cast<double>(unix_timestamp_ns));
    } else if (format == "iso") {
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
        
        return Napi::String::New(env, ss.str());
    } else if (format == "high-res") {
        auto high_res_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        return Napi::Number::New(env, static_cast<double>(high_res_time));
    } else {
        Napi::TypeError::New(env, "Invalid timestamp format").ThrowAsJavaScriptException();
        return env.Null();
    }
}

/**
 * Creates a high-precision timer
 * @param info - CallbackInfo containing callback function and interval
 * @returns Timer handle object
 */
Napi::Value CreateTimer(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsFunction() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Callback function and interval in microseconds required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Function callback = info[0].As<Napi::Function>();
    uint64_t interval = info[1].As<Napi::Number>().Uint32Value();
    
    if (interval == 0) {
        Napi::TypeError::New(env, "Timer interval must be greater than 0").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    try {
        auto timerInfo = std::make_unique<TimerInfo>();
        timerInfo->id = ++timerCounter;
        timerInfo->interval = interval;
        
        // Create timer thread
        timerInfo->timerThread = std::make_unique<std::thread>([callback, interval, env, timerInfo = timerInfo.get()]() {
            auto nextTime = std::chrono::high_resolution_clock::now();
            
            while (timerInfo->running.load()) {
                nextTime += std::chrono::microseconds(interval);
                
                // Call the JavaScript callback
                try {
                    callback.Call(env.Global(), {});
                } catch (...) {
                    // Handle callback exceptions
                    break;
                }
                
                // High-precision sleep until next interval
                std::this_thread::sleep_until(nextTime);
            }
        });
        
        uint64_t timerId = timerInfo->id;
        
        {
            std::lock_guard<std::mutex> lock(timerMutex);
            timers[timerId] = std::move(timerInfo);
        }
        
        Napi::Object handle = Napi::Object::New(env);
        handle.Set("id", Napi::Number::New(env, static_cast<double>(timerId)));
        handle.Set("interval", Napi::Number::New(env, static_cast<double>(interval)));
        
        return handle;
    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Failed to create timer: ") + e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

/**
 * Stops and destroys a timer
 * @param info - CallbackInfo containing timer handle
 * @returns Success status
 */
Napi::Value DestroyTimer(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Timer handle object required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    Napi::Object handle = info[0].As<Napi::Object>();
    uint64_t timerId = static_cast<uint64_t>(handle.Get("id").As<Napi::Number>().DoubleValue());
    
    std::lock_guard<std::mutex> lock(timerMutex);
    auto it = timers.find(timerId);
    if (it == timers.end()) {
        return Napi::Boolean::New(env, false);
    }
    
    // Stop the timer
    it->second->running = false;
    
    // Wait for thread to finish
    if (it->second->timerThread && it->second->timerThread->joinable()) {
        it->second->timerThread->join();
    }
    
    timers.erase(it);
    return Napi::Boolean::New(env, true);
}

/**
 * Gets CPU time usage for current process
 * @param info - CallbackInfo (no parameters required)
 * @returns CPU time in microseconds
 */
Napi::Value GetCPUTime(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
#ifdef _WIN32
    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime)) {
        // Convert FILETIME to microseconds
        auto convertFileTime = [](const FILETIME& ft) -> uint64_t {
            ULARGE_INTEGER ull;
            ull.LowPart = ft.dwLowDateTime;
            ull.HighPart = ft.dwHighDateTime;
            return ull.QuadPart / 10; // Convert from 100ns to microseconds
        };
        
        uint64_t totalCpuTime = convertFileTime(kernelTime) + convertFileTime(userTime);
        return Napi::Number::New(env, static_cast<double>(totalCpuTime));
    } else {
        return Napi::Number::New(env, 0);
    }
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        uint64_t userTime = static_cast<uint64_t>(usage.ru_utime.tv_sec) * 1000000 + usage.ru_utime.tv_usec;
        uint64_t systemTime = static_cast<uint64_t>(usage.ru_stime.tv_sec) * 1000000 + usage.ru_stime.tv_usec;
        return Napi::Number::New(env, static_cast<double>(userTime + systemTime));
    } else {
        return Napi::Number::New(env, 0);
    }
#endif
}

/**
 * Gets thread CPU time
 * @param info - CallbackInfo (no parameters required)
 * @returns Thread CPU time in microseconds
 */
Napi::Value GetThreadCPUTime(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
#ifdef _WIN32
    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetThreadTimes(GetCurrentThread(), &creationTime, &exitTime, &kernelTime, &userTime)) {
        auto convertFileTime = [](const FILETIME& ft) -> uint64_t {
            ULARGE_INTEGER ull;
            ull.LowPart = ft.dwLowDateTime;
            ull.HighPart = ft.dwHighDateTime;
            return ull.QuadPart / 10;
        };
        
        uint64_t totalCpuTime = convertFileTime(kernelTime) + convertFileTime(userTime);
        return Napi::Number::New(env, static_cast<double>(totalCpuTime));
    } else {
        return Napi::Number::New(env, 0);
    }
#else
    struct rusage usage;
    if (getrusage(RUSAGE_THREAD, &usage) == 0) {
        uint64_t userTime = static_cast<uint64_t>(usage.ru_utime.tv_sec) * 1000000 + usage.ru_utime.tv_usec;
        uint64_t systemTime = static_cast<uint64_t>(usage.ru_stime.tv_sec) * 1000000 + usage.ru_stime.tv_usec;
        return Napi::Number::New(env, static_cast<double>(userTime + systemTime));
    } else {
        return Napi::Number::New(env, 0);
    }
#endif
}

/**
 * Gets monotonic time (unaffected by system clock changes)
 * @param info - CallbackInfo (no parameters required)
 * @returns Monotonic time in nanoseconds
 */
Napi::Value GetMonotonicTime(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    auto now = std::chrono::steady_clock::now();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    
    return Napi::Number::New(env, static_cast<double>(nanoseconds));
}

/**
 * Measures elapsed time between two high-resolution time points
 * @param info - CallbackInfo containing start and end times
 * @returns Elapsed time in nanoseconds
 */
Napi::Value MeasureElapsed(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Start time and end time required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    double startTime = info[0].As<Napi::Number>().DoubleValue();
    double endTime = info[1].As<Napi::Number>().DoubleValue();
    
    double elapsed = endTime - startTime;
    return Napi::Number::New(env, elapsed);
}

/**
 * Gets time zone information
 * @param info - CallbackInfo (no parameters required)
 * @returns Time zone information object
 */
Napi::Value GetTimeZoneInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);
    
#ifdef _WIN32
    TIME_ZONE_INFORMATION tzInfo;
    DWORD tzResult = GetTimeZoneInformation(&tzInfo);
    
    result.Set("bias", Napi::Number::New(env, tzInfo.Bias));
    
    // Convert wide strings to regular strings
    std::wstring standardName(tzInfo.StandardName);
    std::wstring daylightName(tzInfo.DaylightName);
    
    std::string standardNameStr(standardName.begin(), standardName.end());
    std::string daylightNameStr(daylightName.begin(), daylightName.end());
    
    result.Set("standardName", Napi::String::New(env, standardNameStr));
    result.Set("daylightName", Napi::String::New(env, daylightNameStr));
    result.Set("isDST", Napi::Boolean::New(env, tzResult == TIME_ZONE_ID_DAYLIGHT));
#else
    // Get timezone information on Unix systems
    time_t rawtime;
    struct tm * timeinfo;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    result.Set("bias", Napi::Number::New(env, timezone / 60)); // Convert seconds to minutes
    result.Set("standardName", Napi::String::New(env, tzname[0]));
    result.Set("daylightName", Napi::String::New(env, tzname[1]));
    result.Set("isDST", Napi::Boolean::New(env, timeinfo->tm_isdst > 0));
#endif
    
    return result;
}

}