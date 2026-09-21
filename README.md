# libcamlite
libcamlite is a C++ wrapper for creating h264 and RGB video streams on a raspberry pi.

This is particularly useful in cv workflows, where we both want a compressed stream for viewing alongside raw RGB frames for inference. This library quickly sets up both streams at minimal cpu cost - less than 10% of cpu even on a pi zero 2W.


## Prerequisites
Previously rpicam-apps on pi os was too old. As of trixie everything comes from the apt repo:

```
sudo apt install -y build-essential cmake pkg-config \
    libcamera-dev librpicam-app-dev libboost-program-options-dev \
    libavformat-dev libavcodec-dev libavutil-dev
```

libcamlite needs rpicam-apps 1.9.1 or newer; it is currently tested with rpicam-apps
1.13.0 and libcamera 0.7.2. CMake checks the rpicam-apps version and stops with an
error if it is too old; `sudo apt full-upgrade` to get a current one.

Warning: in the past rpicam-apps has changed its C++ API between releases without changing its library
version (`librpicam_app.so.1`), so rebuild libcamlite after rpicam-apps updates.

### Building libcamera and rpicam-apps from source (optional)
Only needed if your OS ships an rpicam-apps older than 1.9.1, or you want unreleased
changes. Source installs go to `/usr/local`, which CMake prefers over the apt
packages. It is best to follow their individual guides; roughly:

#### libcamera
Follow the instructions under "Getting Started" in the libcamera git page, roughly:

```
sudo apt install -y python-pip git python3-jinja2
sudo apt install -y libboost-dev
sudo apt install -y libgnutls28-dev openssl libtiff-dev pybind11-dev
sudo apt install -y qtbase5-dev libqt5core5a libqt5widgets
sudo apt install -y meson cmake
sudo apt install -y python3-yaml python3-ply
sudo apt install -y libglib2.0-dev libgstreamer-plugins-base1.0-dev
git clone https://github.com/raspberrypi/libcamera.git
cd libcamera
meson setup build --buildtype=release -Dpipelines=rpi/vc4,rpi/pisp -Dipas=rpi/vc4,rpi/pisp -Dv4l2=true -Dgstreamer=enabled -Dtest=false -Dlc-compliance=disabled -Dcam=disabled -Dqcam=disabled -Ddocumentation=disabled -Dpycamera=enabled
ninja -C build install
```

#### rpicam-apps
Follow the instructions under https://www.raspberrypi.com/documentation/computers/camera_software.html#building-rpicam-apps

```
sudo apt install -y cmake libboost-program-options-dev libdrm-dev libexif-dev
sudo apt install -y meson ninja-build
git clone https://github.com/raspberrypi/rpicam-apps.git
cd rpicam-apps
meson setup build -Denable_libav=disabled -Denable_drm=enabled -Denable_egl=disabled -Denable_qt=disabled -Denable_opencv=disabled -Denable_tflite=disabled -Denable_hailo=disabled
meson compile -C build
sudo meson install -C build
```


## Build
You can build as follows:

```
  cmake -S . -B build
  cmake --build build -j4
```

This produces `build/libcamlite.so` and the `build/vid_test` demo.

### Cross-compiling (recommended for the Pi Zero 2 W)
Building on a Pi Zero 2 W is slow, so there is a cross build that runs on any
x86_64 Linux machine with podman or docker:

```
cross/build.sh
```

This produces `build-aarch64/libcamlite.so` and `build-aarch64/vid_test` for any 64-bit
Raspberry Pi OS trixie board. The first run creates a Debian trixie container with the
arm64 libcamera / rpicam-apps / ffmpeg packages from the Raspberry Pi archive (about a
minute); after that a full rebuild takes seconds. Run `cross/build.sh --pull` after
updating your Pis so it builds against the same package versions they have.

To use your own cross environment instead, pass the toolchain file to CMake:
`cmake -S . -B build-aarch64 -DCMAKE_TOOLCHAIN_FILE=cross/aarch64-linux-gnu.cmake`.
It expects Debian multiarch (arm64 dev packages installed alongside the host's).

To try it on a Pi:

```
scp build-aarch64/libcamlite.so build-aarch64/vid_test pi:
ssh pi 'LD_LIBRARY_PATH=. ./vid_test'
```

## Demo

Once built run vid_test - this will setup simultaneous h264 and raw captures and show their capture frame rates. Output on  zero 2W is something like:

```
carson@atomcam-2:~/libcamlite/build $ ./vid_test 
[19:32:12.714576068] [10603]  INFO Camera camera_manager.cpp:330 libcamera v0.5.2+99-bfd68f78
[19:32:12.750937665] [10604]  INFO IPAProxy ipa_proxy.cpp:180 Using tuning file /usr/local/share/libcamera/ipa/rpi/vc4/imx290.json
[19:32:12.754119682] [10604]  INFO Camera camera_manager.cpp:220 Adding camera '/base/soc/i2c0mux/i2c@1/imx290@1a' for pipeline handler rpi/vc4
[19:32:12.754224718] [10604]  INFO RPI vc4.cpp:440 Registered camera /base/soc/i2c0mux/i2c@1/imx290@1a to Unicam device /dev/media3 and ISP device /dev/media0
[19:32:12.754266348] [10604]  INFO RPI pipeline_base.cpp:1107 Using configuration file '/usr/local/share/libcamera/pipeline/rpi/vc4/rpi_apps.yaml'
Made DRM preview window
Mode selection for 1280:720:12:P(30)
    SRGGB10_CSI2P,1280x720/60.0024 - Score: 1000
    SRGGB10_CSI2P,1920x1080/60.0024 - Score: 1250
    SRGGB12_CSI2P,1280x720/60.0024 - Score: 0
    SRGGB12_CSI2P,1920x1080/60.0024 - Score: 250
Stream configuration adjusted
[19:32:12.768190794] [10603]  INFO Camera camera.cpp:1215 configuring streams: (0) 1280x720-YUV420/Rec709 (1) 1280x720-SRGGB12_CSI2P/RAW (2) 300x300-YUV420/Rec709
[19:32:12.768553144] [10604]  INFO RPI vc4.cpp:615 Sensor: /base/soc/i2c0mux/i2c@1/imx290@1a - Selected sensor format: 1280x720-SRGGB12_1X12/RAW - Selected unicam format: 1280x720-pRCC/RAW
Discovered stream; cpar 1280x720; time 1/1200000
Vidtest: lowres received 22.25 fps
VidTest: h264 received 22.15 fps 135994 bytes/sec
Vidtest: lowres received 30.02 fps
VidTest: h264 received 30.03 fps 247091 bytes/sec
Vidtest: lowres received 29.95 fps
VidTest: h264 received 29.86 fps 251681 bytes/sec
Vidtest: lowres received 30.02 fps
VidTest: h264 received 30.01 fps 252420 bytes/sec
Vidtest: lowres received 30.02 fps
VidTest: h264 received 30.02 fps 254364 bytes/sec
Vidtest: lowres received 30.02 fps
VidTest: h264 received 30.02 fps 254127 bytes/sec
Vidtest: lowres received 30.02 fps
VidTest: h264 received 30.02 fps 255815 bytes/sec
```

Note this shows an excellent 30fps of both h264 (full 1920x1080 frame rate) and 300x300 lowres RGB!  This is acheived in under .08 loading (<10%) of a rpi zero 2w. Very nice.
