#!/bin/bash
# Build script for Yuki-Frame v2.0

set -e

echo "===================================="
echo "Building Yuki-Frame v2.0"
echo "===================================="
echo ""

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure
echo "[1/2] Configuring..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "[2/2] Building..."
cmake --build . --parallel

echo ""
echo "===================================="
echo "Build Complete!"
echo "===================================="
echo ""
echo "Executable: build/yuki-frame"
echo ""
echo "Next steps:"
echo "  1. Copy yuki-frame.conf.example to yuki-frame.conf"
echo "  2. Edit configuration"
echo "  3. Run: ./build/yuki-frame -c yuki-frame.conf"
echo ""
