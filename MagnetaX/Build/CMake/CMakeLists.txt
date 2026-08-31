# ====================================
# Main CMake build config for MagnetaX
# ====================================
cmake_minimum_required(VERSION 3.20)

# Set properties
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set language and standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Check and set platform
set(MXB_PLATFORM_WINDOWS 0)
set(MXB_PLATFORM_LINUX 0)
set(MXB_PLATFORM_APPLE 0)

if(WIN32)
  set(MXB_PLATFORM_WINDOWS 1)
elseif(UNIX AND NOT APPLE)
  set(MXB_PLATFORM_LINUX 1)
elseif(APPLE)
  set(MXB_PLATFORM_APPLE 1)
else()
  message(FATAL_ERROR "MXB: Unsupported platform")
endif()

# Enable Objective-C for Apple
if(MXB_PLATFORM_APPLE)
  enable_language(OBJCXX)
  set(CMAKE_OBJCXX_STANDARD 20)
  set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
  set(CMAKE_OBJCXX_EXTENSIONS OFF)
endif()

# Set directories
set(MXB_DIR_ROOT "${CMAKE_SOURCE_DIR}/MagnetaX")
set(MXB_DIR_BINARIES "${MXB_DIR_ROOT}/Binaries")
set(MXB_DIR_BUILD "${MXB_DIR_ROOT}/Build")
set(MXB_DIR_SOURCES "${MXB_DIR_ROOT}/Sources")
set(MXB_DIR_THIRDPARTY "${MXB_DIR_ROOT}/ThirdParty")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MXB_DIR_BINARIES}/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${MXB_DIR_BINARIES}/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${MXB_DIR_BINARIES}/$<CONFIG>")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${MXB_DIR_BINARIES}/$<CONFIG>")

# Set compiler options
if(MSVC)
  add_compile_options(
  /permissive-
  /Zc:__cplusplus
  /Zc:preprocessor
  /MP
  /W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Add third party dependencies
include("ThirdParty.cmake")

# Add build flag
add_compile_definitions(
  $<$<CONFIG:Debug>:MX_DEBUG=1>
  $<$<NOT:$<CONFIG:Debug>>:MX_DEBUG=0>
)

# Add engine
add_subdirectory("${MXB_DIR_SOURCES}/Engine" Engine)

# Add game
add_subdirectory("${MXB_DIR_SOURCES}/Game" Game)

# Add editor
add_subdirectory("${MXB_DIR_SOURCES}/Editor" Editor)

# Add tests
add_subdirectory("${MXB_DIR_SOURCES}/Tests" Tests)

# Print summary
message(STATUS "MXB: MagnetaX configured!")