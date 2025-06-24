#!/bin/bash

echo "Setting up DashChain environment..."

# --- Detect Linux Distro ---
distro="unknown"
if [ -f /etc/os-release ]; then
    . /etc/os-release
    distro=$ID
fi

echo "Detected distro: $distro"

# --- Dependency Installer ---
install_python_dev() {
    case "$distro" in
        debian|ubuntu)
            echo "Installing dependencies via apt..."
            sudo apt-get update
            sudo apt-get install -y python3.13 python3.13-dev cmake g++ openssl libssl-dev
            ;;
        arch)
            echo "Installing dependencies via pacman..."
            sudo pacman -Sy --noconfirm python3.13 python3.13-devel cmake gcc openssl
            ;;
        fedora|rhel|centos)
            echo "Installing dependencies via dnf..."
            sudo dnf install -y python3.13 python3.13-devel cmake gcc-c++ openssl-devel
            ;;
        *)
            echo "Unsupported distro detected. Please install Python 3.13 and dev tools manually."
            ;;
    esac
}

# --- Check Python Version ---
PY_VER=$(python3 --version 2>/dev/null | grep -oP '\d+\.\d+')
if [[ "$PY_VER" != "3.13" ]]; then
    echo "❗ Python 3.13 not detected. Attempting to install..."
    install_python_dev
else
    echo "Python 3.13 already installed."
fi

# --- Patch CMakeLists.txt ---
echo "Patching CMakeLists.txt for Python 3.13..."
sed -i 's|set(Python3_FIND_VERSION .*|set(Python3_FIND_VERSION 3.13)|' CMakeLists.txt
sed -i 's|set(Python3_INCLUDE_DIR .*|set(Python3_INCLUDE_DIR "/usr/include/python3.13")|' CMakeLists.txt
sed -i 's|set(Python3_LIBRARY .*|set(Python3_LIBRARY "/usr/lib/x86_64-linux-gnu/libpython3.13.so")|' CMakeLists.txt

# --- Create Virtual Environment ---
if [ -d "venv" ]; then
    echo "♻️  Virtual environment already exists. Reusing..."
else
    echo "Creating Python virtual environment..."
    python3.13 -m venv venv
fi

# --- Activate Venv and Install Python Packages ---
source venv/bin/activate
echo "Installing Python dependencies..."
pip install --upgrade pip
pip install -r python/requirements.txt

# --- Build C++ Project ---
echo "Building DashChain from source..."
mkdir -p build && cd build
cmake .. -DPython3_EXECUTABLE=/usr/bin/python3.13
make

echo "Setup complete. Run your program with: ./DashChain"
