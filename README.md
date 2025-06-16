# go-mediafileinfo

This header file is part of the [archeopternix/go-mediafileinfo](https://github.com/archeopternix/go-mediafileinfo) project. It provides a minimal C interface to safely allocate and free FFmpeg codec parameter structures, designed to be used with Go via cgo or SWIG.

## Overview

`avwrapper.h` acts as a simple wrapper around FFmpeg's `AVCodecParameters`, making it easier to manage codec parameter memory from Go code.

## Functions

```c
AVCodecParameters* create_codec_parameters(void);
void free_codec_parameters(AVCodecParameters* params);
```

* create_codec_parameters
Allocates and initializes an AVCodecParameters structure.

* free_codec_parameters
Frees the memory allocated for an AVCodecParameters structure.

## Requirements

FFmpeg/libavcodec development libraries installed
Go (for usage within the go-mediafileinfo project)
C compiler (e.g., gcc or clang)
