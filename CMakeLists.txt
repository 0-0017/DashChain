cmake_minimum_required(VERSION 3.31) # 3.25
project(DashChain)

set(CMAKE_CXX_STANDARD 20)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/OLC_NET)

file(GLOB SOURCES "*.cpp")
add_executable(DashChain main.cpp ${SOURCES})

# Link OpenSSL and libcurl to your project
target_link_libraries(DashChain OpenSSL::Crypto)
