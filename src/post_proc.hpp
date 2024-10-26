#pragma once

#include "core/libcamera_app.h"
#include "libcamlite.hpp"

#include <mutex>

namespace libcamlite {

// This represents a "post processing" stage in that it mirrors (and somewhat copies) the rpicamapps post_proc stages
// The intent is that a consumer of this class can write their own post processor - this class will be what provides
// RGB lower res images... 
class PostProc {

public:
	PostProc(RPiCamApp *app, libcamlite::LowResCallback callback);
        void Configure(); 
        void Process(CompletedRequestPtr &completed_request); 
	void convertAndProcess();
private:
	RPiCamApp* app_;
	libcamera::Stream *lores_stream_;
	std::mutex future_mutex_;
	std::unique_ptr<std::future<void>> future_;
	std::vector<uint8_t> lores_copy_;
	StreamInfo lores_info_;
	libcamlite::LowResCallback callback;
};

}
