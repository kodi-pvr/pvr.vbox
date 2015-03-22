project(libxmltv)
enable_language(CXX)
cmake_minimum_required(VERSION 2.6)

# Source files
set(XMLTV_SOURCES 
                  xmltv/Guide.h
                  xmltv/Guide.cpp
                  xmltv/Programme.h
                  xmltv/Programme.cpp
                  xmltv/Schedule.h
                  xmltv/Utilities.h
                  xmltv/Utilities.cpp)

source_group("Source Files" FILES ${XMLTV_SOURCES})

# Resource files
set(XMLTV_RESOURCES 
                  CMakeLists.txt)

source_group("Resource Files" FILES ${XMLTV_RESOURCES})

# Append our custom file definitions
list(APPEND XMLTV_SOURCES
    ${XMLTV_RESOURCES})

add_library(libxmltv STATIC ${XMLTV_SOURCES})
