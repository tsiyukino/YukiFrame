#!/bin/bash
# Build script for Yuki-Frame (Unix/Linux)

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Yuki-Frame Build Script ===${NC}"
echo ""

# Parse arguments
BUILD_TYPE="${1:-Release}"
BUILD_DIR="build"

echo -e "${YELLOW}Build type: $BUILD_TYPE${NC}"
echo ""

# Create build directory
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure
echo -e "${YELLOW}Configuring...${NC}"
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" .. || {
    echo -e "${RED}Configuration failed!${NC}"
    exit 1
}

# Build
echo -e "${YELLOW}Building...${NC}"
cmake --build . -- -j$(nproc 2>/dev/null || echo 4) || {
    echo -e "${RED}Build failed!${NC}"
    exit 1
}

echo ""
echo -e "${GREEN}=== Build successful! ===${NC}"
echo ""
echo "Executable: $BUILD_DIR/yuki-frame"
echo ""
echo "To run:"
echo "  cd $BUILD_DIR && ./yuki-frame ../yuki-frame.conf.example"
echo ""
echo "To install:"
echo "  cd $BUILD_DIR && sudo cmake --install ."
echo ""
