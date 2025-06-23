/**
 * @file ffmpeg_avi_to_h264_aac.c
 * @brief Implementation of AVI to MP4 (H.264/AAC) conversion using FFmpeg.
 *
 * This file contains the implementation of the convert_avi_to_h264_aac function,
 * which supports conversion of AVI files with flexible codec and bitrate configuration,
 * including support for H.264 CRF mode and preset selection based on CRF value.
 */

#include "ffmpeg_avi_to_h264_aac.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

/**
 * @brief Converts an AVI file to an MP4 file with the specified codecs and bitrates.
 *
 * This function opens the input AVI file, finds the video and audio streams,
 * decodes them, and encodes them into the output MP4 file using the codecs and
 * bitrates specified in the Params structure. If H.264 is used for video and the
 * bitrate is less than 60, CRF mode is enabled and the preset is configured according
 * to the CRF value:
 *   - CRF < 18: preset "slower"
 *   - CRF > 30: preset "faster"
 *   - Otherwise: preset "medium"
 *
 * @param input_filename  Path to the input AVI file.
 * @param output_filename Path to the output MP4 file.
 * @param params          Pointer to a Params struct specifying codec and bitrate options.
 * @return 0 on success, nonzero on error.
 */
int convert_avi_to_h264_aac(const char *input_filename, const char *output_filename, const Params *params)
{
    AVFormatContext *input_fmt_ctx = NULL;
    AVFormatContext *output_fmt_ctx = NULL;
    AVCodecContext *dec_ctx_video = NULL, *dec_ctx_audio = NULL;
    AVCodecContext *enc_ctx_video = NULL, *enc_ctx_audio = NULL;
    AVStream *in_stream_video = NULL, *in_stream_audio = NULL;
    AVStream *out_stream_video = NULL, *out_stream_audio = NULL;
    int video_stream_index = -1, audio_stream_index = -1;
    int ret = 0;

    av_register_all();

    // Open input file
    if ((ret = avformat_open_input(&input_fmt_ctx, input_filename, NULL, NULL)) < 0) {
        fprintf(stderr, "Could not open input file '%s'\n", input_filename);
        goto end;
    }

    if ((ret = avformat_find_stream_info(input_fmt_ctx, NULL)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information\n");
        goto end;
    }

    // Find the video and audio streams
    for (unsigned int i = 0; i < input_fmt_ctx->nb_streams; i++) {
        if (input_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && video_stream_index < 0)
            video_stream_index = i;
        else if (input_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audio_stream_index < 0)
            audio_stream_index = i;
    }
    if (video_stream_index == -1) {
        fprintf(stderr, "Did not find a video stream in the input file\n");
        ret = -1;
        goto end;
    }
    in_stream_video = input_fmt_ctx->streams[video_stream_index];
    if (audio_stream_index != -1)
        in_stream_audio = input_fmt_ctx->streams[audio_stream_index];

    // Video decoder
    AVCodec *decoder_video = avcodec_find_decoder(in_stream_video->codecpar->codec_id);
    dec_ctx_video = avcodec_alloc_context3(decoder_video);
    avcodec_parameters_to_context(dec_ctx_video, in_stream_video->codecpar);
    avcodec_open2(dec_ctx_video, decoder_video, NULL);

    // Audio decoder
    if (in_stream_audio) {
        AVCodec *decoder_audio = avcodec_find_decoder(in_stream_audio->codecpar->codec_id);
        dec_ctx_audio = avcodec_alloc_context3(decoder_audio);
        avcodec_parameters_to_context(dec_ctx_audio, in_stream_audio->codecpar);
        avcodec_open2(dec_ctx_audio, decoder_audio, NULL);
    }

    // Prepare output file
    avformat_alloc_output_context2(&output_fmt_ctx, NULL, NULL, output_filename);
    if (!output_fmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = -1;
        goto end;
    }

    // Video encoder setup
    enum AVCodecID video_codec_id = params && params->video_codec_id ? params->video_codec_id : AV_CODEC_ID_H264;
    AVCodec *encoder_video = avcodec_find_encoder(video_codec_id);
    if (!encoder_video) {
        fprintf(stderr, "Could not find video encoder for id %d\n", video_codec_id);
        ret = -1;
        goto end;
    }
    out_stream_video = avformat_new_stream(output_fmt_ctx, NULL);
    enc_ctx_video = avcodec_alloc_context3(encoder_video);
    enc_ctx_video->height = dec_ctx_video->height;
    enc_ctx_video->width = dec_ctx_video->width;
    enc_ctx_video->sample_aspect_ratio = dec_ctx_video->sample_aspect_ratio;
    enc_ctx_video->pix_fmt = encoder_video->pix_fmts ? encoder_video->pix_fmts[0] : dec_ctx_video->pix_fmt;
    enc_ctx_video->time_base = in_stream_video->time_base;
    enc_ctx_video->framerate = av_guess_frame_rate(input_fmt_ctx, in_stream_video, NULL);

    // Set bitrate or CRF mode for H.264 if required
    if (params && params->video_bitrate > 0) {
        if (video_codec_id == AV_CODEC_ID_H264 && params->video_bitrate < 60) {
            int crf = params->video_bitrate;
            av_opt_set_double(enc_ctx_video->priv_data, "crf", crf, 0);
            // Set preset depending on CRF value
            if (crf < 18) {
                av_opt_set(enc_ctx_video->priv_data, "preset", "slower", 0);
            } else if (crf > 30) {
                av_opt_set(enc_ctx_video->priv_data, "preset", "faster", 0);
            } else {
                av_opt_set(enc_ctx_video->priv_data, "preset", "medium", 0);
            }
            enc_ctx_video->bit_rate = 0; // Avoid CBR mode
        } else {
            enc_ctx_video->bit_rate = params->video_bitrate;
        }
    }
    if (output_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        enc_ctx_video->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    avcodec_open2(enc_ctx_video, encoder_video, NULL);
    avcodec_parameters_from_context(out_stream_video->codecpar, enc_ctx_video);
    out_stream_video->time_base = enc_ctx_video->time_base;

    // Audio encoder setup
    if (in_stream_audio) {
        enum AVCodecID audio_codec_id = params && params->audio_codec_id ? params->audio_codec_id : AV_CODEC_ID_AAC;
        AVCodec *encoder_audio = avcodec_find_encoder(audio_codec_id);
        if (!encoder_audio) {
            fprintf(stderr, "Could not find audio encoder for id %d\n", audio_codec_id);
            ret = -1;
            goto end;
        }
        out_stream_audio = avformat_new_stream(output_fmt_ctx, NULL);
        enc_ctx_audio = avcodec_alloc_context3(encoder_audio);
        enc_ctx_audio->sample_rate = dec_ctx_audio->sample_rate;
        enc_ctx_audio->channel_layout = dec_ctx_audio->channel_layout
            ? dec_ctx_audio->channel_layout
            : av_get_default_channel_layout(dec_ctx_audio->channels);
        enc_ctx_audio->channels = av_get_channel_layout_nb_channels(enc_ctx_audio->channel_layout);
        enc_ctx_audio->sample_fmt = encoder_audio->sample_fmts ? encoder_audio->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        enc_ctx_audio->bit_rate = (params && params->audio_bitrate > 0) ? params->audio_bitrate : 128000;
        enc_ctx_audio->time_base = (AVRational){1, enc_ctx_audio->sample_rate};
        if (output_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            enc_ctx_audio->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        avcodec_open2(enc_ctx_audio, encoder_audio, NULL);
        avcodec_parameters_from_context(out_stream_audio->codecpar, enc_ctx_audio);
        out_stream_audio->time_base = enc_ctx_audio->time_base;
    }

    // Open output file for writing
    if (!(output_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_open(&output_fmt_ctx->pb, output_filename, AVIO_FLAG_WRITE);
    }

    avformat_write_header(output_fmt_ctx, NULL);

    AVPacket *packet = av_packet_alloc();
    AVFrame *frame_video = av_frame_alloc();
    AVFrame *frame_audio = av_frame_alloc();
    AVFrame *sws_frame = av_frame_alloc();
    AVFrame *swr_frame = av_frame_alloc();

    // Setup video pixel format conversion if needed
    struct SwsContext *sws_ctx = NULL;
    if (dec_ctx_video->pix_fmt != enc_ctx_video->pix_fmt) {
        sws_ctx = sws_getContext(
            dec_ctx_video->width, dec_ctx_video->height, dec_ctx_video->pix_fmt,
            enc_ctx_video->width, enc_ctx_video->height, enc_ctx_video->pix_fmt,
            SWS_BICUBIC, NULL, NULL, NULL);
        sws_frame->format = enc_ctx_video->pix_fmt;
        sws_frame->width = enc_ctx_video->width;
        sws_frame->height = enc_ctx_video->height;
        av_frame_get_buffer(sws_frame, 0);
    }

    // Setup audio sample format conversion if needed
    struct SwrContext *swr_ctx = NULL;
    if (in_stream_audio && (dec_ctx_audio->sample_fmt != enc_ctx_audio->sample_fmt ||
        dec_ctx_audio->sample_rate != enc_ctx_audio->sample_rate ||
        dec_ctx_audio->channel_layout != enc_ctx_audio->channel_layout)) {
        swr_ctx = swr_alloc_set_opts(NULL,
            enc_ctx_audio->channel_layout, enc_ctx_audio->sample_fmt, enc_ctx_audio->sample_rate,
            dec_ctx_audio->channel_layout, dec_ctx_audio->sample_fmt, dec_ctx_audio->sample_rate,
            0, NULL);
        swr_init(swr_ctx);
        swr_frame->channel_layout = enc_ctx_audio->channel_layout;
        swr_frame->sample_rate = enc_ctx_audio->sample_rate;
        swr_frame->format = enc_ctx_audio->sample_fmt;
        swr_frame->nb_samples = enc_ctx_audio->frame_size ? enc_ctx_audio->frame_size : 1024;
        av_frame_get_buffer(swr_frame, 0);
    }

    // Main encode loop: read packets, decode, convert, encode, write
    while (av_read_frame(input_fmt_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            avcodec_send_packet(dec_ctx_video, packet);
            while (avcodec_receive_frame(dec_ctx_video, frame_video) == 0) {
                AVFrame *enc_frame = frame_video;
                if (sws_ctx) {
                    sws_scale(
                        sws_ctx, (const uint8_t * const*)frame_video->data, frame_video->linesize, 0, frame_video->height,
                        sws_frame->data, sws_frame->linesize);
                    sws_frame->pts = frame_video->pts;
                    enc_frame = sws_frame;
                }
                avcodec_send_frame(enc_ctx_video, enc_frame);
                AVPacket out_pkt;
                av_init_packet(&out_pkt);
                while (avcodec_receive_packet(enc_ctx_video, &out_pkt) == 0) {
                    out_pkt.stream_index = out_stream_video->index;
                    out_pkt.pts = av_rescale_q(out_pkt.pts, enc_ctx_video->time_base, out_stream_video->time_base);
                    out_pkt.dts = av_rescale_q(out_pkt.dts, enc_ctx_video->time_base, out_stream_video->time_base);
                    out_pkt.duration = av_rescale_q(out_pkt.duration, enc_ctx_video->time_base, out_stream_video->time_base);
                    av_interleaved_write_frame(output_fmt_ctx, &out_pkt);
                    av_packet_unref(&out_pkt);
                }
            }
        } else if (in_stream_audio && packet->stream_index == audio_stream_index) {
            avcodec_send_packet(dec_ctx_audio, packet);
            while (avcodec_receive_frame(dec_ctx_audio, frame_audio) == 0) {
                AVFrame *enc_frame = frame_audio;
                if (swr_ctx) {
                    swr_convert(swr_ctx, swr_frame->data, swr_frame->nb_samples,
                        (const uint8_t **)frame_audio->data, frame_audio->nb_samples);
                    swr_frame->pts = frame_audio->pts;
                    enc_frame = swr_frame;
                }
                avcodec_send_frame(enc_ctx_audio, enc_frame);
                AVPacket out_pkt;
                av_init_packet(&out_pkt);
                while (avcodec_receive_packet(enc_ctx_audio, &out_pkt) == 0) {
                    out_pkt.stream_index = out_stream_audio->index;
                    out_pkt.pts = av_rescale_q(out_pkt.pts, enc_ctx_audio->time_base, out_stream_audio->time_base);
                    out_pkt.dts = av_rescale_q(out_pkt.dts, enc_ctx_audio->time_base, out_stream_audio->time_base);
                    out_pkt.duration = av_rescale_q(out_pkt.duration, enc_ctx_audio->time_base, out_stream_audio->time_base);
                    av_interleaved_write_frame(output_fmt_ctx, &out_pkt);
                    av_packet_unref(&out_pkt);
                }
            }
        }
        av_packet_unref(packet);
    }

    // Flush video encoder
    avcodec_send_frame(enc_ctx_video, NULL);
    AVPacket out_pkt;
    av_init_packet(&out_pkt);
    while (avcodec_receive_packet(enc_ctx_video, &out_pkt) == 0) {
        out_pkt.stream_index = out_stream_video->index;
        av_interleaved_write_frame(output_fmt_ctx, &out_pkt);
        av_packet_unref(&out_pkt);
    }

    // Flush audio encoder
    if (in_stream_audio) {
        avcodec_send_frame(enc_ctx_audio, NULL);
        while (avcodec_receive_packet(enc_ctx_audio, &out_pkt) == 0) {
            out_pkt.stream_index = out_stream_audio->index;
            av_interleaved_write_frame(output_fmt_ctx, &out_pkt);
            av_packet_unref(&out_pkt);
        }
    }

    av_write_trailer(output_fmt_ctx);

end:
    if (input_fmt_ctx) avformat_close_input(&input_fmt_ctx);
    if (output_fmt_ctx && !(output_fmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&output_fmt_ctx->pb);
    if (output_fmt_ctx) avformat_free_context(output_fmt_ctx);
    if (dec_ctx_video) avcodec_free_context(&dec_ctx_video);
    if (dec_ctx_audio) avcodec_free_context(&dec_ctx_audio);
    if (enc_ctx_video) avcodec_free_context(&enc_ctx_video);
    if (enc_ctx_audio) avcodec_free_context(&enc_ctx_audio);
    if (frame_video) av_frame_free(&frame_video);
    if (frame_audio) av_frame_free(&frame_audio);
    if (sws_frame) av_frame_free(&sws_frame);
    if (swr_frame) av_frame_free(&swr_frame);
    if (packet) av_packet_free(&packet);
    if (sws_ctx) sws_freeContext(sws_ctx);
    if (swr_ctx) swr_free(&swr_ctx);

    return ret < 0 ? 1 : 0;
}
