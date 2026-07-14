cmake_minimum_required(VERSION 3.15)

# Название проекта
project(DefenderOfTheInternetFromCats)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Путь к вашему clang-format (опционально)
set(CLANG_FORMAT_PATH "C:/Users/Evgenius/DefenderOfTheInternetFromCats/clang-format.exe")

# Найти пакеты
find_package(OpenCV REQUIRED)
find_package(SFML REQUIRED COMPONENTS audio)

# Установка путей к заголовочным файлам
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${SFML_INCLUDE_DIR})

# Сборка исполняемого файла
add_executable(${PROJECT_NAME}
    src/alarm_system.cpp
    src/cat_detector.cpp
    src/utils.cpp
    src/sound_player.cpp
    src/udp_communicator.cpp
    src/main.cpp
    include/alarm_system.h
    include/cat_detector.h
    include/sound_player.h
    include/udp_communicator.h
    include/utils.h
)

# Подключение библиотек
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${SFML_LIBRARIES}
    ws2_32
    winmm
)

# Копирование необходимых ресурсов (модели YOLO, звуки)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/models"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/models"
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/multimedia/sound"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/multimedia/sound"
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/multimedia/video"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/multimedia/video"
)

# Опционально: добавление проверки форматирования кода
if(CLANG_FORMAT_PATH AND EXISTS ${CLANG_FORMAT_PATH})
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_PATH} -i -style=file
            ${CMAKE_SOURCE_DIR}/include/*.h
            ${CMAKE_SOURCE_DIR}/src/*.cpp
        COMMENT "Running clang-format on source files"
    )
endif()

