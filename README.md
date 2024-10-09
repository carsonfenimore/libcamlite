# libcamlite
C++ wrapper for accessing libcamera for h264 and lowres callbacks

You can build as follows:

```
  mkdir build
  cd build
  cmake ..
  make
```


A sample application, called vid_test, shows how libcamlite can be used to quickly get up and running.  If you run, besure you have a good enough camera attached. You should see output like the following:


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


Note this shows an excellent 30fps of both h264 (full 1920x1080 frame rate) and 300x300 lowres RGB!  This is acheived in under .08 loading (<10%) of a rpi zero 2w. Very nice.
