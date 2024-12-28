#!/bin/bash

# store CWD, cd to the directory where this script is located
CWD=$(pwd)
cd "$(dirname "$0")"

# check if CMake is installed
if ! [ -x "$(command -v cmake)" ]; then
  echo "Error: CMake is not installed." >&2
  exit 1
fi

echo "Building InTCPtor..."

# build using CMake
mkdir -p build
cd build
cmake .. >/dev/null 2>&1
make >/dev/null 2>&1

echo "Exporting InTCPtor to PATH..."

# append the build directory to PATH
export PATH=$PATH:$(pwd)

echo "Done."

# cd back to original directory
cd $CWD

bash
