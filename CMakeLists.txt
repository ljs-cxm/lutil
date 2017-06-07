cmake_minimum_required(VERSION 3.0)
project(lutil C)

set(_POSSIBLE_LUA_INCLUDE lua51 lua5.1 lua-5.1 luajit-2.0 luajit-2.1)
find_path(LUA_INCLUDE_DIR
    NAMES lua.h
    HINTS $ENV{LUA_DIR}
    PATHS ${CMAKE_CURRENT_SOURCE_DIR} /usr/include /usr/local/include
    PATH_SUFFIXES ${_POSSIBLE_LUA_INCLUDE}
    NO_DEFAULT_PATH
    )

if(NOT LUA_INCLUDE_DIR)
    message(FATAL_ERROR "Lua5.1 include directory not found")
else()
    message(STATUS "LUA_INCLUDE_DIR: ${LUA_INCLUDE_DIR}")
endif()

if(UNIX)
    set(EXTRA_C_FLAGS "-O3 -std=c99 -Wall -pedantic -fomit-frame-pointer -Wall -DNDEBUG")
    if(APPLE)
        set(LINK_FLAGS "-dynamiclib -single_module -undefined dynamic_lookup")
    else(APPLE)
        set(LINK_FLAGS "-shared")
    endif(APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC ${EXTRA_C_FLAGS}")
else(UNIX)
    if(WIN32)
        add_definitions(-DLUA_BUILD_AS_DLL -DLUA_LIB)
    else(WIN32)
        message(FATAL_ERROR "unsupported platform")
    endif(WIN32)
endif(UNIX)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LUA_INCLUDE_DIR}
    )

set(SOURCES
    ltrim.c
    lsplit.c
    lip.c
    lutil.c
    )

add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
if(LINK_FLAGS)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS ${LINK_FLAGS}
        )
endif()
