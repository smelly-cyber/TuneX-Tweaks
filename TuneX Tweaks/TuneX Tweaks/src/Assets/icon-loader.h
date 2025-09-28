#pragma once
#include <d3d9.h>

// Function to load PNG bytes into a Direct3D9 texture
bool LoadTextureFromMemory(
    const unsigned char* data,
    unsigned int data_size,
    LPDIRECT3DTEXTURE9* out_texture,
    int* out_width,
    int* out_height,
    LPDIRECT3DDEVICE9 device
);
