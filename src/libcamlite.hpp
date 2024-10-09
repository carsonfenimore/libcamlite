#pragma once

#include <string>
#include <functional>
#include <memory>

enum StreamFormat {
	STREAM_FORMAT_RGB,
	STREAM_FORMAT_H264,
	STREAM_FORMAT_YUV420
};

struct CamParam {
	int width;
	int height;
	StreamFormat format;
	int framerate;

	int h264IntraPeriod;
	std::string h264Profile;
	std::string h264Bitrate;
};
	
typedef std::function<void (void* mem, size_t size, int64_t timestamp_us, bool keyframe)> H264Callback;
typedef std::function<void (const std::vector<uint8_t>&)> LowResCallback;


class LibCamLite {
public:
	LibCamLite(CamParam h264Config, H264Callback, CamParam lowresConfig, LowResCallback);
	void start();
	void stop();

private:
	class Impl;
	std::shared_ptr<Impl> impl;
};
