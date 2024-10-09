#include "post_proc.hpp"

#include "yuv.hpp"

template <class R = std::micro, class T = std::chrono::steady_clock, class F, class... Args>
static auto ExecutionTime(F &&f, Args &&... args)
{
	auto t1 = T::now();
	std::invoke(std::forward<decltype(f)>(f), std::forward<Args>(args)...);
	auto t2 = T::now();
	return std::chrono::duration<double, R>(t2 - t1);
}

PostProc::PostProc(RPiCamApp *app, Callback callback_):
	app_(app),
	callback(callback_){
}

void PostProc::Configure() {
	lores_stream_ = app_->LoresStream();
	if (lores_stream_) {
		lores_info_ = app_->GetStreamInfo(lores_stream_);
	}
	else {
		std::cerr << "No low res stream!!" << std::endl;
	}
}


// Borrowed from rpicamapps
void PostProc::Process(CompletedRequestPtr &completed_request) {
	if (!lores_stream_)
		return;

	{
		std::unique_lock<std::mutex> lck(future_mutex_);
		if (!future_ || future_->wait_for(std::chrono::seconds(0)) == std::future_status::ready){
			BufferReadSync r(app_, completed_request->buffers[lores_stream_]);
			libcamera::Span<uint8_t> buffer = r.Get()[0];

			// Copy the lores image here and let the asynchronous thread convert it to RGB.
			// Doing the "extra" copy is in fact hugely beneficial because it turns uncacned
			// memory into cached memory, which is then *much* quicker.
			lores_copy_.assign(buffer.data(), buffer.data() + buffer.size());

			future_ = std::make_unique<std::future<void>>();
			*future_ = std::async(std::launch::async, [this] {
				ExecutionTime<std::micro>(&PostProc::convertAndProcess, this).count();
				//printf("Post proc time time: %.4f microsec\n", time_taken );
			});
		} 
	}
}

void PostProc::convertAndProcess(){
	StreamInfo tf_info;
	tf_info.width = lores_info_.width;
	tf_info.height = lores_info_.height;
	tf_info.stride = tf_info.width * 3;
	std::vector<uint8_t> rgb_image = Yuv420ToRgb(lores_copy_.data(), lores_info_, tf_info);

	callback(rgb_image);
}
