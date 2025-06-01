#include "headers/lljs.h"
#include <fstream>
#include <filesystem>
#include <map>
#include <vector>
#include <cstring>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
// Prevent Windows API name conflicts
#ifdef ReadFile
#undef ReadFile
#endif
#ifdef WriteFile
#undef WriteFile
#endif
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#endif

namespace LLJS::IO {

// Global file handle management
static std::map<int, std::string> openFiles;
static int nextFileDescriptor = 1000;

/**
 * Reads file content with low-level system calls
 * @param info - CallbackInfo containing path, offset, length parameters
 * @returns File content buffer
 */
Napi::Value ReadFile(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "File path parameter required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string path = info[0].As<Napi::String>();
    size_t offset = 0;
    size_t length = 0;
    
    if (info.Length() > 1 && info[1].IsNumber()) {
        offset = info[1].As<Napi::Number>().Uint32Value();
    }
    
    if (info.Length() > 2 && info[2].IsNumber()) {
        length = info[2].As<Napi::Number>().Uint32Value();
    }
    
#ifdef _WIN32
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, 
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        Napi::Error::New(env, "Failed to open file").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        Napi::Error::New(env, "Failed to get file size").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (length == 0) {
        length = static_cast<size_t>(fileSize.QuadPart) - offset;
    }
    
    auto buffer = Napi::Buffer<uint8_t>::New(env, length);
    
    if (offset > 0) {
        LARGE_INTEGER movePtr;
        movePtr.QuadPart = offset;
        SetFilePointerEx(hFile, movePtr, NULL, FILE_BEGIN);
    }
    
    DWORD bytesRead;
    if (!::ReadFile(hFile, buffer.Data(), static_cast<DWORD>(length), &bytesRead, NULL)) {
        CloseHandle(hFile);
        Napi::Error::New(env, "Failed to read file").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    CloseHandle(hFile);
#else
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        Napi::Error::New(env, "Failed to open file").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    struct stat fileStat;
    if (fstat(fd, &fileStat) == -1) {
        close(fd);
        Napi::Error::New(env, "Failed to get file stats").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (length == 0) {
        length = fileStat.st_size - offset;
    }
    
    auto buffer = Napi::Buffer<uint8_t>::New(env, length);
    
    if (offset > 0) {
        lseek(fd, offset, SEEK_SET);
    }
    
    ssize_t bytesRead = read(fd, buffer.Data(), length);
    close(fd);
    
    if (bytesRead == -1) {
        Napi::Error::New(env, "Failed to read file").ThrowAsJavaScriptException();
        return env.Null();
    }
#endif
    
    return buffer;
}

/**
 * Writes data to file with low-level system calls
 * @param info - CallbackInfo containing path, data, offset parameters
 * @returns Number of bytes written
 */
Napi::Value WriteFile(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsBuffer()) {
        Napi::TypeError::New(env, "File path and data buffer required").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
    
    std::string path = info[0].As<Napi::String>();
    Napi::Buffer<uint8_t> data = info[1].As<Napi::Buffer<uint8_t>>();
    size_t offset = 0;
    
    if (info.Length() > 2 && info[2].IsNumber()) {
        offset = info[2].As<Napi::Number>().Uint32Value();
    }
    
#ifdef _WIN32
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, 
                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        Napi::Error::New(env, "Failed to create/open file").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
    
    if (offset > 0) {
        LARGE_INTEGER movePtr;
        movePtr.QuadPart = offset;
        SetFilePointerEx(hFile, movePtr, NULL, FILE_BEGIN);
    }
    
    DWORD bytesWritten;
    if (!::WriteFile(hFile, data.Data(), static_cast<DWORD>(data.Length()), &bytesWritten, NULL)) {
        CloseHandle(hFile);
        Napi::Error::New(env, "Failed to write file").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
    
    CloseHandle(hFile);
    return Napi::Number::New(env, bytesWritten);
#else
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        Napi::Error::New(env, "Failed to create/open file").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
    
    if (offset > 0) {
        lseek(fd, offset, SEEK_SET);
    }
    
    ssize_t bytesWritten = write(fd, data.Data(), data.Length());
    close(fd);
    
    return Napi::Number::New(env, bytesWritten);
#endif
}

/**
 * Opens a file for operations
 * @param info - CallbackInfo containing path and mode parameters
 * @returns File handle object
 */
Napi::Value OpenFile(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString()) {
        Napi::TypeError::New(env, "File path and mode required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string path = info[0].As<Napi::String>();
    std::string mode = info[1].As<Napi::String>();
    
    int flags = 0;
    
#ifdef _WIN32
    DWORD access = 0;
    DWORD creation = 0;
    
    if (mode.find('r') != std::string::npos) access |= GENERIC_READ;
    if (mode.find('w') != std::string::npos) {
        access |= GENERIC_WRITE;
        creation = CREATE_ALWAYS;
    }
    if (mode.find('a') != std::string::npos) {
        access |= GENERIC_WRITE;
        creation = OPEN_ALWAYS;
    }
    
    HANDLE hFile = CreateFileA(path.c_str(), access, FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        Napi::Error::New(env, "Failed to open file").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    int fd = nextFileDescriptor++;
    openFiles[fd] = path;
    
    Napi::Object handle = Napi::Object::New(env);
    handle.Set("fd", Napi::Number::New(env, fd));
    handle.Set("path", Napi::String::New(env, path));
    handle.Set("mode", Napi::String::New(env, mode));
    handle.Set("_handle", Napi::External<void>::New(env, hFile));
    
    return handle;
#else
    if (mode.find('r') != std::string::npos && mode.find('w') != std::string::npos) {
        flags = O_RDWR;
    } else if (mode.find('w') != std::string::npos) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (mode.find('a') != std::string::npos) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    } else {
        flags = O_RDONLY;
    }
    
    int fd = open(path.c_str(), flags, 0644);
    if (fd == -1) {
        Napi::Error::New(env, "Failed to open file").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    openFiles[fd] = path;
    
    Napi::Object handle = Napi::Object::New(env);
    handle.Set("fd", Napi::Number::New(env, fd));
    handle.Set("path", Napi::String::New(env, path));
    handle.Set("mode", Napi::String::New(env, mode));
    
    return handle;
#endif
}

/**
 * Closes a file handle
 * @param info - CallbackInfo containing file handle
 * @returns Success status
 */
Napi::Value CloseFile(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "File handle object required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    Napi::Object handle = info[0].As<Napi::Object>();
    int fd = handle.Get("fd").As<Napi::Number>().Int32Value();
    
#ifdef _WIN32
    if (handle.Has("_handle")) {
        HANDLE hFile = static_cast<HANDLE>(handle.Get("_handle").As<Napi::External<void>>().Data());
        CloseHandle(hFile);
    }
#else
    close(fd);
#endif
    
    openFiles.erase(fd);
    return Napi::Boolean::New(env, true);
}

/**
 * Seeks to position in file
 * @param info - CallbackInfo containing handle, position, whence parameters
 * @returns New position
 */
Napi::Value SeekFile(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsObject() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "File handle and position required").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }
    
    Napi::Object handle = info[0].As<Napi::Object>();
    long position = info[1].As<Napi::Number>().Int64Value();
    int whence = SEEK_SET;
    
    if (info.Length() > 2 && info[2].IsNumber()) {
        whence = info[2].As<Napi::Number>().Int32Value();
    }
    
#ifdef _WIN32
    HANDLE hFile = static_cast<HANDLE>(handle.Get("_handle").As<Napi::External<void>>().Data());
    LARGE_INTEGER movePtr, newPtr;
    movePtr.QuadPart = position;
    
    DWORD moveMethod = FILE_BEGIN;
    if (whence == SEEK_CUR) moveMethod = FILE_CURRENT;
    else if (whence == SEEK_END) moveMethod = FILE_END;
    
    if (!SetFilePointerEx(hFile, movePtr, &newPtr, moveMethod)) {
        return Napi::Number::New(env, -1);
    }
    
    return Napi::Number::New(env, static_cast<double>(newPtr.QuadPart));
#else
    int fd = handle.Get("fd").As<Napi::Number>().Int32Value();
    off_t result = lseek(fd, position, whence);
    return Napi::Number::New(env, static_cast<double>(result));
#endif
}

/**
 * Flushes file buffers
 * @param info - CallbackInfo containing file handle
 * @returns Success status
 */
Napi::Value FlushFile(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "File handle object required").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }
    
    Napi::Object handle = info[0].As<Napi::Object>();
    
#ifdef _WIN32
    HANDLE hFile = static_cast<HANDLE>(handle.Get("_handle").As<Napi::External<void>>().Data());
    return Napi::Boolean::New(env, FlushFileBuffers(hFile) != 0);
#else
    int fd = handle.Get("fd").As<Napi::Number>().Int32Value();
    return Napi::Boolean::New(env, fsync(fd) == 0);
#endif
}

/**
 * Gets file information
 * @param info - CallbackInfo containing file path
 * @returns File information object
 */
Napi::Value GetFileInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "File path parameter required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string path = info[0].As<Napi::String>();
    Napi::Object result = Napi::Object::New(env);
    
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fileData;
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &fileData)) {
        Napi::Error::New(env, "Failed to get file information").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    LARGE_INTEGER fileSize;
    fileSize.LowPart = fileData.nFileSizeLow;
    fileSize.HighPart = fileData.nFileSizeHigh;
    
    result.Set("size", Napi::Number::New(env, static_cast<double>(fileSize.QuadPart)));
    result.Set("isDirectory", Napi::Boolean::New(env, 
              (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0));
    result.Set("permissions", Napi::Number::New(env, fileData.dwFileAttributes));
    
    // Convert FILETIME to JavaScript Date
    auto convertFileTime = [&](const FILETIME& ft) -> Napi::Date {
        ULARGE_INTEGER ull;
        ull.LowPart = ft.dwLowDateTime;
        ull.HighPart = ft.dwHighDateTime;
        // Convert from Windows epoch (1601) to Unix epoch (1970)
        uint64_t unixTime = (ull.QuadPart / 10000) - 11644473600000ULL;
        return Napi::Date::New(env, static_cast<double>(unixTime));
    };
    
    result.Set("created", convertFileTime(fileData.ftCreationTime));
    result.Set("modified", convertFileTime(fileData.ftLastWriteTime));
    result.Set("accessed", convertFileTime(fileData.ftLastAccessTime));
#else
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == -1) {
        Napi::Error::New(env, "Failed to get file information").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    result.Set("size", Napi::Number::New(env, static_cast<double>(fileStat.st_size)));
    result.Set("isDirectory", Napi::Boolean::New(env, S_ISDIR(fileStat.st_mode)));
    result.Set("permissions", Napi::Number::New(env, fileStat.st_mode & 0777));
    
    // Convert time_t to JavaScript Date (milliseconds)
    result.Set("created", Napi::Date::New(env, static_cast<double>(fileStat.st_ctime) * 1000));
    result.Set("modified", Napi::Date::New(env, static_cast<double>(fileStat.st_mtime) * 1000));
    result.Set("accessed", Napi::Date::New(env, static_cast<double>(fileStat.st_atime) * 1000));
#endif
    
    return result;
}

/**
 * Directory operations (create, delete, list)
 * @param info - CallbackInfo containing operation and path parameters
 * @returns Operation result
 */
Napi::Value DirectoryOperations(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString()) {
        Napi::TypeError::New(env, "Operation and path parameters required").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string operation = info[0].As<Napi::String>();
    std::string path = info[1].As<Napi::String>();
    
    if (operation == "create") {
#ifdef _WIN32
        return Napi::Boolean::New(env, CreateDirectoryA(path.c_str(), NULL) != 0);
#else
        return Napi::Boolean::New(env, mkdir(path.c_str(), 0755) == 0);
#endif
    } else if (operation == "delete") {
#ifdef _WIN32
        return Napi::Boolean::New(env, RemoveDirectoryA(path.c_str()) != 0);
#else
        return Napi::Boolean::New(env, rmdir(path.c_str()) == 0);
#endif
    } else if (operation == "list") {
        Napi::Array result = Napi::Array::New(env);
        uint32_t index = 0;
        
#ifdef _WIN32
        WIN32_FIND_DATAA findData;
        std::string searchPath = path + "\\*";
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                    Napi::Object entry = Napi::Object::New(env);
                    entry.Set("name", Napi::String::New(env, findData.cFileName));
                    entry.Set("isDirectory", Napi::Boolean::New(env, 
                              (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0));
                    result.Set(index++, entry);
                }
            } while (FindNextFileA(hFind, &findData));
            FindClose(hFind);
        }
#else
        DIR* dir = opendir(path.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    Napi::Object entryObj = Napi::Object::New(env);
                    entryObj.Set("name", Napi::String::New(env, entry->d_name));
                    entryObj.Set("isDirectory", Napi::Boolean::New(env, entry->d_type == DT_DIR));
                    result.Set(index++, entryObj);
                }
            }
            closedir(dir);
        }
#endif
        
        return result;
    }
    
    Napi::TypeError::New(env, "Invalid operation").ThrowAsJavaScriptException();
    return env.Null();
}

}