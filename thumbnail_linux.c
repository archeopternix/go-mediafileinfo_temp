#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Returns 1 on success, 0 on failure.
 * jpegBuffer: will be allocated (use g_free to release), size: jpegSize
 */
int GetImageThumbnailJPEGBuffer(const char* filePath, int thumbWidth, int thumbHeight, unsigned char** jpegBuffer, gsize* jpegSize) {
    *jpegBuffer = NULL;
    *jpegSize = 0;

    // Load image from file
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(filePath, &error);
    if (!pixbuf) {
        fprintf(stderr, "Failed to load image: %s\n", error ? error->message : "Unknown error");
        if (error) g_error_free(error);
        return 0;
    }

    // Scale the image to thumbnail size
    GdkPixbuf* scaled = gdk_pixbuf_scale_simple(pixbuf, thumbWidth, thumbHeight, GDK_INTERP_BILINEAR);
    g_object_unref(pixbuf);
    if (!scaled) return 0;

    // Save to JPEG buffer
    error = NULL;
    gboolean ok = gdk_pixbuf_save_to_buffer(scaled, (char**)jpegBuffer, jpegSize, "jpeg", &error, "quality", "85", NULL);
    g_object_unref(scaled);

    if (!ok) {
        fprintf(stderr, "Failed to save thumbnail: %s\n", error ? error->message : "Unknown error");
        if (error) g_error_free(error);
        return 0;
    }
    return 1;
}
