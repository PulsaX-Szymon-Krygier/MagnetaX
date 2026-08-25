# ==================
# Game build config
# ==================
cmake_minimum_required(VERSION 3.20)

# Add library
add_library(MXGame STATIC)

# Set directories
set(MXB_DIR_GAME ${CMAKE_CURRENT_LIST_DIR})

set(MXB_DIR_GAME_INCLUDE "${MXB_DIR_GAME}/Include")
set(MXB_DIR_GAME_INCLUDE_PUBLIC "${MXB_DIR_GAME_INCLUDE}/Public")
set(MXB_DIR_GAME_SOURCE "${MXB_DIR_GAME}/Source")

# Set sources
file(GLOB_RECURSE MXB_SOURCE_GAME_PUBLIC CONFIGURE_DEPENDS
  "${MXB_DIR_GAME_INCLUDE_PUBLIC}/MX/*.h"
)

file(GLOB_RECURSE MXB_SOURCE_GAME_PRIVATE CONFIGURE_DEPENDS
  "${MXB_DIR_GAME_SOURCE}/*.h"
  "${MXB_DIR_GAME_SOURCE}/*.cpp"
)

# Set target sources and include directories
target_sources(MXGame
  PRIVATE
    ${MXB_SOURCE_GAME_PUBLIC}
    ${MXB_SOURCE_GAME_PRIVATE}
)

target_include_directories(MXGame
  PUBLIC
    ${MXB_DIR_GAME_INCLUDE_PUBLIC}

  PRIVATE
    ${MXB_DIR_GAME_SOURCE}
    "${MXB_DIR_SOURCES}/Engine/Include/Internal"
)

# Add dependencies
target_link_libraries(MXGame PUBLIC MXEngine)

# Print summary
message(STATUS "MXB: Game configured!")