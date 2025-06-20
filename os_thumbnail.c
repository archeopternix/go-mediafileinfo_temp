// os_thumbnail.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#include <shobjidl.h>
#include <wincodec.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "windowscodecs.lib")
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>
#include <ImageIO/ImageIO.h>
#else // Linux/Unix
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <limits.h>
#include <unistd.h>
#endif

int LoadOSThumbnailJPEGBuffer(const char* filePath, int width, int height, unsigned char** jpegBuffer, size_t* jpegSize) {
    *jpegBuffer = NULL;
    *jpegSize = 0;

#if defined(_WIN32)
    // --- Windows implementation ---
    int wlen = MultiByteToWideChar(CP_UTF8, 0, filePath, -1, NULL, 0);
    wchar_t* wfile = (wchar_t*)malloc(wlen * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, filePath, -1, wfile, wlen);

    HRESULT hr;
    IShellItem* item = NULL;
    hr = SHCreateItemFromParsingName(wfile, NULL, &IID_IShellItem, (void**)&item);
    free(wfile);
    if (FAILED(hr)) return 0;

    IShellItemImageFactory* factory = NULL;
    hr = item->lpVtbl->QueryInterface(item, &IID_IShellItemImageFactory, (void**)&factory);
    item->lpVtbl->Release(item);
    if (FAILED(hr)) return 0;

    SIZE size = { width, height };
    HBITMAP hbitmap = NULL;
    hr = factory->lpVtbl->GetImage(factory, size, SIIGBF_BIGGERSIZEOK, &hbitmap);
    factory->lpVtbl->Release(factory);
    if (FAILED(hr) || !hbitmap) return 0;

    // Convert HBITMAP to JPEG buffer using WIC
    IWICImagingFactory* pFactory = NULL;
    IWICBitmap* pWICBitmap = NULL;
    IStream* pStream = NULL;
    IWICBitmapEncoder* pEncoder = NULL;
    IWICBitmapFrameEncode* pFrame = NULL;
    PROPVARIANT option;
    ULONG cbRead;
    HRESULT hr2;

    CoInitialize(NULL);
    hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                          &IID_IWICImagingFactory, (LPVOID*)&pFactory);
    if (FAILED(hr)) { DeleteObject(hbitmap); return 0; }

    hr = pFactory->lpVtbl->CreateBitmapFromHBITMAP(pFactory, hbitmap, NULL, WICBitmapIgnoreAlpha, &pWICBitmap);
    if (FAILED(hr)) { pFactory->lpVtbl->Release(pFactory); DeleteObject(hbitmap); return 0; }

    CreateStreamOnHGlobal(NULL, TRUE, &pStream);
    hr = pFactory->lpVtbl->CreateEncoder(pFactory, &GUID_ContainerFormatJpeg, NULL, &pEncoder);
    if (FAILED(hr)) { pWICBitmap->lpVtbl->Release(pWICBitmap); pFactory->lpVtbl->Release(pFactory); DeleteObject(hbitmap); return 0; }

    hr = pEncoder->lpVtbl->Initialize(pEncoder, pStream, WICBitmapEncoderNoCache);
    hr2 = pEncoder->lpVtbl->CreateNewFrame(pEncoder, &pFrame, NULL);
    pFrame->lpVtbl->Initialize(pFrame, NULL);
    pFrame->lpVtbl->WriteSource(pFrame, (IWICBitmapSource*)pWICBitmap, NULL);
    pFrame->lpVtbl->Commit(pFrame);
    pEncoder->lpVtbl->Commit(pEncoder);

    // Get JPEG buffer
    STATSTG stat;
    LARGE_INTEGER zero = {0};
    ULARGE_INTEGER pos;
    pStream->lpVtbl->Seek(pStream, zero, STREAM_SEEK_SET, &pos);
    pStream->lpVtbl->Stat(pStream, &stat, STATFLAG_NONAME);
    *jpegSize = (size_t)stat.cbSize.QuadPart;
    *jpegBuffer = (unsigned char*)CoTaskMemAlloc(*jpegSize);
    pStream->lpVtbl->Read(pStream, *jpegBuffer, (ULONG)*jpegSize, &cbRead);

    // Cleanup
    pFrame->lpVtbl->Release(pFrame);
    pEncoder->lpVtbl->Release(pEncoder);
    pStream->lpVtbl->Release(pStream);
    pWICBitmap->lpVtbl->Release(pWICBitmap);
    pFactory->lpVtbl->Release(pFactory);
    DeleteObject(hbitmap);
    CoUninitialize();
    return 1;

#elif defined(__APPLE__)
    // --- macOS implementation ---
    *jpegBuffer = NULL;
    *jpegSize = 0;
    CFStringRef cfPath = CFStringCreateWithCString(NULL, filePath, kCFStringEncodingUTF8);
    CFURLRef url = CFURLCreateWithFileSystemPath(NULL, cfPath, kCFURLPOSIXPathStyle, false);
    CFRelease(cfPath);

    CGSize size = { width, height };
    CGImageRef image = QLThumbnailImageCreate(kCFAllocatorDefault, url, size, NULL);
    CFRelease(url);
    if (!image) return 0;

    CFMutableDataRef destData = CFDataCreateMutable(NULL, 0);
    CGImageDestinationRef dest = CGImageDestinationCreateWithData(destData, kUTTypeJPEG, 1, NULL);
    CGImageDestinationAddImage(dest, image, NULL);
    bool success = CGImageDestinationFinalize(dest);
    CFRelease(dest);
    CGImageRelease(image);

    if (!success) {
        CFRelease(destData);
        return 0;
    }

    *jpegSize = CFDataGetLength(destData);
    *jpegBuffer = malloc(*jpegSize);
    memcpy(*jpegBuffer, CFDataGetBytePtr(destData), *jpegSize);
    CFRelease(destData);
    return 1;

#else
    // --- Linux implementation ---
    *jpegBuffer = NULL;
    *jpegSize = 0;
    char* abs_path = realpath(filePath, NULL);
    if (!abs_path) {
        fprintf(stderr, "Could not resolve absolute path for %s\n", filePath);
        return 0;
    }

    // Compute md5 hash of the absolute file path
    GChecksum* checksum = g_checksum_new(G_CHECKSUM_MD5);
    g_checksum_update(checksum, (const guchar*)abs_path, strlen(abs_path));
    const char* md5_hex = g_checksum_get_string(checksum);
    g_checksum_free(checksum);
    free(abs_path);

    const char* thumb_dirs[] = { "normal", "large" };
    char thumb_path[PATH_MAX];
    for (int i = 0; i < 2; i++) {
        snprintf(thumb_path, sizeof(thumb_path), "%s/.cache/thumbnails/%s/%s.jpg",
                 g_get_home_dir(), thumb_dirs[i], md5_hex);

        if (g_file_test(thumb_path, G_FILE_TEST_EXISTS)) {
            // Load the thumbnail file into a pixbuf
            GError* error = NULL;
            GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(thumb_path, &error);
            if (!pixbuf) {
                fprintf(stderr, "Failed to load OS thumbnail: %s\n", error ? error->message : "Unknown error");
                if (error) g_error_free(error);
                return 0;
            }
            // Save to JPEG buffer
            error = NULL;
            gboolean ok = gdk_pixbuf_save_to_buffer(
                pixbuf, (char**)jpegBuffer, (gsize*)jpegSize, "jpeg", &error, "quality", "85", NULL);
            g_object_unref(pixbuf);

            if (!ok) {
                fprintf(stderr, "Failed to save thumbnail buffer: %s\n", error ? error->message : "Unknown error");
                if (error) g_error_free(error);
                return 0;
            }
            return 1;
        }
    }
    fprintf(stderr, "OS thumbnail not found for %s\n", filePath);
    return 0;
#endif
}

// Example main for testing
#ifdef TEST_OS_THUMBNAIL
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <file-path>\n", argv[0]);
        return 1;
    }
    unsigned char* jpegBuffer = NULL;
    size_t jpegSize = 0;
    if (LoadOSThumbnailJPEGBuffer(argv[1], 256, 256, &jpegBuffer, &jpegSize)) {
        FILE* f = fopen("thumbnail.jpg", "wb");
        fwrite(jpegBuffer, 1, jpegSize, f);
        fclose(f);
#if defined(_WIN32)
        CoTaskMemFree(jpegBuffer);
#elif defined(__APPLE__)
        free(jpegBuffer);
#else
        g_free(jpegBuffer);
#endif
        printf("Thumbnail saved to thumbnail.jpg\n");
    } else {
        printf("Failed to load OS thumbnail.\n");
    }
    return 0;
}
#endif
