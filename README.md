# Music Visualizer

A real-time music visualization application built with C++, OpenGL, and GLFW. This application generates synthetic audio and visualizes frequency data using Fast Fourier Transform (FFT) with animated frequency bars.

## Features

- Real-time audio frequency visualization
- FFT-based frequency analysis
- Smooth animated frequency bars
- Color-coded visualization based on amplitude
- OpenGL 3.3 Core Profile rendering
- Synthetic audio generation with multiple frequency components

## Prerequisites

To build and run this application, you need:

- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake 3.10 or higher
- OpenGL 3.3 or higher
- GLFW3
- GLEW

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libglfw3-dev libglew-dev libgl1-mesa-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc-c++ cmake glfw-devel glew-devel mesa-libGL-devel
```

**macOS (with Homebrew):**
```bash
brew install cmake glfw glew
```

## Building

1. Clone the repository:
```bash
git clone https://github.com/B0dz1o/Music-viz.git
cd Music-viz
```

2. Create a build directory and compile:
```bash
mkdir build
cd build
cmake ..
make
```

3. Run the application:
```bash
./music_viz
```

## Usage

- The application will start and display a window with animated frequency bars
- The visualization shows frequency spectrum analysis in real-time
- Press `ESC` to exit the application

## Project Structure

```
Music-viz/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── src/
│   ├── main.cpp           # Main application entry point
│   ├── audio_processor.h  # Audio processing header
│   ├── audio_processor.cpp # FFT and audio generation
│   ├── visualizer.h       # Visualization header
│   ├── visualizer.cpp     # OpenGL rendering logic
│   ├── shader.h           # Shader management header
│   └── shader.cpp         # Shader compilation and management
└── shaders/
    ├── vertex.glsl        # Vertex shader
    └── fragment.glsl      # Fragment shader
```

## Technical Details

### Audio Processing
- Sample rate: 44100 Hz
- Buffer size: 2048 samples (power of 2 for FFT)
- FFT implementation: Cooley-Tukey algorithm
- Frequency bins: 1024 (half of buffer size)

### Visualization
- Number of bars: 64
- Color mapping: Dynamic based on amplitude
- Smoothing: Exponential moving average
- Update rate: ~50 FPS

### Shaders
- OpenGL Shading Language (GLSL) version 3.30
- Simple vertex and fragment shaders for rendering colored bars

## Future Enhancements

- Audio file input (WAV, MP3, etc.)
- Microphone input support
- Multiple visualization modes
- Configurable color schemes
- Full-screen mode
- Recording/export functionality

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.