cmake_minimum_required(VERSION 3.16)
project(AdjustDisplay LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Gui DBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Gui DBus)

set(SOURCES
    src/main.cpp
    src/i18n.h
    src/i18n.cpp
    src/pattern_types.h
    src/screen_manager.h
    src/screen_manager.cpp
    src/hardware_bridge.h
    src/hardware_bridge.cpp
    src/pattern_widget.h
    src/pattern_widget.cpp
    src/wizard_controller.h
    src/wizard_controller.cpp
    src/main_window.h
    src/main_window.cpp
)

add_executable(adjust-display ${SOURCES})

target_link_libraries(adjust-display PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::DBus
)

install(TARGETS adjust-display DESTINATION bin)
