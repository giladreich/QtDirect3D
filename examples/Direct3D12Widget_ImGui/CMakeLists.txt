
cmake_minimum_required(VERSION 3.12)

##################################################################################################################
# Setup
##################################################################################################################
project(Direct3D12Widget_ImGui)


add_compile_definitions(
    # https://github.com/ocornut/imgui/pull/2406
    ImTextureID=ImU64
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QTDIRECT3D_WIDGETS_DIR}/QDirect3D12Widget/ImGui
    ${IMGUI_ROOT_DIR}
    ${IMGUI_ROOT_DIR}/examples
)

##################################################################################################################
# Project Files
##################################################################################################################
set(WIDGETS_CXX_FILES
    ${QTDIRECT3D_WIDGETS_DIR}/QDirect3D12Widget/ImGui/QDirect3D12Widget.h
    ${QTDIRECT3D_WIDGETS_DIR}/QDirect3D12Widget/ImGui/QDirect3D12Widget.cpp
)
set(IMGUI_IMPL_HEADERS_CXX_FILES
    ${IMGUI_ROOT_DIR}/backends/imgui_impl_dx12.h
    ${IMGUI_ROOT_DIR}/backends/imgui_impl_win32.h
)
set(IMGUI_IMPL_SOURCES_CXX_FILES
    ${IMGUI_ROOT_DIR}/backends/imgui_impl_dx12.cpp
    ${IMGUI_ROOT_DIR}/backends/imgui_impl_win32.cpp
)
set(IMGUI_FILES
    ${IMGUI_HEADERS_CXX_FILES}
    ${IMGUI_SOURCES_CXX_FILES}
    ${IMGUI_IMPL_HEADERS_CXX_FILES}
    ${IMGUI_IMPL_SOURCES_CXX_FILES}
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

##################################################################################################################
# IDE Representation
##################################################################################################################
source_group("Source Files" FILES
    ${SOURCES_CXX_FILES}
    ${HEADERS_CXX_FILES}
)
source_group("Resources" FILES
    ${UI_FILES}
)
source_group("Widgets" FILES
    ${WIDGETS_CXX_FILES}
)
source_group("ImGui" FILES
    ${IMGUI_HEADERS_CXX_FILES}
    ${IMGUI_SOURCES_CXX_FILES}
)
source_group("ImGui\\Backend" FILES
    ${IMGUI_IMPL_HEADERS_CXX_FILES}
    ${IMGUI_IMPL_SOURCES_CXX_FILES}
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
    ${IMGUI_FILES}
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    d3d12
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ImGui)
