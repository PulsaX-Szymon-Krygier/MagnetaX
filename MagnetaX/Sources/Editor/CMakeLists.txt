# ====================
# Editor build config
# ====================
cmake_minimum_required(VERSION 3.20)

# Add executable
add_executable(MXEditor)

# Set directories
set(MXB_DIR_EDITOR ${CMAKE_CURRENT_LIST_DIR})

set(MXB_DIR_EDITOR_SOURCE "${MXB_DIR_EDITOR}/Source")

# Set sources
file(GLOB_RECURSE MXB_SOURCE_EDITOR_PRIVATE CONFIGURE_DEPENDS
  "${MXB_DIR_EDITOR_SOURCE}/*.h"
  "${MXB_DIR_EDITOR_SOURCE}/*.cpp"
)

# Set target sources and include directories
target_sources(MXEditor
  PRIVATE
    ${MXB_SOURCE_EDITOR_PRIVATE}
)

target_include_directories(MXEditor
  PRIVATE
    ${MXB_DIR_EDITOR_SOURCE}
)

# Add dependencies
target_link_libraries(MXEditor PRIVATE MXEngine)
target_link_libraries(MXEditor PRIVATE MX_ThirdParty_imgui)

# Set properties
set_target_properties(MXEditor PROPERTIES
  FOLDER "Editor"
  RUNTIME_OUTPUT_DIRECTORY "${MXB_DIR_BINARIES}/$<CONFIG>/MXEditor"
)

# Print summary
message(STATUS "MXB: Editor configured!")