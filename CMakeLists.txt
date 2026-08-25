# =================================
# Root CMakeLists.txt for MagnetaX
# =================================
cmake_minimum_required(VERSION 3.20)

project(MagnetaX LANGUAGES CXX)

# Delegate to MagnetaX/Build/CMake
add_subdirectory("MagnetaX/Build/CMake")