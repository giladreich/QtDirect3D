
cmake_minimum_required(VERSION 3.12)

##################################################################################################################
# Setup
##################################################################################################################
project(Direct3D11Widget)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QTDIRECT3D_WIDGETS_DIR}/QDirect3D11Widget
)

##################################################################################################################
# Project Files
##################################################################################################################
set(WIDGETS_CXX_FILES
    ${QTDIRECT3D_WIDGETS_DIR}/QDirect3D11Widget/QDirect3D11Widget.h
    ${QTDIRECT3D_WIDGETS_DIR}/QDirect3D11Widget/QDirect3D11Widget.cpp
)
set(HEADERS_CXX_FILES
    MainWindow.h
)
set(SOURCES_CXX_FILES
    main.cpp
    MainWindow.cpp
)
set(UI_FILES
    mainwindow.ui
)
file(GLOB RESOURCE_FILES
    *.qrc
)

##################################################################################################################
# IDE Representation
##################################################################################################################
source_group("Source Files" FILES
    ${SOURCES_CXX_FILES}
    ${HEADERS_CXX_FILES}
)
source_group("Resources" FILES
    ${UI_FILES}
    ${RESOURCE_FILES}
)
source_group("Widgets" FILES
    ${WIDGETS_CXX_FILES}
)

##################################################################################################################
# Qt Framework
##################################################################################################################
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
#qt5_wrap_ui()
#qt5_wrap_cpp()

##################################################################################################################
# Target
##################################################################################################################
add_executable(${PROJECT_NAME} WIN32
    ${WIDGETS_CXX_FILES}
    ${HEADERS_CXX_FILES}
    ${SOURCES_CXX_FILES}
    ${UI_FILES}
    ${RESOURCE_FILES}
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    d3d11
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)
