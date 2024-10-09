#pragma

#include <vector>

#include "core/stream_info.hpp"

std::vector<uint8_t> Yuv420ToRgb(const uint8_t *src, StreamInfo &src_info, StreamInfo &dst_info);

void Yuv420ToRgb(uint8_t *dst, const uint8_t *src, StreamInfo &src_info, StreamInfo &dst_info);
