# LLJS Quick Start Guide

## 🚀 First Time Setup

### 1. Build the Native C++ Module

Before running tests or using LLJS, you need to compile the native C++ module:

```bash
# Option A: Build everything
npm run build

# Option B: Build just the native module
npm run build:native

# Option C: Windows specific (if build fails)
npm run fix-windows
```

### 2. Run Tests

```bash
# Test with mock functions (works without native build)
npm test

# Test with native module (requires successful build)
npm run test:native

# Full test suite
npm run test:full
```

## 🔧 Troubleshooting

### Build Fails on Windows?
```bash
npm run fix-windows
```

### TypeScript Compilation Issues?
```bash
npm run build:ts
```

### Clean Start?
```bash
npm run clean
npm run build
```

## 📋 Current Status

The package has two modes:

1. **Mock Mode**: Works without native compilation for basic testing
2. **Native Mode**: Full functionality with C++ acceleration

### Check Which Mode You're In:

```typescript
import { CPU } from 'lljs';

const cpuInfo = CPU.getCPUInfo();
console.log(cpuInfo.vendor); 

// "Mock" = Mock mode (native not built)
// "Intel"/"AMD"/etc = Native mode (C++ working)
```

## ⚡ Performance Comparison

Once you have the native module built, you can run performance benchmarks:

```typescript
import { Math as LLJSMath, Time } from 'lljs';

// Compare native vs JavaScript performance
const start = Time.getHighResTime();
const result = LLJSMath.fastSqrt(123456);
const end = Time.getHighResTime();

console.log(`Native sqrt took: ${end - start}ns`);
```

## 🎯 Next Steps

1. ✅ Build native module: `npm run build:native`
2. ✅ Run full tests: `npm run test:native` 
3. ✅ Try examples: See `/examples` folder
4. ✅ Check performance: Run benchmarks

## 🆘 Common Issues

| Problem | Solution |
|---------|----------|
| Tests fail with "module not found" | Run `npm run build:native` first |
| Windows build errors | Run `npm run fix-windows` |
| Permission errors | Run as Administrator (Windows) or use `sudo` (Linux) |
| Node.js version issues | Use Node.js 16+ |
| Python not found | Install Python 3.x |

## 📞 Support

If you're still having issues:
1. Check the error messages carefully
2. Look at SETUP.md for detailed instructions
3. Open an issue on GitHub with your error log