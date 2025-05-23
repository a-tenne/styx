#!/bin/sh

build_types="--debug | --release | --test"

if [ "$#" -ne 1 ]; then
    echo "Specify build type: $build_types"
    exit 1
elif [ "$1" = "--debug" ]; then
    cmake_build_type="Debug"
elif [ "$1" = "--release" ]; then
    cmake_build_type="Release"
elif [ "$1" = "--test" ]; then
    cmake_build_type="Test"
else
    echo "Invalid build type: \"$1\". Valid build types: $build_types"
    exit 1
fi
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE="${cmake_build_type}" -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build
