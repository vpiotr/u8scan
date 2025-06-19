#!/bin/bash

# U8SCAN Test Runner Script
# ========================
# This script runs all tests for the U8SCAN project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"

echo -e "${BLUE}U8SCAN Test Runner${NC}"
echo -e "${BLUE}===================${NC}"

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Build directory not found. Running build script...${NC}"
    if [ -x "$PROJECT_ROOT/build.sh" ]; then
        "$PROJECT_ROOT/build.sh"
    else
        echo -e "${RED}Build script not found or not executable: $PROJECT_ROOT/build.sh${NC}"
        exit 1
    fi
fi

# Check if test binaries exist
U8SCAN_SCANNING_TEST_BIN="$BUILD_DIR/bin/u8scan_scanning_test"
U8SCAN_STL_TEST_BIN="$BUILD_DIR/bin/u8scan_stl_test"
U8SCAN_EMOJI_TEST_BIN="$BUILD_DIR/bin/u8scan_emoji_test"
U8SCAN_COPY_TEST_BIN="$BUILD_DIR/bin/u8scan_copy_test"

if [ ! -x "$U8SCAN_SCANNING_TEST_BIN" ] || [ ! -x "$U8SCAN_STL_TEST_BIN" ] || [ ! -x "$U8SCAN_EMOJI_TEST_BIN" ] || [ ! -x "$U8SCAN_COPY_TEST_BIN" ]; then
    echo -e "${RED}Test binaries not found or not executable:${NC}"
    [ ! -x "$U8SCAN_SCANNING_TEST_BIN" ] && echo -e "${RED}- $U8SCAN_SCANNING_TEST_BIN${NC}"
    [ ! -x "$U8SCAN_STL_TEST_BIN" ] && echo -e "${RED}- $U8SCAN_STL_TEST_BIN${NC}"
    [ ! -x "$U8SCAN_EMOJI_TEST_BIN" ] && echo -e "${RED}- $U8SCAN_EMOJI_TEST_BIN${NC}"
    [ ! -x "$U8SCAN_COPY_TEST_BIN" ] && echo -e "${RED}- $U8SCAN_COPY_TEST_BIN${NC}"
    echo -e "${YELLOW}Try running the build script first: ./build.sh${NC}"
    exit 1
fi

# Run the tests
echo -e "${GREEN}Running U8SCAN tests...${NC}"
echo "====================="
echo ""

# Execute the test binaries
echo -e "${BLUE}Running U8Scan Scanning Tests:${NC}"
"$U8SCAN_SCANNING_TEST_BIN"
scanning_exit_code=$?

echo ""
echo -e "${BLUE}Running U8Scan STL Tests:${NC}"
"$U8SCAN_STL_TEST_BIN"
stl_exit_code=$?

echo ""
echo -e "${BLUE}Running U8Scan Emoji Tests:${NC}"
"$U8SCAN_EMOJI_TEST_BIN"
emoji_exit_code=$?

echo ""
echo -e "${BLUE}Running U8Scan Copy Tests:${NC}"
"$U8SCAN_COPY_TEST_BIN"
copy_exit_code=$?

# Check exit codes
if [ $scanning_exit_code -eq 0 ] && [ $stl_exit_code -eq 0 ] && [ $emoji_exit_code -eq 0 ] && [ $copy_exit_code -eq 0 ]; then
    exit_code=0
else
    exit_code=1
fi

echo ""
if [ $exit_code -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed successfully!${NC}"
else
    echo -e "${RED}✗ Some tests failed (exit code: $exit_code)${NC}"
fi

exit $exit_code
