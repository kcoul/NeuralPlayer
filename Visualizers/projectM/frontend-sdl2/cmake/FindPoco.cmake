add_library(Poco STATIC IMPORTED)

message("${CMAKE_CURRENT_SOURCE_DIR}/libs/Poco/iOS-static/libPoco.a")

set_property(TARGET Poco PROPERTY
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/libs/Poco/iOS-static/libPoco.a")