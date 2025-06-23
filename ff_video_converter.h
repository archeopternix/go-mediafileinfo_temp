#ifndef FFMPEG_AVI_TO_H264_AAC_H
#define FFMPEG_AVI_TO_H264_AAC_H

#include <libavcodec/avcodec.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ffmpeg_avi_to_h264_aac.h
 * @brief Video/audio conversion utility using FFmpeg libraries.
 *
 * This header provides a simple API for converting AVI files to MP4 files with H.264 video and AAC audio,
 * supporting configurable codecs and bitrates, and CRF mode for H.264.
 */

/**
 * @struct Params
 * @brief Parameters for codec and bitrate configuration for conversion.
 *
 * This struct allows customizing the conversion process:
 * - `video_codec_id`: The codec to use for the output video stream (e.g., AV_CODEC_ID_H264).
 * - `video_bitrate`: Bitrate in bits per second. If H.264 and < 60, it is interpreted as CRF value.
 * - `audio_codec_id`: The codec to use for the output audio stream (e.g., AV_CODEC_ID_AAC).
 * - `audio_bitrate`: Bitrate in bits per second for the audio stream.
 */
typedef struct {
    enum AVCodecID video_codec_id; /**< Output video codec (e.g. AV_CODEC_ID_H264) */
    int video_bitrate; /**< Output video bitrate in bps. If H.264 and < 60, used as CRF. */
    enum AVCodecID audio_codec_id; /**< Output audio codec (e.g. AV_CODEC_ID_AAC) */
    int audio_bitrate; /**< Output audio bitrate in bps. */
} Params;

/**
 * @brief Convert an AVI file to MP4 with configurable codecs and bitrates.
 *
 * This function converts a video file from AVI format to MP4 format, encoding the video with the specified video codec
 * and the audio with the specified audio codec. It supports H.264 CRF mode if `video_codec_id` is AV_CODEC_ID_H264 and
 * `video_bitrate` is less than 60. In this case:
 *   - If `crf < 18`, preset "slower" is used.
 *   - If `crf > 30`, preset "faster" is used.
 *   - Otherwise, preset "medium" is used.
 *
 * @param input_filename  Path to the input AVI file.
 * @param output_filename Path to the output MP4 file.
 * @param params          Pointer to a Params struct specifying codec and bitrate options.
 * @return 0 on success, nonzero on error.
 *
 * @note Requires FFmpeg development libraries (libavformat, libavcodec, libswscale, libswresample, libavutil).
 * @note This function assumes the input AVI file contains a single video stream and optionally an audio stream.
 * @note This function is not thread-safe.
 *
 * @see Params
 */
int convert_avi_to_h264_aac(const char *input_filename, const char *output_filename, const Params *params);

#ifdef __cplusplus
}
#endif

#endif // FFMPEG_AVI_TO_H264_AAC_H
