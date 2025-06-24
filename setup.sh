#!/bin/bash

echo "DashChain Python 3.11 Environment Setup"

# Detect Linux distro
distro="unknown"
if [ -f /etc/os-release ]; then
    . /etc/os-release
    distro=$ID
fi
echo "Detected distro: $distro"

# Install Python 3.11 + dev headers
install_python_311() {
    case "$distro" in
        debian|ubuntu)
            sudo apt-get update
            sudo apt-get install -y python3.11 python3.11-dev cmake g++ openssl libssl-dev
            ;;
        arch)
            sudo pacman -Sy --noconfirm python python-pip cmake gcc openssl
            ;;
        fedora|rhel|centos)
            sudo dnf install -y python3.11 python3.11-devel cmake gcc-c++ openssl-devel
            ;;
        *)
            echo "Unsupported distro. Please install Python 3.11 and dependencies manually."
            ;;
    esac
}

# Check for python3.11
if ! command -v python3.11 &> /dev/null; then
    echo "Python 3.11 not found. Installing..."
    install_python_311
else
    echo "Python 3.11 is installed."
fi

# Patch CMakeLists.txt with 3.11 values
echo "Updating CMakeLists.txt to use Python 3.11..."
sed -i 's|set(Python3_FIND_VERSION .*|set(Python3_FIND_VERSION 3.11)|' CMakeLists.txt
sed -i 's|set(Python3_EXECUTABLE .*|set(Python3_EXECUTABLE "/usr/bin/python3.11")|' CMakeLists.txt
sed -i 's|set(Python3_INCLUDE_DIR .*|set(Python3_INCLUDE_DIR "/usr/include/python3.11")|' CMakeLists.txt
sed -i 's|set(Python3_LIBRARY .*|set(Python3_LIBRARY "/usr/lib/x86_64-linux-gnu/libpython3.11.so")|' CMakeLists.txt

# Create virtual environment
if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3.11 -m venv venv
fi

source venv/bin/activate
pip install --upgrade pip
pip install -r python/requirements.txt

# Build project
echo "Building DashChain..."
mkdir -p build && cd build
cmake .. -DPython3_EXECUTABLE=/usr/bin/python3.11
make

echo "Setup complete. Run with: ./DashChain"

