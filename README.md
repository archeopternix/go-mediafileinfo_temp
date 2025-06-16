# go-mediafileinfo

This header file is part of the [archeopternix/go-mediafileinfo](https://github.com/archeopternix/go-mediafileinfo) project. It provides a minimal C interface to safely allocate and free FFmpeg codec parameter structures, designed to be used with Go via cgo or SWIG.

## Overview

`avwrapper.h` acts as a simple wrapper around FFmpeg's `AVCodecParameters`, making it easier to manage codec parameter memory from Go code.

## Functions

```c
AVCodecParameters* create_codec_parameters(void);
void free_codec_parameters(AVCodecParameters* params);
```

### create_codec_parameters

Allocates and initializes an `AVCodecParameters` structure.

### free_codec_parameters

Frees the memory allocated for an `AVCodecParameters` structure.

## Requirements

* FFmpeg/libavcodec development libraries installed
* Go (for usage within the go-mediafileinfo project)
* C compiler (e.g., gcc or clang)

## AVCodecParameters

| Type             | Name      | Description                                 |
|------------------|-----------|---------------------------------------------|
|enum AVMediaType 	|codec_type | 	General type of the encoded data. More...  |
 
enum AVCodecID 	codec_id
 	Specific type of the encoded data (the codec used). More...
 
uint32_t 	codec_tag
 	Additional information about the codec (corresponds to the AVI FOURCC). More...
 
uint8_t * 	extradata
 	Extra binary data needed for initializing the decoder, codec-dependent. More...
 
int 	extradata_size
 	Size of the extradata content in bytes. More...
 
AVPacketSideData * 	coded_side_data
 	Additional data associated with the entire stream. More...
 
int 	nb_coded_side_data
 	Amount of entries in coded_side_data. More...
 
int 	format
 
int64_t 	bit_rate
 	The average bitrate of the encoded data (in bits per second). More...
 
int 	bits_per_coded_sample
 	The number of bits per sample in the codedwords. More...
 
int 	bits_per_raw_sample
 	This is the number of valid bits in each output sample. More...
 
int 	profile
 	Codec-specific bitstream restrictions that the stream conforms to. More...
 
int 	level
 
int 	width
 	Video only. More...
 
int 	height
 
AVRational 	sample_aspect_ratio
 	Video only. More...
 
AVRational 	framerate
 	Video only. More...
 
enum AVFieldOrder 	field_order
 	Video only. More...
 
enum AVColorRange 	color_range
 	Video only. More...
 
enum AVColorPrimaries 	color_primaries
 
enum AVColorTransferCharacteristic 	color_trc
 
enum AVColorSpace 	color_space
 
enum AVChromaLocation 	chroma_location
 
int 	video_delay
 	Video only. More...
 
AVChannelLayout 	ch_layout
 	Audio only. More...
 
int 	sample_rate
 	Audio only. More...
 
int 	block_align
 	Audio only. More...
 
int 	frame_size
 	Audio only. More...
 
int 	initial_padding
 	Audio only. More...
 
int 	trailing_padding
 	Audio only. More...
 
int 	seek_preroll
 	Audio only. More...
