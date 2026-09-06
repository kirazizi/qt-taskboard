cmake_minimum_required(VERSION 3.22)
project(qt-taskboard VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 auto-tools: run moc/uic/rcc automatically on files that need it
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Widgets)

# -----------------------------------------------------------------------
# Source files -- grouped by layer (core / data / ui)
# -----------------------------------------------------------------------
set(CORE_SOURCES
    src/core/Task.cpp
    src/core/Board.cpp
)

set(DATA_SOURCES
    src/data/JsonStore.cpp
)

set(UI_SOURCES
    src/ui/TaskEditDialog.h
    src/ui/BoardColumnWidget.h
    src/ui/TaskCardWidget.h
    src/ui/MainWindow.h
    src/ui/TaskEditDialog.cpp
    src/ui/BoardColumnWidget.cpp
    src/ui/TaskCardWidget.cpp
    src/ui/MainWindow.cpp
)

add_executable(qt-taskboard
    src/main.cpp
    ${CORE_SOURCES}
    ${DATA_SOURCES}
    ${UI_SOURCES}
    resources.qrc
)

target_include_directories(qt-taskboard PRIVATE src)

target_link_libraries(qt-taskboard PRIVATE
    Qt6::Core
    Qt6::Widgets)

# Treat warnings as errors in our own code
target_compile_options(qt-taskboard PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)
