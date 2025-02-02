# Highest version compatible with Steam Runtime container
cmake_minimum_required(VERSION 3.13)
include(CheckSymbolExists)
enable_testing()

project(sdl-bits C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
include(BuildType)

# https://stackoverflow.com/a/58711983
string(REGEX REPLACE "/W3" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# For dependency management, we use vcpkg.
# If that's not available, we use the default package manager and pkg-config for Linux and MinGW.
set(VCPKG_FOUND ${VCPKG_TOOLCHAIN} CACHE INTERNAL "")

if((NOT VCPKG_FOUND) AND (UNIX OR MINGW))
  find_package(PkgConfig REQUIRED)
elseif(NOT VCPKG_FOUND)
  message(FATAL_ERROR "Missing vcpkg toolchain file argument")
endif()

# Dependency: m
if(UNIX)
  list(APPEND CMAKE_REQUIRED_LIBRARIES "m")
endif()
check_symbol_exists(ceil math.h HAVE_CEIL)

# Dependency: SDL
find_package(SDL2 CONFIG REQUIRED)

# Dependency: Freetype
if(VCPKG_FOUND)
  find_package(freetype CONFIG REQUIRED)
elseif(PKG_CONFIG_FOUND)
  pkg_check_modules(FREETYPE2 REQUIRED freetype2)
endif()

# Dependency: Lua
add_subdirectory(third-party/lua)

# Macros
macro(target_compile_options_default target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion)
  endif()
endmacro(target_compile_options_default)

macro(target_link_SDL2 target)
  # On MinGW builds, this call to target_link_libraries must be before the call for SDL2::SDL2main
  if(MINGW)
    target_link_libraries(${target} PRIVATE mingw32)
  endif()
  if(BUILD_SHARED_LIBS)
    target_link_libraries(${target} PRIVATE SDL2::SDL2main SDL2::SDL2)
  else()
    target_link_libraries(${target} PRIVATE SDL2::SDL2main SDL2::SDL2-static)
  endif()
endmacro(target_link_SDL2)

macro(target_link_freetype target)
  if(VCPKG_FOUND)
    target_link_libraries(${target} PRIVATE freetype)
  elseif(PKG_CONFIG_FOUND AND FREETYPE2_FOUND)
    target_include_directories(${target} PRIVATE ${FREETYPE2_INCLUDE_DIRS})
    if(BUILD_SHARED_LIBS)
      target_link_libraries(${target} PRIVATE ${FREETYPE2_LIBRARIES})
    else()
      target_link_libraries(${target} PRIVATE ${FREETYPE2_STATIC_LIBRARIES})
    endif()
  endif()
endmacro(target_link_freetype)

macro(target_link_lua target)
  target_link_libraries(${target} PRIVATE lua)
endmacro(target_link_lua)

# Directory: assets
add_subdirectory(assets)

# Directory: src
add_subdirectory(src)

# Directory: test
add_subdirectory(test)

# Directory: tool
add_subdirectory(tool)
