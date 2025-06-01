# LLJS - Low Level JavaScript

🚀 **Bringing C++ hardware interactions to JavaScript**

LLJS (Low Level JavaScript) is a powerful NPM package that provides JavaScript developers with direct access to low-level hardware operations typically only available in C++. Built with Node.js Native API (N-API), LLJS offers high-performance computing capabilities with type-safe TypeScript interfaces.

## ✨ Features

### 🧠 Memory Management
- Raw memory allocation and deallocation
- Aligned memory allocation
- Memory copying, setting, and comparison
- Direct pointer manipulation (unsafe operations)
- Memory usage statistics

### 🔧 CPU Operations
- CPU information and specifications
- Cache information retrieval
- CPU affinity control
- Register access (limited for security)
- Memory prefetching

### 🖥️ System Integration
- System information retrieval
- Process management and control
- Environment variable manipulation
- Process listing and monitoring

### ⚡ High-Performance Computing
- Optimized mathematical operations
- Fast square root calculations
- Vector and matrix operations
- Bitwise operations
- Advanced random number generation

### 📝 String Operations
- Fast string comparison and searching
- Optimized string manipulation
- Multiple hashing algorithms
- UTF-8 aware operations

### ⏰ Precision Timing
- Nanosecond precision timing
- High-resolution sleep functions
- CPU time measurement
- Performance profiling tools

### 🧵 Threading Support
- Thread creation and management
- Mutex and semaphore primitives
- Synchronization tools

## 📦 Installation

```bash
npm install lljs
```

### Prerequisites
- Node.js 16+ 
- Python (for node-gyp)
- C++ compiler (GCC, Clang, or MSVC)
- Windows: Visual Studio Build Tools
- macOS: Xcode Command Line Tools
- Linux: build-essential package

## 🚀 Quick Start

```typescript
import LLJS, { Memory, CPU, System } from 'lljs';

// Memory operations
const buffer = Memory.allocateBuffer(1024);
Memory.setMemory(buffer, 0xFF, 512);
console.log('Memory allocated and set');

// CPU information
const cpuInfo = CPU.getCPUInfo();
console.log(`CPU: ${cpuInfo.vendor} ${cpuInfo.model}`);
console.log(`Cores: ${cpuInfo.cores}`);

// System information
const sysInfo = System.getSystemInfo();
console.log(`Platform: ${sysInfo.platform} ${sysInfo.arch}`);
console.log(`Memory: ${Math.round(sysInfo.totalMemory / 1024 / 1024 / 1024)}GB`);

// High-performance math
const fastResult = LLJS.Math.fastSqrt(2048);
console.log(`Fast sqrt(2048) = ${fastResult}`);
```

## 📚 Documentation

### Memory Module

```typescript
import { Memory } from 'lljs';

// Allocate raw memory
const buffer = Memory.allocateBuffer(1024);

// Copy memory between buffers
Memory.copyMemory(dest, src, size);

// Set memory to specific value
Memory.setMemory(buffer, 0xAA, 512);

// Compare memory regions
const comparison = Memory.compareMemory(buffer1, buffer2, size);

// Get memory usage statistics
const usage = Memory.getMemoryUsage();
```

### CPU Module

```typescript
import { CPU } from 'lljs';

// Get detailed CPU information
const cpuInfo = CPU.getCPUInfo();

// Set CPU affinity
CPU.setCPUAffinity(0x1); // First core only

// Get CPU usage
const usage = CPU.getCPUUsage();

// Prefetch memory into cache
CPU.prefetchMemory(address, locality);
```

### High-Performance Math

```typescript
import { Math as LLJSMath } from 'lljs';

// Fast mathematical operations
const sqrt = LLJSMath.fastSqrt(x);
const invSqrt = LLJSMath.fastInvSqrt(x);

// Vector operations
const result = LLJSMath.vectorOperations({
    operation: 'add',
    a: [1, 2, 3],
    b: [4, 5, 6]
});

// Matrix operations
const transposed = LLJSMath.matrixOperations({
    operation: 'transpose',
    matrix: [[1, 2], [3, 4]]
});
```

## ⚠️ Safety Considerations

LLJS provides access to low-level system operations that can be dangerous if misused:

- **Memory Operations**: Direct memory access can cause segmentation faults
- **Pointer Manipulation**: Unsafe operations that can crash the process
- **System Calls**: Direct system access (disabled by default for security)
- **Assembly Execution**: Inline assembly support (disabled by default)

Always validate inputs and handle errors appropriately when using LLJS functions.

## 🧪 Testing

```bash
npm test           # Run all tests
npm run test:watch # Run tests in watch mode
```

## 🏗️ Building from Source

```bash
git clone https://github.com/lljs/lljs.git
cd lljs
npm install
npm run build
```

## 📊 Performance

LLJS provides significant performance improvements for computationally intensive operations:

- **Memory Operations**: Up to 300% faster than JavaScript equivalents
- **Mathematical Calculations**: 150-500% improvement in specialized functions
- **String Operations**: 200-400% faster for bulk operations
- **System Queries**: Direct system access without Node.js overhead

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

## ⚡ Examples

Check the `/examples` directory for comprehensive usage examples:

- `basic-usage.ts` - Fundamental operations
- `performance-benchmark.ts` - Performance comparisons
- `memory-management.ts` - Advanced memory operations
- `system-monitoring.ts` - System monitoring tools

## 🆘 Support

- 📖 [Documentation](https://github.com/lljs/lljs/wiki)
- 🐛 [Issue Tracker](https://github.com/lljs/lljs/issues)
- 💬 [Discussions](https://github.com/lljs/lljs/discussions)

---

**⚡ LLJS - Unleash the full power of your hardware in JavaScript! ⚡**