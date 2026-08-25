# =======================
# Test apps build config
# =======================
cmake_minimum_required(VERSION 3.20)

function(MXBAddTestApp MXB_APP_NAME)
  file(GLOB_RECURSE MXB_SOURCE_TESTAPP CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
  )

  add_executable(${MXB_APP_NAME} ${MXB_SOURCE_TESTAPP})

  target_link_libraries(${MXB_APP_NAME} PRIVATE MXGame)
  target_include_directories(${MXB_APP_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source")
  target_compile_features(${MXB_APP_NAME} PRIVATE cxx_std_20)

  set_target_properties(${MXB_APP_NAME} PROPERTIES
    FOLDER "Tests" 
    RUNTIME_OUTPUT_DIRECTORY "${MXB_DIR_BINARIES}/$<CONFIG>/${MXB_APP_NAME}"
  )

  message(STATUS "MXB: ${MXB_APP_NAME} configured!")
endfunction()

add_subdirectory("TestGame")