#include "libcamlite.hpp"

#include <functional>

#include "core/libcamera_app.h"
#include "core/libcamera_encoder.h"

#include "post_proc.hpp"

class LibCamLite::Impl {
    public:
	Impl(CamParam h264Config_, H264Callback h264Callback_, CamParam lowresConfig_, LowResCallback lowresCallback_):
		h264Config(h264Config_),
		h264Callback(h264Callback_),
		lowresConfig(lowresConfig_),
		lowresCallback(lowresCallback_){
	}

        void start(){
            VideoOptions *options = app.GetOptions();
	    
	    // From here we provide some sane defaults for libcamera - non-customizeable
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
            options->inline_headers = true; // include headers on every frame

	    // libcamlite-params are given from here 
            options->lores_height = lowresConfig.height;
            options->lores_width = lowresConfig.width;
	    options->framerate = h264Config.framerate;
	    options->profile = h264Config.h264Profile;
            options->width = h264Config.width;
            options->height = h264Config.height;
            Bitrate bitrate;
	    bitrate.set(h264Config.h264Bitrate);
            options->bitrate = bitrate;
            options->intra = h264Config.h264IntraPeriod; 
	    //printf("Config with h264 %dx%d, intra %d, bitrate %s, lowres %dx%d\n",  options->width,  
	    //										options->height, 
	    //										options->intra, 
	    //										h264Config.h264Bitrate.c_str(), 
	    //										options->lores_width, 
	    //										options->lores_height);

            app.SetEncodeOutputReadyCallback(h264Callback);

	    PostProc proc(&app, lowresCallback);
            app.OpenCamera();
            app.ConfigureVideo(RPiCamEncoder::FLAG_VIDEO_NONE);
            app.StartEncoder();
            app.StartCamera();
	    proc.Configure();

	    // Main processing loop
            for (unsigned int count = 0; ; count++) {
                RPiCamEncoder::Msg msg = app.Wait();
                if (msg.type == RPiCamApp::MsgType::Timeout)
                {
                    LOG_ERROR("ERROR: Device timeout detected, attempting a restart!!!");
                    app.StopCamera();
                    app.StartCamera();
                    continue;
                }
                if (msg.type == RPiCamEncoder::MsgType::Quit)
                    return;
                else if (msg.type != RPiCamEncoder::MsgType::RequestComplete)
                    throw std::runtime_error("unrecognised message!");

                CompletedRequestPtr &completed_request = std::get<CompletedRequestPtr>(msg.payload);
		proc.Process(completed_request);
                app.EncodeBuffer(completed_request, app.VideoStream());
            }
        }

        void stop(){
            app.StopCamera(); // stop complains if encoder very slow to close
            app.StopEncoder();
        }
private:
	RPiCamEncoder app;
	CamParam h264Config;
	H264Callback h264Callback;
	CamParam lowresConfig;
	LowResCallback lowresCallback;
};


LibCamLite::LibCamLite(CamParam h264Config, H264Callback h264Callback, CamParam lowresConfig, LowResCallback lowResCallback){ 
	impl.reset(new LibCamLite::Impl(h264Config, h264Callback, lowresConfig, lowResCallback));
}

void LibCamLite::start(){
	impl->start();
}

void LibCamLite::stop(){
	impl->stop();
}

