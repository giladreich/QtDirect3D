
cmake_minimum_required(VERSION 3.12)

##################################################################################################################
# Setup
##################################################################################################################
project(QtDirect3D
    LANGUAGES C CXX
    DESCRIPTION "DirectX Widgets for Qt and demos."
)

include(CMakeOptions.cmake)

##################################################################################################################
# IDE Representation
##################################################################################################################

# Groups CMake pre-defined projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY 
    VS_STARTUP_PROJECT Direct3D9Widget
)

##################################################################################################################
# Submodules
##################################################################################################################
add_subdirectory(src)
add_subdirectory(examples)

