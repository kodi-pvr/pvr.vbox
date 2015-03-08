project(pvr.vbox)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(kodi REQUIRED)
find_package(kodiplatform REQUIRED)
find_package(TinyXML2 REQUIRED)

include_directories(${kodiplatform_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}
                    ${TINYXML2_INCLUDE_DIRS})

set(VBOX_SOURCES src/client.cpp
                 src/vbox/Channel.cpp
                 src/vbox/Recording.cpp
                 src/vbox/VBox.cpp
                 src/vbox/request/Request.cpp
                 src/vbox/response/Content.cpp
                 src/vbox/response/Response.cpp
                 src/vbox/xmltv/Programme.cpp
                 src/vbox/xmltv/Utilities.cpp)

set(DEPLIBS ${kodiplatform_LIBRARIES} ${TINYXML2_LIBRARIES})
build_addon(pvr.vbox VBOX DEPLIBS)

include(CPack)
