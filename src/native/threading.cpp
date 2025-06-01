#include "headers/lljs.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <map>
#include <memory>
#include <functional>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>
#endif

namespace LLJS::Threading {

// Global counters for handle management
static std::atomic<uint64_t> threadCounter{0};
static std::atomic<uint64_t> mutexCounter{0};
static std::atomic<uint64_t> semaphoreCounter{0};

// Thread management structures
struct ThreadInfo {
    std::unique_ptr<std::thread> thread;
    std::atomic<bool> running{true};
    int exitCode{0};
    uint64_t id;
};

struct MutexInfo {
    std::unique_ptr<std::mutex> mutex;
    std::unique_ptr<std::recursive_mutex> recursiveMutex;
    uint64_t id;
    bool isRecursive{false};
};

struct SemaphoreInfo {
    #ifdef _WIN32
    HANDLE semaphore;
    #else
    sem_t* semaphore;
    #endif
    uint64_t id;
    int maxCount;
    std::atomic<int> currentCount;
};

// Global storage for handles
static std::map<uint64_t, std::unique_ptr<ThreadInfo>> threads;
static std::map<uint64_t, std::unique_ptr<MutexInfo>> mutexes;
static std::map<uint64_t, std::unique_ptr<SemaphoreInfo>> semaphores;
static std::mutex globalMutex;

/**
 * Creates a new thread
 * @param info - CallbackInfo containing function and arguments
 * @returns Thread handle object
 */
Napi::Value CreateThread(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsFunction()) {
        Napi::TypeError::New(env, "Function parameter required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Function func = info[0].As<Napi::Function>();
    
    // Create thread info
    auto threadInfo = std::make_unique<ThreadInfo>();
    threadInfo->id = ++threadCounter;
    
    // Create the actual thread
    try {
        threadInfo->thread = std::make_unique<std::thread>([func, env]() {
            // Call the JavaScript function in the thread
            // Note: This is a simplified implementation
            // In a real implementation, you'd need proper V8 isolate handling
            try {
                func.Call(env.Global(), {});
            } catch (...) {
                // Handle exceptions
            }
        });
        
        uint64_t threadId = threadInfo->id;
        
        {
            std::lock_guard<std::mutex> lock(globalMutex);
            threads[threadId] = std::move(threadInfo);
        }
        
        // Return thread handle
        Napi::Object handle = Napi::Object::New(env);
        handle.Set("id", Napi::Number::New(env, static_cast<double>(threadId)));
        handle.Set("handle", Napi::External<void>::New(env, nullptr));
        
        return handle;
    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Failed to create thread: ") + e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

/**
 * Waits for thread to complete
 * @param info - CallbackInfo containing thread handle
 * @returns Thread exit code
 */
Napi::Value JoinThread(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Thread handle object required").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
    
    Napi::Object handle = info[0].As<Napi::Object>();
    uint64_t threadId = static_cast<uint64_t>(handle.Get("id").As<Napi::Number>().DoubleValue());
    
    std::lock_guard<std::mutex> lock(globalMutex);
    auto it = threads.find(threadId);
    if (it == threads.end()) {
        Napi::Error::New(env, "Invalid thread handle").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
    
    try {
        if (it->second->thread && it->second->thread->joinable()) {
            it->second->thread->join();
        }
        
        int exitCode = it->second->exitCode;
        threads.erase(it);
        
        return Napi::Number::New(env, exitCode);
    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Failed to join thread: ") + e.what()).ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
}

/**
 * Detaches a thread
 * @param info - CallbackInfo containing thread handle
 * @returns Success status
 */
Napi::Value DetachThread(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Thread handle object required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    Napi::Object handle = info[0].As<Napi::Object>();
    uint64_t threadId = static_cast<uint64_t>(handle.Get("id").As<Napi::Number>().DoubleValue());
    
    std::lock_guard<std::mutex> lock(globalMutex);
    auto it = threads.find(threadId);
    if (it == threads.end()) {
        return Napi::Boolean::New(env, false);
    }
    
    try {
        if (it->second->thread && it->second->thread->joinable()) {
            it->second->thread->detach();
        }
        
        threads.erase(it);
        return Napi::Boolean::New(env, true);
    } catch (const std::exception& e) {
        return Napi::Boolean::New(env, false);
    }
}

/**
 * Gets current thread ID
 * @param info - CallbackInfo (no parameters required)
 * @returns Thread ID
 */
Napi::Value GetThreadId(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
#ifdef _WIN32
    DWORD threadId = GetCurrentThreadId();
    return Napi::Number::New(env, static_cast<double>(threadId));
#else
    pid_t threadId = syscall(SYS_gettid);
    return Napi::Number::New(env, static_cast<double>(threadId));
#endif
}

/**
 * Creates a mutex
 * @param info - CallbackInfo with optional recursive parameter
 * @returns Mutex handle object
 */
Napi::Value CreateMutex(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    bool recursive = false;
    if (info.Length() > 0 && info[0].IsBoolean()) {
        recursive = info[0].As<Napi::Boolean>();
    }
    
    try {
        auto mutexInfo = std::make_unique<MutexInfo>();
        mutexInfo->id = ++mutexCounter;
        mutexInfo->isRecursive = recursive;
        
        if (recursive) {
            mutexInfo->recursiveMutex = std::make_unique<std::recursive_mutex>();
        } else {
            mutexInfo->mutex = std::make_unique<std::mutex>();
        }
        
        uint64_t mutexId = mutexInfo->id;
        
        {
            std::lock_guard<std::mutex> lock(globalMutex);
            mutexes[mutexId] = std::move(mutexInfo);
        }
        
        Napi::Object handle = Napi::Object::New(env);
        handle.Set("id", Napi::Number::New(env, static_cast<double>(mutexId)));
        handle.Set("handle", Napi::External<void>::New(env, nullptr));
        
        return handle;
    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Failed to create mutex: ") + e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

/**
 * Locks a mutex
 * @param info - CallbackInfo containing mutex handle and optional timeout
 * @returns Success status
 */
Napi::Value LockMutex(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Mutex handle object required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    Napi::Object handle = info[0].As<Napi::Object>();
    uint64_t mutexId = static_cast<uint64_t>(handle.Get("id").As<Napi::Number>().DoubleValue());
    
    int timeout = -1; // Infinite timeout by default
    if (info.Length() > 1 && info[1].IsNumber()) {
        timeout = info[1].As<Napi::Number>().Int32Value();
    }
    
    std::lock_guard<std::mutex> lock(globalMutex);
    auto it = mutexes.find(mutexId);
    if (it == mutexes.end()) {
        return Napi::Boolean::New(env, false);
    }
    
    try {
        if (timeout == -1) {
            // Blocking lock
            if (it->second->isRecursive) {
                it->second->recursiveMutex->lock();
            } else {
                it->second->mutex->lock();
            }
            return Napi::Boolean::New(env, true);
        } else {
            // Timed lock
            auto timeoutDuration = std::chrono::milliseconds(timeout);
            bool success;
            
            if (it->second->isRecursive) {
                success = it->second->recursiveMutex->try_lock_for(timeoutDuration);
            } else {
                success = it->second->mutex->try_lock_for(timeoutDuration);
            }
            
            return Napi::Boolean::New(env, success);
        }
    } catch (const std::exception& e) {
        return Napi::Boolean::New(env, false);
    }
}

/**
 * Unlocks a mutex
 * @param info - CallbackInfo containing mutex handle
 * @returns Success status
 */
Napi::Value UnlockMutex(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Mutex handle object required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    Napi::Object handle = info[0].As<Napi::Object>();
    uint64_t mutexId = static_cast<uint64_t>(handle.Get("id").As<Napi::Number>().DoubleValue());
    
    std::lock_guard<std::mutex> lock(globalMutex);
    auto it = mutexes.find(mutexId);
    if (it == mutexes.end()) {
        return Napi::Boolean::New(env, false);
    }
    
    try {
        if (it->second->isRecursive) {
            it->second->recursiveMutex->unlock();
        } else {
            it->second->mutex->unlock();
        }
        
        return Napi::Boolean::New(env, true);
    } catch (const std::exception& e) {
        return Napi::Boolean::New(env, false);
    }
}

/**
 * Creates a semaphore
 * @param info - CallbackInfo containing initial count and max count
 * @returns Semaphore handle object
 */
Napi::Value CreateSemaphore(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Initial count and max count parameters required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    int initialCount = info[0].As<Napi::Number>().Int32Value();
    int maxCount = info[1].As<Napi::Number>().Int32Value();
    
    if (initialCount < 0 || maxCount <= 0 || initialCount > maxCount) {
        Napi::TypeError::New(env, "Invalid semaphore parameters").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    try {
        auto semInfo = std::make_unique<SemaphoreInfo>();
        semInfo->id = ++semaphoreCounter;
        semInfo->maxCount = maxCount;
        semInfo->currentCount = initialCount;
        
        #ifdef _WIN32
        semInfo->semaphore = CreateSemaphoreA(NULL, initialCount, maxCount, NULL);
        if (semInfo->semaphore == NULL) {
            Napi::Error::New(env, "Failed to create semaphore").ThrowAsJavaScriptException();
            return env.Null();
        }
        #else
        semInfo->semaphore = sem_open(nullptr, O_CREAT, 0644, initialCount);
        if (semInfo->semaphore == SEM_FAILED) {
            Napi::Error::New(env, "Failed to create semaphore").ThrowAsJavaScriptException();
            return env.Null();
        }
        #endif
        
        uint64_t semId = semInfo->id;
        
        {
            std::lock_guard<std::mutex> lock(globalMutex);
            semaphores[semId] = std::move(semInfo);
        }
        
        Napi::Object handle = Napi::Object::New(env);
        handle.Set("id", Napi::Number::New(env, static_cast<double>(semId)));
        handle.Set("handle", Napi::External<void>::New(env, nullptr));
        handle.Set("count", Napi::Number::New(env, initialCount));
        
        return handle;
    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Failed to create semaphore: ") + e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

/**
 * Waits on a semaphore
 * @param info - CallbackInfo containing semaphore handle and optional timeout
 * @returns Success status
 */
Napi::Value WaitSemaphore(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Semaphore handle object required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    Napi::Object handle = info[0].As<Napi::Object>();
    uint64_t semId = static_cast<uint64_t>(handle.Get("id").As<Napi::Number>().DoubleValue());
    
    int timeout = -1; // Infinite timeout by default
    if (info.Length() > 1 && info[1].IsNumber()) {
        timeout = info[1].As<Napi::Number>().Int32Value();
    }
    
    std::lock_guard<std::mutex> lock(globalMutex);
    auto it = semaphores.find(semId);
    if (it == semaphores.end()) {
        return Napi::Boolean::New(env, false);
    }
    
    try {
        bool success;
        
        if (timeout == -1) {
            // Blocking wait
            #ifdef _WIN32
            DWORD result = WaitForSingleObject(it->second->semaphore, INFINITE);
            success = (result == WAIT_OBJECT_0);
            #else
            success = (sem_wait(it->second->semaphore) == 0);
            #endif
        } else {
            // Timed wait
            #ifdef _WIN32
            DWORD result = WaitForSingleObject(it->second->semaphore, timeout);
            success = (result == WAIT_OBJECT_0);
            #else
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += timeout / 1000;
            ts.tv_nsec += (timeout % 1000) * 1000000;
            success = (sem_timedwait(it->second->semaphore, &ts) == 0);
            #endif
        }
        
        if (success) {
            it->second->currentCount--;
        }
        
        return Napi::Boolean::New(env, success);
    } catch (const std::exception& e) {
        return Napi::Boolean::New(env, false);
    }
}

/**
 * Signals a semaphore
 * @param info - CallbackInfo containing semaphore handle and release count
 * @returns Previous count
 */
Napi::Value SignalSemaphore(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Semaphore handle object required").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
    
    Napi::Object handle = info[0].As<Napi::Object>();
    uint64_t semId = static_cast<uint64_t>(handle.Get("id").As<Napi::Number>().DoubleValue());
    
    int count = 1; // Default release count
    if (info.Length() > 1 && info[1].IsNumber()) {
        count = info[1].As<Napi::Number>().Int32Value();
    }
    
    if (count <= 0) {
        return Napi::Number::New(env, -1);
    }
    
    std::lock_guard<std::mutex> lock(globalMutex);
    auto it = semaphores.find(semId);
    if (it == semaphores.end()) {
        return Napi::Number::New(env, -1);
    }
    
    try {
        int previousCount = it->second->currentCount.load();
        
        // Check if we can release without exceeding max count
        if (previousCount + count > it->second->maxCount) {
            return Napi::Number::New(env, -1);
        }
        
        // Release the semaphore
        #ifdef _WIN32
        if (!ReleaseSemaphore(it->second->semaphore, count, NULL)) {
            return Napi::Number::New(env, -1);
        }
        #else
        for (int i = 0; i < count; i++) {
            if (sem_post(it->second->semaphore) != 0) {
                return Napi::Number::New(env, -1);
            }
        }
        #endif
        
        it->second->currentCount += count;
        
        return Napi::Number::New(env, previousCount);
    } catch (const std::exception& e) {
        return Napi::Number::New(env, -1);
    }
}

}