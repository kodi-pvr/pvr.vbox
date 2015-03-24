project(libxmltv)
enable_language(CXX)
cmake_minimum_required(VERSION 2.6)

# Source files
set(XMLTV_SOURCES 
                  Guide.h
                  Guide.cpp
                  Programme.h
                  Programme.cpp
                  Schedule.h
                  Utilities.h
                  Utilities.cpp)

source_group("Source Files" FILES ${XMLTV_SOURCES})

# Resource files
set(XMLTV_RESOURCES 
                  CMakeLists.txt)

source_group("Resource Files" FILES ${XMLTV_RESOURCES})

# Append our custom file definitions
list(APPEND XMLTV_SOURCES
    ${XMLTV_RESOURCES})

# Create position-independent code (non-Windows	)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

add_library(libxmltv STATIC ${XMLTV_SOURCES})
