import LLJS, { Memory, CPU, System, Time, Math as LLJSMath } from '../src/index';

// Check if native module is available
const isNativeAvailable = (() => {
  try {
    require('../build/Release/lljs.node');
    return true;
  } catch {
    return false;
  }
})();

const describeWithNative = isNativeAvailable ? describe : describe.skip;
const describeMock = isNativeAvailable ? describe.skip : describe;

describe('LLJS Module Loading', () => {
  test('should load module without errors', () => {
    expect(LLJS).toBeDefined();
    expect(Memory).toBeDefined();
    expect(CPU).toBeDefined();
    expect(System).toBeDefined();
    expect(Time).toBeDefined();
    expect(LLJSMath).toBeDefined();
  });

  test('should have all expected exports', () => {
    expect(typeof Memory.allocateBuffer).toBe('function');
    expect(typeof CPU.getCPUInfo).toBe('function');
    expect(typeof System.getSystemInfo).toBe('function');
    expect(typeof Time.getHighResTime).toBe('function');
    expect(typeof LLJSMath.fastSqrt).toBe('function');
  });
});

describeMock('LLJS Mock Mode (Native module not built)', () => {
  test('should work with mock functions', () => {
    const cpuInfo = CPU.getCPUInfo();
    expect(cpuInfo).toHaveProperty('vendor');
    expect(cpuInfo.vendor).toBe('Mock');
  });

  test('should handle memory operations in mock mode', () => {
    const usage = Memory.getMemoryUsage();
    expect(usage).toHaveProperty('rss');
    expect(typeof usage.rss).toBe('number');
  });

  test('should handle system operations in mock mode', () => {
    const sysInfo = System.getSystemInfo();
    expect(sysInfo).toHaveProperty('platform');
    expect(sysInfo.platform).toBe('mock');
  });
});

describeWithNative('LLJS Memory Module (Native)', () => {
  test('should allocate and free buffer', () => {
    const buffer = Memory.allocateBuffer(1024);
    expect(buffer).not.toBeNull();
    expect(buffer?.length).toBe(1024);
    
    if (buffer) {
      const result = Memory.freeBuffer(buffer);
      expect(result).toBe(true);
    }
  });

  test('should copy memory between buffers', () => {
    const source = Buffer.from('Hello, World!');
    const dest = Memory.allocateBuffer(20);
    
    if (dest) {
      const result = Memory.copyMemory(dest, source, source.length);
      expect(result).toBe(true);
      expect(dest.toString('utf8', 0, source.length)).toBe('Hello, World!');
    }
  });

  test('should set memory to specific value', () => {
    const buffer = Memory.allocateBuffer(10);
    
    if (buffer) {
      const result = Memory.setMemory(buffer, 0xFF, 10);
      expect(result).toBe(true);
      expect(buffer[0]).toBe(0xFF);
      expect(buffer[9]).toBe(0xFF);
    }
  });

  test('should compare memory regions', () => {
    const buffer1 = Buffer.from('test');
    const buffer2 = Buffer.from('test');
    const buffer3 = Buffer.from('different');
    
    expect(Memory.compareMemory(buffer1, buffer2, 4)).toBe(0);
    expect(Memory.compareMemory(buffer1, buffer3, 4)).not.toBe(0);
  });

  test('should get memory usage statistics', () => {
    const usage = Memory.getMemoryUsage();
    expect(usage).toHaveProperty('rss');
    expect(typeof usage.rss).toBe('number');
    expect(usage.rss).toBeGreaterThan(0);
  });

  test('should allocate aligned memory', () => {
    const buffer = Memory.alignedAlloc(1024, 16);
    expect(buffer).not.toBeNull();
    expect(buffer?.length).toBe(1024);
  });
});

describeWithNative('LLJS CPU Module (Native)', () => {
  test('should get CPU information', () => {
    const cpuInfo = CPU.getCPUInfo();
    expect(cpuInfo).toHaveProperty('vendor');
    expect(cpuInfo).toHaveProperty('model');
    expect(cpuInfo).toHaveProperty('cores');
    expect(cpuInfo).toHaveProperty('cache');
    expect(cpuInfo).toHaveProperty('features');
    expect(typeof cpuInfo.cores).toBe('number');
    expect(cpuInfo.cores).toBeGreaterThan(0);
  });

  test('should get core count', () => {
    const coreCount = CPU.getCoreCount();
    expect(typeof coreCount).toBe('number');
    expect(coreCount).toBeGreaterThan(0);
  });

  test('should get cache information', () => {
    const cache = CPU.getCacheInfo();
    expect(cache).toHaveProperty('l1d');
    expect(cache).toHaveProperty('l1i');
    expect(cache).toHaveProperty('l2');
    expect(cache).toHaveProperty('l3');
  });

  test('should get CPU usage', () => {
    const usage = CPU.getCPUUsage();
    expect(typeof usage).toBe('number');
    expect(usage).toBeGreaterThanOrEqual(0);
    expect(usage).toBeLessThanOrEqual(100);
  });

  test('should set CPU affinity', () => {
    const result = CPU.setCPUAffinity(1); // First core only
    expect(typeof result).toBe('boolean');
    // Note: Might fail on some systems due to permissions
  });

  test('should get register values', () => {
    const registers = CPU.getRegisters();
    expect(registers).toHaveProperty('eax');
    expect(registers).toHaveProperty('warning');
  });

  test('should get CPU temperature', () => {
    const temp = CPU.getCPUTemperature();
    expect(typeof temp).toBe('number');
    // Temperature can be -1 if not available
    expect(temp).toBeGreaterThanOrEqual(-1);
  });

  test('should get CPU frequency', () => {
    const freq = CPU.getCPUFrequency();
    expect(freq).toHaveProperty('base');
    expect(freq).toHaveProperty('current');
    expect(freq).toHaveProperty('max');
    expect(typeof freq.current).toBe('number');
  });
});

describeWithNative('LLJS System Module (Native)', () => {
  test('should get system information', () => {
    const sysInfo = System.getSystemInfo();
    expect(sysInfo).toHaveProperty('platform');
    expect(sysInfo).toHaveProperty('arch');
    expect(sysInfo).toHaveProperty('totalMemory');
    expect(sysInfo).toHaveProperty('freeMemory');
    expect(typeof sysInfo.totalMemory).toBe('number');
    expect(sysInfo.totalMemory).toBeGreaterThan(0);
  });

  test('should get and set environment variables', () => {
    const testVar = 'LLJS_TEST_VAR';
    const testValue = 'test_value_123';
    
    const setResult = System.setEnvironmentVariable(testVar, testValue);
    expect(setResult).toBe(true);
    
    const getValue = System.getEnvironmentVariable(testVar);
    expect(getValue).toBe(testValue);
    
    const nonExistent = System.getEnvironmentVariable('NON_EXISTENT_VAR_12345');
    expect(nonExistent).toBeNull();
  });

  test('should get process ID', () => {
    const pid = System.getProcessId();
    expect(typeof pid).toBe('number');
    expect(pid).toBeGreaterThan(0);
  });

  test('should get process list', () => {
    const processes = System.getProcessList();
    expect(Array.isArray(processes)).toBe(true);
    expect(processes.length).toBeGreaterThan(0);
    
    if (processes.length > 0) {
      const process = processes[0];
      expect(process).toHaveProperty('pid');
      expect(process).toHaveProperty('name');
      expect(typeof process.pid).toBe('number');
      expect(typeof process.name).toBe('string');
    }
  });
});

describeWithNative('LLJS Time Module (Native)', () => {
  test('should get high resolution time', () => {
    const time1 = Time.getHighResTime();
    const time2 = Time.getHighResTime();
    expect(typeof time1).toBe('number');
    expect(typeof time2).toBe('number');
    expect(time2).toBeGreaterThanOrEqual(time1);
  });

  test('should sleep for specified time', () => {
    const start = Date.now();
    Time.sleep(10); // 10ms
    const end = Date.now();
    expect(end - start).toBeGreaterThanOrEqual(9); // Allow some tolerance
  });

  test('should get timestamp', () => {
    const unixTime = Time.getTimestamp('unix');
    const isoTime = Time.getTimestamp('iso');
    const highResTime = Time.getTimestamp('high-res');
    
    expect(typeof unixTime).toBe('number');
    expect(typeof isoTime).toBe('string');
    expect(typeof highResTime).toBe('number');
  });

  test('should get CPU time', () => {
    const cpuTime = Time.getCPUTime();
    expect(typeof cpuTime).toBe('number');
    expect(cpuTime).toBeGreaterThanOrEqual(0);
  });

  test('should get thread CPU time', () => {
    const threadCpuTime = Time.getThreadCPUTime();
    expect(typeof threadCpuTime).toBe('number');
    expect(threadCpuTime).toBeGreaterThanOrEqual(0);
  });

  test('should get monotonic time', () => {
    const monotonicTime = Time.getMonotonicTime();
    expect(typeof monotonicTime).toBe('number');
    expect(monotonicTime).toBeGreaterThan(0);
  });

  test('should get timezone info', () => {
    const tzInfo = Time.getTimeZoneInfo();
    expect(tzInfo).toHaveProperty('bias');
    expect(tzInfo).toHaveProperty('standardName');
    expect(tzInfo).toHaveProperty('daylightName');
    expect(tzInfo).toHaveProperty('isDST');
  });
});

describeWithNative('LLJS Math Module (Native)', () => {
  test('should calculate fast square root', () => {
    const result = LLJSMath.fastSqrt(16);
    expect(result).toBeCloseTo(4, 5);
    
    const result2 = LLJSMath.fastSqrt(2);
    expect(result2).toBeCloseTo(Math.sqrt(2), 5);
  });

  test('should calculate fast inverse square root', () => {
    const result = LLJSMath.fastInvSqrt(16);
    expect(result).toBeCloseTo(1/4, 5);
    
    const result2 = LLJSMath.fastInvSqrt(4);
    expect(result2).toBeCloseTo(0.5, 5);
  });

  test('should perform vector operations', () => {
    const addResult = LLJSMath.vectorOperations({
      operation: 'add',
      a: [1, 2, 3],
      b: [4, 5, 6]
    });
    expect(addResult).toEqual([5, 7, 9]);
    
    const dotResult = LLJSMath.vectorOperations({
      operation: 'dot',
      a: [1, 2, 3],
      b: [4, 5, 6]
    });
    expect(dotResult).toBe(32); // 1*4 + 2*5 + 3*6
  });

  test('should perform matrix operations', () => {
    const matrix = [[1, 2], [3, 4]];
    const transposed = LLJSMath.matrixOperations({
      operation: 'transpose',
      matrix: matrix
    });
    expect(transposed).toEqual([[1, 3], [2, 4]]);
  });

  test('should perform bitwise operations', () => {
    const andResult = LLJSMath.bitwiseOperations('and', 0xFF, 0x0F);
    expect(andResult).toBe(0x0F);
    
    const orResult = LLJSMath.bitwiseOperations('or', 0xF0, 0x0F);
    expect(orResult).toBe(0xFF);
    
    const xorResult = LLJSMath.bitwiseOperations('xor', 0xFF, 0xFF);
    expect(xorResult).toBe(0x00);
  });

  test('should generate random numbers', () => {
    const randomNumbers = LLJSMath.randomNumbers(10, 0, 1, 'uniform');
    expect(randomNumbers).toHaveLength(10);
    
    randomNumbers.forEach(num => {
      expect(num).toBeGreaterThanOrEqual(0);
      expect(num).toBeLessThanOrEqual(1);
    });
  });

  test('should perform FFT', () => {
    const input = [[1, 0], [0, 0], [0, 0], [0, 0]]; // Simple input
    const result = LLJSMath.fastFourierTransform(input);
    expect(Array.isArray(result)).toBe(true);
    expect(result.length).toBeGreaterThan(0);
    
    if (result.length > 0) {
      expect(Array.isArray(result[0])).toBe(true);
      expect(result[0]).toHaveLength(2); // [real, imaginary]
    }
  });
});

describeWithNative('LLJS String Module (Native)', () => {
  test('should perform fast string comparison', () => {
    const result1 = LLJS.String.fastStringCompare('hello', 'hello');
    expect(result1).toBe(0);
    
    const result2 = LLJS.String.fastStringCompare('hello', 'world');
    expect(result2).not.toBe(0);
    
    const result3 = LLJS.String.fastStringCompare('Hello', 'hello', false);
    expect(result3).toBe(0); // Case insensitive
  });

  test('should get string length', () => {
    const length = LLJS.String.stringLength('hello');
    expect(length).toBe(5);
    
    const unicodeLength = LLJS.String.stringLength('héllo');
    expect(unicodeLength).toBe(5);
  });

  test('should concatenate strings', () => {
    const result = LLJS.String.stringConcat(['hello', ' ', 'world']);
    expect(result).toBe('hello world');
  });

  test('should search strings', () => {
    const index = LLJS.String.stringSearch('hello world', 'world');
    expect(index).toBe(6);
    
    const notFound = LLJS.String.stringSearch('hello world', 'xyz');
    expect(notFound).toBe(-1);
  });

  test('should hash strings', () => {
    const hash1 = LLJS.String.stringHash('test', 'djb2');
    const hash2 = LLJS.String.stringHash('test', 'djb2');
    const hash3 = LLJS.String.stringHash('different', 'djb2');
    
    expect(hash1).toBe(hash2); // Same string should produce same hash
    expect(hash1).not.toBe(hash3); // Different strings should produce different hashes
  });

  test('should validate strings', () => {
    const isAscii = LLJS.String.stringValidate('hello', 'ascii');
    expect(isAscii).toBe(true);
    
    const isUtf8 = LLJS.String.stringValidate('hello', 'utf8');
    expect(isUtf8).toBe(true);
    
    const sanitized = LLJS.String.stringValidate('<script>alert("xss")</script>', 'sanitize_html');
    expect(typeof sanitized).toBe('string');
    expect(sanitized).not.toContain('<script>');
  });

  test('should replace strings', () => {
    const result = LLJS.String.stringReplace('hello world', 'world', 'universe');
    expect(result).toBe('hello universe');
  });
});

describeWithNative('LLJS Integration Tests (Native)', () => {
  test('should handle memory allocation stress test', () => {
    const buffers: (Buffer | null)[] = [];
    
    // Allocate multiple buffers
    for (let i = 0; i < 10; i++) {
      const buffer = Memory.allocateBuffer(1024);
      buffers.push(buffer);
      expect(buffer).not.toBeNull();
    }
    
    // Free all buffers
    buffers.forEach(buffer => {
      if (buffer) {
        const result = Memory.freeBuffer(buffer);
        expect(result).toBe(true);
      }
    });
  });

  test('should measure performance improvement', () => {
    const iterations = 1000;
    
    // Test native sqrt vs Math.sqrt
    const start1 = Time.getHighResTime();
    for (let i = 0; i < iterations; i++) {
      Math.sqrt(i + 1);
    }
    const end1 = Time.getHighResTime();
    
    const start2 = Time.getHighResTime();
    for (let i = 0; i < iterations; i++) {
      LLJSMath.fastSqrt(i + 1);
    }
    const end2 = Time.getHighResTime();
    
    const jsTime = end1 - start1;
    const nativeTime = end2 - start2;
    
    expect(jsTime).toBeGreaterThan(0);
    expect(nativeTime).toBeGreaterThan(0);
    
    // Note: Performance comparison might vary by system
    console.log(`JS Math.sqrt: ${jsTime}ns, Native fastSqrt: ${nativeTime}ns`);
  });
});