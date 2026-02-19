# Music Visualizer

A real-time music visualization application built with C++, OpenGL, and GLFW. This application generates synthetic audio and visualizes frequency data using Fast Fourier Transform (FFT) with animated frequency bars.

## Features

- Real-time audio frequency visualization
- **Multiple audio sources:**
  - Microphone input for live audio visualization
  - Synthetic audio generation for demo/testing
- FFT-based frequency analysis
- **Four visualization modes:**
  - Frequency Bars (equalizer style)
  - Circular Spectrum (radial display)
  - Waveform (oscilloscope style)
  - Circle Pulse (pulsing bands)
- Smooth animated transitions
- Color-coded visualization based on amplitude
- OpenGL 3.3 Core Profile rendering
- Keyboard controls for easy mode switching

## Prerequisites

To build and run this application, you need:

- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake 3.10 or higher
- OpenGL 3.3 or higher
- GLFW3
- GLEW
- PortAudio (for microphone input)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libglfw3-dev libglew-dev libgl1-mesa-dev portaudio19-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc-c++ cmake glfw-devel glew-devel mesa-libGL-devel portaudio-devel
```

**macOS (with Homebrew):**
```bash
brew install cmake glfw glew portaudio
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

## Visualization Types

This application now provides **four visualization types** that you can switch between using keyboard controls:

### 1. Frequency Bars (Press '1')
- **Description**: Real-time animated frequency spectrum bars
- **Features**:
  - 64 individual frequency bars representing different frequency ranges
  - Dynamic color mapping based on amplitude (blue to pink gradient)
  - Smooth animations with exponential moving average
  - Logarithmic scaling for better visual distribution
  - FFT-based frequency analysis

The bars animate in real-time, responding to the audio frequency data. Lower frequencies appear on the left, higher frequencies on the right. The height and color of each bar indicates the amplitude at that frequency range.

### 2. Circular Spectrum (Press '2')
- **Description**: Radial frequency spectrum arranged in a circle
- **Features**:
  - 64 frequency bands arranged in a circular pattern
  - Rainbow color gradient around the circle
  - Bars extend outward from a central ring
  - Dynamic brightness based on amplitude
  - Creates a visually striking mandala-like effect

### 3. Waveform (Press '3')
- **Description**: Real-time audio waveform display
- **Features**:
  - Shows the actual audio signal waveform
  - Smooth Cyan line visualization
  - 512 sample points for detailed representation
  - Temporal smoothing for stable display
  - Classic oscilloscope-style visualization

### 4. Circle Pulse (Press '4')
- **Description**: Pulsing concentric circles based on frequency bands
- **Features**:
  - 4 concentric circles representing different frequency bands
  - Each circle pulses based on its frequency band amplitude
  - Color-coded bands: Red (bass), Green (low-mid), Blue (high-mid), Yellow (treble)
  - Creates a hypnotic pulsing effect
  - Great for visualizing rhythm and beat

## Usage

- The application will start with synthetic audio and the default frequency bars visualization
- **Switch between visualization modes:**
  - Press `1` for Frequency Bars
  - Press `2` for Circular Spectrum
  - Press `3` for Waveform
  - Press `4` for Circle Pulse
- **Toggle audio source:**
  - Press `M` to switch between Microphone and Synthetic audio
  - When using microphone mode, make sure you have a working microphone connected
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
- Additional visualization modes:
  - 3D visualization
  - Particle effects
  - Spectrogram display
- Configurable color schemes
- Full-screen mode
- Recording/export functionality
- Audio output device selection

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.