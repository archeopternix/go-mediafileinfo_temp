package avwrapper

/*
#cgo pkg-config: libavcodec
#include "avwrapper.h"
*/
import "C"
import (
    "runtime"
    "unsafe"
)

type SmartAVCodecParameters struct {
    ptr *C.struct_AVCodecParameters
}

func NewSmartAVCodecParameters() *SmartAVCodecParameters {
    p := C.create_codec_parameters()
    if p == nil {
        return nil
    }
    w := &SmartAVCodecParameters{ptr: p}
    runtime.SetFinalizer(w, func(w *SmartAVCodecParameters) {
        C.free_codec_parameters(w.ptr)
    })
    return w
}

// Zugriff auf lesbare Felder

func (w *SmartAVCodecParameters) CodecTag() uint32 {
    return uint32(w.ptr.codec_tag)
}
func (w *SmartAVCodecParameters) Extradata() unsafe.Pointer {
    return unsafe.Pointer(w.ptr.extradata)
}
func (w *SmartAVCodecParameters) ExtradataSize() int {
    return int(w.ptr.extradata_size)
}
func (w *SmartAVCodecParameters) Format() int {
    return int(w.ptr.format)
}
func (w *SmartAVCodecParameters) BitsPerCodedSample() int {
    return int(w.ptr.bits_per_coded_sample)
}
func (w *SmartAVCodecParameters) BitsPerRawSample() int {
    return int(w.ptr.bits_per_raw_sample)
}
func (w *SmartAVCodecParameters) Profile() int {
    return int(w.ptr.profile)
}
func (w *SmartAVCodecParameters) Level() int {
    return int(w.ptr.level)
}
func (w *SmartAVCodecParameters) SampleAspectRatioNum() int {
    return int(w.ptr.sample_aspect_ratio_num)
}
func (w *SmartAVCodecParameters) SampleAspectRatioDen() int {
    return int(w.ptr.sample_aspect_ratio_den)
}
func (w *SmartAVCodecParameters) FieldOrder() int {
    return int(w.ptr.field_order)
}
func (w *SmartAVCodecParameters) ColorRange() int {
    return int(w.ptr.color_range)
}
func (w *SmartAVCodecParameters) ColorPrimaries() int {
    return int(w.ptr.color_primaries)
}
func (w *SmartAVCodecParameters) ColorTrc() int {
    return int(w.ptr.color_trc)
}
func (w *SmartAVCodecParameters) ColorSpace() int {
    return int(w.ptr.color_space)
}
func (w *SmartAVCodecParameters) ChromaLocation() int {
    return int(w.ptr.chroma_location)
}
func (w *SmartAVCodecParameters) VideoDelay() int {
    return int(w.ptr.video_delay)
}
func (w *SmartAVCodecParameters) ChannelLayout() uint64 {
    return uint64(w.ptr.channel_layout)
}
func (w *SmartAVCodecParameters) Channels() int {
    return int(w.ptr.channels)
}
func (w *SmartAVCodecParameters) SampleRate() int {
    return int(w.ptr.sample_rate)
}
func (w *SmartAVCodecParameters) BlockAlign() int {
    return int(w.ptr.block_align)
}
func (w *SmartAVCodecParameters) FrameSize() int {
    return int(w.ptr.frame_size)
}
func (w *SmartAVCodecParameters) InitialPadding() int {
    return int(w.ptr.initial_padding)
}
func (w *SmartAVCodecParameters) TrailingPadding() int {
    return int(w.ptr.trailing_padding)
}
func (w *SmartAVCodecParameters) SeekPreroll() int {
    return int(w.ptr.seek_preroll)
}

// Optional: get codec name via C-Helper
func (w *SmartAVCodecParameters) CodecName() string {
    cstr := C.get_codec_name(w.ptr.codec_id)
    return C.GoString(cstr)
}

// Expose raw pointer falls benötigt
func (w *SmartAVCodecParameters) Ptr() *C.struct_AVCodecParameters {
    return w.ptr
}
