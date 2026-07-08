cmake_minimum_required(VERSION 3.16)
project(MagnetLinkGenerator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 依你目前的開發環境調整（Qt 6.11.0 / MSVC 2022 x64）
find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(MagnetLinkGenerator
    main.cpp
    mainwindow.h
    mainwindow.cpp
    app.rc
)

target_link_libraries(MagnetLinkGenerator PRIVATE Qt6::Widgets)

set_target_properties(MagnetLinkGenerator PROPERTIES
    WIN32_EXECUTABLE ON
)

# 讓 windeployqt 之類的工具容易找到執行檔（可選）
install(TARGETS MagnetLinkGenerator
    RUNTIME DESTINATION .
)
