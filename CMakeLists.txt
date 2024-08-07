# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

cmake_minimum_required(VERSION 3.13.0)
if(MSVC)
  cmake_policy(SET CMP0091 NEW)
endif()

file(STRINGS "version.txt" version_txt)
project(vpl-tools VERSION ${version_txt})

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
# Project configuration options
#

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Default CMAKE_BUILD_TYPE not set using Release")
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE
        STRING
        "Choose build type from: None Debug Release RelWithDebInfo MinSizeRel"
        FORCE)
endif()

# More code hardening on Windows
if(MSVC)
  if(${use_control_flow_guard})
    message("Setting CONTROL FLOW GUARD")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /guard:cf")
    set(CMAKE_EXE_LINKER_FLAGS "/guard:cf /DYNAMICBASE")
  endif()
  if(${use_qspectre})
    message("Setting QSPECTRE")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Qspectre")
  endif()
endif()

# Project options
option(BUILD_SHARED_LIBS "Build shared instead of static libraries." OFF)
option(BUILD_TESTS "Build tests." OFF)

set(BUILD_EXPERIMENTAL
    ON
    CACHE BOOL "Build tools with EXPERIMENTAL APIs.")

# Test tool options for surface sharing
if(WIN32)
  set(TOOLS_DEFAULT_SCREEN_CAPTURE ON)
  set(TOOLS_DEFAULT_RENDER ON)
  set(TOOLS_DEFAULT_OPENCL ON)
  set(TOOLS_DEFAULT_X11 OFF)
else()
  set(TOOLS_DEFAULT_SCREEN_CAPTURE ON)
  set(TOOLS_DEFAULT_RENDER ON)
  set(TOOLS_DEFAULT_OPENCL OFF)
  set(TOOLS_DEFAULT_X11 ON)
endif()

option(TOOLS_ENABLE_SCREEN_CAPTURE
       "Enable screen capture in surface sharing test tools."
       ${TOOLS_DEFAULT_SCREEN_CAPTURE})

option(TOOLS_ENABLE_RENDER "Enable rendering in surface sharing test tools."
       ${TOOLS_DEFAULT_RENDER})

option(TOOLS_ENABLE_OPENCL "Enable OpenCL in surface sharing test tools."
       ${TOOLS_DEFAULT_OPENCL})

option(TOOLS_ENABLE_X11 "Enable X11 in surface sharing test tools."
       ${TOOLS_DEFAULT_X11})

option(USE_MSVC_STATIC_RUNTIME
       "Link MSVC runtime statically to all components." OFF)

if(USE_MSVC_STATIC_RUNTIME)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

include(cmake/InstallDirs.cmake)
include(cmake/PackageTarget.cmake)

# Set POSITION_INDEPENDENT_CODE property for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE true)

# Keep executable bits on shared objects when installing regardless of distro
set(CMAKE_INSTALL_SO_NO_EXE 0)

include(cmake/CompileOptions.cmake)

if(BUILD_TESTS)
  include(CTest)
  enable_testing()
  # Build googletest
  set(BUILD_GMOCK
      OFF
      CACHE BOOL "" FORCE)
  set(INSTALL_GTEST
      OFF
      CACHE BOOL "" FORCE)
  set(gtest_disable_pthreads
      OFF
      CACHE BOOL "" FORCE)
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  set(gtest_hide_internal_symbols
      OFF
      CACHE BOOL "" FORCE)

  add_subdirectory(ext/googletest)
  add_library(GTest::gtest ALIAS gtest)
  add_library(GTest::gtest_main ALIAS gtest_main)
endif()

add_subdirectory(tools)

install(
  FILES third-party-programs.txt
  DESTINATION ${VPL_INSTALL_LICENSEDIR}
  COMPONENT ${VPL_COMPONENT_LIB})

install(
  FILES LICENSE
  RENAME license.txt
  DESTINATION ${VPL_INSTALL_LICENSEDIR}
  COMPONENT ${VPL_COMPONENT_LIB})

set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT ${VPL_COMPONENT_LIB})
include(InstallRequiredSystemLibraries)

message(
  STATUS "---------------- Configuration summary ------------------------------"
)
message(STATUS "CMake:")
message(STATUS "  CMAKE_VERSION                   : ${CMAKE_VERSION}")
message(STATUS "  CMAKE_GENERATOR                 : ${CMAKE_GENERATOR}")
message(STATUS "  CMAKE_BUILD_TOOL                : ${CMAKE_BUILD_TOOL}")
message(STATUS "Target:")
message(STATUS "  CMAKE_SYSTEM_NAME               : ${CMAKE_SYSTEM_NAME}")
message(STATUS "  CMAKE_SYSTEM_VERSION            : ${CMAKE_SYSTEM_VERSION}")
message(STATUS "  CMAKE_SYSTEM_PROCESSOR          : ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "General:")
message(STATUS "  CMAKE_BUILD_TYPE                : ${CMAKE_BUILD_TYPE}")
message(STATUS "  CMAKE_TOOLCHAIN_FILE            : ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "  CMAKE_C_COMPILER                : ${CMAKE_C_COMPILER}")
message(STATUS "  CMAKE_CXX_COMPILER              : ${CMAKE_CXX_COMPILER}")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(STATUS "  Build architecture              : 64-bit")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(STATUS "  Build architecture              : 32-bit")
endif()
message(STATUS "Flags:")
message(STATUS "  CMAKE_C_FLAGS                   : ${CMAKE_C_FLAGS}")
message(STATUS "  CMAKE_CXX_FLAGS                 : ${CMAKE_CXX_FLAGS}")
message(STATUS "Release flags:")
message(STATUS "  CMAKE_C_FLAGS_RELEASE           : ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "  CMAKE_CXX_FLAGS_RELEASE         : ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "Debug flags:")
message(STATUS "  CMAKE_C_FLAGS_DEBUG             : ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "  CMAKE_CXX_FLAGS_DEBUG           : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "RelWithDebRT flags:")
message(
  STATUS "  CMAKE_C_FLAGS_RELWITHDEBRT      : ${CMAKE_C_FLAGS_RELWITHDEBRT}")
message(
  STATUS "  CMAKE_CXX_FLAGS_RELWITHDEBRT    : ${CMAKE_CXX_FLAGS_RELWITHDEBRT}")
message(STATUS "Install:")
message(STATUS "  CMAKE_INSTALL_PREFIX            : ${CMAKE_INSTALL_PREFIX}")
message(
  STATUS "  CMAKE_INSTALL_FULL_BINDIR       : ${CMAKE_INSTALL_FULL_BINDIR}")
message(
  STATUS "  CMAKE_INSTALL_FULL_INCLUDEDIR   : ${CMAKE_INSTALL_FULL_INCLUDEDIR}")
message(
  STATUS "  CMAKE_INSTALL_FULL_LIBDIR       : ${CMAKE_INSTALL_FULL_LIBDIR}")
message(
  STATUS "  CMAKE_INSTALL_FULL_DOCDIR       : ${CMAKE_INSTALL_FULL_DOCDIR}")
message(
  STATUS "  VPL_INSTALL_FULL_PKGCONFIGDIR   : ${VPL_INSTALL_FULL_PKGCONFIGDIR}")
message(
  STATUS "  VPL_INSTALL_FULL_CMAKEDIR       : ${VPL_INSTALL_FULL_CMAKEDIR}")
message(STATUS "  VPL_INSTALL_FULL_ENVDIR         : ${VPL_INSTALL_FULL_ENVDIR}")
message(
  STATUS "  VPL_INSTALL_FULL_MODFILEDIR     : ${VPL_INSTALL_FULL_MODFILEDIR}")
message(
  STATUS "  VPL_INSTALL_FULL_EXAMPLEDIR     : ${VPL_INSTALL_FULL_EXAMPLEDIR}")
message(
  STATUS "  VPL_INSTALL_FULL_LICENSEDIR     : ${VPL_INSTALL_FULL_LICENSEDIR}")
message(
  STATUS "  VPL_INSTALL_FULL_PYTHONDIR      : ${VPL_INSTALL_FULL_PYTHONDIR}")
message(STATUS "Build:")
message(STATUS "  BUILD_SHARED_LIBS               : ${BUILD_SHARED_LIBS}")
message(STATUS "  BUILD_TESTS                     : ${BUILD_TESTS}")
message(STATUS "  BUILD_EXPERIMENTAL              : ${BUILD_EXPERIMENTAL}")
if(MSVC)
  message(
    STATUS "  USE_MSVC_STATIC_RUNTIME         : ${USE_MSVC_STATIC_RUNTIME}")
endif()
if(CMAKE_SYSTEM_NAME MATCHES Linux)
  message(STATUS "  ENABLE_VA                       : ${ENABLE_VA}")
  message(STATUS "  ENABLE_DRM                      : ${ENABLE_DRM}")
  message(STATUS "  ENABLE_WAYLAND                  : ${ENABLE_WAYLAND}")
  message(STATUS "  ENABLE_X11                      : ${ENABLE_X11}")
  message(STATUS "  ENABLE_DRI3                     : ${ENABLE_DRI3}")
endif()

message(STATUS "Surface sharing test tools:")
message(
  STATUS "  TOOLS_ENABLE_SCREEN_CAPTURE     : ${TOOLS_ENABLE_SCREEN_CAPTURE}")
message(STATUS "  TOOLS_ENABLE_RENDER             : ${TOOLS_ENABLE_RENDER}")
message(STATUS "  TOOLS_ENABLE_OPENCL             : ${TOOLS_ENABLE_OPENCL}")
message(STATUS "  TOOLS_ENABLE_X11                : ${TOOLS_ENABLE_X11}")
