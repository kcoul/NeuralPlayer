set(TargetName "SpleeterRTBin")

project(${TargetName} VERSION 0.0.1)

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
    target_include_directories(${TargetName} PUBLIC "/opt/intel/oneapi/mkl/latest/include")
    target_link_directories(${TargetName} PUBLIC "/opt/intel/oneapi/mkl/latest/lib")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_include_directories(${TargetName} PUBLIC "C:\\Program Files (x86)\\Intel\\oneAPI\\mkl\\latest\\include")
    target_link_directories(${TargetName} PUBLIC
            "C:\\Program Files (x86)\\Intel\\oneAPI\\mkl\\latest\\lib\\intel64")
endif()

target_link_libraries(${TargetName} PUBLIC
        #Linking order matters! Alphabetic often will not work here.
        mkl_intel_lp64 #Depends on mkl_core
        mkl_core
        mkl_sequential)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${TargetName} PUBLIC
            openblas
            m)
endif()