# libcamlite
libcamlite is a C++ wrapper for creating h264 and RGB video streams on a raspberry pi.

This is particularly useful in cv workflows, where we both want a compressed stream for viewing alongside raw RGB frames for inference. This library quickly sets up both streams at minimal cpu cost - less than 10% of cpu even on a pi zero 2W.


## Prerequisites
To build this library you will need to build libcamera and rpicam-apps.  We currently are building against the following versions:

rpicam-apps build: 1.9.1
libcamera build: v0.5.2+99-bfd68f78

Although it is best to follow their individual guides, here's a brief summary of what it takes:

### Build libcamera
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

### Build rpicam-apps
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
  mkdir build
  cd build
  cmake ..
  make
```

On very constrained platforms, such as the pi zero 2W, I recommend setting up a cross compiler.  Roughly the easiest way I have found to do this is:
  - Make an ubuntu vm
  - use sbuild/chroot to make a bullseye arm64 environment
  - apt install all needed dependencies

This will be slower than, say, native per core - but given enough cores and ram it will allow libcamera, rpicam-apps, and libcamlite to all build in minutes rather than hours.

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
