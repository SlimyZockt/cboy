cmake_minimum_required(VERSION 3.16)
project(cboy LANGUAGES C)

# set the output directory for built object
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL)

# Create your game executable target as usual
add_executable(cboy src/main.c src/emulation.c src/memory.c src/instruction.c src/cJSON.c)
target_include_directories(cboy PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/" )
target_compile_options(cboy PRIVATE -Wall -Wextra -Wunused)

# Link to the actual SDL3 library.
target_link_libraries(cboy PRIVATE SDL3::SDL3 )

   
