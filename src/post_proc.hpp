#pragma once

#include "core/rpicam_app.hpp"
#include "libcamlite.hpp"

#include <condition_variable>
#include <mutex>
#include <thread>

namespace libcamlite {

// This represents a "post processing" stage in that it mirrors (and somewhat copies) the rpicamapps post_proc stages
// The intent is that a consumer of this class can write their own post processor - this class will be what provides
// RGB lower res images... 
//
// A single worker thread converts and delivers frames. If it is still busy
// with the previous frame when a new one arrives, the new frame is skipped, so
// a slow consumer (e.g. object detection) never backs up the camera.
class PostProc {

public:
	PostProc(RPiCamApp *app, libcamlite::LowResCallback callback);
	~PostProc();
	void Configure(); 
	void Process(CompletedRequestPtr &completed_request); 
	// Stops the worker; no callbacks are made after this returns.
	void Stop();
private:
	void worker();
	void convertAndProcess();

	RPiCamApp* app_;
	libcamera::Stream *lores_stream_ = nullptr;
	std::mutex mutex_;
	std::condition_variable cv_;
	bool pending_ = false;   // a frame is waiting in lores_copy_ or being processed
	bool stopping_ = false;
	std::thread thread_;
	std::vector<uint8_t> lores_copy_;
	std::vector<uint8_t> rgb_;
	StreamInfo lores_info_;
	libcamlite::LowResCallback callback;
};

}
