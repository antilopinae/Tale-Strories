#!/bin/bash
set -e

LIB_ROOT="."

PLATFORM=$(uname)

if [ "$PLATFORM" = "Darwin" ]; then
  OUT=Mac
  export PATH="/opt/homebrew/bin/:$PATH"
  export PATH="/usr/local/bin:$PATH"
else
  OUT=Linux
  export PATH="/usr/local/bin:$PATH"
fi

BUILD_DEBUG="$LIB_ROOT/build/$OUT/Debug"
BUILD_RELEASE="$LIB_ROOT/build/$OUT/Release"

mkdir -p "$BUILD_DEBUG"
mkdir -p "$BUILD_RELEASE"

echo "=== Building LibUE Debug ==="
cmake -S "$LIB_ROOT" -B "$BUILD_DEBUG" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$BUILD_DEBUG"

echo "=== Building LibUE Release ==="
cmake -S "$LIB_ROOT" -B "$BUILD_RELEASE" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_RELEASE"
