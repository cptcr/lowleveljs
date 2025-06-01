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
#include <tlhelp32.h>
#include <psapi.h>
// Prevent Windows API name conflicts
#ifdef SetEnvironmentVariable
#undef SetEnvironmentVariable
#endif
#ifdef GetEnvironmentVariable
#undef GetEnvironmentVariable
#endif
#else
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <fstream>
#endif

namespace LLJS::System {

/**
 * Gets system information
 * @param info - CallbackInfo (no parameters required)
 * @returns Object containing system information
 */
Napi::Value GetSystemInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Object result = Napi::Object::New(env);
    
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    
    // Get Windows version - using GetVersionEx with proper version checking
    result.Set("platform", Napi::String::New(env, "win32"));
    result.Set("arch", Napi::String::New(env, sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? "x64" : "x86"));
    result.Set("totalMemory", Napi::Number::New(env, static_cast<double>(memInfo.ullTotalPhys)));
    result.Set("freeMemory", Napi::Number::New(env, static_cast<double>(memInfo.ullAvailPhys)));
    result.Set("uptime", Napi::Number::New(env, GetTickCount64() / 1000.0));
    
    // Use a more reliable method for Windows version
    HKEY hKey;
    char versionBuffer[256] = "Unknown";
    DWORD bufferSize = sizeof(versionBuffer);
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                     "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, "ProductName", NULL, NULL, 
                           reinterpret_cast<LPBYTE>(versionBuffer), &bufferSize) == ERROR_SUCCESS) {
            result.Set("version", Napi::String::New(env, versionBuffer));
        } else {
            result.Set("version", Napi::String::New(env, "Windows"));
        }
        RegCloseKey(hKey);
    } else {
        result.Set("version", Napi::String::New(env, "Windows"));
    }
#else
    struct utsname unameData;
    if (uname(&unameData) == 0) {
        result.Set("platform", Napi::String::New(env, unameData.sysname));
        result.Set("arch", Napi::String::New(env, unameData.machine));
        result.Set("version", Napi::String::New(env, unameData.release));
    }
    
    struct sysinfo sysInfo;
    if (sysinfo(&sysInfo) == 0) {
        result.Set("totalMemory", Napi::Number::New(env, static_cast<double>(sysInfo.totalram * sysInfo.mem_unit)));
        result.Set("freeMemory", Napi::Number::New(env, static_cast<double>(sysInfo.freeram * sysInfo.mem_unit)));
        result.Set("uptime", Napi::Number::New(env, static_cast<double>(sysInfo.uptime)));
    }
#endif
    
    return result;
}

/**
 * Executes a system call (DANGEROUS)
 * @param info - CallbackInfo containing syscall number and arguments
 * @returns System call result
 */
Napi::Value ExecuteSystemCall(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    // System calls are extremely dangerous and platform-specific
    // For security reasons, this is disabled
    Napi::Error::New(env, "Direct system calls disabled for security reasons").ThrowAsJavaScriptException();
    return Napi::Number::New(env, -1);
}

/**
 * Gets environment variable value
 * @param info - CallbackInfo containing variable name
 * @returns Variable value or null if not found
 */
Napi::Value GetEnvironmentVariable(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Variable name parameter required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string name = info[0].As<Napi::String>();
    const char* value = getenv(name.c_str());
    
    if (value) {
        return Napi::String::New(env, value);
    } else {
        return env.Null();
    }
}

/**
 * Sets environment variable
 * @param info - CallbackInfo containing variable name and value
 * @returns Success status
 */
Napi::Value SetEnvironmentVariable(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString()) {
        Napi::TypeError::New(env, "Variable name and value parameters required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    std::string name = info[0].As<Napi::String>();
    std::string value = info[1].As<Napi::String>();
    
#ifdef _WIN32
    BOOL result = SetEnvironmentVariableA(name.c_str(), value.c_str());
    return Napi::Boolean::New(env, result != 0);
#else
    return Napi::Boolean::New(env, setenv(name.c_str(), value.c_str(), 1) == 0);
#endif
}

/**
 * Gets current process ID
 * @param info - CallbackInfo (no parameters required)
 * @returns Process ID
 */
Napi::Value GetProcessId(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
#ifdef _WIN32
    return Napi::Number::New(env, GetCurrentProcessId());
#else
    return Napi::Number::New(env, getpid());
#endif
}

/**
 * Kills a process by ID
 * @param info - CallbackInfo containing process ID and optional signal
 * @returns Success status
 */
Napi::Value KillProcess(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Process ID parameter required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    uint32_t pid = info[0].As<Napi::Number>().Uint32Value();
    int signal = 15; // SIGTERM default
    
    if (info.Length() > 1 && info[1].IsNumber()) {
        signal = info[1].As<Napi::Number>().Int32Value();
    }
    
#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        return Napi::Boolean::New(env, false);
    }
    
    BOOL result = TerminateProcess(hProcess, signal);
    CloseHandle(hProcess);
    return Napi::Boolean::New(env, result != 0);
#else
    return Napi::Boolean::New(env, kill(pid, signal) == 0);
#endif
}

/**
 * Creates a new process
 * @param info - CallbackInfo containing command, args, and options
 * @returns Process ID or -1 on failure
 */
Napi::Value CreateProcess(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Command parameter required").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
    
    std::string command = info[0].As<Napi::String>();
    
#ifdef _WIN32
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    // Create the child process
    if (!CreateProcessA(NULL, const_cast<char*>(command.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return Napi::Number::New(env, -1);
    }
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return Napi::Number::New(env, pi.dwProcessId);
#else
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        exit(127);
    } else if (pid > 0) {
        // Parent process
        return Napi::Number::New(env, pid);
    } else {
        // Fork failed
        return Napi::Number::New(env, -1);
    }
#endif
}

/**
 * Gets list of running processes
 * @param info - CallbackInfo (no parameters required)
 * @returns Array of process information
 */
Napi::Value GetProcessList(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Array result = Napi::Array::New(env);
    uint32_t index = 0;
    
#ifdef _WIN32
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return result;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(hSnapshot, &pe32)) {
        do {
            Napi::Object processInfo = Napi::Object::New(env);
            processInfo.Set("pid", Napi::Number::New(env, pe32.th32ProcessID));
            processInfo.Set("name", Napi::String::New(env, pe32.szExeFile));
            processInfo.Set("cpuUsage", Napi::Number::New(env, 0.0)); // Would need additional API calls
            processInfo.Set("memoryUsage", Napi::Number::New(env, 0)); // Would need additional API calls
            
            result.Set(index++, processInfo);
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
#else
    // On Linux, read from /proc directory
    DIR* procDir = opendir("/proc");
    if (procDir == nullptr) {
        return result;
    }
    
    struct dirent* entry;
    while ((entry = readdir(procDir)) != nullptr) {
        // Check if directory name is a number (PID)
        char* endptr;
        long pid = strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0') continue;
        
        // Read process information
        std::string commPath = "/proc/" + std::string(entry->d_name) + "/comm";
        std::ifstream commFile(commPath);
        std::string processName;
        
        if (commFile.is_open()) {
            std::getline(commFile, processName);
            commFile.close();
            
            Napi::Object processInfo = Napi::Object::New(env);
            processInfo.Set("pid", Napi::Number::New(env, pid));
            processInfo.Set("name", Napi::String::New(env, processName));
            processInfo.Set("cpuUsage", Napi::Number::New(env, 0.0));
            processInfo.Set("memoryUsage", Napi::Number::New(env, 0));
            
            result.Set(index++, processInfo);
        }
    }
    
    closedir(procDir);
#endif
    
    return result;
}

}