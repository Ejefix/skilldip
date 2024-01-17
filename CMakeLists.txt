cmake_minimum_required(VERSION 3.11)

project(Diplom LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_subdirectory(./library)


add_executable(Diplom
    main.cpp
)

configure_file(version.h.in version_main.h)

target_link_libraries(Diplom PRIVATE LibDiplom nlohmann_json::nlohmann_json)
target_include_directories(LibDiplom PRIVATE ${CMAKE_BINARY_DIR})
target_include_directories(Diplom PRIVATE ${CMAKE_BINARY_DIR})


install(TARGETS Diplom DESTINATION bin)
file(COPY config.json DESTINATION ${CMAKE_BINARY_DIR})
file(COPY requests.json DESTINATION ${CMAKE_BINARY_DIR})
file(COPY txtfile DESTINATION ${CMAKE_BINARY_DIR})
