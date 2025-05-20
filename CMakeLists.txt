cmake_minimum_required(VERSION 3.12)
project(Game_of_life)

find_package(SDL2 REQUIRED)

if(SDL2_FOUND)
    include_directories(${SDL2_INCLUDE_DIRS})
    add_executable(${PROJECT_NAME} source.cpp)
    target_link_libraries(${PROJECT_NAME} SDL2::SDL2 SDL2::SDL2main)
else()
    message(FATAL_ERROR "SDL2 library not found!")
endif()