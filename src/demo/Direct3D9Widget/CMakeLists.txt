
cmake_minimum_required(VERSION 3.12)

##################################################################################################################
# Setup
##################################################################################################################
project(Direct3D9Widget)

find_package(Qt5 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
)

if (Qt5_FOUND)
    message(STATUS "[INFO] Qt5 Found: ${Qt5_DIR}")
else()
    message(FATAL_ERROR "[ERROR] Could not find Qt5 package...")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QtDirect3D_Widgets_Dir}/QDirect3D9Widget
)

message(STATUS "[INFO] Include Directories:")
get_property(_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${_dirs})
  message(STATUS "Dir: ${dir}")
endforeach()

##################################################################################################################
# Project Files
##################################################################################################################
set(WIDGETS_CXX_FILES
    ${QtDirect3D_Widgets_Dir}/QDirect3D9Widget/QDirect3D9Widget.h
    ${QtDirect3D_Widgets_Dir}/QDirect3D9Widget/QDirect3D9Widget.cpp
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
file(GLOB_RECURSE GENERATED_CXX_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/*.h
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/*.cpp
)
list(FILTER GENERATED_CXX_FILES EXCLUDE REGEX "moc_.*\.*")

##################################################################################################################
# IDE Representation
##################################################################################################################
source_group("Sources" FILES
    ${SOURCES_CXX_FILES}
    ${HEADERS_CXX_FILES}
)
source_group("Form Files" FILES
    ${UI_FILES}
)
source_group("Widgets" FILES
    ${WIDGETS_CXX_FILES}
)
source_group("Resource Files" FILES
    ${RESOURCE_FILES}
)
source_group("Generated Files" FILES
    ${GENERATED_CXX_FILES}
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
    ${GENERATED_CXX_FILES}
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    d3d9
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)
