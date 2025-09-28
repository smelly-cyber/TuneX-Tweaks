#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <string>
#include "imgui.h"

inline std::atomic<bool> OverlayActive = false;
inline std::atomic<float> OverlayProgress = 0.0f;
inline std::string OverlayMessage;
inline std::thread OverlayThread;

inline void RunTweakAsync(const char* msg, std::function<void()> job) {
    if (OverlayActive) return;
    OverlayActive = true;
    OverlayProgress = 0.0f;
    OverlayMessage = msg;

    OverlayThread = std::thread([job]() {
        job();
        for (int i = 0; i <= 100; i++) {
            OverlayProgress = i / 100.0f;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        OverlayActive = false;
        });
    OverlayThread.detach();
}

inline void DrawOverlay() {
    if (!OverlayActive) return;

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(320, 120));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::Begin("Overlay", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings);
    ImGui::Text("%s", OverlayMessage.c_str());
    ImGui::ProgressBar(OverlayProgress, ImVec2(-1, 0),
        OverlayProgress < 1.0f ? "Processing..." : "Done");
    ImGui::End();
    ImGui::PopStyleVar();
}
