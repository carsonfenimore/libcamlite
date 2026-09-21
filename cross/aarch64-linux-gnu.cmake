# CMake toolchain for cross-compiling to arm64 Raspberry Pi OS with Debian
# multiarch (the arm64 libraries and headers are installed alongside the host's,
# under /usr/lib/aarch64-linux-gnu and /usr/include/aarch64-linux-gnu).
#
#   cmake -S . -B build-aarch64 -DCMAKE_TOOLCHAIN_FILE=cross/aarch64-linux-gnu.cmake
#
# Normally used through cross/build.sh, which provides such an environment.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Makes find_library / find_package search the arm64 multiarch directories.
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)

# pkg-config must only see arm64 .pc files.
set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "")
