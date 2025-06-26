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
            ;;
        arch)
            sudo pacman -Sy --noconfirm \
                base-devel \
                cmake \
                gcc \
                openssl \
                asio \
                nlohmann-json \
            ;;
        fedora|rhel|centos)
            sudo dnf install -y \
                gcc-c++ \
                make \
                cmake \
                openssl-devel \
                asio-devel \
                json-devel \
            ;;
        *)
            echo "Unsupported distro. Please install dependencies manually."
            ;;
    esac
}

install_dependencies


# ---------------------
# 3. Build the C++ Project
# ---------------------
echo "Building DashChain..."
mkdir build && cd build
cmake ..
make

echo "Setup complete. Run the program with: ./DashChain"


