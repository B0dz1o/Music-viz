#!/bin/bash
# Build script for Music Visualizer

set -e

echo "Building Music Visualizer..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
cmake ..

# Build with all available cores
make -j$(nproc)

echo ""
echo "Build complete! Executable: build/music_viz"
echo "Run with: ./build/music_viz"
