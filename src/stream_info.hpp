#pragma once

#include <memory>

extern "C" {
#include <libavformat/avformat.h>
}

class StreamInfo {
public:
	StreamInfo();

	// Adds the given packet to our discovery buffer, producing an AVStream if discovery is complete
	AVStream* analyze(uint8_t* mem, size_t size);
private:
	struct Impl;
	std::shared_ptr<Impl> impl;

};

