cmake_minimum_required(VERSION 3.11)

project(Skilldip LANGUAGES CXX VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)


add_subdirectory(./library)


add_executable(Skilldip
    main.cpp
)

configure_file(version.h.in version_main.h)

target_link_libraries(Skilldip PRIVATE LibDiplom )
target_include_directories(LibDiplom PRIVATE ${CMAKE_BINARY_DIR} nlohmann_json::nlohmann_json)
target_include_directories(Skilldip PRIVATE ${CMAKE_BINARY_DIR})


install(TARGETS Skilldip DESTINATION bin)

install(FILES ${CMAKE_SOURCE_DIR}/config.json DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/requests.json DESTINATION bin)

add_custom_command(TARGET Skilldip POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/config.json
            $<TARGET_FILE_DIR:Skilldip>)
add_custom_command(TARGET Skilldip POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/requests.json
            $<TARGET_FILE_DIR:Skilldip>
        )

option(BUILD_TESTS "Build the tests" ON)
if(BUILD_TESTS)
    enable_testing()

    add_executable(SkilldipTests tests/test_main.cpp)
    add_custom_command(TARGET SkilldipTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/tests/file
                $<TARGET_FILE_DIR:SkilldipTests>/tests/file
            )

    target_link_libraries(SkilldipTests PRIVATE LibDiplom )
    target_include_directories(SkilldipTests PRIVATE ${CMAKE_BINARY_DIR})

    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/heads/main.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    add_executable(GTests tests.cpp)
    target_link_libraries(GTests gtest_main LibDiplom)
    target_include_directories(GTests PRIVATE ${CMAKE_BINARY_DIR})
    include(GoogleTest)
    gtest_discover_tests(GTests)

endif()




