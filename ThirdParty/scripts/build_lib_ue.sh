#!/bin/bash
set -e

LIB_ROOT="."

PLATFORM=$(uname)

if [ "$PLATFORM" = "Darwin" ]; then
  OUT=Mac
  export PATH="/opt/homebrew/bin/:$PATH"
  export PATH="/usr/local/bin:$PATH"
  TRIPLET="arm64-osx"
else
  OUT=Linux
  export PATH="/usr/local/bin:$PATH"
  if [ "$ARCH" = "x86_64" ]; then
      TRIPLET="x64-linux"
    else
      TRIPLET="arm64-linux"
    fi
fi

BUILD_DEBUG="$LIB_ROOT/build/$OUT/Debug"
BUILD_RELEASE="$LIB_ROOT/build/$OUT/Release"

mkdir -p "$BUILD_DEBUG"
mkdir -p "$BUILD_RELEASE"

echo "=== Building LibUE Debug ==="
cmake -S "$LIB_ROOT" -B "$BUILD_DEBUG" -DCMAKE_BUILD_TYPE=Debug -DVCPKG_TARGET_TRIPLET=$TRIPLET
cmake --build "$BUILD_DEBUG"

echo "=== Building LibUE Release ==="
cmake -S "$LIB_ROOT" -B "$BUILD_RELEASE" -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=$TRIPLET
cmake --build "$BUILD_RELEASE"
