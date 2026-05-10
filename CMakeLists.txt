cmake_minimum_required(VERSION 3.0)
project(StickersManager)
# ------------------
# 设置是否为debug模式
set(D_OR_R OFF)#ON for Debug, OFF for Release

# 是否开启控制台调试输出
set(CONSOLE OFF)
# ------------------

# 启用 Unicode 模式（Windows 平台）
if (WIN32)
    add_compile_definitions(UNICODE _UNICODE)
endif ()

set(CMAKE_PREFIX_PATH "D:/Qt/6.10.1/mingw_64")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
# 设置第三方库路径
set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Concurrent
        REQUIRED)
FILE(GLOB SOURCE_FILES "./src/*.cpp")
FILE(GLOB HEADER_FILES "./src/*.h")
FILE(GLOB HPP_FILES "./src/*.hpp")

# 头文件搜索路径
include_directories(
        ./
        src/
        thirdparty/stb
)
# 资源文件
set(RESOURCE_FILES
        resources.qrc
)

if (NOT CONSOLE)
    set(CON WIN32)
endif ()
# 明确设置 UI 文件目录
set(UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/ui")

include_directories(StickersManager PRIVATE
        ${UI_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}  # 重要：包含构建目录，ui_*.h文件在这里生成
)

add_executable(StickersManager
        ${CON}
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${HPP_FILES}
        ${RESOURCE_FILES}
)
target_link_libraries(${PROJECT_NAME}
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Concurrent
)
# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
        src
        ${THIRDPARTY_DIR}/stb
)
# 设置图标（Windows）
if (WIN32)
    # 1. 定义图标文件路径和rc脚本路径
    set(APP_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/assets/st.ico")
    set(APP_RC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.rc")

    # 2. 检查图标和rc脚本是否存在
    if (EXISTS ${APP_ICON_FILE} AND EXISTS ${APP_RC_FILE})
        # 3. 将rc脚本文件加入项目编译（核心：通过rc脚本绑定图标）
        target_sources(${PROJECT_NAME} PRIVATE ${APP_RC_FILE})

        # 4. 配合你的CONSOLE开关，设置是否为无控制台的GUI程序（完美联动你的原有逻辑）
        if (NOT CONSOLE)
            set_target_properties(${PROJECT_NAME} PROPERTIES
                    WIN32_EXECUTABLE TRUE  # 无控制台窗口
            )
        else ()
            set_target_properties(${PROJECT_NAME} PROPERTIES
                    WIN32_EXECUTABLE FALSE # 带控制台窗口
            )
        endif ()
    else ()
        message(WARNING "Program icon file or RC script is missing, the default icon will be used!")
    endif ()
endif ()

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
