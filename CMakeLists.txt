cmake_minimum_required(VERSION 3.20)  # Обновите до 3.20+ для стабильной работы с FetchContent
project(DefenderOfTheInternetFromCats)

# Установка C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включение FetchContent
include(FetchContent)

# --- OpenCV ---
FetchContent_Declare(
  opencv
  GIT_REPOSITORY https://github.com/opencv/opencv.git
  GIT_TAG 4.9.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(opencv)

# --- SFML (только аудио) ---
FetchContent_Declare(
  sfml
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 2.6.x
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(sfml)

# --- Сборка исполняемого файла ---
add_executable(${PROJECT_NAME}
  src/alarm_system.cpp
  src/cat_detector.cpp
  src/utils.cpp
  src/sound_player.cpp
  src/udp_communicator.cpp
  src/main.cpp
)

# --- Подключение библиотек ---
target_link_libraries(${PROJECT_NAME} PRIVATE
  ${OpenCV_LIBS}
  sfml-audio
  ws2_32    # Для UDP
  winmm     # Для звука на Windows
)

# --- Копирование ресурсов (звуки, видео) после сборки ---
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

# --- Опциональное форматирование кода ---
if(EXISTS "${CMAKE_SOURCE_DIR}/.clang-format")
  find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
  if(CLANG_FORMAT_EXECUTABLE)
    add_custom_target(format
      COMMAND ${CLANG_FORMAT_EXECUTABLE} -i -style=file
        ${CMAKE_SOURCE_DIR}/include/*.h
        ${CMAKE_SOURCE_DIR}/src/*.cpp
      COMMENT "Running clang-format on source files"
    )
  endif()
endif()
