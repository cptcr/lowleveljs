#!/usr/bin/env node
/**
 * Windows Build Fix Script v3
 * Fixes all Windows compilation issues for LLJS
 */

const fs = require('fs');
const path = require('path');

console.log('🔧 Applying Windows build fixes v3...');

// Fix 1: Fix system.cpp Windows API conflicts and syntax errors
const systemFile = path.join(__dirname, 'src/native/system.cpp');
if (fs.existsSync(systemFile)) {
    let content = fs.readFileSync(systemFile, 'utf8');
    let changed = false;
    
    // Fix the ::SetEnvironmentVariableA call
    if (content.includes('::SetEnvironmentVariableA')) {
        console.log('✅ Fixing ::SetEnvironmentVariableA call in system.cpp');
        content = content.replace(
            /::SetEnvironmentVariableA\s*\(/g,
            'SetEnvironmentVariableA('
        );
        changed = true;
    }
    
    // Fix double WIN32_LEAN_AND_MEAN definition
    if (content.includes('#define WIN32_LEAN_AND_MEAN') && !content.includes('#ifndef WIN32_LEAN_AND_MEAN')) {
        console.log('✅ Fixing double WIN32_LEAN_AND_MEAN definition in system.cpp');
        content = content.replace(
            '#define WIN32_LEAN_AND_MEAN',
            '#ifndef WIN32_LEAN_AND_MEAN\n#define WIN32_LEAN_AND_MEAN\n#endif'
        );
        changed = true;
    }
    
    if (changed) {
        fs.writeFileSync(systemFile, content);
    }
}

// Fix 2: Fix memory.cpp double definition
const memoryFile = path.join(__dirname, 'src/native/memory.cpp');
if (fs.existsSync(memoryFile)) {
    let content = fs.readFileSync(memoryFile, 'utf8');
    
    // Fix double WIN32_LEAN_AND_MEAN definition
    if (content.includes('#define WIN32_LEAN_AND_MEAN') && !content.includes('#ifndef WIN32_LEAN_AND_MEAN')) {
        console.log('✅ Fixing double WIN32_LEAN_AND_MEAN definition in memory.cpp');
        content = content.replace(
            '#define WIN32_LEAN_AND_MEAN',
            '#ifndef WIN32_LEAN_AND_MEAN\n#define WIN32_LEAN_AND_MEAN\n#endif'
        );
        fs.writeFileSync(memoryFile, content);
    }
}

// Fix 3: Apply std:: prefixes to all files
const files = [
    'src/native/memory.cpp',
    'src/native/cpu.cpp', 
    'src/native/system.cpp',
    'src/native/io.cpp',
    'src/native/threading.cpp',
    'src/native/time.cpp',
    'src/native/math.cpp',
    'src/native/string.cpp'
];

files.forEach(file => {
    const filePath = path.join(__dirname, file);
    if (fs.existsSync(filePath)) {
        let content = fs.readFileSync(filePath, 'utf8');
        let changed = false;
        
        // Replace memcpy, memset, memcmp with std:: versions where needed
        const replacements = [
            [/(?<!std::)memcpy\s*\(/g, 'std::memcpy('],
            [/(?<!std::)memset\s*\(/g, 'std::memset('],
            [/(?<!std::)memcmp\s*\(/g, 'std::memcmp(']
        ];
        
        replacements.forEach(([pattern, replacement]) => {
            if (pattern.test(content)) {
                content = content.replace(pattern, replacement);
                changed = true;
            }
        });
        
        if (changed) {
            console.log(`✅ Fixed std:: prefixes in ${file}`);
            fs.writeFileSync(filePath, content);
        }
    }
});

// Fix 4: Remove problematic compiler flags from binding.gyp
const bindingGypFile = path.join(__dirname, 'binding.gyp');
if (fs.existsSync(bindingGypFile)) {
    let bindingContent = fs.readFileSync(bindingGypFile, 'utf8');
    
    try {
        const gyp = JSON.parse(bindingContent);
        
        // Ensure Windows-specific settings are properly configured
        const windowsCondition = gyp.targets[0].conditions?.find(cond => cond[0] === "OS=='win'");
        if (windowsCondition) {
            // Update Windows condition with safer settings
            windowsCondition[1] = {
                "defines": [
                    "_CRT_SECURE_NO_WARNINGS",
                    "_WIN32_WINNT=0x0600"
                ],
                "msvs_settings": {
                    "VCCLCompilerTool": {
                        "ExceptionHandling": 1,
                        "AdditionalOptions": ["/W3"]
                    }
                }
            };
            
            console.log('✅ Updated binding.gyp Windows settings (removed problematic defines)');
            fs.writeFileSync(bindingGypFile, JSON.stringify(gyp, null, 2));
        }
    } catch (e) {
        console.log('⚠️ Could not parse binding.gyp as JSON');
    }
}

console.log('🎉 Windows build fixes v3 applied successfully!');
console.log('');
console.log('Now try running:');
console.log('  npm run clean');
console.log('  npm run build:native');