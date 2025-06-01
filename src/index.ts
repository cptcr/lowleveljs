/**
 * LLJS - Low Level JavaScript
 * Bringing C++ hardware interactions to JavaScript
 */

// Import the native module with error handling
let native: any;
try {
  native = require('../build/Release/lljs.node');
} catch (error) {
  console.warn('LLJS native module not found. Please run "npm run build:native" first.');
  // Create a mock native object for testing without native module
  native = createMockNative();
}

/**
 * Creates a mock native object for testing when the native module is not available
 */
function createMockNative(): any {
  const mockFunction = () => {
    throw new Error('LLJS native module not compiled. Run "npm run build:native" first.');
  };
  
  return {
    // Memory functions
    allocateBuffer: mockFunction,
    freeBuffer: mockFunction,
    copyMemory: mockFunction,
    setMemory: mockFunction,
    compareMemory: mockFunction,
    getMemoryUsage: () => ({ rss: 0, peak: 0, pageFaults: 0 }),
    alignedAlloc: mockFunction,
    getPointerValue: mockFunction,
    setPointerValue: mockFunction,
    
    // CPU functions
    getCPUInfo: () => ({ vendor: 'Mock', model: 'Mock CPU', cores: 1, features: {}, cache: {} }),
    getCoreCount: () => 1,
    getCacheInfo: () => ({ l1d: 0, l1i: 0, l2: 0, l3: 0 }),
    executeAssembly: mockFunction,
    getCPUUsage: () => 0,
    setCPUAffinity: () => false,
    getRegisters: () => ({ eax: 0, ebx: 0, ecx: 0, edx: 0 }),
    prefetchMemory: () => false,
    getCPUTemperature: () => -1,
    getCPUFrequency: () => ({ base: 0, current: 0, max: 0 }),
    
    // System functions
    getSystemInfo: () => ({ platform: 'mock', arch: 'mock', version: '0.0.0', totalMemory: 0, freeMemory: 0, uptime: 0 }),
    executeSystemCall: mockFunction,
    getEnvironmentVariable: () => null,
    setEnvironmentVariable: () => false,
    getProcessId: () => 0,
    killProcess: () => false,
    createProcess: () => -1,
    getProcessList: () => [],
    
    // I/O functions
    readFile: mockFunction,
    writeFile: mockFunction,
    openFile: mockFunction,
    closeFile: mockFunction,
    seekFile: mockFunction,
    flushFile: mockFunction,
    getFileInfo: mockFunction,
    directoryOperations: mockFunction,
    
    // Threading functions
    createThread: mockFunction,
    joinThread: mockFunction,
    detachThread: mockFunction,
    getThreadId: () => 0,
    createMutex: mockFunction,
    lockMutex: mockFunction,
    unlockMutex: mockFunction,
    createSemaphore: mockFunction,
    waitSemaphore: mockFunction,
    signalSemaphore: mockFunction,
    
    // Time functions
    getHighResTime: () => Date.now() * 1000000,
    sleep: mockFunction,
    sleepMicroseconds: mockFunction,
    getTimestamp: (format: string = 'unix') => format === 'unix' ? globalThis.Math.floor(Date.now() / 1000) : new Date().toISOString(),
    createTimer: mockFunction,
    destroyTimer: mockFunction,
    getCPUTime: () => 0,
    getThreadCPUTime: () => 0,
    getMonotonicTime: () => Date.now() * 1000000,
    measureElapsed: (start: number, end: number) => end - start,
    getTimeZoneInfo: () => ({ bias: 0, standardName: 'Mock', daylightName: 'Mock', isDST: false }),
    
    // Math functions
    fastSqrt: globalThis.Math.sqrt,
    fastInvSqrt: (x: number) => 1 / globalThis.Math.sqrt(x),
    vectorOperations: mockFunction,
    matrixOperations: mockFunction,
    bitwiseOperations: mockFunction,
    randomNumbers: (count: number) => Array(count).fill(0).map(() => globalThis.Math.random()),
    fastFourierTransform: mockFunction,
    
    // String functions
    fastStringCompare: (a: string, b: string) => a.localeCompare(b),
    stringLength: (str: string) => str.length,
    stringCopy: mockFunction,
    stringConcat: (strings: string[]) => strings.join(''),
    stringSearch: (haystack: string, needle: string) => haystack.indexOf(needle),
    stringHash: () => 0,
    stringValidate: mockFunction,
    stringReplace: (str: string, search: string, replace: string) => str.replace(search, replace)
  };
}

// Type definitions for native functions
export interface MemoryUsage {
  rss: number;
  peak?: number;
  pageFaults?: number;
  userTime?: number;
  systemTime?: number;
}

export interface CPUInfo {
  vendor: string;
  model: string;
  speed: number;
  cores: number;
  features: {
    mmx: boolean;
    sse: boolean;
    sse2: boolean;
    sse3: boolean;
    ssse3: boolean;
    sse41: boolean;
    sse42: boolean;
    avx: boolean;
    fma: boolean;
  };
  cache: {
    l1d: number;
    l1i: number;
    l2: number;
    l3: number;
    lineSize: number;
    associativity: string;
  };
}

export interface SystemInfo {
  platform: string;
  arch: string;
  version: string;
  totalMemory: number;
  freeMemory: number;
  uptime: number;
}

export interface ProcessInfo {
  pid: number;
  name: string;
  cpuUsage: number;
  memoryUsage: number;
}

export interface ThreadHandle {
  id: number;
  handle: any;
}

export interface MutexHandle {
  id: number;
  handle: any;
}

export interface SemaphoreHandle {
  id: number;
  handle: any;
  count: number;
}

export interface FileHandle {
  fd: number;
  path: string;
  mode: string;
}

export interface FileInfo {
  size: number;
  created: Date;
  modified: Date;
  accessed: Date;
  isDirectory: boolean;
  permissions: number;
}

export interface CPUFrequencyInfo {
  base: number;
  current: number;
  max: number;
}

export interface TimeZoneInfo {
  bias: number;
  standardName: string;
  daylightName: string;
  isDST: boolean;
}

export interface VectorOperation {
  operation: 'add' | 'subtract' | 'multiply' | 'divide' | 'dot' | 'cross' | 'magnitude' | 'normalize';
  a: number[];
  b?: number[];
}

export interface MatrixOperation {
  operation: 'multiply' | 'transpose' | 'inverse' | 'determinant';
  matrix: number[][];
  matrix2?: number[][];
}

/**
 * Memory Management Module
 * Provides low-level memory operations
 */
export namespace Memory {
  /**
   * Allocates a raw memory buffer
   * @param size - Size in bytes to allocate
   * @returns Buffer object or null on failure
   */
  export function allocateBuffer(size: number): Buffer | null {
    return native.allocateBuffer(size);
  }

  /**
   * Frees a memory buffer
   * @param buffer - Buffer to free
   * @returns Success status
   */
  export function freeBuffer(buffer: Buffer): boolean {
    return native.freeBuffer(buffer);
  }

  /**
   * Copies memory from source to destination
   * @param dest - Destination buffer
   * @param src - Source buffer
   * @param size - Number of bytes to copy
   * @returns Success status
   */
  export function copyMemory(dest: Buffer, src: Buffer, size: number): boolean {
    return native.copyMemory(dest, src, size);
  }

  /**
   * Sets memory to a specific value
   * @param buffer - Target buffer
   * @param value - Value to set (0-255)
   * @param size - Number of bytes to set
   * @returns Success status
   */
  export function setMemory(buffer: Buffer, value: number, size: number): boolean {
    return native.setMemory(buffer, value, size);
  }

  /**
   * Compares two memory regions
   * @param buffer1 - First buffer
   * @param buffer2 - Second buffer
   * @param size - Number of bytes to compare
   * @returns Comparison result (-1, 0, 1)
   */
  export function compareMemory(buffer1: Buffer, buffer2: Buffer, size: number): number {
    return native.compareMemory(buffer1, buffer2, size);
  }

  /**
   * Gets current memory usage statistics
   * @returns Memory usage information
   */
  export function getMemoryUsage(): MemoryUsage {
    return native.getMemoryUsage();
  }

  /**
   * Allocates aligned memory
   * @param size - Size in bytes
   * @param alignment - Alignment requirement (must be power of 2)
   * @returns Aligned buffer or null on failure
   */
  export function alignedAlloc(size: number, alignment: number): Buffer | null {
    return native.alignedAlloc(size, alignment);
  }

  /**
   * Gets value at pointer address (UNSAFE)
   * @param address - Memory address
   * @param type - Data type ('int32', 'uint32', 'float', 'double')
   * @returns Value at address
   * @warning This is an unsafe operation that can cause crashes
   */
  export function getPointerValue(address: number, type: string): number | null {
    return native.getPointerValue(address, type);
  }

  /**
   * Sets value at pointer address (UNSAFE)
   * @param address - Memory address
   * @param type - Data type ('int32', 'uint32', 'float', 'double')
   * @param value - Value to set
   * @returns Success status
   * @warning This is an unsafe operation that can cause crashes
   */
  export function setPointerValue(address: number, type: string, value: number): boolean {
    return native.setPointerValue(address, type, value);
  }
}

/**
 * CPU Operations Module
 * Provides CPU-level operations and information
 */
export namespace CPU {
  /**
   * Gets detailed CPU information
   * @returns CPU information object
   */
  export function getCPUInfo(): CPUInfo {
    return native.getCPUInfo();
  }

  /**
   * Gets the number of CPU cores
   * @returns Number of logical cores
   */
  export function getCoreCount(): number {
    return native.getCoreCount();
  }

  /**
   * Gets CPU cache information
   * @returns Cache sizes for each level
   */
  export function getCacheInfo(): CPUInfo['cache'] {
    return native.getCacheInfo();
  }

  /**
   * Executes inline assembly code (x86/x64 only)
   * @param assembly - Assembly code string
   * @param inputs - Input values
   * @returns Execution result
   * @warning Extremely dangerous - can crash the process
   */
  export function executeAssembly(assembly: string, inputs?: number[]): any {
    return native.executeAssembly(assembly, inputs);
  }

  /**
   * Gets current CPU usage percentage
   * @returns CPU usage (0-100)
   */
  export function getCPUUsage(): number {
    return native.getCPUUsage();
  }

  /**
   * Sets CPU affinity for current process
   * @param mask - CPU core mask
   * @returns Success status
   */
  export function setCPUAffinity(mask: number): boolean {
    return native.setCPUAffinity(mask);
  }

  /**
   * Gets CPU register values
   * @returns Register values object
   */
  export function getRegisters(): Record<string, number> {
    return native.getRegisters();
  }

  /**
   * Prefetches memory into CPU cache
   * @param address - Memory address to prefetch
   * @param locality - Temporal locality (0-3)
   * @returns Success status
   */
  export function prefetchMemory(address: number, locality: number = 1): boolean {
    return native.prefetchMemory(address, locality);
  }

  /**
   * Gets CPU temperature (if available)
   * @returns Temperature in Celsius or -1 if unavailable
   */
  export function getCPUTemperature(): number {
    return native.getCPUTemperature();
  }

  /**
   * Gets CPU frequency information
   * @returns Object with base, current, and max frequencies
   */
  export function getCPUFrequency(): CPUFrequencyInfo {
    return native.getCPUFrequency();
  }
}

/**
 * System Operations Module
 * Provides system-level operations and information
 */
export namespace System {
  /**
   * Gets system information
   * @returns System information object
   */
  export function getSystemInfo(): SystemInfo {
    return native.getSystemInfo();
  }

  /**
   * Executes a system call
   * @param syscall - System call number
   * @param args - Arguments array
   * @returns System call result
   */
  export function executeSystemCall(syscall: number, args?: number[]): number {
    return native.executeSystemCall(syscall, args);
  }

  /**
   * Gets environment variable value
   * @param name - Variable name
   * @returns Variable value or null if not found
   */
  export function getEnvironmentVariable(name: string): string | null {
    return native.getEnvironmentVariable(name);
  }

  /**
   * Sets environment variable
   * @param name - Variable name
   * @param value - Variable value
   * @returns Success status
   */
  export function setEnvironmentVariable(name: string, value: string): boolean {
    return native.setEnvironmentVariable(name, value);
  }

  /**
   * Gets current process ID
   * @returns Process ID
   */
  export function getProcessId(): number {
    return native.getProcessId();
  }

  /**
   * Kills a process by ID
   * @param pid - Process ID to kill
   * @param signal - Signal to send (default: SIGTERM)
   * @returns Success status
   */
  export function killProcess(pid: number, signal?: number): boolean {
    return native.killProcess(pid, signal);
  }

  /**
   * Creates a new process
   * @param command - Command to execute
   * @param args - Command arguments
   * @param options - Process options
   * @returns Process ID or -1 on failure
   */
  export function createProcess(command: string, args?: string[], options?: any): number {
    return native.createProcess(command, args, options);
  }

  /**
   * Gets list of running processes
   * @returns Array of process information
   */
  export function getProcessList(): ProcessInfo[] {
    return native.getProcessList();
  }
}

/**
 * I/O Operations Module
 * Provides low-level file and I/O operations
 */
export namespace IO {
  /**
   * Reads file content
   * @param path - File path
   * @param offset - Read offset
   * @param length - Number of bytes to read
   * @returns File content buffer
   */
  export function readFile(path: string, offset?: number, length?: number): Buffer {
    return native.readFile(path, offset, length);
  }

  /**
   * Writes data to file
   * @param path - File path
   * @param data - Data to write
   * @param offset - Write offset
   * @returns Number of bytes written
   */
  export function writeFile(path: string, data: Buffer, offset?: number): number {
    return native.writeFile(path, data, offset);
  }

  /**
   * Opens a file for operations
   * @param path - File path
   * @param mode - Open mode ('r', 'w', 'a', etc.)
   * @returns File handle
   */
  export function openFile(path: string, mode: string): FileHandle {
    return native.openFile(path, mode);
  }

  /**
   * Closes a file handle
   * @param handle - File handle to close
   * @returns Success status
   */
  export function closeFile(handle: FileHandle): boolean {
    return native.closeFile(handle);
  }

  /**
   * Seeks to position in file
   * @param handle - File handle
   * @param position - Position to seek to
   * @param whence - Seek origin (0=start, 1=current, 2=end)
   * @returns New position
   */
  export function seekFile(handle: FileHandle, position: number, whence: number = 0): number {
    return native.seekFile(handle, position, whence);
  }

  /**
   * Flushes file buffers
   * @param handle - File handle
   * @returns Success status
   */
  export function flushFile(handle: FileHandle): boolean {
    return native.flushFile(handle);
  }

  /**
   * Gets file information
   * @param path - File path
   * @returns File information object
   */
  export function getFileInfo(path: string): FileInfo {
    return native.getFileInfo(path);
  }

  /**
   * Directory operations
   * @param operation - Operation type ('create', 'delete', 'list')
   * @param path - Directory path
   * @returns Operation result
   */
  export function directoryOperations(operation: string, path: string): any {
    return native.directoryOperations(operation, path);
  }
}

/**
 * Threading Module
 * Provides threading and synchronization primitives
 */
export namespace Threading {
  /**
   * Creates a new thread
   * @param func - Function to execute in thread
   * @param args - Arguments for the function
   * @returns Thread handle
   */
  export function createThread(func: Function, args?: any[]): ThreadHandle {
    return native.createThread(func, args);
  }

  /**
   * Waits for thread to complete
   * @param handle - Thread handle
   * @returns Thread exit code
   */
  export function joinThread(handle: ThreadHandle): number {
    return native.joinThread(handle);
  }

  /**
   * Detaches a thread
   * @param handle - Thread handle
   * @returns Success status
   */
  export function detachThread(handle: ThreadHandle): boolean {
    return native.detachThread(handle);
  }

  /**
   * Gets current thread ID
   * @returns Thread ID
   */
  export function getThreadId(): number {
    return native.getThreadId();
  }

  /**
   * Creates a mutex
   * @returns Mutex handle
   */
  export function createMutex(): MutexHandle {
    return native.createMutex();
  }

  /**
   * Locks a mutex
   * @param handle - Mutex handle
   * @param timeout - Timeout in milliseconds
   * @returns Success status
   */
  export function lockMutex(handle: MutexHandle, timeout?: number): boolean {
    return native.lockMutex(handle, timeout);
  }

  /**
   * Unlocks a mutex
   * @param handle - Mutex handle
   * @returns Success status
   */
  export function unlockMutex(handle: MutexHandle): boolean {
    return native.unlockMutex(handle);
  }

  /**
   * Creates a semaphore
   * @param initialCount - Initial count
   * @param maxCount - Maximum count
   * @returns Semaphore handle
   */
  export function createSemaphore(initialCount: number, maxCount: number): SemaphoreHandle {
    return native.createSemaphore(initialCount, maxCount);
  }

  /**
   * Waits on a semaphore
   * @param handle - Semaphore handle
   * @param timeout - Timeout in milliseconds
   * @returns Success status
   */
  export function waitSemaphore(handle: SemaphoreHandle, timeout?: number): boolean {
    return native.waitSemaphore(handle, timeout);
  }

  /**
   * Signals a semaphore
   * @param handle - Semaphore handle
   * @param count - Number of signals to release
   * @returns Previous count
   */
  export function signalSemaphore(handle: SemaphoreHandle, count: number = 1): number {
    return native.signalSemaphore(handle, count);
  }
}

/**
 * Time Operations Module
 * Provides high-precision timing operations
 */
export namespace Time {
  /**
   * Gets high-resolution time
   * @returns Time in nanoseconds
   */
  export function getHighResTime(): number {
    return native.getHighResTime();
  }

  /**
   * Sleeps for specified milliseconds
   * @param ms - Milliseconds to sleep
   */
  export function sleep(ms: number): void {
    return native.sleep(ms);
  }

  /**
   * Sleeps for specified microseconds
   * @param us - Microseconds to sleep
   */
  export function sleepMicroseconds(us: number): void {
    return native.sleepMicroseconds(us);
  }

  /**
   * Gets current timestamp
   * @param format - Format ('unix', 'iso', 'high-res')
   * @returns Timestamp in requested format
   */
  export function getTimestamp(format: string = 'unix'): number | string {
    return native.getTimestamp(format);
  }

  /**
   * Creates a high-precision timer
   * @param callback - Callback function
   * @param interval - Interval in microseconds
   * @returns Timer handle
   */
  export function createTimer(callback: Function, interval: number): any {
    return native.createTimer(callback, interval);
  }

  /**
   * Gets CPU time usage
   * @returns CPU time in microseconds
   */
  export function getCPUTime(): number {
    return native.getCPUTime();
  }

  /**
   * Destroys a timer
   * @param handle - Timer handle
   * @returns Success status
   */
  export function destroyTimer(handle: any): boolean {
    return native.destroyTimer(handle);
  }

  /**
   * Gets thread CPU time
   * @returns Thread CPU time in microseconds
   */
  export function getThreadCPUTime(): number {
    return native.getThreadCPUTime();
  }

  /**
   * Gets monotonic time (unaffected by system clock changes)
   * @returns Monotonic time in nanoseconds
   */
  export function getMonotonicTime(): number {
    return native.getMonotonicTime();
  }

  /**
   * Measures elapsed time between two time points
   * @param startTime - Start time in nanoseconds
   * @param endTime - End time in nanoseconds
   * @returns Elapsed time in nanoseconds
   */
  export function measureElapsed(startTime: number, endTime: number): number {
    return native.measureElapsed(startTime, endTime);
  }

  /**
   * Gets time zone information
   * @returns Time zone information object
   */
  export function getTimeZoneInfo(): TimeZoneInfo {
    return native.getTimeZoneInfo();
  }
}

/**
 * Math Operations Module
 * Provides optimized mathematical operations
 */
export namespace Math {
  /**
   * Fast square root implementation
   * @param x - Input value
   * @returns Square root of x
   */
  export function fastSqrt(x: number): number {
    return native.fastSqrt(x);
  }

  /**
   * Fast inverse square root (Quake algorithm)
   * @param x - Input value
   * @returns 1/sqrt(x)
   */
  export function fastInvSqrt(x: number): number {
    return native.fastInvSqrt(x);
  }

  /**
   * Vector operations
   * @param operation - Operation configuration
   * @returns Operation result
   */
  export function vectorOperations(operation: VectorOperation): number[] | number {
    return native.vectorOperations(operation);
  }

  /**
   * Matrix operations
   * @param operation - Operation configuration
   * @returns Operation result
   */
  export function matrixOperations(operation: MatrixOperation): number[][] | number {
    return native.matrixOperations(operation);
  }

  /**
   * Bitwise operations
   * @param operation - Operation type
   * @param a - First operand
   * @param b - Second operand
   * @returns Operation result
   */
  export function bitwiseOperations(operation: string, a: number, b?: number): number {
    return native.bitwiseOperations(operation, a, b);
  }

  /**
   * Generate random numbers
   * @param count - Number of values to generate
   * @param min - Minimum value (or mean for normal distribution)
   * @param max - Maximum value (or stddev for normal distribution)
   * @param distribution - Distribution type ('uniform', 'normal', 'exponential', 'gamma', 'poisson')
   * @returns Array of random numbers
   */
  export function randomNumbers(
    count: number, 
    min: number = 0, 
    max: number = 1, 
    distribution: string = 'uniform'
  ): number[] {
    return native.randomNumbers(count, min, max, distribution);
  }

  /**
   * Fast Fourier Transform
   * @param data - Array of complex numbers or real numbers
   * @returns FFT result as array of [real, imaginary] pairs
   */
  export function fastFourierTransform(data: number[][] | number[]): number[][] {
    return native.fastFourierTransform(data);
  }
}

/**
 * String Operations Module
 * Provides optimized string operations
 */
export namespace String {
  /**
   * Fast string comparison
   * @param str1 - First string
   * @param str2 - Second string
   * @param caseSensitive - Case sensitive comparison
   * @returns Comparison result (-1, 0, 1)
   */
  export function fastStringCompare(str1: string, str2: string, caseSensitive: boolean = true): number {
    return native.fastStringCompare(str1, str2, caseSensitive);
  }

  /**
   * Gets string length (UTF-8 aware)
   * @param str - Input string
   * @returns String length in characters
   */
  export function stringLength(str: string): number {
    return native.stringLength(str);
  }

  /**
   * Fast string copy
   * @param src - Source string
   * @param dest - Destination buffer
   * @param maxLength - Maximum length to copy
   * @returns Number of characters copied
   */
  export function stringCopy(src: string, dest: Buffer, maxLength: number): number {
    return native.stringCopy(src, dest, maxLength);
  }

  /**
   * Fast string concatenation
   * @param strings - Array of strings to concatenate
   * @returns Concatenated string
   */
  export function stringConcat(strings: string[]): string {
    return native.stringConcat(strings);
  }

  /**
   * Fast string search
   * @param haystack - String to search in
   * @param needle - String to search for
   * @param caseSensitive - Case sensitive search
   * @returns Index of first occurrence or -1
   */
  export function stringSearch(haystack: string, needle: string, caseSensitive: boolean = true): number {
    return native.stringSearch(haystack, needle, caseSensitive);
  }

  /**
   * Fast string hashing
   * @param str - String to hash
   * @param algorithm - Hash algorithm ('djb2', 'fnv1a', 'murmur3', 'crc32', 'sdbm')
   * @returns Hash value
   */
  export function stringHash(str: string, algorithm: string = 'djb2'): number {
    return native.stringHash(str, algorithm);
  }

  /**
   * Advanced string manipulation with regular expressions
   * @param string - Input string
   * @param pattern - Pattern to find
   * @param replacement - Replacement string
   * @returns Modified string
   */
  export function stringReplace(string: string, pattern: string, replacement: string): string {
    return native.stringReplace(string, pattern, replacement);
  }

  /**
   * String validation and sanitization
   * @param string - Input string
   * @param validationType - Type of validation ('utf8', 'ascii', 'sanitize_html')
   * @returns Validation result or sanitized string
   */
  export function stringValidate(string: string, validationType: string): boolean | string {
    return native.stringValidate(string, validationType);
  }
}

// Default export with all modules
export default {
  Memory,
  CPU,
  System,
  IO,
  Threading,
  Time,
  Math,
  String
};