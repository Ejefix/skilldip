cmake_minimum_required(VERSION 3.11)

project(LibDiplom LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_library(LibDiplom
    converterjson.cpp
    converterjson.h
    searchserver.h searchserver.cpp
    relativeindex.h relativeindex.cpp
    readfile.h readfile.cpp
    answersjson.h answersjson.cpp
)


target_include_directories(LibDiplom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(LibDiplom PUBLIC ../${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LibDiplom PUBLIC nlohmann_json::nlohmann_json)


install(TARGETS LibDiplom ARCHIVE DESTINATION lib)

