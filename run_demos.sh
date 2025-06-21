#!/bin/bash

# U8SCAN Demo Runner Script
# ========================
# This script runs all demos for the U8SCAN project

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

echo -e "${BLUE}U8SCAN Demo Runner${NC}"
echo -e "${BLUE}===================${NC}"

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Build directory not found. Running rebuild script...${NC}"
    if [ -x "$PROJECT_ROOT/rebuild.sh" ]; then
        "$PROJECT_ROOT/rebuild.sh"
    else
        echo -e "${RED}Rebuild script not found or not executable: $PROJECT_ROOT/rebuild.sh${NC}"
        exit 1
    fi
fi

# Run demos directly
echo -e "${GREEN}Running U8SCAN demos...${NC}"
echo "===================="

# Run the scanning demo
echo ""
echo -e "${BLUE}--- Running u8scan_scanning_demo ---${NC}"
if "$BUILD_DIR/bin/u8scan_scanning_demo"; then
    echo -e "${GREEN}--- u8scan_scanning_demo completed successfully ---${NC}"
    scanning_demo_success=true
else
    echo -e "${RED}--- u8scan_scanning_demo failed ---${NC}"
    scanning_demo_success=false
fi

# Run the STL demo
echo ""
echo -e "${BLUE}--- Running u8scan_stl_demo ---${NC}"
if "$BUILD_DIR/bin/u8scan_stl_demo"; then
    echo -e "${GREEN}--- u8scan_stl_demo completed successfully ---${NC}"
    stl_demo_success=true
else
    echo -e "${RED}--- u8scan_stl_demo failed ---${NC}"
    stl_demo_success=false
fi

# Run the access demo
echo ""
echo -e "${BLUE}--- Running u8scan_access_demo ---${NC}"
if "$BUILD_DIR/bin/u8scan_access_demo"; then
    echo -e "${GREEN}--- u8scan_access_demo completed successfully ---${NC}"
    access_demo_success=true
else
    echo -e "${RED}--- u8scan_access_demo failed ---${NC}"
    access_demo_success=false
fi

# Summary
echo ""
echo -e "${BLUE}Demo Summary:${NC}"
echo "============="

if $scanning_demo_success && $stl_demo_success && $access_demo_success; then
    echo -e "${GREEN}✓ All demos completed successfully!${NC}"
    exit 0
else
    echo -e "${RED}✗ Some demos failed${NC}"
    exit 1
fi
