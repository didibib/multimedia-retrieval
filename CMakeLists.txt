cmake_minimum_required(VERSION 3.0.2)

# Check if project is build in seperate folder
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

include(cmake/set_startup_project.cmake)
include(cmake/set_target_output_directories.cmake)

project(multimedia-retrieval VERSION 1.0)

set(MMR_ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

# Use folder structure 
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# Specify the c++ standard
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED true)
# Setup output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/$<CONFIG>)

# compile PMP library
set(PMP_BUILD_APPS     OFF CACHE BOOL "")
set(PMP_BUILD_EXAMPLES OFF CACHE BOOL "")
set(PMP_BUILD_TESTS    OFF CACHE BOOL "")
set(PMP_BUILD_DOCS     OFF CACHE BOOL "")
add_subdirectory(external/pmp-library)

set(PMP_FILTER "pmp")
set_target_properties(pmp PROPERTIES FOLDER ${PMP_FILTER})
set_target_properties(pmp_vis PROPERTIES FOLDER ${PMP_FILTER})

set(GLFW_VENDOR_FILTER "pmp/external/GLFW3")
set_target_properties(glfw PROPERTIES FOLDER ${GLFW_VENDOR_FILTER})
set_target_properties(uninstall PROPERTIES FOLDER ${GLFW_VENDOR_FILTER})
set_target_properties(update_mappings PROPERTIES FOLDER ${GLFW_VENDOR_FILTER})

set(PMP_EXTERNAL "pmp/external")
set_target_properties(glew PROPERTIES FOLDER ${PMP_EXTERNAL})
set_target_properties(imgui PROPERTIES FOLDER ${PMP_EXTERNAL})
set_target_properties(rply PROPERTIES FOLDER ${PMP_EXTERNAL})
set_target_properties(stb_image PROPERTIES FOLDER ${PMP_EXTERNAL})

# add include directories
include_directories(${PROJECT_SOURCE_DIR}/external/pmp-library/src)
include_directories(${PROJECT_SOURCE_DIR}/external/pmp-library/external/imgui)
include_directories(${PROJECT_SOURCE_DIR}/external/pmp-library/external/eigen)
include_directories(${PROJECT_SOURCE_DIR}/external/pmp-library/external/glfw/include)
include_directories(${PROJECT_SOURCE_DIR}/external/pmp-library/external/glew/include)

# set default compiler flags
if((UNIX OR APPLE) AND CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND NOT EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra -Wno-unused-parameter")
endif()

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_MATH_DEFINES -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS")
endif()

if (EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -s ALLOW_MEMORY_GROWTH=1 -s USE_GLFW=3 -s WASM=0 -s USE_WEBGL2=1")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# make a release build by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
endif()


# which directories to process
add_subdirectory(src/mmr)

set_startup_project(mmr)