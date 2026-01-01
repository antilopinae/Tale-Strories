cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/cmake")

if (APPLE)
    list(APPEND CMAKE_IGNORE_PATH "/opt/homebrew")
endif ()

include(CCache OPTIONAL)
include(NoInSourceBuilds OPTIONAL)

option(USE_VCPKG_TOOLCHAIN "Use vcpkg toolchain file" ON)

if (USE_VCPKG_TOOLCHAIN)
    # set(VCPKG_OVERLAY_PORTS "${CMAKE_SOURCE_DIR}/vcpkg_ports")

    if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Vcpkg toolchain file")
    endif ()
endif ()

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(APPLE)
        set(VCPKG_TARGET_TRIPLET "arm64-osx")
    else()
        set(VCPKG_TARGET_TRIPLET "arm64-linux")
    endif()
endif()

if (WIN32 AND MINGW)
    set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "vcpkg triplet" FORCE)
elseif (WIN32 AND MSVC)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg triplet" FORCE)
endif ()

project(DedicatedServer VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

get_target_property(PROTOC_EXE protobuf::protoc LOCATION)
get_target_property(GRPC_CPP_PLUGIN_EXE gRPC::grpc_cpp_plugin LOCATION)

# Генерируем файлы из прото
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../Api/proto/game.proto")
set(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Api/proto")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

add_custom_command(
        OUTPUT
        "${PROTO_GEN_DIR}/game.pb.cc"
        "${PROTO_GEN_DIR}/game.pb.h"
        "${PROTO_GEN_DIR}/game.grpc.pb.cc"
        "${PROTO_GEN_DIR}/game.grpc.pb.h"
        COMMAND ${PROTOC_EXE}
        ARGS -I "${CMAKE_CURRENT_SOURCE_DIR}/../Api/proto/"
        --cpp_out ${PROTO_GEN_DIR}
        --grpc_out ${PROTO_GEN_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXE}
        ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating gRPC and protobuf files from ${PROTO_FILE}"
)

add_executable(game-server
        src/main.cpp
        ${PROTO_GEN_DIR}/game.pb.cc
        ${PROTO_GEN_DIR}/game.grpc.pb.cc
)

target_include_directories(game-server PUBLIC ${PROTO_GEN_DIR})
target_link_libraries(game-server PUBLIC gRPC::grpc++ protobuf::libprotobuf)