#include "libcamlite.hpp"

#include <functional>

#include "core/libcamera_app.h"
#include "core/libcamera_encoder.h"

#include "post_proc.hpp"

namespace libcamlite {

// Global vars
RPiCamEncoder gApp;
H264Params* gH264Params = NULL;
LowResParams* gLowResParams = NULL;

void setupLowresStream(LowResParams lowresParams){
	printf("Setup lowres %dx%d\n", lowresParams.stream.width, lowresParams.stream.height);
	gLowResParams = new LowResParams(lowresParams);
}

void setupH264Stream(H264Params h264Params){
	printf("Setup h264 %dx%d profile %s\n", h264Params.stream.width, h264Params.stream.height, h264Params.profile);
	gH264Params = new H264Params(h264Params);
}

void h264Callback(void* mem, size_t size, int64_t timestamp_us, bool keyframe){
	gH264Params->callback((uint8_t*)mem, size, timestamp_us, keyframe);
}

void start(){
	using namespace std::placeholders;  // for _1, _2, _3...
						//
	// From here we provide some sane defaults for libcamera - non-customizeable
	VideoOptions *options = gApp.GetOptions();
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

	PostProc* proc = NULL;
	if (gLowResParams){
		// libcamlite-params are given from here 
		options->lores_height = gLowResParams->stream.height;
		options->lores_width = gLowResParams->stream.width;
		proc = new PostProc(&gApp, gLowResParams->callback);
	}

	if (gH264Params){
	    options->framerate = gH264Params->stream.framerate;
            options->width = gH264Params->stream.width;
            options->height = gH264Params->stream.height;
	    // h264-specific stuff below:
            options->intra = gH264Params->intraPeriod;
	    options->profile = gH264Params->profile;
            Bitrate bitrate;
	    bitrate.set(gH264Params->bitrate);
            options->bitrate = bitrate;
            gApp.SetEncodeOutputReadyCallback(std::bind(&h264Callback, _1, _2, _3, _4));
	}

	gApp.OpenCamera();
	gApp.ConfigureVideo(RPiCamEncoder::FLAG_VIDEO_NONE);
	gApp.StartEncoder();
	gApp.StartCamera();
	if (proc){
		proc->Configure();
	}

	// Main processing loop
	for (unsigned int count = 0; ; count++) {
		RPiCamEncoder::Msg msg = gApp.Wait();
		if (msg.type == RPiCamApp::MsgType::Timeout)
		{
			LOG_ERROR("ERROR: Device timeout detected, attempting a restart!!!");
			gApp.StopCamera();
			gApp.StartCamera();
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
		gApp.EncodeBuffer(completed_request, gApp.VideoStream());
	}
}

void stop(){
	gApp.StopCamera(); // stop complains if encoder very slow to close
	gApp.StopEncoder();
}


} // namespace libcamlite

