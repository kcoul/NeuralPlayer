
set(PROJECTM_CONFIGURATION_FILE "${CMAKE_CURRENT_BINARY_DIR}/projectMSDL.properties")
set(PROJECTM_CONFIGURATION_FILE "${PROJECTM_CONFIGURATION_FILE}" PARENT_SCOPE)
configure_file(resources/projectMSDL.properties.in "${PROJECTM_CONFIGURATION_FILE}" @ONLY)

add_executable(projectMSDL WIN32
        AudioCapture.cpp
        AudioCapture.h
        FPSLimiter.cpp
        FPSLimiter.h
        main.cpp
        ProjectMSDLApplication.cpp
        ProjectMSDLApplication.h
        ProjectMWrapper.cpp
        ProjectMWrapper.h
        RenderLoop.cpp
        RenderLoop.h
        SDLRenderingWindow.h
        SDLRenderingWindow.cpp
        SDLRenderingWindow.cpp
        )

target_include_directories(projectMSDL PUBLIC
        "${CMAKE_SOURCE_DIR}/Visualizers/projectM/projectm-4.0.0/src/api/include"
        "${CMAKE_SOURCE_DIR}/Visualizers/projectM/projectm-4.0.0/src/playlist/api"
        "${CMAKE_BINARY_DIR}/Visualizers/projectM/projectm-4.0.0/src/api/include"
        "${CMAKE_BINARY_DIR}/Visualizers/projectM/projectm-4.0.0/src/playlist/include")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(projectMSDL
            PRIVATE
            AudioCaptureImpl_WASAPI.h
            AudioCaptureImpl_WASAPI.cpp
            )
    target_compile_definitions(projectMSDL
            PRIVATE
            AUDIO_IMPL_HEADER="AudioCaptureImpl_WASAPI.h"
            )
else()
    target_sources(projectMSDL
            PRIVATE
            AudioCaptureImpl_SDL.h
            AudioCaptureImpl_SDL.cpp
            )
    target_compile_definitions(projectMSDL
            PRIVATE
            AUDIO_IMPL_HEADER="AudioCaptureImpl_SDL.h"
            )
endif()

target_compile_definitions(projectMSDL
        PRIVATE
        PROJECTMSDL_VERSION="${PROJECT_VERSION}"
        )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
set_property(TARGET libprojectM::projectM PROPERTY
        IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/Visualizers/projectM/projectm-4.0.0/src/libprojectM/libprojectM-4d.0.0.1.dylib"
        )
set_property(TARGET libprojectM::playlist PROPERTY
        IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/Visualizers/projectM/projectm-4.0.0/src/playlist/libprojectM-4-playlistd.0.0.1.dylib"
        )
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET libprojectM::projectM PROPERTY
            #IMPORTED_LOCATION "/usr/local/lib/libprojectM-4.dylib"
            IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/Visualizers/projectM/projectm-4.0.0/src/libprojectM/libprojectM-4.0.0.1.dylib"
            )
    set_property(TARGET libprojectM::playlist PROPERTY
            #IMPORTED_LOCATION "/usr/local/lib/libprojectM-4-playlist.4.dylib"
            IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/Visualizers/projectM/projectm-4.0.0/src/playlist/libprojectM-4-playlist.0.0.1.dylib"
            )
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set_property(TARGET libprojectM::projectM PROPERTY
            #IMPORTED_LOCATION "/usr/local/lib/libprojectM-4.dylib"
            IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/Visualizers/projectM/projectm-4.0.0/src/libprojectM/libprojectM-4.0.0.1.dylib"
            )
    set_property(TARGET libprojectM::playlist PROPERTY
            #IMPORTED_LOCATION "/usr/local/lib/libprojectM-4-playlist.4.dylib"
            IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/Visualizers/projectM/projectm-4.0.0/src/playlist/libprojectM-4-playlist.0.0.1.dylib"
            )
endif()

target_link_libraries(projectMSDL
        PRIVATE
        libprojectM::playlist
        Poco::Util
        SDL2::SDL2$<$<STREQUAL:${SDL2_LINKAGE},static>:-static>
        SDL2::SDL2main
        Boost::filesystem
        )

if(MSVC)
    set_target_properties(projectMSDL
            PROPERTIES
            VS_DPI_AWARE "PerMonitor"
            )
endif()
