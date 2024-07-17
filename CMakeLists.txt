cmake_minimum_required(VERSION 3.10)
project(FileTag)

set(CMAKE_CXX_STANDARD 17)

# 启用自动生成MOC文件
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)  # 启用自动生成UIC文件

# 查找Qt6包
set(CMAKE_PREFIX_PATH "/opt/homebrew/Cellar/qt/6.7.0_1/lib/cmake")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

# 添加Qt6模块
set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)

# 将所有源文件和头文件添加到可执行文件 FileTag 中
add_executable(FileTag
        src/main.cpp
        src/file_tag_system.cpp
        src/tag_manager.cpp
        src/user_manager.cpp
        src/mainwindow.cpp
        src/MultiSelectDialog.cpp
        src/Logger.cpp
        src/FileProcessor.cpp
        src/FileSearch.cpp
        resources/resources.qrc
)

# 设置包含目录
target_include_directories(FileTag PRIVATE ${CMAKE_SOURCE_DIR}/src)

# 链接Qt6库
target_link_libraries(FileTag ${QT_LIBRARIES})

# 添加自定义目标 clean-all，用于清理生成的文件
add_custom_target(clean-all
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_clean.cmake
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
)

# 创建 cmake_clean.cmake 文件，用于清理生成目录
file(WRITE ${CMAKE_BINARY_DIR}/cmake_clean.cmake "file(REMOVE_RECURSE ${CMAKE_BINARY_DIR})")
