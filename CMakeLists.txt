
cmake_minimum_required(VERSION 3.14)
project(ChronoPath)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.10.5
)

# Protobuf & GTFS Bindings: 
find_package(Protobuf REQUIRED)

FetchContent_Declare(
    gtfs-realtime-bindings
    GIT_REPOSITORY https://github.com/google/gtfs-realtime-bindings.git
    GIT_TAG v0.0.7
)

FetchContent_MakeAvailable(cpr gtfs-realtime-bindings)

set(GTFS_REALTIME_PROTO ${gtfs-realtime-bindings_SOURCE_DIR}/gtfs-realtime.proto)
set(GTFS_REALTIME_SRC ${CMAKE_CURRENT_BINARY_DIR}/gtfs-realtime.pb.cc)
set(GTFS_REALTIME_HDR ${CMAKE_CURRENT_BINARY_DIR}/gtfs-realtime.pb.h)
add_custom_command(
    OUTPUT ${GTFS_REALTIME_SRC} ${GTFS_REALTIME_HDR}
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} -I${gtfs-realtime-bindings_SOURCE_DIR} ${GTFS_REALTIME_PROTO}
    DEPENDS ${GTFS_REALTIME_PROTO}
)
add_library(gtfs_realtime_proto ${GTFS_REALTIME_SRC} ${GTFS_REALTIME_HDR})
target_link_libraries(gtfs_realtime_proto PUBLIC ${PROTOBUF_LIBRARY})
target_include_directories(gtfs_realtime_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})


add_executable(chronopath main.cpp)

target_link_libraries(chronopath PRIVATE
    cpr::cpr              # For HTTP requests
    gtfs_realtime_proto   # For parsing GTFS-Realtime
)

target_include_directories(chronopath PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(chronopath PRIVATE -O2 -Wall)

if (UNIX)
    target_link_libraries(chronopath PRIVATE pthread)
endif()
