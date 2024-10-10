# libcamlite
libcamlite is a C++ wrapper for creating parallel h264 (1080p and beyond) and RGB (low res for AI object detection) using a raspberry pi.

Have you ever wanted to stream both h264 and run object detection simultaneously on a raspberry pi?  Most examples for doing this rely on opencv or external applications such as rpicam apps.  With libcamlite you can setup parallel streams, one for display (h264) and one for object detection. The library is quick and efficient - consuming less than 10% CPU. 


## Prerequisites
You will need a current version of libcamera (https://github.com/raspberrypi/libcamera.git) and rpicam-apps (https://github.com/raspberrypi/rpicam-apps.git).  We are currently building against versions tag v0.3.1+rpt20240906 and 1.5.1, respectively.

## Build
You can build as follows:

```
  mkdir build
  cd build
  cmake ..
  make
```

On very constrained platforms, such as the pi zero 2 w, I recommend setting up a cross compiler.  Roughly the easiest way I have found to do this is:
  - Make an ubuntu vm
  - use sbuild/chroot to make a bullseye arm64 environment
  - apt install all needed dependencies

This will be slower than, say, native per core - but given enough cores and ram it will allow libcamera, rpicam-apps, and libcamlite to all build in minutes rather than hours.

## Demo

A sample application, called vid_test, shows how libcamlite can be used to quickly get up and running.  It should output something like the following:

```
  carson@dev-1:~ $ ./vid_test 
  Made DRM preview window
  [169:15:16.185533996] [12293]  INFO Camera camera_manager.cpp:316 libcamera v0.3.1+50-69a894c4
  [169:15:16.285672363] [12299]  WARN RPiSdn sdn.cpp:40 Using legacy SDN tuning - please consider moving SDN inside rpi.denoise
  [169:15:16.288979464] [12299]  INFO RPI vc4.cpp:447 Registered camera /base/soc/i2c0mux/i2c@1/imx290@1a to Unicam device /dev/media3 and ISP device /dev/media0
  [169:15:16.289115660] [12299]  INFO RPI pipeline_base.cpp:1125 Using configuration file '/usr/share/libcamera/pipeline/rpi/vc4/rpi_apps.yaml'
  Mode selection for 1280:720:12:P(30)
      SRGGB10_CSI2P,1280x720/60.0024 - Score: 1000
      SRGGB10_CSI2P,1920x1080/60.0024 - Score: 1250
      SRGGB12_CSI2P,1280x720/60.0024 - Score: 0
      SRGGB12_CSI2P,1920x1080/60.0024 - Score: 250
  Stream configuration adjusted
  [169:15:16.300547571] [12293]  INFO Camera camera.cpp:1191 configuring streams: (0) 1280x720-YUV420 (1) 1280x720-SRGGB12_CSI2P (2) 300x300-YUV420
  [169:15:16.301138346] [12299]  INFO RPI vc4.cpp:622 Sensor: /base/soc/i2c0mux/i2c@1/imx290@1a - Selected sensor format: 1280x720-SRGGB12_1X12 - Selected unicam format: 1280x720-pRCC
  Vidtest: lowres received 19.91 fps
  VidTest: h264 received 19.78 fps 163214 bytes/sec
  Vidtest: lowres received 29.97 fps
  VidTest: h264 received 29.97 fps 250832 bytes/sec
  Vidtest: lowres received 30.04 fps
  VidTest: h264 received 29.98 fps 262106 bytes/sec
  Vidtest: lowres received 30.00 fps
  VidTest: h264 received 30.06 fps 249294 bytes/sec
```

Note this shows an excellent 30fps of both h264 (full 1920x1080 frame rate) and 300x300 lowres RGB!  This is acheived in under .08 loading (<10%) of a rpi zero 2w. Very nice.
