include(CPM)

CPMAddPackage(
        NAME OnnxRuntime
        GITHUB_REPOSITORY Microsoft/onnxruntime
        GIT_TAG c57cf374b67f72575546d7b4c69a1af4972e2b54) #c57cf maps to 1.14.1 release tag

#Build for host architecture only for now, "--build" skips unit tests even when not cross-compiling
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
            execute_process(COMMAND bash ${OnnxRuntime_SOURCE_DIR}/build.sh --config RelWithDebInfo
                    --build --build_shared_lib --parallel --cmake_extra_defines CMAKE_OSX_ARCHITECTURES=x86_64)
        elseif(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
            execute_process(COMMAND bash ${OnnxRuntime_SOURCE_DIR}/build.sh --config RelWithDebInfo
                    --build --build_shared_lib --parallel --cmake_extra_defines CMAKE_OSX_ARCHITECTURES=arm64)
            #to build FAT Binary, longer build time, can't be built on Intel mac
            #execute_process(
            #        COMMAND bash ${OnnxRuntime_SOURCE_DIR}/build.sh --config RelWithDebInfo
            #        --build --build_shared_lib --parallel --cmake_extra_defines CMAKE_OSX_ARCHITECTURES="x86_64;arm64")
        endif()
endif()