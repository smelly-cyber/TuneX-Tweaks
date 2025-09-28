#pragma once
#include <d3d9.h>

bool LoadTextureFromMemory(const unsigned char* data, unsigned int data_size,
    LPDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height,
    LPDIRECT3DDEVICE9 device);

extern const unsigned char logo_png[];
extern const unsigned int logo_png_len;

extern const unsigned char cpu_png[];
extern const unsigned int cpu_png_len;

extern const unsigned char gpu_png[];
extern const unsigned int gpu_png_len;

extern const unsigned char memory_png[];
extern const unsigned int memory_png_len;

extern const unsigned char clean_png[];
extern const unsigned int clean_png_len;

extern const unsigned char network_png[];
extern const unsigned int network_png_len;

extern const unsigned char youtube_png[];
extern const unsigned int youtube_png_len;

extern const unsigned char discord_png[];
extern const unsigned int discord_png_len;

extern const unsigned char other_png[];
extern const unsigned int other_png_len;
