cmake_minimum_required(VERSION 3.14)

project(Diplom LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(./library)

add_executable(diplom main.cpp)

target_link_libraries(diplom PRIVATE LibDiplom)

install(TARGETS diplom DESTINATION bin)
file(COPY config.json DESTINATION ${CMAKE_BINARY_DIR})
