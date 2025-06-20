// os_thumbnail.h
#include <stdint.h>
#include <stdlib.h>

// Example signature: generates a JPEG thumbnail from input file, returns buffer and size
int os_thumbnail(const char* input_filename, uint8_t** output_buf, size_t* output_size);

void free_thumbnail_buffer(uint8_t* buf);
