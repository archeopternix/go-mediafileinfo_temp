// avwrapper.h
#ifndef AVWRAPPER_H
#define AVWRAPPER_H

#include <libavcodec/avcodec.h>

AVCodecParameters* get_video_codec_parameters(const char *filename);
void free_codec_parameters(AVCodecParameters* params);

#endif
