cmake_minimum_required(VERSION 3.16)
project(ImageStitcher LANGUAGES CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Release 为默认构建类型（如果未指定）
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Win7 SP1 兼容（0x0601 = Windows 7）
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601 WINVER=0x0601 NOMINMAX)
endif()

# MSVC 处理中文源码（UTF-8）
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 查找 Qt5
find_package(Qt5 5.15 COMPONENTS Widgets Gui Core Xml Concurrent REQUIRED)

# 可执行目标
add_executable(ImageStitcher
    main.cpp
    MainWindow.h
    MainWindow.cpp
    NumberUtil.h
    NumberUtil.cpp
    FileManager.h
    FileManager.cpp
    Stitcher.h
    Stitcher.cpp
    XmlEditor.h
    XmlEditor.cpp
    PathResolver.h PathResolver.cpp XmlChecker.h XmlChecker.cpp CheckResultDialog.h CheckResultDialog.cpp
)

target_link_libraries(ImageStitcher PRIVATE
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    Qt5::Xml
    Qt5::Concurrent
)

# 允许 Qt5.15 全部 API
target_compile_definitions(ImageStitcher PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0x060000)

# MSVC 输出 WIN32 可执行（带 GUI，无控制台窗口）
if(MSVC)
    set_target_properties(ImageStitcher PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# MinGW 同样隐藏控制台窗口
if(MINGW)
    target_link_options(ImageStitcher PRIVATE -mwindows)
endif()
