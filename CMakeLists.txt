cmake_minimum_required(VERSION 3.10)
project(FileTag)

set(CMAKE_CXX_STANDARD 17)

# 启用自动生成 MOC 文件、RCC 文件和 UIC 文件
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找 Qt6 包，并指定插件路径
set(CMAKE_PREFIX_PATH "/opt/homebrew/Cellar/qt/6.7.0_1/lib/cmake")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Sql)

# 添加 Qt6 模块
set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Sql)

# 将所有源文件和头文件添加到可执行文件 FileTag 中
add_executable(FileTag
        src/main.cpp
        src/file_tag_system.cpp
        src/tag_manager.cpp
        src/user_manager.cpp
        src/mainwindow.cpp
        src/mainwindow.ui
        src/MultiSelectDialog.cpp
        src/Logger.cpp
        src/FileProcessor.cpp
        src/FileSearch.cpp
        src/file_tag_system.h
        src/tag_manager.h
        src/user_manager.h
        src/mainwindow.h
        src/MultiSelectDialog.h
        src/Logger.h
        src/FileProcessor.h
        src/FileSearch.h
        src/FileSearch.ui
        resources/resources.qrc
        src/CustomModel.h
        src/CustomModel.cpp
        src/FileSearchThread.cpp
        src/FileSearchThread.h
        src/about.h
        src/about.cpp
        src/about.ui
        src/FileDatabase.cpp
        src/FileDatabase.h
)

# 设置包含目录
target_include_directories(FileTag PRIVATE ${CMAKE_SOURCE_DIR}/src)

# 链接 Qt6 库
target_link_libraries(FileTag ${QT_LIBRARIES})

# 添加自定义目标 clean-all，用于清理生成的文件
add_custom_target(clean-all
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_clean.cmake
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
)

# 创建 cmake_clean.cmake 文件，用于清理生成目录
file(WRITE ${CMAKE_BINARY_DIR}/cmake_clean.cmake "file(REMOVE_RECURSE ${CMAKE_BINARY_DIR})")

# 将 qt.conf 配置文件安装到可执行文件目录中，以便 Qt 能够找到插件
file(WRITE ${CMAKE_BINARY_DIR}/qt.conf
        "[Paths]\n"
        "Plugins = /opt/homebrew/Cellar/qt/6.7.0_1/share/qt/plugins\n"
)

# 复制 qt.conf 到构建目录和目标目录中
add_custom_command(TARGET FileTag POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/qt.conf
        $<TARGET_FILE_DIR:FileTag>
)

# 将 Qt 的 sqldrivers 目录复制到可执行文件所在的目录
add_custom_command(TARGET FileTag POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "/opt/homebrew/Cellar/qt/6.7.0_1/share/qt/plugins/sqldrivers"
        "$<TARGET_FILE_DIR:FileTag>/sqldrivers"
)
