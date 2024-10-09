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

find_package(PkgConfig)
pkg_check_modules(libcamera REQUIRED libcamera)


list(APPEND LCL_SRCS
    ${LCL_SOURCE_DIR}/libcamlite.cpp
    ${LCL_SOURCE_DIR}/yuv.cpp
    ${LCL_SOURCE_DIR}/post_proc.cpp
  )
add_library(camlite STATIC ${LCL_SRCS})
set(rpicamapp_include_dirs "/usr/local/include/rpicam-apps")
target_include_directories(camlite PRIVATE "${rpicamapp_include_dirs}")
target_include_directories(camlite PRIVATE "${libcamera_INCLUDE_DIRS}")



# TODO: Come back and use find package
add_library( rpicamapp_libs SHARED IMPORTED )
set_target_properties( rpicamapp_libs PROPERTIES IMPORTED_LOCATION "/usr/local/lib/aarch64-linux-gnu/rpicam_app.so")
target_link_libraries( camlite rpicamapp_libs )

# TODO: Come back and use find package
#add_library( libcambase_libs SHARED IMPORTED )
#set_target_properties( libcambase_libs PROPERTIES IMPORTED_LOCATION "/usr/local/lib/aarch64-linux-gnu/libcamera-base.so.0.3")
#target_link_libraries( camlite libcambase_libs )

# TODO: Come back and use find package
add_library( boostprog SHARED IMPORTED )
set_target_properties( boostprog PROPERTIES IMPORTED_LOCATION "/usr/lib/aarch64-linux-gnu/libboost_program_options.so.1.74.0")

target_link_libraries(camlite "${libcamera_LIBRARIES}")
target_link_libraries(camlite rpicamapp_libs)
target_link_libraries(camlite boostprog)

list(APPEND vidTestSrc
    ${LCL_SOURCE_DIR}/vid_test.cpp
  )
add_executable(vid_test "${vidTestSrc}")
target_link_libraries(vid_test camlite)

