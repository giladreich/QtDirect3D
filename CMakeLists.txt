
cmake_minimum_required(VERSION 3.12)

##################################################################################################################
# Setup
##################################################################################################################
project(QtDirect3D
    LANGUAGES C CXX
    DESCRIPTION "Qt Direct3D Widgets & Examples."
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
if (BUILD_WITH_IMGUI)
    add_subdirectory(thirdparty)
endif()

add_subdirectory(source)
add_subdirectory(examples)

##################################################################################################################
# clang-format
##################################################################################################################
if (USE_CLANG_FORMAT)
    set(DIRS_TO_FORMAT
        source/*
        examples/*
    )
    file(GLOB_RECURSE FILES_TO_FORMAT ${DIRS_TO_FORMAT})
    list(FILTER FILES_TO_FORMAT INCLUDE REGEX "^.*\.(cpp|h|hpp|cc|cxx)$")
    list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
    include(ClangFormat)
    clangformat_setup(${FILES_TO_FORMAT})
endif()
