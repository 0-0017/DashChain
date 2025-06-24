cmake_minimum_required(VERSION 3.25) # 3.31
project(DashChain)

set(CMAKE_CXX_STANDARD 20)
set(Python3_ROOT_DIR "/usr")
set(Python3_FIND_STRATEGY LOCATION)
set(Python3_FIND_VERSION 3.13)
set(Python3_INCLUDE_DIR "/usr/include/python3.13")
set(Python3_LIBRARY "/usr/lib/libpython3.13.so")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")

# Find OpenSSL
find_package(OpenSSL REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# Add include directories
include_directories(${Python3_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/OLC_NET)

file(GLOB SOURCES "*.cpp")
add_executable(DashChain main.cpp ${SOURCES})

# Link OpenSSL and libcurl to your project
target_link_libraries(DashChain OpenSSL::Crypto ${Python3_LIBRARIES})
