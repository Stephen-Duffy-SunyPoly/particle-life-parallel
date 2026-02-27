cmake_minimum_required(VERSION 3.21)

project(openFrameworks LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Collect core sources
file(GLOB_RECURSE OF_SRC
        libs/openFrameworks/*.cpp
)


add_library(openFrameworks STATIC ${OF_SRC})

target_include_directories(openFrameworks PUBLIC
        libs/openFrameworks
        addons/ofxGui/src
)

target_compile_features(openFrameworks PUBLIC cxx_std_20)

# ---- Dependencies ----
find_package(OpenGL REQUIRED)

# Build bundled GLFW
add_subdirectory(libs/glfw)

# Link to it

if (WIN32)
target_link_libraries(openFrameworks
        PUBLIC
        OpenGL::GL
        glfw
        winmm
        ws2_32
        glew
)
else ()
    target_link_libraries(openFrameworks
            PRIVATE
            OpenGL::GL
            GLU
            X11
            pthread
            dl
            glfw
            glew
            GLEW::GLEW
    )
endif ()