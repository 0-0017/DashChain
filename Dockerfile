FROM ubuntu:22.04

# Install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libasio-dev \
    nlohmann-json-dev \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Create build directory and compile
RUN mkdir build && cd build && cmake .. && make

# Run the compiled binary
CMD ["./build/DashChain"]
