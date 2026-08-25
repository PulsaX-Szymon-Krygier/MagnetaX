# ===================================
# Third party libraries build config
# ===================================

# TinyObjLoader
set(MXB_DIR_THIRDPARTY_TINYOBJLOADER "${MXB_DIR_THIRDPARTY}/tinyobjloader")

if(NOT EXISTS "${MXB_DIR_THIRDPARTY_TINYOBJLOADER}/tiny_obj_loader.cc")
  message(FATAL_ERROR "MXB: TinyObjLoader dependency not found!")
endif()

add_library(MX_ThirdParty_tinyobjloader STATIC "${MXB_DIR_THIRDPARTY_TINYOBJLOADER}/tiny_obj_loader.cc")
target_include_directories(MX_ThirdParty_tinyobjloader PUBLIC ${MXB_DIR_THIRDPARTY_TINYOBJLOADER})
target_compile_definitions(MX_ThirdParty_tinyobjloader PUBLIC TINYOBJLOADER_DISABLE_FAST_FLOAT)

if(MSVC)
  target_compile_options(MX_ThirdParty_tinyobjloader PRIVATE /W0)
else()
  target_compile_options(MX_ThirdParty_tinyobjloader PRIVATE -w)
endif()

set_target_properties(MX_ThirdParty_tinyobjloader PROPERTIES FOLDER "ThirdParty")

# stb
set(MXB_DIR_THIRDPARTY_STB "${MXB_DIR_THIRDPARTY}/stb")

if(NOT EXISTS "${MXB_DIR_THIRDPARTY_STB}/stb_image.h")
  message(FATAL_ERROR "MXB: stb dependency not found!")
endif()

add_library(MX_ThirdParty_stb INTERFACE)
target_include_directories(MX_ThirdParty_stb INTERFACE ${MXB_DIR_THIRDPARTY_STB})

set_target_properties(MX_ThirdParty_stb PROPERTIES FOLDER "ThirdParty")

# imgui
set(MXB_DIR_THIRDPARTY_IMGUI "${MXB_DIR_THIRDPARTY}/imgui")

if(NOT EXISTS "${MXB_DIR_THIRDPARTY_IMGUI}/imgui.cpp")
  message(FATAL_ERROR "MXB: Dear ImGui dependency not found!")
endif()

add_library(MX_ThirdParty_imgui STATIC
  "${MXB_DIR_THIRDPARTY_IMGUI}/imgui.cpp"
  "${MXB_DIR_THIRDPARTY_IMGUI}/imgui_draw.cpp"
  "${MXB_DIR_THIRDPARTY_IMGUI}/imgui_tables.cpp"
  "${MXB_DIR_THIRDPARTY_IMGUI}/imgui_widgets.cpp"
)

target_include_directories(MX_ThirdParty_imgui PUBLIC ${MXB_DIR_THIRDPARTY_IMGUI})

if(MSVC)
  target_compile_options(MX_ThirdParty_imgui PRIVATE /W0)
else()
  target_compile_options(MX_ThirdParty_imgui PRIVATE -w)
endif()

set_target_properties(MX_ThirdParty_imgui PROPERTIES FOLDER "ThirdParty")

message(STATUS "MXB: Third party dependencies configured!")