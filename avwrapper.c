// avwrapper.c
#include "avwrapper.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <stdio.h>

// Returns a pointer to AVCodecParameters of the first video stream.
// Returns NULL on error. You must NOT free the returned pointer.
AVCodecParameters* get_video_codec_parameters(const char *filename) {
    AVFormatContext *fmt_ctx = NULL;

    if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open input file '%s'\n", filename);
        return NULL;
    }

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        avformat_close_input(&fmt_ctx);
        return NULL;
    }

    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVCodecParameters *codecpar = fmt_ctx->streams[i]->codecpar;
        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            // Do NOT free codecpar — it's owned by fmt_ctx
            avformat_close_input(&fmt_ctx); // Optional: remove to use fmt_ctx elsewhere
            return codecpar;
        }
    }

    fprintf(stderr, "No video stream found in file '%s'\n", filename);
    avformat_close_input(&fmt_ctx);
    return NULL;
}

void free_codec_parameters(AVCodecParameters* params) {
    avcodec_parameters_free(&params);
}
