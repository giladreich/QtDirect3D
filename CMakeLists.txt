
cmake_minimum_required(VERSION 3.12)

project(QtDirect3D
    LANGUAGES C CXX
    DESCRIPTION "DirectX Widgets for Qt and demos."
)

add_subdirectory(src)

set_property (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Direct3D9Widget)
