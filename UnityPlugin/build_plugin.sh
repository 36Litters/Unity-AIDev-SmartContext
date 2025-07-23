#!/bin/bash
# Unity LLM Context Generator Plugin Build Script

set -e

echo "Building Unity LLM Context Generator Plugin..."

# Check if we're in the correct directory
if [ ! -f "../CMakeLists.txt" ]; then
    echo "Error: Please run this script from the UnityPlugin directory"
    exit 1
fi

# Create build directory
BUILD_DIR="../build"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

echo "Running CMake configuration..."
cmake .. -DBUILD_UNITY_PLUGIN=ON

echo "Building the project..."
make unity_context_core_plugin -j$(nproc)

echo "Verifying plugin build..."
if [ -f "UnityPlugin/Native/x64/unity_context_core.so" ]; then
    echo "✓ Unity plugin DLL built successfully: UnityPlugin/Native/x64/unity_context_core.so"
    
    # Check dependencies
    echo "Plugin dependencies:"
    ldd UnityPlugin/Native/x64/unity_context_core.so
    
    # Check file size
    SIZE=$(stat -f%z UnityPlugin/Native/x64/unity_context_core.so 2>/dev/null || stat -c%s UnityPlugin/Native/x64/unity_context_core.so)
    echo "Plugin size: ${SIZE} bytes"
    
else
    echo "✗ Plugin build failed - DLL not found"
    exit 1
fi

echo "✓ Unity Plugin build completed successfully!"
echo ""
echo "To use this plugin in Unity:"
echo "1. Copy the entire UnityPlugin directory to your Unity project's Packages folder"
echo "2. Unity will automatically import the package"
echo "3. Open Tools -> LLM Context Generator from Unity's menu"
echo "4. Select your Unity project's Assets directory for analysis"