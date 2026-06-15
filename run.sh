#!/bin/bash
set -e # Exit on error

GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Ensure we are in the script's directory
OCTOPI_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$OCTOPI_DIR"

echo -e "${BLUE}=== 0. Checking system dependencies ===${NC}"
# Check if vala is installed
if ! command -v valac &> /dev/null; then
    echo -e "${BLUE}Installing vala (required to compile)...${NC}"
    sudo pacman -S --noconfirm vala
fi

echo -e "${BLUE}=== 1. Checking dependencies (alpm_octopi_utils) ===${NC}"
# Check if the library headers directory exists in the system
if [ ! -d "/usr/include/alpm_octopi_utils" ]; then
    echo -e "${BLUE}The alpm_octopi_utils library is not installed. Downloading and installing...${NC}"
    
    # Move out of the octopi folder temporarily to clone the dependency
    cd ..
    
    if [ ! -d "alpm_octopi_utils" ]; then
        git clone https://github.com/aarnt/alpm_octopi_utils.git
    else
        echo "The alpm_octopi_utils directory already exists, updating..."
        cd alpm_octopi_utils && git pull && cd ..
    fi
    
    cd alpm_octopi_utils
    rm -rf build
    mkdir -p build
    cd build
    cmake -G "Unix Makefiles" .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
    make -j$(nproc)
    
    echo -e "${GREEN}Your password (sudo) is required to install alpm_octopi_utils in the system:/usr${NC}"
    sudo make install
    
    # Return to the octopi directory
    cd "$OCTOPI_DIR"
else
    echo -e "${GREEN}✓ Dependency alpm_octopi_utils is already installed.${NC}"
fi

echo -e "${BLUE}=== 2. Configuring Octopi build ===${NC}"
if [ ! -f "build/Makefile" ]; then
    echo -e "${BLUE}Configuring CMake for the first time...${NC}"
    mkdir -p build
    cd build
    cmake -G "Unix Makefiles" .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
    cd "$OCTOPI_DIR"
else
    echo -e "${GREEN}✓ Project already configured with CMake.${NC}"
fi

echo -e "${BLUE}=== 3. Compiling local changes ===${NC}"
cd build
make -j$(nproc)

echo -e "${GREEN}=== 4. Running Octopi ===${NC}"
./octopi "$@"
