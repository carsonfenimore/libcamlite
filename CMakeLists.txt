cmake_minimum_required(VERSION 3.16)
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to Release, for debug builds use"
    "'-DCMAKE_BUILD_TYPE=Debug'.")
  set(CMAKE_BUILD_TYPE "Release")
endif()


# Double colon in target name means ALIAS or IMPORTED target.
cmake_policy(SET CMP0028 NEW)
# Enable MACOSX_RPATH (@rpath) for built dynamic libraries.
cmake_policy(SET CMP0042 NEW)
project(libcamlite C CXX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(LCL_SOURCE_DIR "" CACHE PATH
	"Directory that contains the libcamlite project"
)
if(NOT LCL_SOURCE_DIR)
    get_filename_component(LCL_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/src"
    ABSOLUTE
  )
endif()

set(CMAKE_CXX_STANDARD 20)  # Some components require C++14.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libcamera REQUIRED IMPORTED_TARGET libcamera)
pkg_check_modules(libavformat REQUIRED IMPORTED_TARGET libavformat)
pkg_check_modules(libavcodec REQUIRED IMPORTED_TARGET libavcodec)
pkg_check_modules(libavutil REQUIRED IMPORTED_TARGET libavutil)
find_package(Boost REQUIRED CONFIG COMPONENTS program_options)
find_package(Threads REQUIRED)

# rpicam-apps ships a rpicam_app.pc, but it lists image libraries (libexif,
# libjpeg, libtiff, libpng) under Requires.private, so pkg-config fails unless
# all their -dev packages are installed. Find the library and headers directly;
# this covers both the apt package (/usr) and source installs (/usr/local).
find_path(RPICAM_APP_INCLUDE_DIR core/rpicam_app.hpp PATH_SUFFIXES rpicam-apps REQUIRED)
find_library(RPICAM_APP_LIBRARY rpicam_app REQUIRED)

# rpicam-apps changes its C++ API between releases (even patch releases) without
# changing the library soname, so check the version explicitly. --modversion
# works even though resolving the flags of rpicam_app.pc does not.
set(RPICAM_APP_MIN_VERSION 1.9.1)  # the API libcamlite was ported to; tested with 1.13.0
execute_process(
    COMMAND ${PKG_CONFIG_EXECUTABLE} --modversion rpicam_app
    OUTPUT_VARIABLE RPICAM_APP_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
if(NOT RPICAM_APP_VERSION)
    message(WARNING "Could not determine the rpicam-apps version (no rpicam_app.pc found); "
                    "libcamlite needs rpicam-apps >= ${RPICAM_APP_MIN_VERSION}.")
elseif(RPICAM_APP_VERSION VERSION_LESS RPICAM_APP_MIN_VERSION)
    message(FATAL_ERROR "rpicam-apps ${RPICAM_APP_VERSION} is too old: libcamlite needs "
                        ">= ${RPICAM_APP_MIN_VERSION}. Update it (sudo apt full-upgrade) "
                        "or build rpicam-apps from source.")
endif()
message(STATUS "rpicam-apps ${RPICAM_APP_VERSION}: ${RPICAM_APP_LIBRARY} (headers in ${RPICAM_APP_INCLUDE_DIR})")

list(APPEND LCL_SRCS
    ${LCL_SOURCE_DIR}/libcamlite.cpp
    ${LCL_SOURCE_DIR}/yuv.cpp
    ${LCL_SOURCE_DIR}/stream_info.cpp
    ${LCL_SOURCE_DIR}/post_proc.cpp
  )
add_library(camlite SHARED ${LCL_SRCS})
target_include_directories(camlite
    PUBLIC ${LCL_SOURCE_DIR}
    PRIVATE ${RPICAM_APP_INCLUDE_DIR})
target_link_libraries(camlite
    PUBLIC
        PkgConfig::libavformat
        PkgConfig::libavcodec
        PkgConfig::libavutil
    PRIVATE
        PkgConfig::libcamera
        ${RPICAM_APP_LIBRARY}
        Boost::program_options
        Threads::Threads)

list(APPEND vidTestSrc
    ${LCL_SOURCE_DIR}/vid_test.cpp
  )
add_executable(vid_test "${vidTestSrc}")
target_link_libraries(vid_test camlite)
