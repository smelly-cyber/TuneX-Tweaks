#include "icon-loader.h"
#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")

// Ensure GDI+ is initialized once
static bool gdi_init = false;
static ULONG_PTR gdi_token = 0;

static void EnsureGDIInit()
{
    if (!gdi_init)
    {
        Gdiplus::GdiplusStartupInput input;
        if (Gdiplus::GdiplusStartup(&gdi_token, &input, nullptr) == Gdiplus::Ok)
        {
            gdi_init = true;
        }
    }
}

// Function definition
bool LoadTextureFromMemory(const unsigned char* data, unsigned int data_size,
    LPDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height,
    LPDIRECT3DDEVICE9 device)
{
    EnsureGDIInit();
    if (!device || !data || data_size == 0)
        return false;

    // SHCreateMemStream requires non-const BYTE*
    IStream* stream = SHCreateMemStream((BYTE*)data, (UINT)data_size);
    if (!stream)
        return false;

    Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromStream(stream);
    stream->Release();

    if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok)
    {
        delete bmp;
        return false;
    }

    *out_width = bmp->GetWidth();
    *out_height = bmp->GetHeight();

    // Lock bitmap data
    Gdiplus::BitmapData bmpData;
    Gdiplus::Rect rect(0, 0, *out_width, *out_height);
    if (bmp->LockBits(&rect, Gdiplus::ImageLockModeRead,
        PixelFormat32bppARGB, &bmpData) != Gdiplus::Ok)
    {
        delete bmp;
        return false;
    }

    // Create Direct3D texture in MANAGED pool (safer than DEFAULT)
    if (FAILED(device->CreateTexture(*out_width, *out_height, 1,
        0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, out_texture, NULL)))
    {
        bmp->UnlockBits(&bmpData);
        delete bmp;
        return false;
    }

    // Copy pixels row by row
    D3DLOCKED_RECT texRect;
    if (FAILED((*out_texture)->LockRect(0, &texRect, NULL, 0)))
    {
        bmp->UnlockBits(&bmpData);
        delete bmp;
        return false;
    }

    for (UINT y = 0; y < *out_height; y++)
    {
        memcpy((BYTE*)texRect.pBits + y * texRect.Pitch,
            (BYTE*)bmpData.Scan0 + y * bmpData.Stride,
            *out_width * 4);
    }

    (*out_texture)->UnlockRect(0);
    bmp->UnlockBits(&bmpData);
    delete bmp;

    return true;
}
