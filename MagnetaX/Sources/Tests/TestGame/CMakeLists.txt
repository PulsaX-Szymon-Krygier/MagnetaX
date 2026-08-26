# ======================
# TestGame build config
# ======================
cmake_minimum_required(VERSION 3.20)

MXBAddTestApp(TestGame)

# Copy Assets dir to executable dir
add_custom_command(TARGET TestGame POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:TestGame>/Assets"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:TestGame>/Assets"
)