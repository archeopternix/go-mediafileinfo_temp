#include <windows.h>
#include <shobjidl.h>
#include <gdiplus.h>
#include <stdio.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdiplus.lib")

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0, size = 0;
    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    free(pImageCodecInfo);
    return -1;
}

/**
 * Returns 1 on success, 0 on failure.
 * jpegBuffer: will be allocated (use free to release), size: jpegSize
 */
int GetThumbnailJPEGBuffer(const wchar_t* filePath, int thumbWidth, int thumbHeight, BYTE** jpegBuffer, ULONG* jpegSize) {
    HRESULT hr;
    int ret = 0;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    *jpegBuffer = NULL;
    *jpegSize = 0;

    hr = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    if (hr != 0) {
        return 0;
    }
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return 0;
    }

    IShellItem* psi = NULL;
    hr = SHCreateItemFromParsingName(filePath, NULL, IID_PPV_ARGS(&psi));
    if (SUCCEEDED(hr)) {
        IShellItemImageFactory* pif = NULL;
        hr = psi->QueryInterface(IID_PPV_ARGS(&pif));
        if (SUCCEEDED(hr)) {
            SIZE sz = { thumbWidth, thumbHeight };
            HBITMAP hBmp = NULL;
            hr = pif->GetImage(sz, SIIGBF_BIGGERSIZEOK, &hBmp);
            if (SUCCEEDED(hr)) {
                Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromHBITMAP(hBmp, NULL);
                if (bmp) {
                    CLSID jpgClsid;
                    if (GetEncoderClsid(L"image/jpeg", &jpgClsid) != -1) {
                        IStream* pStream = NULL;
                        if (CreateStreamOnHGlobal(NULL, TRUE, &pStream) == S_OK) {
                            if (bmp->Save(pStream, &jpgClsid, NULL) == Gdiplus::Ok) {
                                // Get the size and data of the stream
                                STATSTG stat;
                                if (pStream->Stat(&stat, STATFLAG_NONAME) == S_OK) {
                                    *jpegSize = (ULONG)stat.cbSize.QuadPart;
                                    HGLOBAL hMem = NULL;
                                    if (GetHGlobalFromStream(pStream, &hMem) == S_OK) {
                                        LPVOID pMem = GlobalLock(hMem);
                                        if (pMem) {
                                            *jpegBuffer = (BYTE*)malloc(*jpegSize);
                                            if (*jpegBuffer) {
                                                memcpy(*jpegBuffer, pMem, *jpegSize);
                                                ret = 1;
                                            }
                                            GlobalUnlock(hMem);
                                        }
                                    }
                                }
                            }
                            pStream->Release();
                        }
                    }
                    delete bmp;
                }
                DeleteObject(hBmp);
            }
            pif->Release();
        }
        psi->Release();
    }

    CoUninitialize();
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return ret;
}
