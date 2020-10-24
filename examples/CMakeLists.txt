
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
    message(STATUS "[INFO] Qt5 Found -> ${Qt5_DIR}")
else()
    message(FATAL_ERROR "[ERROR] Failed to find Qt5. Did you add Qt installation kit bin directory to the system PATH?")
endif()

##################################################################################################################
# Submodules
##################################################################################################################
if (INCLUDE_DX9_EXAMPLE)
    add_subdirectory(Direct3D9Widget)
endif()

if (INCLUDE_DX10_EXAMPLE)
    add_subdirectory(Direct3D10Widget)
endif()

if (INCLUDE_DX11_EXAMPLE)
    add_subdirectory(Direct3D11Widget)
endif()

if (INCLUDE_DX12_EXAMPLE)
    add_subdirectory(Direct3D12Widget)
endif()

if (BUILD_WITH_IMGUI)
    message(STATUS "[INFO] Adding Dear ImGui examples.")
    if (INCLUDE_DX9_EXAMPLE)
        add_subdirectory(Direct3D9Widget_ImGui)
    endif()

    if (INCLUDE_DX10_EXAMPLE)
        add_subdirectory(Direct3D10Widget_ImGui)
    endif()

    if (INCLUDE_DX11_EXAMPLE)
        add_subdirectory(Direct3D11Widget_ImGui)
    endif()

    if (INCLUDE_DX12_EXAMPLE)
        add_subdirectory(Direct3D12Widget_ImGui)
    endif()
endif()
