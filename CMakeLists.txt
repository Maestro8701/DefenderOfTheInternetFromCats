cmake_minimum_required(VERSION 3.20)
project(DefenderOfTheInternetFromCats)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  opencv
  GIT_REPOSITORY https://github.com/opencv/opencv.git
  GIT_TAG 4.9.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(opencv)

FetchContent_Declare(
  sfml
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 2.6.x
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(sfml)

add_executable(${PROJECT_NAME}
    src/alarm_system.cpp
    src/cat_detector.cpp
    src/utils.cpp
    src/sound_player.cpp
    src/udp_communicator_unix.cpp
    src/udp_communicator.cpp
    src/main.cpp
)

if(WIN32)
    find_package(winmm REQUIRED)
    find_package(ws2_32 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${OpenCV_LIBS}
        sfml-audio
        winmm
        ws2_32
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${OpenCV_LIBS}
        sfml-audio
    )
endif()

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/multimedia/sound"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/multimedia/sound"
)
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/multimedia/video"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/multimedia/video"
)

if(EXISTS "${CMAKE_SOURCE_DIR}/.clang-format")
    find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
    if(CLANG_FORMAT_EXECUTABLE)
        add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXECUTABLE} -i -style=file
                "${CMAKE_SOURCE_DIR}/include/*.h"
                "${CMAKE_SOURCE_DIR}/src/*.cpp"
            COMMENT "Running clang-format on source files"
        )
    endif()
endif()

option(BUILD_TESTING "Build tests" OFF)

if(BUILD_TESTING)
    add_executable(run_tests
        tests/main_test.cp
    )

    if(WIN32)
        find_package(winmm REQUIRED)
        find_package(ws2_32 REQUIRED)
    endif()

    target_link_libraries(run_tests PRIVATE
        ${OpenCV_LIBS}
        sfml-audio
    )
    if(WIN32)
        target_link_libraries(run_tests PRIVATE winmm ws2_32)
    endif()

    FetchContent_Declare(
        gtest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(gtest)

    find_package(gtest REQUIRED)
    target_link_libraries(run_tests PRIVATE GTest::gtest_main gtest)

    add_custom_target(run_tests_target ALL DEPENDS run_tests)
endif()

message(STATUS "Тестовая сборка: use 'make run_tests' или 'cmake --build . --target run_tests' (только при BUILD_TESTING=ON)")
message(STATUS "Для тестов используйте main_test.cpp вместо main.cpp")
