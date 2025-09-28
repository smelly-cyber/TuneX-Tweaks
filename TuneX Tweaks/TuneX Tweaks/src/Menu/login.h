#pragma once
#include <d3d9.h>
#include <windows.h>

// global login state
extern bool loggedIn;

// functions exposed from tunex_login.cpp
void DrawLoginUI(LPDIRECT3DDEVICE9 device, HWND hwnd);
bool IsLoggedIn();
void LoadLogoTexture(LPDIRECT3DDEVICE9 device);
void ReleaseLogoTexture();
