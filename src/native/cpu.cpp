#include "headers/lljs.h"
#include <thread>
#include <chrono>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#include <psapi.h>
#include <pdh.h>
#include <powerbase.h>
#else
#include <cpuid.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <fstream>
#include <sched.h>
#include <immintrin.h>
#endif

namespace LLJS::CPU {

// CPU usage tracking variables
#ifdef _WIN32
static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
static HANDLE self;
static bool cpuUsageInitialized = false;
#else
static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
static bool cpuUsageInitialized = false;
#endif

/**
 * Initialize CPU usage tracking
 */
void InitCPUUsage() {
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    FILETIME ftime, fsys, fuser;

    GetSystemInfo(&sysInfo);
    numProcessors = sysInfo.dwNumberOfProcessors;

    GetSystemTimeAsFileTime(&ftime);
    std::memcpy(&lastCPU, &ftime, sizeof(FILETIME));

    self = GetCurrentProcess();
    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    std::memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
    std::memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
#else
    FILE* file = fopen("/proc/stat", "r");
    if (file) {
        fscanf(file, "cpu %llu %llu %llu %llu", &lastTotalUser, &lastTotalUserLow,
               &lastTotalSys, &lastTotalIdle);
        fclose(file);
    }
#endif
    cpuUsageInitialized = true;
}

/**
 * Gets detailed CPU information using CPUID instruction
 * @param info - CallbackInfo (no parameters required)
 * @returns Object containing comprehensive CPU information
 */
Napi::Value GetCPUInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);
    
    // CPU vendor and model information
    char vendor[13] = {0};
    char brand[49] = {0};
    
#ifdef _WIN32
    int cpuInfo[4];
    
    // Get vendor string
    __cpuid(cpuInfo, 0);
    *reinterpret_cast<int*>(vendor) = cpuInfo[1];      // EBX
    *reinterpret_cast<int*>(vendor + 4) = cpuInfo[3];  // EDX
    *reinterpret_cast<int*>(vendor + 8) = cpuInfo[2];  // ECX
    
    // Get brand string
    for (int i = 0x80000002; i <= 0x80000004; ++i) {
        __cpuid(cpuInfo, i);
        std::memcpy(brand + (i - 0x80000002) * 16, cpuInfo, sizeof(cpuInfo));
    }
    
    // Get additional CPU features
    __cpuid(cpuInfo, 1);
    int features = cpuInfo[3]; // EDX features
    int extFeatures = cpuInfo[2]; // ECX features
    
    // Get cache information
    __cpuid(cpuInfo, 0x80000006);
    int l2CacheSize = (cpuInfo[2] >> 16) & 0xFFFF; // L2 cache size in KB
    int l3CacheSize = (cpuInfo[3] >> 18) & 0x3FFF; // L3 cache size in 512KB units
    
#else
    unsigned int eax, ebx, ecx, edx;
    
    // Get vendor string
    if (__get_cpuid(0, &eax, &ebx, &ecx, &edx)) {
        *reinterpret_cast<unsigned int*>(vendor) = ebx;
        *reinterpret_cast<unsigned int*>(vendor + 4) = edx;
        *reinterpret_cast<unsigned int*>(vendor + 8) = ecx;
    }
    
    // Get brand string
    for (unsigned int i = 0x80000002; i <= 0x80000004; ++i) {
        if (__get_cpuid(i, &eax, &ebx, &ecx, &edx)) {
            unsigned int* ptr = reinterpret_cast<unsigned int*>(brand + (i - 0x80000002) * 16);
            ptr[0] = eax; ptr[1] = ebx; ptr[2] = ecx; ptr[3] = edx;
        }
    }
    
    // Get CPU features
    int features = 0, extFeatures = 0;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        features = edx;
        extFeatures = ecx;
    }
    
    // Get cache information
    int l2CacheSize = 0, l3CacheSize = 0;
    if (__get_cpuid(0x80000006, &eax, &ebx, &ecx, &edx)) {
        l2CacheSize = (ecx >> 16) & 0xFFFF;
        l3CacheSize = (edx >> 18) & 0x3FFF;
    }
#endif
    
    // Clean up strings
    std::string vendorStr(vendor);
    std::string brandStr(brand);
    
    // Remove extra whitespace from brand string
    size_t start = brandStr.find_first_not_of(" \t");
    size_t end = brandStr.find_last_not_of(" \t");
    if (start != std::string::npos && end != std::string::npos) {
        brandStr = brandStr.substr(start, end - start + 1);
    }
    
    result.Set("vendor", Napi::String::New(env, vendorStr));
    result.Set("model", Napi::String::New(env, brandStr));
    result.Set("cores", Napi::Number::New(env, std::thread::hardware_concurrency()));
    
    // CPU Features
    Napi::Object featuresObj = Napi::Object::New(env);
    featuresObj.Set("mmx", Napi::Boolean::New(env, (features & (1 << 23)) != 0));
    featuresObj.Set("sse", Napi::Boolean::New(env, (features & (1 << 25)) != 0));
    featuresObj.Set("sse2", Napi::Boolean::New(env, (features & (1 << 26)) != 0));
    featuresObj.Set("sse3", Napi::Boolean::New(env, (extFeatures & (1 << 0)) != 0));
    featuresObj.Set("ssse3", Napi::Boolean::New(env, (extFeatures & (1 << 9)) != 0));
    featuresObj.Set("sse41", Napi::Boolean::New(env, (extFeatures & (1 << 19)) != 0));
    featuresObj.Set("sse42", Napi::Boolean::New(env, (extFeatures & (1 << 20)) != 0));
    featuresObj.Set("avx", Napi::Boolean::New(env, (extFeatures & (1 << 28)) != 0));
    featuresObj.Set("fma", Napi::Boolean::New(env, (extFeatures & (1 << 12)) != 0));
    result.Set("features", featuresObj);
    
    // Cache information
    Napi::Object cache = Napi::Object::New(env);
    cache.Set("l1d", Napi::Number::New(env, 32 * 1024));  // Typical L1D size
    cache.Set("l1i", Napi::Number::New(env, 32 * 1024));  // Typical L1I size
    cache.Set("l2", Napi::Number::New(env, l2CacheSize * 1024));
    cache.Set("l3", Napi::Number::New(env, l3CacheSize * 512 * 1024)); // L3 is in 512KB units
    result.Set("cache", cache);
    
    // CPU Clock Speed (approximate)
#ifdef _WIN32
    HKEY hKey;
    DWORD speed = 0;
    DWORD dwSize = sizeof(DWORD);
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "~MHz", NULL, NULL, (LPBYTE)&speed, &dwSize);
        RegCloseKey(hKey);
    }
    
    result.Set("speed", Napi::Number::New(env, speed));
#else
    // Try to read from /proc/cpuinfo
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    double speed = 0;
    
    while (std::getline(cpuinfo, line)) {
        if (line.find("cpu MHz") != std::string::npos) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                speed = std::stod(line.substr(colonPos + 1));
                break;
            }
        }
    }
    
    result.Set("speed", Napi::Number::New(env, speed));
#endif
    
    return result;
}

/**
 * Gets the number of CPU cores
 * @param info - CallbackInfo (no parameters required)
 * @returns Number of logical cores
 */
Napi::Value GetCoreCount(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, std::thread::hardware_concurrency());
}

/**
 * Gets detailed CPU cache information
 * @param info - CallbackInfo (no parameters required)
 * @returns Object containing cache sizes and details
 */
Napi::Value GetCacheInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);
    
#ifdef _WIN32
    int cpuInfo[4];
    
    // Get cache information using CPUID
    __cpuid(cpuInfo, 0x80000005); // L1 cache info
    int l1DCache = (cpuInfo[2] >> 24) & 0xFF; // L1 Data cache size in KB
    int l1ICache = (cpuInfo[3] >> 24) & 0xFF; // L1 Instruction cache size in KB
    
    __cpuid(cpuInfo, 0x80000006); // L2/L3 cache info
    int l2Cache = (cpuInfo[2] >> 16) & 0xFFFF; // L2 cache size in KB
    int l3Cache = (cpuInfo[3] >> 18) & 0x3FFF; // L3 cache size in 512KB units
    
    result.Set("l1d", Napi::Number::New(env, l1DCache > 0 ? l1DCache * 1024 : 32768));
    result.Set("l1i", Napi::Number::New(env, l1ICache > 0 ? l1ICache * 1024 : 32768));
    result.Set("l2", Napi::Number::New(env, l2Cache > 0 ? l2Cache * 1024 : 262144));
    result.Set("l3", Napi::Number::New(env, l3Cache > 0 ? l3Cache * 512 * 1024 : 8388608));
#else
    unsigned int eax, ebx, ecx, edx;
    
    // Get cache information
    int l1DCache = 32, l1ICache = 32, l2Cache = 256, l3Cache = 8192; // Default values in KB
    
    if (__get_cpuid(0x80000005, &eax, &ebx, &ecx, &edx)) {
        l1DCache = (ecx >> 24) & 0xFF;
        l1ICache = (edx >> 24) & 0xFF;
    }
    
    if (__get_cpuid(0x80000006, &eax, &ebx, &ecx, &edx)) {
        l2Cache = (ecx >> 16) & 0xFFFF;
        l3Cache = (edx >> 18) & 0x3FFF;
        if (l3Cache > 0) l3Cache *= 512; // Convert to KB
    }
    
    result.Set("l1d", Napi::Number::New(env, l1DCache * 1024));
    result.Set("l1i", Napi::Number::New(env, l1ICache * 1024));
    result.Set("l2", Napi::Number::New(env, l2Cache * 1024));
    result.Set("l3", Napi::Number::New(env, l3Cache * 1024));
#endif
    
    // Additional cache information
    result.Set("lineSize", Napi::Number::New(env, 64)); // Typical cache line size
    result.Set("associativity", Napi::String::New(env, "variable"));
    
    return result;
}

/**
 * Executes inline assembly code (DISABLED for security)
 * @param info - CallbackInfo containing assembly code and inputs
 * @returns Error message
 */
Napi::Value ExecuteAssembly(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    // Assembly execution is extremely dangerous and disabled for security
    Napi::Error::New(env, "Inline assembly execution is disabled for security reasons. "
                          "Use specific CPU instruction functions instead.").ThrowAsJavaScriptException();
    return env.Null();
}

/**
 * Gets current CPU usage percentage
 * @param info - CallbackInfo (no parameters required)
 * @returns CPU usage percentage (0-100)
 */
Napi::Value GetCPUUsage(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!cpuUsageInitialized) {
        InitCPUUsage();
        // Return 0 on first call as we need baseline measurements
        return Napi::Number::New(env, 0.0);
    }
    
#ifdef _WIN32
    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now, sys, user;
    double percent;

    GetSystemTimeAsFileTime(&ftime);
    std::memcpy(&now, &ftime, sizeof(FILETIME));

    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    std::memcpy(&sys, &fsys, sizeof(FILETIME));
    std::memcpy(&user, &fuser, sizeof(FILETIME));
    
    percent = (sys.QuadPart - lastSysCPU.QuadPart) +
              (user.QuadPart - lastUserCPU.QuadPart);
    percent /= (now.QuadPart - lastCPU.QuadPart);
    percent /= numProcessors;
    lastCPU = now;
    lastUserCPU = user;
    lastSysCPU = sys;

    return Napi::Number::New(env, percent * 100.0);
#else
    double percent;
    FILE* file;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

    file = fopen("/proc/stat", "r");
    if (!file) {
        return Napi::Number::New(env, 0.0);
    }
    
    fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
           &totalSys, &totalIdle);
    fclose(file);

    if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
        totalSys < lastTotalSys || totalIdle < lastTotalIdle) {
        // Overflow detection
        percent = 0.0;
    } else {
        total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
                (totalSys - lastTotalSys);
        percent = total;
        total += (totalIdle - lastTotalIdle);
        percent /= total;
        percent *= 100.0;
    }

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    return Napi::Number::New(env, percent);
#endif
}

/**
 * Sets CPU affinity for current process
 * @param info - CallbackInfo containing CPU mask
 * @returns Success status
 */
Napi::Value SetCPUAffinity(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "CPU mask parameter required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    uint64_t mask = info[0].As<Napi::Number>().Uint32Value();
    
#ifdef _WIN32
    DWORD_PTR affinityMask = static_cast<DWORD_PTR>(mask);
    BOOL result = SetProcessAffinityMask(GetCurrentProcess(), affinityMask);
    return Napi::Boolean::New(env, result != 0);
#else
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    
    // Convert mask to cpu_set_t
    for (int i = 0; i < 64; i++) {
        if (mask & (1ULL << i)) {
            CPU_SET(i, &cpuset);
        }
    }
    
    int result = sched_setaffinity(0, sizeof(cpuset), &cpuset);
    return Napi::Boolean::New(env, result == 0);
#endif
}

/**
 * Gets CPU register values (DISABLED for security)
 * @param info - CallbackInfo (no parameters required)
 * @returns Warning message about security
 */
Napi::Value GetRegisters(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);
    
    // Direct register access is dangerous and disabled for security
    result.Set("warning", Napi::String::New(env, "Direct register access is disabled for security reasons"));
    result.Set("eax", Napi::Number::New(env, 0));
    result.Set("ebx", Napi::Number::New(env, 0));
    result.Set("ecx", Napi::Number::New(env, 0));
    result.Set("edx", Napi::Number::New(env, 0));
    result.Set("rsp", Napi::Number::New(env, 0));
    result.Set("rbp", Napi::Number::New(env, 0));
    result.Set("rsi", Napi::Number::New(env, 0));
    result.Set("rdi", Napi::Number::New(env, 0));
    
    return result;
}

/**
 * Prefetches memory into CPU cache
 * @param info - CallbackInfo containing address and locality
 * @returns Success status
 */
Napi::Value PrefetchMemory(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Memory address parameter required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    uintptr_t address = info[0].As<Napi::Number>().Uint32Value();
    int locality = 1; // Default temporal locality
    
    if (info.Length() > 1 && info[1].IsNumber()) {
        locality = info[1].As<Napi::Number>().Int32Value();
        if (locality < 0 || locality > 3) {
            locality = 1; // Clamp to valid range
        }
    }
    
    try {
        void* ptr = reinterpret_cast<void*>(address);
        
#ifdef _WIN32
        // Use Windows prefetch intrinsics
        switch (locality) {
            case 0:
                _mm_prefetch(static_cast<char*>(ptr), _MM_HINT_NTA); // Non-temporal
                break;
            case 1:
                _mm_prefetch(static_cast<char*>(ptr), _MM_HINT_T2);  // Low temporal locality
                break;
            case 2:
                _mm_prefetch(static_cast<char*>(ptr), _MM_HINT_T1);  // Moderate temporal locality
                break;
            case 3:
                _mm_prefetch(static_cast<char*>(ptr), _MM_HINT_T0);  // High temporal locality
                break;
        }
#else
        // Use GCC builtin prefetch
        // Parameters: address, rw (0=read, 1=write), locality (0-3)
        __builtin_prefetch(ptr, 0, locality);
#endif
        
        return Napi::Boolean::New(env, true);
    } catch (...) {
        return Napi::Boolean::New(env, false);
    }
}

/**
 * Gets CPU temperature (if available)
 * @param info - CallbackInfo (no parameters required)
 * @returns Temperature in Celsius or -1 if unavailable
 */
Napi::Value GetCPUTemperature(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
#ifdef _WIN32
    // Windows doesn't provide easy access to CPU temperature
    // Would require WMI or specific hardware libraries
    return Napi::Number::New(env, -1);
#else
    // Try to read from thermal zone (Linux)
    std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    if (tempFile.is_open()) {
        int temp;
        tempFile >> temp;
        tempFile.close();
        
        // Temperature is in millidegrees Celsius
        double tempCelsius = temp / 1000.0;
        return Napi::Number::New(env, tempCelsius);
    }
    
    return Napi::Number::New(env, -1);
#endif
}

/**
 * Gets CPU frequency information
 * @param info - CallbackInfo (no parameters required)
 * @returns Object with frequency information
 */
Napi::Value GetCPUFrequency(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);
    
#ifdef _WIN32
    // Use performance counters to estimate frequency
    LARGE_INTEGER frequency;
    if (QueryPerformanceFrequency(&frequency)) {
        result.Set("base", Napi::Number::New(env, static_cast<double>(frequency.QuadPart)));
        result.Set("current", Napi::Number::New(env, static_cast<double>(frequency.QuadPart)));
        result.Set("max", Napi::Number::New(env, static_cast<double>(frequency.QuadPart)));
    } else {
        result.Set("base", Napi::Number::New(env, 0));
        result.Set("current", Napi::Number::New(env, 0));
        result.Set("max", Napi::Number::New(env, 0));
    }
#else
    // Read from /proc/cpuinfo and /sys/devices/system/cpu/
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    double baseFreq = 0, currentFreq = 0, maxFreq = 0;
    
    // Get base frequency from cpuinfo
    while (std::getline(cpuinfo, line)) {
        if (line.find("cpu MHz") != std::string::npos) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                currentFreq = std::stod(line.substr(colonPos + 1)) * 1000000; // Convert to Hz
                break;
            }
        }
    }
    
    // Try to get scaling information
    std::ifstream maxFreqFile("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (maxFreqFile.is_open()) {
        int freq;
        maxFreqFile >> freq;
        maxFreq = freq * 1000.0; // Convert from kHz to Hz
        maxFreqFile.close();
    }
    
    std::ifstream curFreqFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (curFreqFile.is_open()) {
        int freq;
        curFreqFile >> freq;
        currentFreq = freq * 1000.0; // Convert from kHz to Hz
        curFreqFile.close();
    }
    
    result.Set("base", Napi::Number::New(env, baseFreq > 0 ? baseFreq : currentFreq));
    result.Set("current", Napi::Number::New(env, currentFreq));
    result.Set("max", Napi::Number::New(env, maxFreq > 0 ? maxFreq : currentFreq));
#endif
    
    return result;
}

}