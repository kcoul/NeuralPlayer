# Checks for fast math optimization compiler support if enabled.
# While this speeds up the generated code, there may be some side effects leading to incorrect math results.
# These should only affect edge cases, e.g. if ops result in NaN or Inf. This will in general not cause any
# visible artifacts, but may be undesirable in some uses. In this case, disable the ENABLE_FAST_MATH option
# and specify the safe set of flags via the CMAKE_C_FLAGS and CMAKE_CXX_FLAGS variables.

if(ENABLE_FAST_MATH)
    include(CheckCCompilerFlag)
    if(MSVC)
        check_c_compiler_flag("/fp:fast" FAST_MATH_OPTIMIZATION_SUPPORTED)
        if(FAST_MATH_OPTIMIZATION_SUPPORTED)
            add_compile_options($<$<CONFIG:Release,RelWithDebInfo>:/fp:fast>)
        endif()
    else()
        check_c_compiler_flag("-Ofast" FAST_MATH_OPTIMIZATION_SUPPORTED)
        if(FAST_MATH_OPTIMIZATION_SUPPORTED)
            add_compile_options($<$<CONFIG:Release,RelWithDebInfo>:-Ofast>)
        else()
            check_c_compiler_flag("-O3" ALL_OPTIMIZATIONS_FLAG_SUPPORTED)
            if(ALL_OPTIMIZATIONS_FLAG_SUPPORTED)
                add_compile_options($<$<CONFIG:Release,RelWithDebInfo>:-O3>)
            endif()
        endif()
    endif()
endif()
