cmake_minimum_required(VERSION 3.24)
project(particle_life LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

########################################
# Fetch 3rd‑party libraries
########################################

# ----- GLFW -----
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# ----- GLM -----
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

# ----- GLEW (release archive) -----
FetchContent_Declare(
        glew
        URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz
)
FetchContent_MakeAvailable(glew)

add_library(glew_static STATIC ${glew_SOURCE_DIR}/src/glew.c)
target_include_directories(glew_static PUBLIC ${glew_SOURCE_DIR}/include)
target_compile_definitions(glew_static PRIVATE GLEW_STATIC)

# ----- freetype -----
FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
        GIT_TAG        VER-2-13-2
)
FetchContent_MakeAvailable(freetype)

# ----- libtess2 -----
FetchContent_Declare(
        libtess2
        GIT_REPOSITORY https://github.com/memononen/libtess2.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(libtess2)

file(GLOB TESS2_SOURCES
        "${libtess2_SOURCE_DIR}/Source/*.c"
)
add_library(lib_tess2 STATIC ${TESS2_SOURCES})
target_include_directories(lib_tess2 PUBLIC
        ${libtess2_SOURCE_DIR}/include
)

# ----- utf8cpp -----
FetchContent_Declare(
        utf8cpp
        GIT_REPOSITORY https://github.com/nemtrif/utfcpp.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(utf8cpp)

add_library(utf8cpp_lib INTERFACE)
target_include_directories(utf8cpp_lib INTERFACE
        ${utf8cpp_SOURCE_DIR}/source
)

# ----- stb (header‑only) -----
FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

add_library(stb_headers INTERFACE)
target_include_directories(stb_headers INTERFACE
        ${stb_SOURCE_DIR}
)

# ----- pugixml -----
FetchContent_Declare(
        pugixml
        GIT_REPOSITORY https://github.com/zeux/pugixml.git
        GIT_TAG        v1.13
)
FetchContent_MakeAvailable(pugixml)

add_library(pugixml_lib STATIC
        ${pugixml_SOURCE_DIR}/src/pugixml.cpp
)
target_include_directories(pugixml_lib PUBLIC
        ${pugixml_SOURCE_DIR}/src
)

# ----- tinyfiledialogs -----
FetchContent_Declare(
        tinyfiledialogs
        GIT_REPOSITORY https://git.code.sf.net/p/tinyfiledialogs/code tinyfiledialogs-code
        GIT_TAG        master
)
FetchContent_MakeAvailable(tinyfiledialogs)

add_library(tinyfd_lib STATIC
        ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c
)
target_include_directories(tinyfd_lib PUBLIC
        ${tinyfiledialogs_SOURCE_DIR}
)

# ---- nlohmann json ----
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)


# ---- freeimage ----
FetchContent_Declare(
        freeimage
        GIT_REPOSITORY https://github.com/danoli3/FreeImage.git
        GIT_TAG master
)
FetchContent_MakeAvailable(freeimage)

# ---- uriparser ----
FetchContent_Declare(
        uriparser
        GIT_REPOSITORY https://github.com/uriparser/uriparser.git
        GIT_TAG        uriparser-1.0.0
)

set(URIPARSER_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(URIPARSER_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(uriparser)

# ---- RT Audio ----
#GLFW also defines an uninstall so make this one diffrent

FetchContent_Declare(
        rtaudio
        GIT_REPOSITORY https://github.com/thestk/rtaudio.git
        GIT_TAG master
)
set(RTAUDIO_TARGETNAME_UNINSTALL "rtaudio_uninstall" CACHE STRING "Rename uninstall target to avoid conflicts")
FetchContent_MakeAvailable(rtaudio)

########################################
# Build openFrameworks core
########################################

file(GLOB_RECURSE OF_APP_SRC
        external/openFrameworks/libs/openFrameworks/app/*.cpp
)

list(FILTER OF_APP_SRC EXCLUDE REGEX "ofAppEGLWindow.cpp")
list(FILTER OF_APP_SRC EXCLUDE REGEX "ofAppGLUTWindow.cpp")

file(GLOB_RECURSE OF_GRAPHICS_SRC
        external/openFrameworks/libs/openFrameworks/graphics/*.cpp
)

# Remove Cairo renderer
list(FILTER OF_GRAPHICS_SRC EXCLUDE REGEX "ofCairoRenderer.cpp")
list(FILTER OF_GRAPHICS_SRC EXCLUDE REGEX "ofGraphicsCairo.cpp")

file(GLOB_RECURSE OF_UTILS_SRC
        external/openFrameworks/libs/openFrameworks/utils/*.cpp
)

file(GLOB_RECURSE OF_TYPES_SRC
        external/openFrameworks/libs/openFrameworks/types/*.cpp
)

file(GLOB_RECURSE OF_MATH_SRC
        external/openFrameworks/libs/openFrameworks/math/*.cpp
)

file(GLOB_RECURSE OF_GL_SRC
        external/openFrameworks/libs/openFrameworks/gl/*.cpp
)

file(GLOB_RECURSE OF_3D_SRC
        external/openFrameworks/libs/openFrameworks/3d/*.cpp
)

file(GLOB_RECURSE OF_EVENTS_SRC
        external/openFrameworks/libs/openFrameworks/events/*.cpp
)

file(GLOB_RECURSE OF_SOUNDS_SRC
        external/openFrameworks/libs/openFrameworks/sound/*.cpp
)

file(GLOB_RECURSE OF_COMMUNICATION_SRC
        external/openFrameworks/libs/openFrameworks/communication/*.cpp
)

file(GLOB_RECURSE OF_VIDEO_SRC
        external/openFrameworks/libs/openFrameworks/video/*.cpp
)
if(WIN32)
    list(FILTER OF_VIDEO_SRC EXCLUDE REGEX "ofGstUtils.cpp")
    list(FILTER OF_VIDEO_SRC EXCLUDE REGEX "ofGstVideoGrabber.cpp")
    list(FILTER OF_VIDEO_SRC EXCLUDE REGEX "ofGstVideoPlayer.cpp")
endif()

#add_library(FreeImageLib STATIC IMPORTED)
#set_target_properties(FreeImageLib PROPERTIES
#        IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/_deps/freeimage-build/FreeImage.lib"
#        INTERFACE_INCLUDE_DIRECTORIES "${freeimage_SOURCE_DIR}/Source"
#)
set(FREEIMAGE_BUILD_SHARED OFF CACHE BOOL "" FORCE)  # Build static lib
set(FREEIMAGE_USE_DLL OFF CACHE BOOL "" FORCE)       # Disable DLL linkage

add_library(openFrameworks STATIC
        ${OF_APP_SRC}
        ${OF_GRAPHICS_SRC}
        ${OF_UTILS_SRC}
        ${OF_TYPES_SRC}
        ${OF_MATH_SRC}
        ${OF_GL_SRC}
        ${OF_3D_SRC}
        ${OF_EVENTS_SRC}
        ${OF_SOUNDS_SRC}
        ${OF_COMMUNICATION_SRC}
#        ${OF_VIDEO_SRC}
)

target_include_directories(openFrameworks PUBLIC
        external/openFrameworks/libs/openFrameworks
        external/openFrameworks/libs/openFrameworks/app
        external/openFrameworks/libs/openFrameworks/graphics
        external/openFrameworks/libs/openFrameworks/utils
        external/openFrameworks/libs/openFrameworks/types
        external/openFrameworks/libs/openFrameworks/math
        external/openFrameworks/libs/openFrameworks/gl
        external/openFrameworks/libs/openFrameworks/3d
        external/openFrameworks/libs/openFrameworks/events
        external/openFrameworks/libs/openFrameworks/sound
        external/openFrameworks/libs/openFrameworks/video
        external/openFrameworks/libs/openFrameworks/communication
        ${freeimage_SOURCE_DIR}/Source # may be a kinda tmp solution but it works
        ${rtaudio_SOURCE_DIR}/include
)

target_link_libraries(openFrameworks PUBLIC
        glfw
        glm
        freetype
        glew_static
        lib_tess2
        utf8cpp_lib
        stb_headers
        pugixml_lib
        tinyfd_lib
        nlohmann_json::nlohmann_json
        uriparser::uriparser
        rtaudio
        FreeImage
)


target_link_libraries(openFrameworks PRIVATE FreeImage)

file(GLOB OFXGUI_SRC
        external/openFrameworks/addons/ofxGui/src/*.cpp
)

add_library(ofxGui STATIC ${OFXGUI_SRC})

target_include_directories(ofxGui PUBLIC
        external/openFrameworks/addons/ofxGui/src
        external/openFrameworks/libs/openFrameworks
)

target_link_libraries(ofxGui PUBLIC openFrameworks)

target_compile_features(openFrameworks PUBLIC cxx_std_20)

target_compile_definitions(openFrameworks PUBLIC
        OF_NO_FREEIMAGE
        TARGET_IMPLEMENTS_URL_LOADER
        TARGET_NO_SOUND
        OF_NO_GSTREAMER
        FREEIMAGE_LIB
)

# Windows specific system libraries
if (WIN32)
    target_link_libraries(openFrameworks PUBLIC
            opengl32
            winmm
            ws2_32
            gdi32
            ole32
            uuid
            comdlg32
    )
endif()

########################################
# Your Application
########################################

#copy the data folder
add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data
)

add_executable(particle_life
        src/main.cpp
        src/ofApp.cpp
)

add_dependencies(particle_life copy_assets)

target_include_directories(particle_life PRIVATE
        external/openFrameworks/addons/ofxGui/src
)

target_link_libraries(particle_life PRIVATE openFrameworks ofxGui)