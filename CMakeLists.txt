cmake_minimum_required(VERSION 3.20)
project(game C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(raylib-master)

add_executable(game src/main.c)
target_link_libraries(game PRIVATE raylib)

# --- ADD THIS TO COPY AUDIO FILES AUTOMATICALLY ---
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/music.mp3"
    "${CMAKE_CURRENT_SOURCE_DIR}/pop.wav"
    "${CMAKE_CURRENT_SOURCE_DIR}/gameover.wav"
    $<TARGET_FILE_DIR:game>
)