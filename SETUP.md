# LLJS Setup Instructions

## 🔧 Prerequisites

### Windows
1. **Node.js 16+**: Download from [nodejs.org](https://nodejs.org/)
2. **Visual Studio 2022** (Community Edition is fine):
   - Install with "Desktop development with C++" workload
   - Include Windows 10/11 SDK
3. **Python 3.x**: For node-gyp (usually comes with Node.js)

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y build-essential python3 python3-pip nodejs npm
sudo apt install -y libc6-dev linux-headers-$(uname -r)
```

### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Node.js via Homebrew
brew install node
```

## 📦 Installation Steps

### 1. Clone and Setup
```bash
git clone <your-repo-url>
cd lljs
npm run setup
```

### 2. Build Native Module
```bash
npm run build:native
```

### 3. Build TypeScript
```bash
npm run build:ts
```

### 4. Run Tests
```bash
npm test
```

## 🛠️ Development Setup

### VS Code Configuration
1. Install C++ Extension Pack
2. The project includes `.vscode/c_cpp_properties.json` for IntelliSense
3. Modify paths if your SDK/Visual Studio is in different location

### Environment Variables (Windows)
```cmd
set GYP_MSVS_VERSION=2022
set npm_config_msvs_version=2022
```

### Common Issues

#### "cannot find napi.h"
```bash
# Install node-addon-api dependency
npm install node-addon-api

# Rebuild with debug info
npm run rebuild -- --debug
```

#### Windows: "MSBuild not found"
```bash
# Set Visual Studio version
npm config set msvs_version 2022

# Alternative: use specific VS path
npm run build:native -- --msvs_version=2022
```

#### Linux: "Permission denied"
```bash
# Fix npm permissions
sudo chown -R $(whoami) ~/.npm
sudo chown -R $(whoami) ~/.config
```

## 🔍 Debugging

### Enable Debug Build
```bash
npm run build:native -- --debug
```

### View Build Logs
```bash
npm run build:native -- --verbose
```

### Test Individual Modules
```bash
# Test specific module
npm test -- --testNamePattern="Memory"
```

## 📋 Build Verification

After successful build, verify installation:

```javascript
const LLJS = require('./dist/index.js');

// Test basic functionality
const buffer = LLJS.Memory.allocateBuffer(1024);
console.log('Buffer allocated:', buffer ? 'Success' : 'Failed');

const cpuInfo = LLJS.CPU.getCPUInfo();
console.log('CPU Info:', cpuInfo.vendor, cpuInfo.model);
```

## 🚀 Production Build

```bash
# Clean rebuild for production
npm run clean
npm run build
npm test

# Package for distribution
npm pack
```

## 📝 Notes

- The package uses node-gyp for native compilation
- IntelliSense may show errors until after first successful build
- Some features require elevated permissions (CPU affinity, process management)
- Temperature and frequency monitoring may not work on all systems

## 🆘 Support

If you encounter issues:
1. Check Node.js and npm versions: `node --version && npm --version`
2. Verify build tools: `npm config get msvs_version`
3. Clean and rebuild: `npm run clean && npm run rebuild`
4. Check issue tracker for similar problems