add_library(Boost STATIC IMPORTED)

message("${CMAKE_CURRENT_SOURCE_DIR}/libs/Boost-Filesystem/iOS-static/libboost_filesystem.a")

set_property(TARGET Boost PROPERTY
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/libs/Boost-Filesystem/iOS-static/libboost_filesystem.a")