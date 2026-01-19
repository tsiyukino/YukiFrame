#!/bin/bash
# Test runner for Yuki-Frame (Linux/macOS)

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo ""
echo -e "${BLUE}=== Yuki-Frame Test Suite ===${NC}"
echo ""

BUILD_DIR="build"

if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}Build directory not found. Please build the project first.${NC}"
    echo "Run: ./build.sh"
    exit 1
fi

cd "$BUILD_DIR"

echo -e "${YELLOW}Running unit tests...${NC}"
echo ""

# Run CTest
if cmake --build . --target test_all 2>/dev/null; then
    echo ""
    echo -e "${GREEN}=== All tests passed! ===${NC}"
    echo ""
    cd ..
    exit 0
else
    echo ""
    echo -e "${YELLOW}Running tests with CTest...${NC}"
    if ctest --output-on-failure; then
        echo ""
        echo -e "${GREEN}=== All tests passed! ===${NC}"
        echo ""
        cd ..
        exit 0
    else
        echo ""
        echo -e "${RED}=== Some tests failed! ===${NC}"
        echo ""
        cd ..
        exit 1
    fi
fi
