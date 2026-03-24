cmake_minimum_required(VERSION 3.21)
project(MySQLAccessTester LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 (recommended). If you use Qt5, adapt find_package + target_link_libraries.
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/MainWindow.ui
)

set(RESOURCES
    resources/assets.qrc
)

# Windows icon resource (.rc)
set(WIN_RES resources/app.rc)

qt_add_executable(MySQLAccessTester WIN32
    ${SOURCES}
    ${RESOURCES}
    ${WIN_RES}
)

target_link_libraries(MySQLAccessTester PRIVATE Qt6::Widgets Qt6::Sql)

# Helpful: copy docs next to build output after build (optional)
add_custom_command(TARGET MySQLAccessTester POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:MySQLAccessTester>/docs"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/docs" "$<TARGET_FILE_DIR:MySQLAccessTester>/docs"
)
