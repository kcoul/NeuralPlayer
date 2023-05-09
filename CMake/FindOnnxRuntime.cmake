include(CPM)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    CPMAddPackage(
            NAME OnnxRuntime
            GITHUB_REPOSITORY Microsoft/onnxruntime
            GIT_TAG c57cf374b67f72575546d7b4c69a1af4972e2b54) #c57cf maps to 1.14.1 release tag

    #Build for host architecture only, adding "--build" skips unit tests once sub-build cache is available
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
                execute_process(COMMAND ./build.sh --config Release
                        --build --build_shared_lib --parallel --cmake_extra_defines CMAKE_OSX_ARCHITECTURES=x86_64
                        WORKING_DIRECTORY ${OnnxRuntime_SOURCE_DIR})
            elseif(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
                execute_process(COMMAND ./build.sh --config Release
                        --build --build_shared_lib --parallel --cmake_extra_defines CMAKE_OSX_ARCHITECTURES=arm64
                        WORKING_DIRECTORY ${OnnxRuntime_SOURCE_DIR})
            endif()
    endif()
endif()