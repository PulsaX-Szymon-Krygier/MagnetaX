# ====================
# Engine build config
# ====================
cmake_minimum_required(VERSION 3.20)

# Add libraries
add_library(MXEngine STATIC)

# Set directories
set(MXB_DIR_ENGINE ${CMAKE_CURRENT_LIST_DIR})

set(MXB_DIR_ENGINE_INCLUDE "${MXB_DIR_ENGINE}/Include")
set(MXB_DIR_ENGINE_INCLUDE_PUBLIC "${MXB_DIR_ENGINE_INCLUDE}/Public")
set(MXB_DIR_ENGINE_INCLUDE_INTERNAL "${MXB_DIR_ENGINE_INCLUDE}/Internal")
set(MXB_DIR_ENGINE_SOURCE "${MXB_DIR_ENGINE}/Source")

# Set sources
file(GLOB_RECURSE MXB_SOURCE_ENGINE_PUBLIC CONFIGURE_DEPENDS
  "${MXB_DIR_ENGINE_INCLUDE_PUBLIC}/MX/*.h"
)

file(GLOB_RECURSE MXB_SOURCE_ENGINE_INTERNAL CONFIGURE_DEPENDS
  "${MXB_DIR_ENGINE_INCLUDE_INTERNAL}/MX/*.h"
)

file(GLOB_RECURSE MXB_SOURCE_ENGINE_PRIVATE CONFIGURE_DEPENDS
  "${MXB_DIR_ENGINE_SOURCE}/*.h"
  "${MXB_DIR_ENGINE_SOURCE}/*.cpp"
)
list(FILTER MXB_SOURCE_ENGINE_PRIVATE EXCLUDE REGEX "/(WinAPI|X11|Wayland|Cocoa|Platform/Windows|Platform/Linux|Platform/Apple|Graphics/Vulkan)/")

file(GLOB_RECURSE MXB_SOURCE_ENGINE_PRIVATE_WINDOWS CONFIGURE_DEPENDS
  "${MXB_DIR_ENGINE_SOURCE}/Platform/Windows/*.h"
  "${MXB_DIR_ENGINE_SOURCE}/Platform/Windows/*.cpp"
  "${MXB_DIR_ENGINE_SOURCE}/*/WinAPI/*.h"
  "${MXB_DIR_ENGINE_SOURCE}/*/WinAPI/*.cpp"
)

file(GLOB_RECURSE MXB_SOURCE_ENGINE_PRIVATE_LINUX CONFIGURE_DEPENDS
  "${MXB_DIR_ENGINE_SOURCE}/Platform/Linux/*.h"
  "${MXB_DIR_ENGINE_SOURCE}/Platform/Linux/*.cpp"
)

file(GLOB_RECURSE MXB_SOURCE_ENGINE_PRIVATE_X11 CONFIGURE_DEPENDS
  "${MXB_DIR_ENGINE_SOURCE}/*/X11/*.h"
  "${MXB_DIR_ENGINE_SOURCE}/*/X11/*.cpp"
)

file(GLOB_RECURSE MXB_SOURCE_ENGINE_PRIVATE_WAYLAND CONFIGURE_DEPENDS
  "${MXB_DIR_ENGINE_SOURCE}/*/Wayland/*.h"
  "${MXB_DIR_ENGINE_SOURCE}/*/Wayland/*.cpp"
)

file(GLOB_RECURSE MXB_SOURCE_ENGINE_PRIVATE_APPLE CONFIGURE_DEPENDS
  "${MXB_DIR_ENGINE_SOURCE}/Platform/Apple/*.h"
  "${MXB_DIR_ENGINE_SOURCE}/Platform/Apple/*.cpp"
  "${MXB_DIR_ENGINE_SOURCE}/*/Cocoa/*.h"
  "${MXB_DIR_ENGINE_SOURCE}/*/Cocoa/*.mm"
)

file(GLOB_RECURSE MXB_SOURCE_ENGINE_PRIVATE_VULKAN CONFIGURE_DEPENDS
  "${MXB_DIR_ENGINE_SOURCE}/Graphics/Vulkan/*.h"
  "${MXB_DIR_ENGINE_SOURCE}/Graphics/Vulkan/*.cpp"
)

file(GLOB_RECURSE MXB_SOURCE_ENGINE_PRIVATE_VULKAN_APPLE CONFIGURE_DEPENDS
  "${MXB_DIR_ENGINE_SOURCE}/Graphics/Vulkan/Present/Metal/*.h"
  "${MXB_DIR_ENGINE_SOURCE}/Graphics/Vulkan/Present/Metal/*.mm"
)

# Set target sources and include directories
target_sources(MXEngine
  PRIVATE
    ${MXB_SOURCE_ENGINE_PUBLIC}
    ${MXB_SOURCE_ENGINE_INTERNAL}
    ${MXB_SOURCE_ENGINE_PRIVATE}
)

target_include_directories(MXEngine
  PUBLIC
    ${MXB_DIR_ENGINE_INCLUDE_PUBLIC}

  PRIVATE
    ${MXB_DIR_ENGINE_INCLUDE_INTERNAL}
    ${MXB_DIR_ENGINE_SOURCE}
)

# Add dependencies
target_link_libraries(MXEngine PRIVATE MX_ThirdParty_tinyobjloader)
target_link_libraries(MXEngine PRIVATE MX_ThirdParty_stb)

# Setup platform
if(MXB_PLATFORM_WINDOWS)
  target_sources(MXEngine PRIVATE ${MXB_SOURCE_ENGINE_PRIVATE_WINDOWS})
  target_link_libraries(MXEngine PRIVATE user32)
  target_compile_definitions(MXEngine PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if(MXB_PLATFORM_LINUX)
  target_sources(MXEngine PRIVATE ${MXB_SOURCE_ENGINE_PRIVATE_LINUX})
  target_sources(MXEngine PRIVATE ${MXB_SOURCE_ENGINE_PRIVATE_WAYLAND})
  find_package(X11 QUIET)

  if(X11_FOUND)
    target_sources(MXEngine PRIVATE ${MXB_SOURCE_ENGINE_PRIVATE_X11})
    target_link_libraries(MXEngine PRIVATE ${X11_LIBRARIES})
    target_include_directories(MXEngine PRIVATE ${X11_INCLUDE_DIR})
    target_compile_definitions(MXEngine PRIVATE MX_PLATFORM_X11=1)
  else()
    target_compile_definitions(MXEngine PRIVATE MX_PLATFORM_X11=0)
  endif()
endif()

if(MXB_PLATFORM_APPLE)
  target_sources(MXEngine PRIVATE ${MXB_SOURCE_ENGINE_PRIVATE_APPLE})
  target_compile_options(MXEngine PRIVATE $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>)

  find_library(MXB_COCOA_FRAMEWORK Cocoa REQUIRED)
  find_library(MXB_QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)

  target_link_libraries(MXEngine PRIVATE ${MXB_COCOA_FRAMEWORK} ${MXB_QUARTZCORE_FRAMEWORK})
endif()

if(MXB_PLATFORM_WINDOWS OR MXB_PLATFORM_LINUX OR MXB_PLATFORM_APPLE)
  find_package(Vulkan QUIET)

  if(Vulkan_FOUND)
    target_link_libraries(MXEngine PRIVATE Vulkan::Vulkan)
    target_compile_definitions(MXEngine PRIVATE
      MX_GRAPHICS_VULKAN=1
      $<$<CONFIG:Debug>:MX_GRAPHICS_VULKAN_DEBUG=1>
      $<$<NOT:$<CONFIG:Debug>>:MX_GRAPHICS_VULKAN_DEBUG=0>
    )
    target_sources(MXEngine PRIVATE ${MXB_SOURCE_ENGINE_PRIVATE_VULKAN})

    if(MXB_PLATFORM_APPLE)
      target_sources(MXEngine PRIVATE ${MXB_SOURCE_ENGINE_PRIVATE_VULKAN_APPLE})
      target_compile_definitions(MXEngine PRIVATE VK_USE_PLATFORM_METAL_EXT=1)
    endif()

    include("${MXB_DIR_BUILD}/CMake/Shaders.cmake")
    MXBAddShaders(MXB_SHADERS_HEADERS_TARGET MXB_SHADERS_INCLUDE_DIR)
    add_dependencies(MXEngine ${MXB_SHADERS_HEADERS_TARGET})
    target_include_directories(MXEngine PRIVATE ${MXB_SHADERS_INCLUDE_DIR})
  else()
    target_compile_definitions(MXEngine PRIVATE MX_GRAPHICS_VULKAN=0)
  endif()
else()
  target_compile_definitions(MXEngine PRIVATE MX_GRAPHICS_VULKAN=0)
endif()

# Print summary
message(STATUS "MXB: Engine configured!")