set(TargetName "SpleeterRTBin")

project(${TargetName} VERSION 0.0.1)

if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/model.c)
    execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_CURRENT_LIST_DIR}/model.7z
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )
endif()

add_executable(${TargetName}
        codelet.c
        cpthread.c
        gemm.c
        im2col_dilated.c
        libsr_samplerate.c
        libsr_src_sinc.c
        main.c
        spleeter.c
        stftFix.c)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_include_directories(${TargetName} PUBLIC "~/intel/oneapi/mkl/latest/include")
    target_link_directories(${TargetName} PUBLIC
            "~/intel/oneapi/mkl/latest/lib/intel64"
            "~/OpenBLAS/lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        target_include_directories(${TargetName} PUBLIC "/opt/intel/oneapi/mkl/latest/include")
        target_link_directories(${TargetName} PUBLIC "/opt/intel/oneapi/mkl/latest/lib")
    endif()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_include_directories(${TargetName} PUBLIC "C:\\Program Files (x86)\\Intel\\oneAPI\\mkl\\latest\\include")
    target_link_directories(${TargetName} PUBLIC
            "C:\\Program Files (x86)\\Intel\\oneAPI\\mkl\\latest\\lib\\intel64")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_definitions(${TargetName} PUBLIC NP_DEBUG=1)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${TargetName} PUBLIC NP_RELEASE=1)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    target_compile_definitions(${TargetName} PUBLIC NP_RELWITHDEBINFO=1)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
    target_compile_definitions(${TargetName} PUBLIC NP_MINSIZEREL=1)
endif()

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    target_link_libraries(${TargetName} PUBLIC
            #Linking order matters! Alphabetic often will not work here.
            mkl_intel_lp64 #Depends on mkl_core
            mkl_core
            mkl_sequential)
else()
    target_link_libraries(${TargetName} PUBLIC
            "-framework Accelerate")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${TargetName} PUBLIC
            openblas
            m)
endif()