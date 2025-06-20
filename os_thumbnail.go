// thumbnail.go
package mediafileinfo

/*
#cgo LDFLAGS: -L. -los_thumbnail   // Adjust if necessary
#include <stdint.h>
#include <stdlib.h>

// Declare C functions
int os_thumbnail(const char* input_filename, uint8_t** output_buf, size_t* output_size);
void free_thumbnail_buffer(uint8_t* buf);
*/
import "C"
import (
    "errors"
    "unsafe"
)

// GetJPEGThumbnail generates a JPEG thumbnail from the given file.
// It returns the JPEG bytes as a Go slice.
func GetJPEGThumbnail(filename string) ([]byte, error) {
    cFilename := C.CString(filename)
    defer C.free(unsafe.Pointer(cFilename))

    var cBuf *C.uint8_t
    var cSize C.size_t

    res := C.os_thumbnail(cFilename, &cBuf, &cSize)
    if res != 0 {
        return nil, errors.New("failed to generate thumbnail")
    }
    defer C.free_thumbnail_buffer(cBuf)

    // Copy JPEG data from C buffer to Go slice
    buf := C.GoBytes(unsafe.Pointer(cBuf), C.int(cSize))
    return buf, nil
}
