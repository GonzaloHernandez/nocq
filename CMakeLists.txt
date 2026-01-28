cmake_minimum_required(VERSION 3.10)
project(NOCQ LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
include(ExternalProject)

# --- 1. Define Chuffed as an External Project ---
ExternalProject_Add(chuffed_project
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/chuffed"
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/chuffed_build"
    CMAKE_ARGS 
        -DCMAKE_BUILD_TYPE=Release
        -DCP_PROFILER=OFF
        -DCHUFFED_BUILD_TESTS=OFF
        -DCHUFFED_BUILD_EXAMPLES=OFF
    INSTALL_COMMAND ""  # We don't need to install it globally
)

# --- 2. Define NOCQ ---
set(SOURCES
    src/utils/satencoder.cpp
    src/utils/zielonka.cpp
    src/utils/tarjan.cpp
    src/utils/game.cpp
    src/utils/fra.cpp
    src/main.cpp
)

add_executable(nocq ${SOURCES})

# --- 3. Link NOCQ to the External Chuffed build ---
# Tell NOCQ where the headers are (the source folder)
target_include_directories(nocq PRIVATE 
    src/
    thirdparty/chuffed
)

# Tell NOCQ where the compiled library file will appear
# Note: Chuffed usually produces 'libchuffed.a' in its build folder
target_link_libraries(nocq PRIVATE 
    "${CMAKE_CURRENT_BINARY_DIR}/chuffed_build/libchuffed.a"
    pthread
)

# IMPORTANT: Tell CMake that NOCQ depends on Chuffed being built first
add_dependencies(nocq chuffed_project)