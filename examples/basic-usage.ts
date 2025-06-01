/**
 * Basic LLJS Usage Examples
 * Demonstrates fundamental low-level operations
 */

import LLJS, { Memory, CPU, System, Time, Math as LLJSMath, String as LLJSString } from '../src/index';

console.log('🚀 LLJS Basic Usage Examples\n');

// Memory Management Examples
console.log('📦 Memory Management:');
console.log('=====================');

// Allocate raw memory buffer
const buffer = Memory.allocateBuffer(1024);
if (buffer) {
    console.log(`✅ Allocated ${buffer.length} bytes of raw memory`);
    
    // Set memory to specific pattern
    Memory.setMemory(buffer, 0xAA, 512);
    console.log(`✅ Set first 512 bytes to 0xAA pattern`);
    
    // Get memory usage
    const usage = Memory.getMemoryUsage();
    console.log(`📊 Current memory usage: ${Math.round(usage.rss / 1024 / 1024)}MB RSS`);
    
    // Free the buffer
    Memory.freeBuffer(buffer);
    console.log('✅ Memory freed\n');
}

// CPU Information and Operations
console.log('🔧 CPU Operations:');
console.log('==================');

const cpuInfo = CPU.getCPUInfo();
console.log(`💻 CPU: ${cpuInfo.vendor} ${cpuInfo.model}`);
console.log(`🔢 Cores: ${cpuInfo.cores}`);
console.log(`🗄️ Cache: L1D=${cpuInfo.cache.l1d/1024}KB, L2=${cpuInfo.cache.l2/1024}KB, L3=${cpuInfo.cache.l3/1024/1024}MB`);

const cpuUsage = CPU.getCPUUsage();
console.log(`📈 CPU Usage: ${cpuUsage.toFixed(1)}%`);

// Try to set CPU affinity (might require elevated permissions)
try {
    const affinitySet = CPU.setCPUAffinity(0x1); // First core only
    console.log(`⚙️ CPU Affinity set: ${affinitySet ? 'Success' : 'Failed'}\n`);
} catch (error) {
    console.log('⚠️ CPU Affinity: Requires elevated permissions\n');
}

// System Information
console.log('🖥️ System Information:');
console.log('======================');

const sysInfo = System.getSystemInfo();
console.log(`🏷️ Platform: ${sysInfo.platform} ${sysInfo.arch}`);
console.log(`💾 Total Memory: ${Math.round(sysInfo.totalMemory / 1024 / 1024 / 1024)}GB`);
console.log(`🆓 Free Memory: ${Math.round(sysInfo.freeMemory / 1024 / 1024 / 1024)}GB`);
console.log(`⏱️ Uptime: ${Math.round(sysInfo.uptime / 3600)}h`);

const pid = System.getProcessId();
console.log(`🆔 Current Process ID: ${pid}`);

// Environment variables
System.setEnvironmentVariable('LLJS_DEMO', 'active');
const envValue = System.getEnvironmentVariable('LLJS_DEMO');
console.log(`🌍 Environment variable LLJS_DEMO: ${envValue}\n`);

// High-Performance Math Operations
console.log('🧮 High-Performance Math:');
console.log('=========================');

// Fast square root comparison
const testValue = 2048;
const start1 = Time.getHighResTime();
const standardSqrt = Math.sqrt(testValue);
const time1 = Time.getHighResTime() - start1;

const start2 = Time.getHighResTime();
const fastSqrt = LLJSMath.fastSqrt(testValue);
const time2 = Time.getHighResTime() - start2;

console.log(`📐 Standard sqrt(${testValue}): ${standardSqrt.toFixed(6)} (${time1}ns)`);
console.log(`⚡ Fast sqrt(${testValue}): ${fastSqrt.toFixed(6)} (${time2}ns)`);
console.log(`🚀 Speed improvement: ${((time1 - time2) / time1 * 100).toFixed(1)}%`);

// Vector operations
const vector1 = [1, 2, 3, 4];
const vector2 = [5, 6, 7, 8];

const vectorSum = LLJSMath.vectorOperations({
    operation: 'add',
    a: vector1,
    b: vector2
});

const dotProduct = LLJSMath.vectorOperations({
    operation: 'dot',
    a: vector1,
    b: vector2
});

console.log(`➕ Vector addition: [${vector1}] + [${vector2}] = [${vectorSum}]`);
console.log(`⚫ Dot product: ${dotProduct}`);

// Matrix operations
const matrix = [[1, 2], [3, 4]];
const transposed = LLJSMath.matrixOperations({
    operation: 'transpose',
    matrix: matrix
});

console.log(`🔄 Matrix transpose: [[${matrix[0]}], [${matrix[1]}]] → [[${(transposed as number[][])[0]}], [${(transposed as number[][])[1]}]]`);

// Bitwise operations
const bitwiseResult = LLJSMath.bitwiseOperations('xor', 0xFF, 0xAA);
console.log(`🔀 Bitwise XOR: 0xFF ⊕ 0xAA = 0x${bitwiseResult.toString(16).toUpperCase()}\n`);

// String Operations
console.log('📝 Optimized String Operations:');
console.log('===============================');

const testString = 'Hello, Low-Level JavaScript World!';
const searchTerm = 'Level';

const stringLength = LLJSString.stringLength(testString);
const searchIndex = LLJSString.stringSearch(testString, searchTerm, false);
const stringHash = LLJSString.stringHash(testString, 'djb2');

console.log(`📏 String length: "${testString}" = ${stringLength} characters`);
console.log(`🔍 Search "${searchTerm}": found at index ${searchIndex}`);
console.log(`#️⃣ String hash (DJB2): ${stringHash}`);

// String comparison performance
const string1 = 'Performance Test String A';
const string2 = 'Performance Test String B';

const start3 = Time.getHighResTime();
const standardCompare = string1.localeCompare(string2);
const time3 = Time.getHighResTime() - start3;

const start4 = Time.getHighResTime();
const fastCompare = LLJSString.fastStringCompare(string1, string2);
const time4 = Time.getHighResTime() - start4;

console.log(`📊 Standard compare: ${standardCompare} (${time3}ns)`);
console.log(`⚡ Fast compare: ${fastCompare} (${time4}ns)`);
console.log(`🚀 Speed improvement: ${((time3 - time4) / time3 * 100).toFixed(1)}%\n`);

// Time Precision Demonstration
console.log('⏰ High-Precision Timing:');
console.log('=========================');

const highResStart = Time.getHighResTime();
Time.sleepMicroseconds(1000); // 1ms = 1000μs
const highResEnd = Time.getHighResTime();

console.log(`⏱️ High-resolution sleep test: ${(highResEnd - highResStart)} nanoseconds`);
console.log(`🎯 Target: 1,000,000ns (1ms), Actual: ${(highResEnd - highResStart)}ns`);
console.log(`📏 Precision: ±${Math.abs(1000000 - (highResEnd - highResStart))}ns\n`);

// Random Number Generation
console.log('🎲 Advanced Random Number Generation:');
console.log('=====================================');

const uniformRandom = LLJSMath.randomNumbers(5, 0, 100, 'uniform');
const normalRandom = LLJSMath.randomNumbers(5, 50, 15, 'normal');

console.log(`🎯 Uniform random (0-100): [${uniformRandom.map(n => n.toFixed(2)).join(', ')}]`);
console.log(`📊 Normal random (μ=50, σ=15): [${normalRandom.map(n => n.toFixed(2)).join(', ')}]\n`);

// Process Information
console.log('🔍 Process Information:');
console.log('======================');

try {
    const processes = System.getProcessList();
    console.log(`📋 Total running processes: ${processes.length}`);
    console.log('🔝 First 5 processes:');
    
    processes.slice(0, 5).forEach((proc, i) => {
        console.log(`  ${i + 1}. PID: ${proc.pid}, Name: "${proc.name}"`);
    });
} catch (error) {
    console.log('⚠️ Process listing requires elevated permissions');
}

console.log('\n✨ LLJS Demo completed! Low-level operations executed successfully.');
console.log('⚡ Experience the power of C++ performance in JavaScript! ⚡');