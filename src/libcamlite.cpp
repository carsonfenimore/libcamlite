#include "libcamlite.hpp"

#include <functional>

#include "core/libcamera_app.h"
#include "core/libcamera_encoder.h"

#include "post_proc.hpp"

namespace libcamlite {

class LibCamLite::Impl {
	friend LibCamLite;

	Impl(): app(std::make_unique<RPiCamEncoder>()){}

	std::unique_ptr<RPiCamEncoder> app;
	std::unique_ptr<H264Params> h264Params;
	H264Callback h264Callback;
	std::unique_ptr<LowResParams> lowResParams;
	LowResCallback lowResCallback;
};

LibCamLite::LibCamLite():impl(new Impl){}

void LibCamLite::setupLowresStream(LowResParams lowresParams, LowResCallback callback){
	printf("Setup lowres %dx%d\n", lowresParams.stream.width, lowresParams.stream.height);
	impl->lowResParams = std::make_unique<LowResParams>(lowresParams);
	impl->lowResCallback = callback;
}

void LibCamLite::setupH264Stream(H264Params h264Params, H264Callback callback){
	printf("Setup h264 %dx%d profile %s\n", h264Params.stream.width, h264Params.stream.height, h264Params.profile);
	impl->h264Params = std::make_unique<H264Params>(h264Params);
	impl->h264Callback = callback;
}

void LibCamLite::start(){
	using namespace std::placeholders;  // for _1, _2, _3...
						//
	// From here we provide some sane defaults for libcamera - non-customizeable
	VideoOptions *options = impl->app->GetOptions();
	// 0sec = run forever
	TimeVal<std::chrono::milliseconds> tv;
	tv.set("0sec");
	options->timeout = tv;
	options->segment = 0;
	options->roi = "0,0,0,0";
	options->viewfinder_height = 0;
	options->afMode = "default";
	options->afWindow = "0,0,0,0";
	options->hdr = false;
	options->sharpness = 1;
	options->afWindow = "0,0,0,0";
	options->qt_preview = false;
	options->preview = "0,0,0,0";
	options->denoise = "auto";
	options->codec = "h264";
	options->metering = "centre";
	options->exposure = "normal";
	options->contrast = 1;
	options->saturation = 1;
	options->saturation = 1;
	options->afRange = "normal";
	options->afSpeed = "normal";
	// include headers on every frame
	options->inline_headers = true; 

	std::unique_ptr<PostProc> proc;
	if (impl->lowResParams){
		// libcamlite-params are given from here 
		options->lores_height = impl->lowResParams->stream.height;
		options->lores_width = impl->lowResParams->stream.width;
		proc = std::make_unique<PostProc>(impl->app.get(), impl->lowResCallback);
	}

	if (impl->h264Params){
	    options->framerate = impl->h264Params->stream.framerate;
            options->width = impl->h264Params->stream.width;
            options->height = impl->h264Params->stream.height;
	    // h264-specific stuff below:
            options->intra = impl->h264Params->intraPeriod;
	    options->profile = impl->h264Params->profile;
            Bitrate bitrate;
	    bitrate.set(impl->h264Params->bitrate);
            options->bitrate = bitrate;
	    // lambda to cast void*mem to uint8_t*mem...
	    std::function<void(void*, size_t, int64_t, bool)> cb = 
		    [this](void* mem, size_t size, int64_t timestamp_us, bool keyframe) {
			impl->h264Callback((uint8_t*)mem, size, timestamp_us, keyframe);
		    };
            impl->app->SetEncodeOutputReadyCallback(cb);
	}

	impl->app->OpenCamera();
	impl->app->ConfigureVideo(RPiCamEncoder::FLAG_VIDEO_NONE);
	impl->app->StartEncoder();
	impl->app->StartCamera();
	if (proc){
		proc->Configure();
	}

	// Main processing loop
	for (unsigned int count = 0; ; count++) {
		RPiCamEncoder::Msg msg = impl->app->Wait();
		if (msg.type == RPiCamApp::MsgType::Timeout)
		{
			LOG_ERROR("ERROR: Device timeout detected, attempting a restart!!!");
			impl->app->StopCamera();
			impl->app->StartCamera();
			continue;
		}
		if (msg.type == RPiCamEncoder::MsgType::Quit)
			return;
		else if (msg.type != RPiCamEncoder::MsgType::RequestComplete)
			throw std::runtime_error("unrecognised message!");

		CompletedRequestPtr &completed_request = std::get<CompletedRequestPtr>(msg.payload);
		if (proc){
			proc->Process(completed_request);
		}
		impl->app->EncodeBuffer(completed_request, impl->app->VideoStream());
	}
}

void LibCamLite::stop(){
	impl->app->StopCamera(); // stop complains if encoder very slow to close
	impl->app->StopEncoder();
}


} // namespace libcamlite

