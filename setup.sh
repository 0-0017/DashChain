#!/bin/bash

echo "Starting DashChain environment setup..."

# ---------------------
# 1. Detect Linux Distro
# ---------------------
distro="unknown"
if [ -f /etc/os-release ]; then
    . /etc/os-release
    distro=$ID
fi
echo "Detected distro: $distro"

# ---------------------
# 2. Install Dependencies
# ---------------------
install_dependencies() {
    echo "Installing required packages..."
    case "$distro" in
        debian|ubuntu)
            sudo apt-get update
            sudo apt-get install -y \
                build-essential \
                cmake \
                g++ \
                libssl-dev \
                libasio-dev \
                nlohmann-json3-dev \
                python3.11 \
                python3.11-dev \
                python3.11-venv
            ;;
        arch)
            sudo pacman -Sy --noconfirm \
                base-devel \
                cmake \
                gcc \
                openssl \
                asio \
                nlohmann-json \
                python \
                python-pip \
                python-virtualenv
            ;;
        fedora|rhel|centos)
            sudo dnf install -y \
                gcc-c++ \
                make \
                cmake \
                openssl-devel \
                asio-devel \
                json-devel \
                python3.11 \
                python3.11-devel \
                python3.11-virtualenv
            ;;
        *)
            echo "Unsupported distro. Please install dependencies manually."
            ;;
    esac
}

install_dependencies

# ---------------------
# 3. Patch CMakeLists.txt
# ---------------------
echo "Updating CMakeLists.txt with Python 3.11 paths..."
sed -i 's|set(Python3_FIND_VERSION .*|set(Python3_FIND_VERSION 3.11)|' CMakeLists.txt
sed -i 's|set(Python3_EXECUTABLE .*|set(Python3_EXECUTABLE "/usr/bin/python3.11")|' CMakeLists.txt
sed -i 's|set(Python3_INCLUDE_DIR .*|set(Python3_INCLUDE_DIR "/usr/include/python3.11")|' CMakeLists.txt
sed -i 's|set(Python3_LIBRARY .*|set(Python3_LIBRARY "/usr/lib/x86_64-linux-gnu/libpython3.11.so")|' CMakeLists.txt

# ---------------------
# 4. Set up Python venv
# ---------------------
if [ ! -d "venv" ]; then
    echo "Creating Python virtual environment..."
    python3.11 -m venv venv
fi

echo "Activating virtual environment and installing Python dependencies..."
source venv/bin/activate
pip install --upgrade pip
pip install -r python/requirements.txt

# ---------------------
# 5. Build the C++ Project
# ---------------------
echo "Building DashChain..."
mkdir -p build && cd build
cmake .. -DPython3_EXECUTABLE=/usr/bin/python3.11
make

echo "Setup complete. Run the program with: ./DashChain"


