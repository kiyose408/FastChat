cmake_minimum_required(VERSION 3.16)

project(fastchat VERSION 0.1 LANGUAGES CXX)

# 启用 Qt 自动处理 .ui / .qrc / Q_OBJECT
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

#find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
# 或者如果您使用的是 Qt6:
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network WebSockets)

# 源文件
set(PROJECT_SOURCES
    src/main.cpp
    src/ui/chatmainwindow.cpp
    src/ui/chatmainwindow.h
    src/ui/chatmainwindow.ui
    src/core/configmanager.cpp
    src/core/configmanager.h
    src/utils/logger.cpp
    src/utils/logger.h
)

# 创建可执行文件
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(fastchat
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/core/configmanager.h src/core/configmanager.cpp
        src/utils/logger.h src/utils/logger.cpp
        resources/icons/call.svg resources/icons/emoji.svg resources/icons/file.svg resources/icons/friend.svg resources/icons/friend_select.svg resources/icons/more.svg resources/icons/msg.svg resources/icons/msg_select.svg resources/icons/quit.svg resources/icons/setting.svg resources/icons/video_call.svg resources/icons/voice.svg
        resources.qrc
        src/ui/logindialog.h src/ui/logindialog.cpp src/ui/logindialog.ui
        src/core/IAuthService.h
        src/core/MockAuthService.h src/core/MockAuthService.cpp
        src/core/FriendModel.h src/core/FriendModel.cpp
        src/ui/FriendDelegate.h src/ui/FriendDelegate.cpp
        src/core/MessageModel.h src/core/MessageModel.cpp
        src/ui/MessageDelegate.h src/ui/MessageDelegate.cpp
        src/core/SessionManager.h src/core/SessionManager.cpp
        src/core/ApiService.h src/core/ApiService.cpp
        src/core/WebSocketClient.h src/core/WebSocketClient.cpp
        resources/icons/app.svg resources/icons/call.svg resources/icons/emoji.svg resources/icons/emoji_select.svg resources/icons/file.svg resources/icons/friend.svg resources/icons/friend_select.svg resources/icons/Logindialog.svg resources/icons/more.svg resources/icons/msg.svg resources/icons/msg_select.svg resources/icons/quit.svg resources/icons/send.svg resources/icons/setting.svg resources/icons/user.png resources/icons/video_call.svg resources/icons/voice.svg

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET fastchat APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(fastchat SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(fastchat
            ${PROJECT_SOURCES}
        )
    endif()
endif()

#设置include路径        set the way of include
target_include_directories(fastchat PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(fastchat PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(fastchat PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
)
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.fastchat)
endif()
set_target_properties(fastchat PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS fastchat
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(fastchat)
endif()
