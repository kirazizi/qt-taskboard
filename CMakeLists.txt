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
    src/core/Command.cpp        # Tier 2 Item 10: undo/redo command classes
    src/core/CommandHistory.cpp # Tier 2 Item 10: undo/redo stack manager
    src/core/ThemeManager.cpp   # Tier 3 Item 14: runtime theme switching
    src/core/BoardManager.cpp   # Tier 3 Item 11: multiple boards
    src/core/TaskListModel.cpp      # Tier 3 Item 13: QAbstractListModel
    src/core/TaskSortFilterModel.cpp # Tier 3 Item 13: QSortFilterProxyModel
)

set(DATA_SOURCES
    src/data/JsonStore.cpp
)

set(UI_SOURCES
    src/ui/TaskEditDialog.h
    src/ui/TaskDetailsDialog.h  # Tier 2 Item 8: read-only details dialog
    src/ui/BoardColumnWidget.h
    src/ui/TaskCardWidget.h
    src/ui/BoardBarWidget.h     # Tier 3 Item 11: board tab strip
    src/ui/MainWindow.h
    src/ui/TaskEditDialog.cpp
    src/ui/TaskDetailsDialog.cpp
    src/ui/BoardColumnWidget.cpp
    src/ui/TaskCardWidget.cpp
    src/ui/BoardBarWidget.cpp   # Tier 3 Item 11
    src/ui/ChartWidget.h
    src/ui/ChartWidget.cpp      # Tier 3 Item 12: animated bar chart
    src/ui/StatsDialog.h
    src/ui/StatsDialog.cpp      # Tier 3 Item 12: statistics dashboard
    src/ui/TaskCardDelegate.h
    src/ui/TaskCardDelegate.cpp  # Tier 3 Item 13: QStyledItemDelegate for cards
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
