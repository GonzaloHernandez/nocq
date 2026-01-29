cmake_minimum_required(VERSION 3.10)
project(NOCQ LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
include(ExternalProject)

ExternalProject_Add(chuffed_project
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/chuffed"
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/chuffed_build"
    CMAKE_ARGS 
        -DCMAKE_BUILD_TYPE=Release
        -DCP_PROFILER=OFF
        -DCHUFFED_BUILD_TESTS=OFF
        -DCHUFFED_BUILD_EXAMPLES=OFF
    INSTALL_COMMAND "" 
)

set(SOURCES
    src/utils/satencoder.cpp
    src/utils/zielonka.cpp
    src/utils/tarjan.cpp
    src/utils/game.cpp
    src/utils/fra.cpp
    src/main.cpp
)

add_executable(nocq ${SOURCES})

option(ENABLE_GECODE "Enable Gecode solver support" OFF)

if(ENABLE_GECODE)
target_compile_definitions(nocq PRIVATE HAS_GECODE)
    target_include_directories(nocq PRIVATE /usr/include)
    target_link_libraries(nocq PRIVATE 
        gecodekernel 
        gecodesearch 
        gecodeint 
        gecodesupport
    )
    message(STATUS "Build: Gecode support ENABLED")
else()
    message(STATUS "Build: Gecode support DISABLED")
endif()

target_include_directories(nocq PRIVATE 
    src/
    thirdparty/chuffed
)

target_link_libraries(nocq PRIVATE 
    "${CMAKE_CURRENT_BINARY_DIR}/chuffed_build/libchuffed.a"
    pthread
)

add_dependencies(nocq chuffed_project)