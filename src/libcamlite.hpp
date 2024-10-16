#pragma once

#include <string>
#include <functional>
#include <memory>

namespace libcamlite {

enum StreamFormat {
	STREAM_FORMAT_RGB,
	STREAM_FORMAT_H264,
	STREAM_FORMAT_YUV420
};

struct StreamParams {
	uint32_t width;
	uint32_t height;
	StreamFormat format;
	uint8_t framerate;
};

typedef std::function<void(uint8_t* mem, size_t size)> LowResCallback;
struct LowResParams {
	StreamParams stream;
};

typedef std::function<void(uint8_t* mem, size_t size, int64_t timestamp_us, bool keyframe)> H264Callback;
struct H264Params {
	StreamParams stream;

	int intraPeriod;
	const char* profile;
	const char* bitrate;
};

class LibCamLite {
  public:
	LibCamLite();
	void setupLowresStream(LowResParams lowresParams, LowResCallback);
	void setupH264Stream(H264Params h264Params, H264Callback);
	void start();
	void stop();
  private:
	class Impl;
	std::shared_ptr<Impl> impl;
};

}
