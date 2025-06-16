// avwrapper.h
#ifndef AVWRAPPER_H
#define AVWRAPPER_H

#include <libavcodec/avcodec.h>

AVCodecParameters* create_codec_parameters(void);
void free_codec_parameters(AVCodecParameters* params);

#endif
