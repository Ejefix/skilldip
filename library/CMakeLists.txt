cmake_minimum_required(VERSION 3.14)

project(LibDiplom LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(json)

add_library(LibDiplom
    converterjson.cpp
    converterjson.h
)

# Указываем, где искать заголовочные файлы для этой библиотеки
target_include_directories(LibDiplom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS LibDiplom ARCHIVE DESTINATION lib)

target_link_libraries(LibDiplom PRIVATE nlohmann_json::nlohmann_json)
