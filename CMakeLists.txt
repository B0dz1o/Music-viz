cmake_minimum_required(VERSION 3.10)
project(MusicViz)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/audio_processor.cpp
    src/visualizer.cpp
    src/shader.cpp
)

# Header files
set(HEADERS
    src/audio_processor.h
    src/visualizer.h
    src/shader.h
)

# Create executable
add_executable(music_viz ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(music_viz PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(music_viz
    ${OPENGL_LIBRARIES}
    glfw
    GLEW::GLEW
    m
)

# Install target
install(TARGETS music_viz DESTINATION bin)
