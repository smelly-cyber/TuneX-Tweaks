#pragma once
#include <windows.h>
#include <d3d9.h>
#include "imgui.h"
#include "theme.h"

// Rects for hit-testing
inline RECT g_CloseBtnRect{ 0,0,0,0 };
inline RECT g_MinBtnRect{ 0,0,0,0 };
inline RECT g_MaxBtnRect{ 0,0,0,0 };

// Drag height (client Y range treated as title-bar for dragging)
inline float g_TitleBarHeight = 84.0f; // match top bar height

inline void DrawWindowControls(HWND hwnd)
{
    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 win_pos = ImGui::GetWindowPos();
    const ImVec2 win_size = ImGui::GetWindowSize();

    const float radius = 8.0f;
    const float diameter = radius * 2.0f;
    const float padding = 10.0f;
    const float spacing = 10.0f;

    ImVec2 center_close_local(win_size.x - padding - radius, padding + radius);
    ImVec2 center_max_local(win_size.x - padding - radius - (diameter + spacing), padding + radius);

    ImVec2 center_close = ImVec2(win_pos.x + center_close_local.x, win_pos.y + center_close_local.y);
    ImVec2 center_max = ImVec2(win_pos.x + center_max_local.x, win_pos.y + center_max_local.y);

    auto mk_rect = [&](const ImVec2& c) -> RECT {
        LONG l = (LONG)(c.x - win_pos.x - radius);
        LONG t = (LONG)(c.y - win_pos.y - radius);
        LONG r = l + (LONG)diameter;
        LONG b = t + (LONG)diameter;
        return RECT{ l, t, r, b };
        };

    g_CloseBtnRect = mk_rect(center_close);
    g_MaxBtnRect = mk_rect(center_max);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // ---------------- CLOSE BUTTON ----------------
    ImGui::SetCursorScreenPos(ImVec2(center_close.x - radius, center_close.y - radius));
    ImGui::InvisibleButton("##tx_btn_close", ImVec2(diameter, diameter));
    const bool over_close = ImGui::IsItemHovered();
    if (ImGui::IsItemClicked()) PostMessage(hwnd, WM_CLOSE, 0, 0);

    // ---------------- FULLSCREEN BUTTON ----------------
    ImGui::SetCursorScreenPos(ImVec2(center_max.x - radius, center_max.y - radius));
    ImGui::InvisibleButton("##tx_btn_max", ImVec2(diameter, diameter));
    const bool over_max = ImGui::IsItemHovered();
    if (ImGui::IsItemClicked()) {
        static bool fullscreen = false;
        static WINDOWPLACEMENT prev = { sizeof(prev) };

        if (!fullscreen) {
            // Save old placement
            GetWindowPlacement(hwnd, &prev);

            // Get screen dimensions
            HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFO mi = { sizeof(mi) };
            if (GetMonitorInfo(hMon, &mi)) {
                SetWindowLong(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
                SetWindowPos(hwnd, HWND_TOP,
                    mi.rcMonitor.left, mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_FRAMECHANGED | SWP_SHOWWINDOW);
            }
            fullscreen = true;
        }
        else {
            // Restore old placement
            SetWindowLong(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
            SetWindowPlacement(hwnd, &prev);
            SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                SWP_FRAMECHANGED | SWP_SHOWWINDOW);
            fullscreen = false;
        }
    }

    // ---------------- DRAW CIRCLES ----------------
    const ImU32 col_close = ImGui::GetColorU32(over_close ? ImVec4(1.0f, 0.1f, 0.1f, 1.0f)   // bright red when hovered
        : ImVec4(0.92f, 0.25f, 0.25f, 1.0f));
    const ImU32 col_max = ImGui::GetColorU32(over_max ? ImVec4(0.1f, 1.0f, 0.1f, 1.0f)   // bright green when hovered
        : ImVec4(0.25f, 0.85f, 0.25f, 1.0f));
    const ImU32 ring = ImGui::GetColorU32(TuneX::Stroke);

    // Close circle
    dl->AddCircleFilled(center_close, radius, col_close, 24);
    dl->AddCircle(center_close, radius + 0.75f, ring, 24, 1.5f);
    if (over_close) dl->AddCircle(center_close, radius + 3.0f, ring, 24, 1.5f);

    // Fullscreen circle
    dl->AddCircleFilled(center_max, radius, col_max, 24);
    dl->AddCircle(center_max, radius + 0.75f, ring, 24, 1.5f);
    if (over_max) dl->AddCircle(center_max, radius + 3.0f, ring, 24, 1.5f);
}
