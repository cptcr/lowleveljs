"use strict";
/**
 * LLJS - Low Level JavaScript
 * Bringing C++ hardware interactions to JavaScript
 */
Object.defineProperty(exports, "__esModule", { value: true });
exports.String = exports.Math = exports.Time = exports.Threading = exports.IO = exports.System = exports.CPU = exports.Memory = void 0;
// Import the native module
const native = require('../build/Release/lljs.node');
/**
 * Memory Management Module
 * Provides low-level memory operations
 */
var Memory;
(function (Memory) {
    /**
     * Allocates a raw memory buffer
     * @param size - Size in bytes to allocate
     * @returns Buffer object or null on failure
     */
    function allocateBuffer(size) {
        return native.allocateBuffer(size);
    }
    Memory.allocateBuffer = allocateBuffer;
    /**
     * Frees a memory buffer
     * @param buffer - Buffer to free
     * @returns Success status
     */
    function freeBuffer(buffer) {
        return native.freeBuffer(buffer);
    }
    Memory.freeBuffer = freeBuffer;
    /**
     * Copies memory from source to destination
     * @param dest - Destination buffer
     * @param src - Source buffer
     * @param size - Number of bytes to copy
     * @returns Success status
     */
    function copyMemory(dest, src, size) {
        return native.copyMemory(dest, src, size);
    }
    Memory.copyMemory = copyMemory;
    /**
     * Sets memory to a specific value
     * @param buffer - Target buffer
     * @param value - Value to set (0-255)
     * @param size - Number of bytes to set
     * @returns Success status
     */
    function setMemory(buffer, value, size) {
        return native.setMemory(buffer, value, size);
    }
    Memory.setMemory = setMemory;
    /**
     * Compares two memory regions
     * @param buffer1 - First buffer
     * @param buffer2 - Second buffer
     * @param size - Number of bytes to compare
     * @returns Comparison result (-1, 0, 1)
     */
    function compareMemory(buffer1, buffer2, size) {
        return native.compareMemory(buffer1, buffer2, size);
    }
    Memory.compareMemory = compareMemory;
    /**
     * Gets current memory usage statistics
     * @returns Memory usage information
     */
    function getMemoryUsage() {
        return native.getMemoryUsage();
    }
    Memory.getMemoryUsage = getMemoryUsage;
    /**
     * Allocates aligned memory
     * @param size - Size in bytes
     * @param alignment - Alignment requirement (must be power of 2)
     * @returns Aligned buffer or null on failure
     */
    function alignedAlloc(size, alignment) {
        return native.alignedAlloc(size, alignment);
    }
    Memory.alignedAlloc = alignedAlloc;
    /**
     * Gets value at pointer address (UNSAFE)
     * @param address - Memory address
     * @param type - Data type ('int32', 'uint32', 'float', 'double')
     * @returns Value at address
     * @warning This is an unsafe operation that can cause crashes
     */
    function getPointerValue(address, type) {
        return native.getPointerValue(address, type);
    }
    Memory.getPointerValue = getPointerValue;
    /**
     * Sets value at pointer address (UNSAFE)
     * @param address - Memory address
     * @param type - Data type ('int32', 'uint32', 'float', 'double')
     * @param value - Value to set
     * @returns Success status
     * @warning This is an unsafe operation that can cause crashes
     */
    function setPointerValue(address, type, value) {
        return native.setPointerValue(address, type, value);
    }
    Memory.setPointerValue = setPointerValue;
})(Memory || (exports.Memory = Memory = {}));
/**
 * CPU Operations Module
 * Provides CPU-level operations and information
 */
var CPU;
(function (CPU) {
    /**
     * Gets detailed CPU information
     * @returns CPU information object
     */
    function getCPUInfo() {
        return native.getCPUInfo();
    }
    CPU.getCPUInfo = getCPUInfo;
    /**
     * Gets the number of CPU cores
     * @returns Number of logical cores
     */
    function getCoreCount() {
        return native.getCoreCount();
    }
    CPU.getCoreCount = getCoreCount;
    /**
     * Gets CPU cache information
     * @returns Cache sizes for each level
     */
    function getCacheInfo() {
        return native.getCacheInfo();
    }
    CPU.getCacheInfo = getCacheInfo;
    /**
     * Executes inline assembly code (x86/x64 only)
     * @param assembly - Assembly code string
     * @param inputs - Input values
     * @returns Execution result
     * @warning Extremely dangerous - can crash the process
     */
    function executeAssembly(assembly, inputs) {
        return native.executeAssembly(assembly, inputs);
    }
    CPU.executeAssembly = executeAssembly;
    /**
     * Gets current CPU usage percentage
     * @returns CPU usage (0-100)
     */
    function getCPUUsage() {
        return native.getCPUUsage();
    }
    CPU.getCPUUsage = getCPUUsage;
    /**
     * Sets CPU affinity for current process
     * @param mask - CPU core mask
     * @returns Success status
     */
    function setCPUAffinity(mask) {
        return native.setCPUAffinity(mask);
    }
    CPU.setCPUAffinity = setCPUAffinity;
    /**
     * Gets CPU register values
     * @returns Register values object
     */
    function getRegisters() {
        return native.getRegisters();
    }
    CPU.getRegisters = getRegisters;
    /**
     * Prefetches memory into CPU cache
     * @param address - Memory address to prefetch
     * @param locality - Temporal locality (0-3)
     * @returns Success status
     */
    function prefetchMemory(address, locality = 1) {
        return native.prefetchMemory(address, locality);
    }
    CPU.prefetchMemory = prefetchMemory;
    /**
     * Gets CPU temperature (if available)
     * @returns Temperature in Celsius or -1 if unavailable
     */
    function getCPUTemperature() {
        return native.getCPUTemperature();
    }
    CPU.getCPUTemperature = getCPUTemperature;
    /**
     * Gets CPU frequency information
     * @returns Object with base, current, and max frequencies
     */
    function getCPUFrequency() {
        return native.getCPUFrequency();
    }
    CPU.getCPUFrequency = getCPUFrequency;
})(CPU || (exports.CPU = CPU = {}));
/**
 * System Operations Module
 * Provides system-level operations and information
 */
var System;
(function (System) {
    /**
     * Gets system information
     * @returns System information object
     */
    function getSystemInfo() {
        return native.getSystemInfo();
    }
    System.getSystemInfo = getSystemInfo;
    /**
     * Executes a system call
     * @param syscall - System call number
     * @param args - Arguments array
     * @returns System call result
     */
    function executeSystemCall(syscall, args) {
        return native.executeSystemCall(syscall, args);
    }
    System.executeSystemCall = executeSystemCall;
    /**
     * Gets environment variable value
     * @param name - Variable name
     * @returns Variable value or null if not found
     */
    function getEnvironmentVariable(name) {
        return native.getEnvironmentVariable(name);
    }
    System.getEnvironmentVariable = getEnvironmentVariable;
    /**
     * Sets environment variable
     * @param name - Variable name
     * @param value - Variable value
     * @returns Success status
     */
    function setEnvironmentVariable(name, value) {
        return native.setEnvironmentVariable(name, value);
    }
    System.setEnvironmentVariable = setEnvironmentVariable;
    /**
     * Gets current process ID
     * @returns Process ID
     */
    function getProcessId() {
        return native.getProcessId();
    }
    System.getProcessId = getProcessId;
    /**
     * Kills a process by ID
     * @param pid - Process ID to kill
     * @param signal - Signal to send (default: SIGTERM)
     * @returns Success status
     */
    function killProcess(pid, signal) {
        return native.killProcess(pid, signal);
    }
    System.killProcess = killProcess;
    /**
     * Creates a new process
     * @param command - Command to execute
     * @param args - Command arguments
     * @param options - Process options
     * @returns Process ID or -1 on failure
     */
    function createProcess(command, args, options) {
        return native.createProcess(command, args, options);
    }
    System.createProcess = createProcess;
    /**
     * Gets list of running processes
     * @returns Array of process information
     */
    function getProcessList() {
        return native.getProcessList();
    }
    System.getProcessList = getProcessList;
})(System || (exports.System = System = {}));
/**
 * I/O Operations Module
 * Provides low-level file and I/O operations
 */
var IO;
(function (IO) {
    /**
     * Reads file content
     * @param path - File path
     * @param offset - Read offset
     * @param length - Number of bytes to read
     * @returns File content buffer
     */
    function readFile(path, offset, length) {
        return native.readFile(path, offset, length);
    }
    IO.readFile = readFile;
    /**
     * Writes data to file
     * @param path - File path
     * @param data - Data to write
     * @param offset - Write offset
     * @returns Number of bytes written
     */
    function writeFile(path, data, offset) {
        return native.writeFile(path, data, offset);
    }
    IO.writeFile = writeFile;
    /**
     * Opens a file for operations
     * @param path - File path
     * @param mode - Open mode ('r', 'w', 'a', etc.)
     * @returns File handle
     */
    function openFile(path, mode) {
        return native.openFile(path, mode);
    }
    IO.openFile = openFile;
    /**
     * Closes a file handle
     * @param handle - File handle to close
     * @returns Success status
     */
    function closeFile(handle) {
        return native.closeFile(handle);
    }
    IO.closeFile = closeFile;
    /**
     * Seeks to position in file
     * @param handle - File handle
     * @param position - Position to seek to
     * @param whence - Seek origin (0=start, 1=current, 2=end)
     * @returns New position
     */
    function seekFile(handle, position, whence = 0) {
        return native.seekFile(handle, position, whence);
    }
    IO.seekFile = seekFile;
    /**
     * Flushes file buffers
     * @param handle - File handle
     * @returns Success status
     */
    function flushFile(handle) {
        return native.flushFile(handle);
    }
    IO.flushFile = flushFile;
    /**
     * Gets file information
     * @param path - File path
     * @returns File information object
     */
    function getFileInfo(path) {
        return native.getFileInfo(path);
    }
    IO.getFileInfo = getFileInfo;
    /**
     * Directory operations
     * @param operation - Operation type ('create', 'delete', 'list')
     * @param path - Directory path
     * @returns Operation result
     */
    function directoryOperations(operation, path) {
        return native.directoryOperations(operation, path);
    }
    IO.directoryOperations = directoryOperations;
})(IO || (exports.IO = IO = {}));
/**
 * Threading Module
 * Provides threading and synchronization primitives
 */
var Threading;
(function (Threading) {
    /**
     * Creates a new thread
     * @param func - Function to execute in thread
     * @param args - Arguments for the function
     * @returns Thread handle
     */
    function createThread(func, args) {
        return native.createThread(func, args);
    }
    Threading.createThread = createThread;
    /**
     * Waits for thread to complete
     * @param handle - Thread handle
     * @returns Thread exit code
     */
    function joinThread(handle) {
        return native.joinThread(handle);
    }
    Threading.joinThread = joinThread;
    /**
     * Detaches a thread
     * @param handle - Thread handle
     * @returns Success status
     */
    function detachThread(handle) {
        return native.detachThread(handle);
    }
    Threading.detachThread = detachThread;
    /**
     * Gets current thread ID
     * @returns Thread ID
     */
    function getThreadId() {
        return native.getThreadId();
    }
    Threading.getThreadId = getThreadId;
    /**
     * Creates a mutex
     * @returns Mutex handle
     */
    function createMutex() {
        return native.createMutex();
    }
    Threading.createMutex = createMutex;
    /**
     * Locks a mutex
     * @param handle - Mutex handle
     * @param timeout - Timeout in milliseconds
     * @returns Success status
     */
    function lockMutex(handle, timeout) {
        return native.lockMutex(handle, timeout);
    }
    Threading.lockMutex = lockMutex;
    /**
     * Unlocks a mutex
     * @param handle - Mutex handle
     * @returns Success status
     */
    function unlockMutex(handle) {
        return native.unlockMutex(handle);
    }
    Threading.unlockMutex = unlockMutex;
    /**
     * Creates a semaphore
     * @param initialCount - Initial count
     * @param maxCount - Maximum count
     * @returns Semaphore handle
     */
    function createSemaphore(initialCount, maxCount) {
        return native.createSemaphore(initialCount, maxCount);
    }
    Threading.createSemaphore = createSemaphore;
    /**
     * Waits on a semaphore
     * @param handle - Semaphore handle
     * @param timeout - Timeout in milliseconds
     * @returns Success status
     */
    function waitSemaphore(handle, timeout) {
        return native.waitSemaphore(handle, timeout);
    }
    Threading.waitSemaphore = waitSemaphore;
    /**
     * Signals a semaphore
     * @param handle - Semaphore handle
     * @param count - Number of signals to release
     * @returns Previous count
     */
    function signalSemaphore(handle, count = 1) {
        return native.signalSemaphore(handle, count);
    }
    Threading.signalSemaphore = signalSemaphore;
})(Threading || (exports.Threading = Threading = {}));
/**
 * Time Operations Module
 * Provides high-precision timing operations
 */
var Time;
(function (Time) {
    /**
     * Gets high-resolution time
     * @returns Time in nanoseconds
     */
    function getHighResTime() {
        return native.getHighResTime();
    }
    Time.getHighResTime = getHighResTime;
    /**
     * Sleeps for specified milliseconds
     * @param ms - Milliseconds to sleep
     */
    function sleep(ms) {
        return native.sleep(ms);
    }
    Time.sleep = sleep;
    /**
     * Sleeps for specified microseconds
     * @param us - Microseconds to sleep
     */
    function sleepMicroseconds(us) {
        return native.sleepMicroseconds(us);
    }
    Time.sleepMicroseconds = sleepMicroseconds;
    /**
     * Gets current timestamp
     * @param format - Format ('unix', 'iso', 'high-res')
     * @returns Timestamp in requested format
     */
    function getTimestamp(format = 'unix') {
        return native.getTimestamp(format);
    }
    Time.getTimestamp = getTimestamp;
    /**
     * Creates a high-precision timer
     * @param callback - Callback function
     * @param interval - Interval in microseconds
     * @returns Timer handle
     */
    function createTimer(callback, interval) {
        return native.createTimer(callback, interval);
    }
    Time.createTimer = createTimer;
    /**
     * Gets CPU time usage
     * @returns CPU time in microseconds
     */
    function getCPUTime() {
        return native.getCPUTime();
    }
    Time.getCPUTime = getCPUTime;
    /**
     * Destroys a timer
     * @param handle - Timer handle
     * @returns Success status
     */
    function destroyTimer(handle) {
        return native.destroyTimer(handle);
    }
    Time.destroyTimer = destroyTimer;
    /**
     * Gets thread CPU time
     * @returns Thread CPU time in microseconds
     */
    function getThreadCPUTime() {
        return native.getThreadCPUTime();
    }
    Time.getThreadCPUTime = getThreadCPUTime;
    /**
     * Gets monotonic time (unaffected by system clock changes)
     * @returns Monotonic time in nanoseconds
     */
    function getMonotonicTime() {
        return native.getMonotonicTime();
    }
    Time.getMonotonicTime = getMonotonicTime;
    /**
     * Measures elapsed time between two time points
     * @param startTime - Start time in nanoseconds
     * @param endTime - End time in nanoseconds
     * @returns Elapsed time in nanoseconds
     */
    function measureElapsed(startTime, endTime) {
        return native.measureElapsed(startTime, endTime);
    }
    Time.measureElapsed = measureElapsed;
    /**
     * Gets time zone information
     * @returns Time zone information object
     */
    function getTimeZoneInfo() {
        return native.getTimeZoneInfo();
    }
    Time.getTimeZoneInfo = getTimeZoneInfo;
})(Time || (exports.Time = Time = {}));
/**
 * Math Operations Module
 * Provides optimized mathematical operations
 */
var Math;
(function (Math) {
    /**
     * Fast square root implementation
     * @param x - Input value
     * @returns Square root of x
     */
    function fastSqrt(x) {
        return native.fastSqrt(x);
    }
    Math.fastSqrt = fastSqrt;
    /**
     * Fast inverse square root (Quake algorithm)
     * @param x - Input value
     * @returns 1/sqrt(x)
     */
    function fastInvSqrt(x) {
        return native.fastInvSqrt(x);
    }
    Math.fastInvSqrt = fastInvSqrt;
    /**
     * Vector operations
     * @param operation - Operation configuration
     * @returns Operation result
     */
    function vectorOperations(operation) {
        return native.vectorOperations(operation);
    }
    Math.vectorOperations = vectorOperations;
    /**
     * Matrix operations
     * @param operation - Operation configuration
     * @returns Operation result
     */
    function matrixOperations(operation) {
        return native.matrixOperations(operation);
    }
    Math.matrixOperations = matrixOperations;
    /**
     * Bitwise operations
     * @param operation - Operation type
     * @param a - First operand
     * @param b - Second operand
     * @returns Operation result
     */
    function bitwiseOperations(operation, a, b) {
        return native.bitwiseOperations(operation, a, b);
    }
    Math.bitwiseOperations = bitwiseOperations;
    /**
     * Generate random numbers
     * @param count - Number of values to generate
     * @param min - Minimum value (or mean for normal distribution)
     * @param max - Maximum value (or stddev for normal distribution)
     * @param distribution - Distribution type ('uniform', 'normal', 'exponential', 'gamma', 'poisson')
     * @returns Array of random numbers
     */
    function randomNumbers(count, min = 0, max = 1, distribution = 'uniform') {
        return native.randomNumbers(count, min, max, distribution);
    }
    Math.randomNumbers = randomNumbers;
    /**
     * Fast Fourier Transform
     * @param data - Array of complex numbers or real numbers
     * @returns FFT result as array of [real, imaginary] pairs
     */
    function fastFourierTransform(data) {
        return native.fastFourierTransform(data);
    }
    Math.fastFourierTransform = fastFourierTransform;
})(Math || (exports.Math = Math = {}));
/**
 * String Operations Module
 * Provides optimized string operations
 */
var String;
(function (String) {
    /**
     * Fast string comparison
     * @param str1 - First string
     * @param str2 - Second string
     * @param caseSensitive - Case sensitive comparison
     * @returns Comparison result (-1, 0, 1)
     */
    function fastStringCompare(str1, str2, caseSensitive = true) {
        return native.fastStringCompare(str1, str2, caseSensitive);
    }
    String.fastStringCompare = fastStringCompare;
    /**
     * Gets string length (UTF-8 aware)
     * @param str - Input string
     * @returns String length in characters
     */
    function stringLength(str) {
        return native.stringLength(str);
    }
    String.stringLength = stringLength;
    /**
     * Fast string copy
     * @param src - Source string
     * @param dest - Destination buffer
     * @param maxLength - Maximum length to copy
     * @returns Number of characters copied
     */
    function stringCopy(src, dest, maxLength) {
        return native.stringCopy(src, dest, maxLength);
    }
    String.stringCopy = stringCopy;
    /**
     * Fast string concatenation
     * @param strings - Array of strings to concatenate
     * @returns Concatenated string
     */
    function stringConcat(strings) {
        return native.stringConcat(strings);
    }
    String.stringConcat = stringConcat;
    /**
     * Fast string search
     * @param haystack - String to search in
     * @param needle - String to search for
     * @param caseSensitive - Case sensitive search
     * @returns Index of first occurrence or -1
     */
    function stringSearch(haystack, needle, caseSensitive = true) {
        return native.stringSearch(haystack, needle, caseSensitive);
    }
    String.stringSearch = stringSearch;
    /**
     * Fast string hashing
     * @param str - String to hash
     * @param algorithm - Hash algorithm ('djb2', 'fnv1a', 'murmur3', 'crc32', 'sdbm')
     * @returns Hash value
     */
    function stringHash(str, algorithm = 'djb2') {
        return native.stringHash(str, algorithm);
    }
    String.stringHash = stringHash;
    /**
     * Advanced string manipulation with regular expressions
     * @param string - Input string
     * @param pattern - Pattern to find
     * @param replacement - Replacement string
     * @returns Modified string
     */
    function stringReplace(string, pattern, replacement) {
        return native.stringReplace(string, pattern, replacement);
    }
    String.stringReplace = stringReplace;
    /**
     * String validation and sanitization
     * @param string - Input string
     * @param validationType - Type of validation ('utf8', 'ascii', 'sanitize_html')
     * @returns Validation result or sanitized string
     */
    function stringValidate(string, validationType) {
        return native.stringValidate(string, validationType);
    }
    String.stringValidate = stringValidate;
})(String || (exports.String = String = {}));
// Default export with all modules
exports.default = {
    Memory,
    CPU,
    System,
    IO,
    Threading,
    Time,
    Math,
    String
};
//# sourceMappingURL=index.js.map