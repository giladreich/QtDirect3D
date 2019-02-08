
cmake_minimum_required(VERSION 3.12)

##################################################################################################################
# Setup
##################################################################################################################
string(TOLOWER ${CMAKE_CXX_COMPILER_ARCHITECTURE_ID} _ARCHITECTURE)
set(BUILD_DIR ${CMAKE_BINARY_DIR}/bin/${_ARCHITECTURE}-vc${MSVC_TOOLSET_VERSION})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

##################################################################################################################
# Packages
##################################################################################################################
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

##################################################################################################################
# Submodules
##################################################################################################################
add_subdirectory(Direct3D9Widget)
#add_subdirectory(Direct3D9ImGuiWidget)
#add_subdirectory(Direct3D10Widget)
#add_subdirectory(Direct3D10ImGuiWidget)
#add_subdirectory(Direct3D11Widget)
#add_subdirectory(Direct3D11ImGuiWidget)
