
cmake_minimum_required(VERSION 3.12)

set(THIRDPARTY_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "")

set(IMGUI_ROOT_DIR ${THIRDPARTY_ROOT_DIR}/imgui CACHE STRING "")

if (NOT EXISTS "${IMGUI_ROOT_DIR}/imgui.h")
    find_package(Git REQUIRED)
    if (Git_FOUND)
        message(STATUS "[INFO] Found Git Client -> ${GIT_EXECUTABLE}")
    else()
        message(FATAL_ERROR "[ERROR] Failed to find Git client. Please install Git and try again.")
    endif()

    set(IMGUI_REPO "https://github.com/ocornut/imgui.git")
    set(IMGUI_TAG  "v1.76")
    message(STATUS "[INFO] Cloning imgui. Please wait...")
    execute_process(
        COMMAND           ${GIT_EXECUTABLE} clone --recursive --branch ${IMGUI_TAG} ${IMGUI_REPO} ${IMGUI_ROOT_DIR}
        RESULT_VARIABLE   GIT_RESULT
        OUTPUT_VARIABLE   GIT_OUTPUT
    )
endif()

set(IMGUI_SOURCES_CXX_FILES
    ${IMGUI_ROOT_DIR}/imgui.cpp
    ${IMGUI_ROOT_DIR}/imgui_demo.cpp
    ${IMGUI_ROOT_DIR}/imgui_draw.cpp
    ${IMGUI_ROOT_DIR}/imgui_widgets.cpp
    CACHE STRING ""
)

set(IMGUI_HEADERS_CXX_FILES
    ${IMGUI_ROOT_DIR}/imconfig.h
    ${IMGUI_ROOT_DIR}/imgui.h
    ${IMGUI_ROOT_DIR}/imgui_internal.h
    ${IMGUI_ROOT_DIR}/imstb_rectpack.h
    ${IMGUI_ROOT_DIR}/imstb_textedit.h
    ${IMGUI_ROOT_DIR}/imstb_truetype.h
    CACHE STRING ""
)
