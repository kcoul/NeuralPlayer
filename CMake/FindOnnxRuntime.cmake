include(CPM)

if(${BUILD_ONNXRUNTIME_FROM_SOURCE})
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        CPMAddPackage(
                NAME OnnxRuntime
                GITHUB_REPOSITORY Microsoft/onnxruntime
                GIT_TAG c57cf374b67f72575546d7b4c69a1af4972e2b54) #c57cf maps to 1.14.1 release tag

        #Build for host architecture only, adding "--build" skips unit tests once sub-build cache is available
        if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
                execute_process(COMMAND ./build.sh --config RelWithDebInfo
                        --build --build_shared_lib --parallel --cmake_extra_defines CMAKE_OSX_ARCHITECTURES=x86_64
                        WORKING_DIRECTORY ${OnnxRuntime_SOURCE_DIR})
            elseif(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
                execute_process(COMMAND ./build.sh --config RelWithDebInfo
                        --build --build_shared_lib --parallel --cmake_extra_defines CMAKE_OSX_ARCHITECTURES=arm64
                        WORKING_DIRECTORY ${OnnxRuntime_SOURCE_DIR})
            endif()
        elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            execute_process(COMMAND ./build.bat --config RelWithDebInfo
                    --build --build_shared_lib --parallel
                    WORKING_DIRECTORY ${OnnxRuntime_SOURCE_DIR})
        endif()
    endif()
endif()

add_library(onnxruntime SHARED IMPORTED)

if(${BUILD_ONNXRUNTIME_FROM_SOURCE})
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        #NOTE: Recompile onnxruntime in Debug mode if you need to debug into it with accurate Debugger stepping
        #NOTE: Looking for libonnxruntime.dylib in our most likely location, change path if your use-case is different
        set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION
                ${CMAKE_SOURCE_DIR}/cmake-build-debug/_deps/onnxruntime-src/build/MacOS/RelWithDebInfo/libonnxruntime.1.14.1.dylib)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION
                ${CMAKE_SOURCE_DIR}/cmake-build-debug/_deps/onnxruntime-src/build/Windows/RelWithDebInfo/onnxruntime.dll)
        set_property(TARGET onnxruntime PROPERTY IMPORTED_IMPLIB
                ${CMAKE_SOURCE_DIR}/cmake-build-debug/_deps/onnxruntime-src/build/Windows/RelWithDebInfo/onnxruntime.lib)

        #Copy onnxruntime .dll to Player build folder
        set(LIBRARY_FILE "${CMAKE_SOURCE_DIR}/cmake-build-debug/_deps/onnxruntime-src/build/Windows/RelWithDebInfo/onnxruntime.dll")

        set(LIBRARY_FILE_NAME "onnxruntime.dll")
        add_custom_command(TARGET ${TargetName} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy
                "${LIBRARY_FILE}"
                "${PROJECT_BINARY_DIR}/Player/Player_artefacts/${CMAKE_BUILD_TYPE}/${LIBRARY_FILE_NAME}"
                COMMENT "Copying library to output directory")
    endif()
else()
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
            set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION
                    ${CMAKE_SOURCE_DIR}/Player/Lib/onnxruntime/Darwin/x64/Release/libonnxruntime.1.14.1.dylib)
        elseif(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
            set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION
                    ${CMAKE_SOURCE_DIR}/Player/Lib/onnxruntime/Darwin/arm64/Release/libonnxruntime.1.14.1.dylib)
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set_property(TARGET onnxruntime PROPERTY IMPORTED_LOCATION
                ${CMAKE_SOURCE_DIR}/Player/Lib/onnxruntime/Windows/x64/Release/onnxruntime.dll)
        set_property(TARGET onnxruntime PROPERTY IMPORTED_IMPLIB
                ${CMAKE_SOURCE_DIR}/Player/Lib/onnxruntime/Windows/x64/Release/onnxruntime.lib)

        #Copy onnxruntime .dll to Player build folder
        if(CMAKE_SIZEOF_VOID_P EQUAL 8) #x64
            set(LIBRARY_FILE "${CMAKE_SOURCE_DIR}/Player/Lib/onnxruntime/Windows/x64/Release/onnxruntime.dll")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4) #x86
            set(LIBRARY_FILE "${CMAKE_SOURCE_DIR}/Player/Lib/onnxruntime/Windows/x86/Release/onnxruntime.dll")
        endif()

        set(LIBRARY_FILE_NAME "onnxruntime.dll")
        add_custom_command(TARGET ${TargetName} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy
                "${LIBRARY_FILE}"
                "${PROJECT_BINARY_DIR}/Player/Player_artefacts/${CMAKE_BUILD_TYPE}/${LIBRARY_FILE_NAME}"
                COMMENT "Copying library to output directory")
    endif()
endif()