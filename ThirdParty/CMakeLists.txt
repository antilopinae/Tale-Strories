cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (APPLE)
    list(APPEND CMAKE_IGNORE_PATH "/opt/homebrew")
endif ()

include(CCache OPTIONAL)
include(NoInSourceBuilds OPTIONAL)

option(USE_VCPKG_TOOLCHAIN "Use vcpkg toolchain file" ON)

if (USE_VCPKG_TOOLCHAIN)
    # set(VCPKG_OVERLAY_PORTS "${CMAKE_SOURCE_DIR}/vcpkg_ports")

    if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Vcpkg toolchain file")
    endif ()
endif ()

if (WIN32 AND MINGW)
    set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "vcpkg triplet" FORCE)
elseif (WIN32 AND MSVC)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg triplet" FORCE)
endif ()

# project setup
project(LibUE VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# dependencies
# find_package(PkgConfig REQUIRED)
find_package(fmt REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../Api/proto/game.proto")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

get_target_property(PROTOC_EXE protobuf::protoc LOCATION)
get_target_property(GRPC_CPP_PLUGIN_EXE gRPC::grpc_cpp_plugin LOCATION)

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

add_library(LibUE STATIC
        src/lib_ue.cpp
        "${PROTO_GEN_DIR}/game.pb.cc"
        "${PROTO_GEN_DIR}/game.grpc.pb.cc"
)

target_link_libraries(LibUE PUBLIC fmt::fmt gRPC::grpc++ protobuf::libprotobuf)

target_include_directories(LibUE PUBLIC include ${PROTO_GEN_DIR})

set(OUT_LIB_PATH $<TARGET_FILE:LibUE>)

if(APPLE)
    set(OUT_LIB_PATH $<TARGET_FILE:LibUE>)

    # 1. Находим ПУТЬ к папке либ vcpkg
    # Это будет что-то вроде ThirdParty/vcpkg_installed/arm64-osx-static/lib
    set(VCPKG_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/lib")

    # 2. Собираем список ВТОРОСТЕПЕННЫХ библиотек (Abseil, gRPC core, и т.д.)
    # Мы берем ВСЕ файлы .a из vcpkg, чтобы ничего не забыть
    file(GLOB_RECURSE ALL_VCPKG_LIBS "${VCPKG_LIB_DIR}/*.a")

    # 3. Объединяем
    add_custom_command(TARGET LibUE POST_BUILD
            COMMAND echo "Bundling ALL static dependencies into libLibUE.a..."
            # Важно: удаляем libz.a из списка, если он там есть, так как Unreal имеет свой zlib
            # (это предотвратит ошибки дублирования символов)
            COMMAND libtool -static -o "${OUT_LIB_PATH}.tmp" "${OUT_LIB_PATH}" ${ALL_VCPKG_LIBS}
            COMMAND mv "${OUT_LIB_PATH}.tmp" "${OUT_LIB_PATH}"
            COMMENT "Merging dependencies using libtool"
            VERBATIM
    )
endif()
