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
	int width;
	int height;
	StreamFormat format;
	int framerate;
};

typedef void (*LowResCallback)(uint8_t* mem, size_t size);
struct LowResParams {
	StreamParams stream;
	LowResCallback callback;
};

typedef void (*H264Callback)(uint8_t* mem, size_t size, int64_t timestamp_us, bool keyframe);
struct H264Params {
	StreamParams stream;

	int intraPeriod;
	const char* profile;
	const char* bitrate;
	H264Callback callback;
};


void setupLowresStream(LowResParams lowresParams);
void setupH264Stream(H264Params h264Params);
void start();
void stop();

}
