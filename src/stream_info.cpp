#include "stream_info.hpp"

#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>
}

namespace libcamlite {

struct buffer_data {
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
    size_t size_total; 
};

static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data *bd = (struct buffer_data *)opaque;
    buf_size = FFMIN(buf_size, bd->size);
    if (!buf_size){
        return AVERROR_EOF;
    }
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr  += buf_size;
    bd->size -= buf_size;

    //printf("size left:%zu, total %zu, total consumed %zu\n", bd->size, bd->size_total, bd->size_total-bd->size );
    return buf_size;
}

struct StreamInfo::Impl {
    friend StreamInfo;
    AVFormatContext *fmt_ctx = NULL;
    AVIOContext *avio_ctx = NULL;
    uint8_t *avio_ctx_buffer = NULL;
    size_t avio_ctx_buffer_size = 4096;
    struct buffer_data bd;

    std::vector<uint8_t> discoveryBuf;
    const unsigned int discoveryMaxBytes = 1024*100; 
    const unsigned int discoveryMinBytes = 8096;

    virtual ~Impl(){
	    if (fmt_ctx)
		    avformat_close_input(&fmt_ctx);
	    if (avio_ctx_buffer)
		    av_freep(&avio_ctx_buffer);
	    if (avio_ctx)
		avio_context_free(&avio_ctx);
    }
 
    AVFormatContext* init(){
	    if (fmt_ctx != NULL){
		    return fmt_ctx;
	    }

	    if (!(fmt_ctx = avformat_alloc_context())) {
		    fprintf(stderr, "Failed to make avformat\n");
		    return NULL;
	    }
	 
	    avio_ctx_buffer = (uint8_t*)av_malloc(avio_ctx_buffer_size);
	    if (!avio_ctx_buffer) {
		    fprintf(stderr, "Failed to make avio_ctx buf\n");
		    avformat_close_input(&fmt_ctx);
		    fmt_ctx = NULL;
		    return NULL;
	    }
	    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
					  0, &bd, &read_packet, NULL, NULL);
	    if (!avio_ctx) {
		fprintf(stderr, "Failed to make avio_ctx\n");
		avformat_close_input(&fmt_ctx);
		av_freep(&avio_ctx_buffer);
		fmt_ctx = NULL;
		return NULL;
	    }
	    fmt_ctx->pb = avio_ctx;
	 
	    int ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
	    if (ret < 0) {
		fprintf(stderr, "Could not open input\n");
		avformat_close_input(&fmt_ctx);
		avio_ctx_buffer = NULL;
		fmt_ctx = NULL;
		avio_ctx = NULL;
	    }
	    return fmt_ctx;
    }

    AVStream* analyze(uint8_t* mem, size_t size){
	    // We'll accumulate bytes up til the max, and analyze if at least we have the min
	    if(discoveryBuf.size() < discoveryMaxBytes){
		for(int i = 0; i < size; i++){
			discoveryBuf.push_back(mem[i]);
		}
	    }

	    if (discoveryBuf.size() < discoveryMinBytes){
		    // not ready yet
		    return NULL;
	    }

	    // always make ptr good, since during init we actually do call callback
	    bd.ptr = discoveryBuf.data();
	    bd.size = discoveryBuf.size();
	    bd.size_total = discoveryBuf.size();
	    if (!init()){
		    return NULL;
	    }

	    int ret = avformat_find_stream_info(fmt_ctx, NULL);
	    if (ret < 0) {
		fprintf(stderr, "Could not find stream information\n");
		return NULL;
	    }
	    discoveryBuf.clear();
	    return fmt_ctx->streams[0];
    }
};

StreamInfo::StreamInfo():impl(std::make_shared<StreamInfo::Impl>()){}

AVStream* StreamInfo::analyze(uint8_t* mem, size_t size){
	return impl->analyze(mem, size);
}

}
