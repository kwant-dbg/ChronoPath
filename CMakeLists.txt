# CMakeLists.txt
# Specifies the project configuration and build instructions.

cmake_minimum_required(VERSION 3.10)
project(TransitRouter)

# Set the C++ standard to C++17, which is required by the just_gtfs library.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the executable and link the source files.
add_executable(transit_router main.cpp)

# Add the 'include' directory to the include path so the compiler can find just_gtfs.h.
target_include_directories(transit_router PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Recommended compiler flags for optimization and warnings.
target_compile_options(transit_router PRIVATE -O2 -Wall)

