// avwrapper.c
#include "avwrapper.h"

AVCodecParameters* create_codec_parameters(void) {
    return avcodec_parameters_alloc();
}

void free_codec_parameters(AVCodecParameters* params) {
    avcodec_parameters_free(&params);
}
