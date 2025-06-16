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

func (w *SmartAVCodecParameters) BitRate() int64 {
    return int64(w.ptr.bit_rate)
}
func (w *SmartAVCodecParameters) Width() int {
    return int(w.ptr.width)
}
func (w *SmartAVCodecParameters) Height() int {
    return int(w.ptr.height)
}
func (w *SmartAVCodecParameters) CodecID() int {
    return int(w.ptr.codec_id)
}
func (w *SmartAVCodecParameters) CodecType() int {
    return int(w.ptr.codec_type)
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
