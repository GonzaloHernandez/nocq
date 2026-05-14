cmake_minimum_required(VERSION 3.10)
project(NOCQ LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Ensure CMake looks in your local folder if it exists
list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}/.local")

# ----------------------------------------------------------------------
# 1. USER OPTIONS (All together at the top)
# ----------------------------------------------------------------------
option(USE_SYSTEM_CHUFFED "Use system-installed Chuffed instead of bundled version" OFF)
option(ENABLE_GECODE      "Enable Gecode solver support" OFF)
option(ENABLE_CADICAL     "Enable CaDiCaL support"       OFF)

# ----------------------------------------------------------------------
# 2. SOURCE DEFINITION & EXECUTABLE
# ----------------------------------------------------------------------
set(SOURCES
    src/utils/satencoder.cpp
    src/utils/zielonka.cpp
    src/utils/tarjan.cpp
    src/utils/game.cpp
    src/utils/fra.cpp
    src/main.cpp
)

add_executable(nocq ${SOURCES})

# Required for threading support used by solvers
find_package(Threads REQUIRED)

# ----------------------------------------------------------------------
# 3. CHUFFED CONFIGURATION
# ----------------------------------------------------------------------
if(USE_SYSTEM_CHUFFED)
    find_path(CHUFFED_INCLUDE_DIR chuffed/core/options.h)
    find_library(CHUFFED_LIB chuffed)
    find_library(CHUFFED_FZN_LIB chuffed_fzn)
    
    if(CHUFFED_INCLUDE_DIR AND CHUFFED_LIB)
        message(STATUS "Build: Using SYSTEM Chuffed")
        target_link_libraries(nocq PRIVATE ${CHUFFED_LIB} ${CHUFFED_FZN_LIB} ${CHUFFED_LIB})
        target_include_directories(nocq PRIVATE ${CHUFFED_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "USE_SYSTEM_CHUFFED enabled but Chuffed not found!")
    endif()
else()
    message(STATUS "Build: Using BUNDLED Chuffed")
    include(ExternalProject)
    ExternalProject_Add(chuffed_project
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/chuffed"
        BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/chuffed_build"
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCP_PROFILER=OFF -DCHUFFED_BUILD_TESTS=OFF -DCHUFFED_BUILD_EXAMPLES=OFF
        INSTALL_COMMAND "" 
    )
    
    add_dependencies(nocq chuffed_project)
    target_include_directories(nocq PRIVATE 
        "thirdparty/chuffed"
        "${CMAKE_CURRENT_BINARY_DIR}/chuffed_build"
    )
    target_link_libraries(nocq PRIVATE 
        "${CMAKE_CURRENT_BINARY_DIR}/chuffed_build/libchuffed.a"
        "${CMAKE_CURRENT_BINARY_DIR}/chuffed_build/libchuffed_fzn.a"
        "${CMAKE_CURRENT_BINARY_DIR}/chuffed_build/libchuffed.a"    
    )
endif()

# ----------------------------------------------------------------------
# 4. GECODE CONFIGURATION
# ----------------------------------------------------------------------
if(ENABLE_GECODE)
    set(GECODE_COMPONENTS kernel search int support)
    set(GECODE_LIBS "")

    foreach(comp ${GECODE_COMPONENTS})
        find_library(GECODE_${comp}_LIB gecode${comp})
        if(GECODE_${comp}_LIB)
            list(APPEND GECODE_LIBS ${GECODE_${comp}_LIB})
        endif()
    endforeach()

    find_path(GECODE_INCLUDE_DIR NAMES gecode/kernel.hh)

    if(GECODE_INCLUDE_DIR AND GECODE_LIBS)
        target_compile_definitions(nocq PRIVATE HAS_GECODE)
        target_include_directories(nocq PRIVATE ${GECODE_INCLUDE_DIR})
        target_link_libraries(nocq PRIVATE ${GECODE_LIBS})
        message(STATUS "Build: Gecode support ENABLED")
    else()
        message(FATAL_ERROR "Gecode enabled but headers/libraries not found!")
    endif()
else()
    message(STATUS "Build: Gecode support DISABLED")
endif()

# ----------------------------------------------------------------------
# 5. CADICAL CONFIGURATION
# ----------------------------------------------------------------------
if(ENABLE_CADICAL)
    find_path(CADICAL_INCLUDE_DIR cadical.hpp)
    find_library(CADICAL_LIBRARY cadical)

    if(CADICAL_INCLUDE_DIR AND CADICAL_LIBRARY)
        target_compile_definitions(nocq PRIVATE HAS_CADICAL)
        target_include_directories(nocq PRIVATE ${CADICAL_INCLUDE_DIR})
        target_link_libraries(nocq PRIVATE ${CADICAL_LIBRARY})
        message(STATUS "Build: CaDiCaL support ENABLED")
    else()
        message(FATAL_ERROR "CaDiCaL enabled but headers/libraries not found!")
    endif()
else()
    message(STATUS "Build: CaDiCaL support DISABLED")
endif()

# ----------------------------------------------------------------------
# 6. GLOBAL INCLUDES & LINKS
# ----------------------------------------------------------------------
target_include_directories(nocq PRIVATE src/)
target_link_libraries(nocq PRIVATE Threads::Threads)