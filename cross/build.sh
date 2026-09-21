#!/bin/bash
# Cross-compile libcamlite for arm64 Raspberry Pi OS (trixie) from an x86_64 Linux
# machine with podman or docker. Output: build-aarch64/libcamlite.so and vid_test,
# which run on any 64-bit Pi (Zero 2 W, 3, 4, 5).
#
#   cross/build.sh           # build (creates the toolchain image on first run)
#   cross/build.sh --pull    # rebuild the image to pick up newer Pi packages first
#
# Then copy to a Pi, e.g.: scp build-aarch64/libcamlite.so build-aarch64/vid_test pi:
set -euo pipefail

cd "$(dirname "$0")/.."
ENGINE=${ENGINE:-$(command -v podman || command -v docker)}
[ -n "$ENGINE" ] || { echo "podman or docker is required" >&2; exit 1; }
IMAGE=localhost/libcamlite-cross:trixie

if [[ ${1:-} == --pull ]] || ! "$ENGINE" image inspect "$IMAGE" >/dev/null 2>&1; then
	"$ENGINE" build --pull --no-cache -t "$IMAGE" -f cross/Containerfile cross
fi

# Rootless podman maps the container's root to you; docker needs -u so the build
# output isn't owned by root.
USER_ARGS=()
[[ $(basename "$ENGINE") == docker ]] && USER_ARGS=(-u "$(id -u):$(id -g)")

"$ENGINE" run --rm "${USER_ARGS[@]}" -v "$PWD:/src:Z" -w /src "$IMAGE" bash -euo pipefail -c '
	cmake -S . -B build-aarch64 -DCMAKE_TOOLCHAIN_FILE=cross/aarch64-linux-gnu.cmake
	cmake --build build-aarch64 -j"$(nproc)"
	file build-aarch64/libcamlite.so build-aarch64/vid_test
'
