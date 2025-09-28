#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <string>
#include <chrono>
#include "imgui.h"
#include "theme.h"

// ---------------- Overlay State ----------------
inline std::atomic<bool> OverlayActive = false;
inline std::atomic<float> OverlayProgress = 0.0f;
inline std::string OverlayMessage;
inline std::thread OverlayThread;

// Global logo texture (already loaded by LoadTuneXIcons in ui.h)
extern LPDIRECT3DTEXTURE9 tex_logo;
extern int logo_w, logo_h;

// ---------------- Run Job Async ----------------
// Allows passing a lambda with a progress callback
inline void RunTweakAsync(const char* msg, std::function<void(std::function<void(float)>)> job) {
    if (OverlayActive) return;
    OverlayActive = true;
    OverlayProgress = 0.0f;
    OverlayMessage = msg;

    OverlayThread = std::thread([job]() {
        try {
            job([](float pct) { OverlayProgress = pct; });
        }
        catch (...) {
            OverlayMessage = "Error running job!";
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        OverlayActive = false;
        });
    OverlayThread.detach();
}

// ---------------- Draw Overlay ----------------
inline void DrawOverlay() {
    if (!OverlayActive) return;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screen = io.DisplaySize;

    // Fullscreen transparent background
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(screen);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.75f));
    ImGui::Begin("Overlay", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoInputs);
    ImGui::PopStyleColor();

    ImVec2 center(screen.x * 0.5f, screen.y * 0.5f);

    // ---- Card ----
    ImGui::SetCursorScreenPos(ImVec2(center.x - 200, center.y - 150));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, TuneX::Ink2);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);

    ImGui::BeginChild("OverlayCard", ImVec2(400, 300), true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // ---- Logo ----
    if (tex_logo) {
        float logoH = 80.0f;
        float aspect = (logo_w > 0 && logo_h > 0) ? (float)logo_w / (float)logo_h : 1.0f;
        float logoW = logoH * aspect;
        ImGui::SetCursorPosX((400 - logoW) * 0.5f);
        ImGui::Image((void*)tex_logo, ImVec2(logoW, logoH));
    }

    ImGui::Dummy(ImVec2(0, 16));
    ImGui::SetCursorPosX((400 - ImGui::CalcTextSize(OverlayMessage.c_str()).x) * 0.5f);
    ImGui::TextColored(TuneX::AccentBlue, "%s", OverlayMessage.c_str());

    ImGui::Dummy(ImVec2(0, 16));
    ImGui::ProgressBar(OverlayProgress, ImVec2(-1, 30),
        OverlayProgress < 1.0f ? "Processing..." : "Done");

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    ImGui::End();
}
