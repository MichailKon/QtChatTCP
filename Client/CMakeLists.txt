cmake_minimum_required(VERSION 3.22)
project(Client)

set(CMAKE_AUTOUIC_SEARCH_PATHS uis)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(AUTOGEN_BUILD_DIR src)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        Quick
        REQUIRED)

file(COPY qss DESTINATION .)

include_directories(headers uis ..)

set(SOURCES ${SOURCES} src/ChooseAddressPort.cpp src/MainWindow.cpp src/ClientConnection.cpp ../TransferProtocol.cpp src/MyButton.cpp src/MyLineEdit.cpp src/ChooseName.cpp src/MyTabBar.cpp src/MyTabWidget.cpp)
set(HEADERS ${HEADERS} headers/ChooseAddressPort.h headers/MainWindow.h headers/ClientConnection.h ../TransferProtocol.h headers/MyButton.h headers/MyLineEdit.h headers/ChooseName.h headers/MyTabBar.h headers/MyTabWidget.h)
set(UIS ${UIS} uis/ChooseAddressPort.ui uis/MainWindow.ui uis/ChooseName.ui)

add_executable(Client src/main.cpp ${SOURCES} ${HEADERS} ${UIS})

target_link_libraries(Client
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network
        Qt::Quick
        )

target_include_directories(Client PRIVATE headers uis ..)

if (WIN32)
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
