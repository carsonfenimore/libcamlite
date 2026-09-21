#include "post_proc.hpp"

#include "yuv.hpp"

namespace libcamlite {

PostProc::PostProc(RPiCamApp *app, libcamlite::LowResCallback callback_):
	app_(app),
	callback(callback_){
	thread_ = std::thread(&PostProc::worker, this);
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
		std::lock_guard<std::mutex> lck(mutex_);
		if (pending_ || stopping_)
			return;  // worker still busy with the previous frame: skip this one

		BufferReadSync r(app_, completed_request->buffers[lores_stream_]);
		libcamera::Span<uint8_t> buffer = r.Get()[0];

		// Copy the lores image here and let the worker convert it to RGB.
		// Doing the "extra" copy is in fact hugely beneficial because it turns uncacned
		// memory into cached memory, which is then *much* quicker.
		lores_copy_.assign(buffer.data(), buffer.data() + buffer.size());
		pending_ = true;
	}
	cv_.notify_one();
}

void PostProc::worker() {
	std::unique_lock<std::mutex> lck(mutex_);
	while (true) {
		cv_.wait(lck, [this] { return pending_ || stopping_; });
		if (stopping_)
			return;
		// While pending_ is set Process() won't touch lores_copy_, so the
		// conversion and callback can run without holding the lock.
		lck.unlock();
		convertAndProcess();
		lck.lock();
		pending_ = false;
	}
}

void PostProc::convertAndProcess(){
	StreamInfo tf_info;
	tf_info.width = lores_info_.width;
	tf_info.height = lores_info_.height;
	tf_info.stride = tf_info.width * 3;
	rgb_.resize(tf_info.height * tf_info.stride);
	Yuv420ToRgb(rgb_.data(), lores_copy_.data(), lores_info_, tf_info);

	callback(rgb_.data(), rgb_.size());
}

void PostProc::Stop() {
	{
		std::lock_guard<std::mutex> lck(mutex_);
		stopping_ = true;
	}
	cv_.notify_one();
	if (thread_.joinable())
		thread_.join();
}

PostProc::~PostProc() {
	Stop();
}

}
