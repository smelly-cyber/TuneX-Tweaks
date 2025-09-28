// Dear ImGui: standalone application for Win32 + DirectX 9
#include <windowsx.h>
#include "chrome.h"
#include "login.h"
#include "ui.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx9.h"
#include <d3d9.h>
#include <tchar.h>
#include <shellapi.h> // for ShellExecute if you later want external links

// ---------------- D3D Globals ----------------
static LPDIRECT3D9           g_pD3D = nullptr;
static LPDIRECT3DDEVICE9     g_pd3dDevice = nullptr;
static D3DPRESENT_PARAMETERS g_d3dpp = {};
static bool                  g_DeviceLost = false;
static UINT                  g_ResizeWidth = 0, g_ResizeHeight = 0;
static HWND                  g_hwnd = nullptr;

// ---------------- Forward Declares ----------------
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


// ---------------- Entry ----------------
int main(int, char**)
{
    const int kWindowW = 900;
    const int kWindowH = 600;

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L,
                       GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
                       L"TuneX_Class", nullptr };
    ::RegisterClassExW(&wc);

    float main_scale = 1.0f;
    g_hwnd = ::CreateWindowExW(
        WS_EX_APPWINDOW,
        wc.lpszClassName,
        L"TuneX Tweaks",
        WS_POPUP | WS_SYSMENU,
        100, 100,
        int(kWindowW * main_scale),
        int(kWindowH * main_scale),
        nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(g_hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(g_hwnd);

    // ---------------- ImGui Setup ----------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Fonts
    io.Fonts->AddFontDefault(); // base font
    ImFontConfig cfg;
    cfg.SizePixels = 18.0f;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 18.0f, &cfg); // Segoe UI Bold
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f);        // Regular body text

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);
    LoadTuneXIcons(g_pd3dDevice);
    LoadLogoTexture(g_pd3dDevice);

    // ---------------- Main Loop ----------------
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        if (g_DeviceLost)
        {
            HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST) { ::Sleep(10); continue; }
            if (hr == D3DERR_DEVICENOTRESET) ResetDevice();
            g_DeviceLost = false;
        }

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ApplyTuneXTheme();

        if (!IsLoggedIn())
            DrawLoginUI(g_pd3dDevice, g_hwnd);
        else
            DrawTuneXApp(g_hwnd);

        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        g_pd3dDevice->Clear(0, nullptr,
            D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
            D3DCOLOR_RGBA(18, 20, 25, 255), 1.0f, 0);

        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        if (g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr) == D3DERR_DEVICELOST)
            g_DeviceLost = true;
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    ReleaseLogoTexture();
    CleanupDeviceD3D();
    ::DestroyWindow(g_hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// ---------------- D3D Helpers ----------------
bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr) return false;
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;
    return true;
}
void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}
void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL) IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// ---------------- WndProc ----------------
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

// chrome.h provides these:
extern RECT g_CloseBtnRect;
extern RECT g_MinBtnRect;
extern float g_TitleBarHeight;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            g_ResizeWidth = (UINT)LOWORD(lParam);
            g_ResizeHeight = (UINT)HIWORD(lParam);
        }
        return 0;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = 900;
        mmi->ptMinTrackSize.y = 600;
        mmi->ptMaxTrackSize.x = 900;
        mmi->ptMaxTrackSize.y = 600;
        return 0;
    }

    case WM_NCHITTEST:
    {
        POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hWnd, &p);

        if (p.y >= 0 && p.y < (LONG)g_TitleBarHeight) {
            if (!PtInRect(&g_CloseBtnRect, p) && !PtInRect(&g_MinBtnRect, p))
                return HTCAPTION;
        }
        return HTCLIENT;
    }

    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
