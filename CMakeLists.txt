cmake_minimum_required(VERSION 3.0)

# Set the project name
project(FileSelectorWithCustomMain)

# Setting C standard, optimization, and warnings
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wextra -pedantic -Wno-unused-parameter -mwindows")

# Define necessary preprocessor macros
add_definitions(-D_POSIX_C_SOURCE=200112L)
add_definitions(-D_FILE_OFFSET_BITS=64)

# Add both C files to the executable
add_executable(file_selector main.c unimgc.c image.c lzo.c custom_printf.c RunCustomMain.c)

# Link against necessary libraries for Windows GUI applications
target_link_libraries(file_selector PRIVATE user32 gdi32 comdlg32 comctl32 uxtheme)

# Output directory for objects
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

