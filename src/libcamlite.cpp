#include "libcamlite.hpp"

#include <functional>
#include <thread>

#include "core/rpicam_app.hpp"
#include "core/rpicam_encoder.hpp"

#include "post_proc.hpp"

namespace libcamlite {

class LibCamLite::Impl {
	friend LibCamLite;

	Impl(): app(std::make_unique<RPiCamEncoder>()),
		running(false){}

	std::unique_ptr<RPiCamEncoder> app;
	std::unique_ptr<H264Params> h264Params;
	H264Callback h264Callback;
	std::unique_ptr<LowResParams> lowResParams;
	LowResCallback lowResCallback;
	std::unique_ptr<PostProc> proc;
	std::unique_ptr<std::thread> runner; 
	bool running;

	void run();
};

LibCamLite::LibCamLite():impl(new Impl){}

void LibCamLite::setupLowresStream(LowResParams lowresParams, LowResCallback callback){
	//printf("Setup lowres %dx%d\n", lowresParams.stream.width, lowresParams.stream.height);
	impl->lowResParams = std::make_unique<LowResParams>(lowresParams);
	impl->lowResCallback = callback;
}

void LibCamLite::setupH264Stream(H264Params h264Params, H264Callback callback){
	//printf("Setup h264 %dx%d profile %s\n", h264Params.stream.width, h264Params.stream.height, h264Params.profile.c_str());
	impl->h264Params = std::make_unique<H264Params>(h264Params);
	impl->h264Callback = callback;
}

void LibCamLite::start(bool detach){
	using namespace std::placeholders;  // for _1, _2, _3...
						//
	// From here we provide some sane defaults for libcamera - non-customizeable
	VideoOptions *options = impl->app->GetOptions();
	int argc = 1;
	const char* argv[] = {"libcamlite", nullptr};
	options->Parse(argc, (char**)argv);
	// 0sec = run forever
	TimeVal<std::chrono::milliseconds> tv;
	tv.set("0sec");
	options->Set().timeout = tv;
	options->Set().segment = 0;
	options->Set().roi = "0,0,0,0";
	options->Set().viewfinder_height = 0;
	options->Set().afMode = "default";
	options->Set().afWindow = "0,0,0,0";
	options->Set().hdr = false;
	options->Set().sharpness = 1;
	options->Set().afWindow = "0,0,0,0";
	options->Set().qt_preview = false;
	options->Set().preview = "0,0,0,0";
	options->Set().denoise = "auto";
	options->Set().codec = "h264";
	options->Set().metering = "centre";
	options->Set().exposure = "normal";
	options->Set().contrast = 1;
	options->Set().saturation = 1;
	options->Set().saturation = 1;
	options->Set().afRange = "normal";
	options->Set().afSpeed = "normal";
	// include headers on every frame
	options->Set().inline_headers = true; 

	if (impl->lowResParams){
		// libcamlite-params are given from here 
		options->Set().lores_height = impl->lowResParams->stream.height;
		options->Set().lores_width = impl->lowResParams->stream.width;
		impl->proc = std::make_unique<PostProc>(impl->app.get(), impl->lowResCallback);
	}

	if (impl->h264Params){
	    //printf("Setup h264 %dx%d profile %s\n", impl->h264Params->stream.width, impl->h264Params->stream.height, impl->h264Params->profile.c_str());
	    options->Set().framerate = impl->h264Params->stream.framerate;
            options->Set().width = impl->h264Params->stream.width;
            options->Set().height = impl->h264Params->stream.height;
	    // h264-specific stuff below:
            options->Set().intra = impl->h264Params->intraPeriod;
	    options->Set().profile = impl->h264Params->profile;
            Bitrate bitrate;
	    bitrate.set(impl->h264Params->bitrate);
            options->Set().bitrate = bitrate;
	    // lambda to cast void*mem to uint8_t*mem...
	    std::function<void(void*, size_t, int64_t, bool)> cb = 
		    [this](void* mem, size_t size, int64_t timestamp_us, bool keyframe) {
			impl->h264Callback((uint8_t*)mem, size, timestamp_us, keyframe);
		    };
            impl->app->SetEncodeOutputReadyCallback(cb);
	}
	//options->Print();

	impl->app->OpenCamera();
	impl->app->ConfigureVideo(RPiCamEncoder::FLAG_VIDEO_NONE);
	impl->app->StartEncoder();
	impl->app->StartCamera();
	if (impl->proc){
		impl->proc->Configure();
	}

	impl->running = true;
	impl->runner = std::make_unique<std::thread>([this]() { impl->run(); });
	if (!detach){
		impl->runner->join();
	}
}

void LibCamLite::Impl::run(){
	while (running) {
		RPiCamEncoder::Msg msg = app->Wait();
		if (msg.type == RPiCamApp::MsgType::Timeout)
		{
			LOG_ERROR("ERROR: Device timeout detected, attempting a restart!!!");
			app->StopCamera();
			app->StartCamera();
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
		app->EncodeBuffer(completed_request, app->VideoStream());
	}
}

void LibCamLite::stop(){
	impl->running = false;
	impl->app->StopCamera(); // stop complains if encoder very slow to close
	impl->app->StopEncoder();
	impl->runner->join();
}


} // namespace libcamlite

