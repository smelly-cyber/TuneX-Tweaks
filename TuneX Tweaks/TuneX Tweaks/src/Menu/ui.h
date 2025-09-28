#pragma once
#include <windows.h>
#include <d3d9.h>
#include <shellapi.h>
#include "imgui.h"

#include "icon-loader.h"
#include "icons.h"
#include "theme.h"
#include "system-info.h"
#include "overlay.h"
#include "chrome.h"
#include "system-health.h"

void DrawWindowControls(HWND hwnd);

static constexpr float kTopBarH = 110.0f;
static constexpr float kSidebarW = 200.0f;

static int activePage = 0;

static LPDIRECT3DTEXTURE9 tex_logo = nullptr;   static int logo_w = 0, logo_h = 0;
static LPDIRECT3DTEXTURE9 tex_youtube = nullptr; static int yt_w = 0, yt_h = 0;
static LPDIRECT3DTEXTURE9 tex_discord = nullptr; static int dc_w = 0, dc_h = 0;

inline void LoadTuneXIcons(LPDIRECT3DDEVICE9 device) {
    LoadTextureFromMemory(logo_png, logo_png_len, &tex_logo, &logo_w, &logo_h, device);
    LoadTextureFromMemory(youtube_png, youtube_png_len, &tex_youtube, &yt_w, &yt_h, device);
    LoadTextureFromMemory(discord_png, discord_png_len, &tex_discord, &dc_w, &dc_h, device);
}

inline bool OutlinedButton(const char* label, const ImVec2& size, ImVec4 col_border, ImVec4 col_text) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.2f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.10f, 0.15f, 0.9f));

    bool pressed = ImGui::Button(label, size);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p_min = ImGui::GetItemRectMin();
    ImVec2 p_max = ImGui::GetItemRectMax();

    dl->AddRect(p_min, p_max, ImGui::GetColorU32(col_border), 6.0f, 0, 2.0f);

    ImVec2 text_sz = ImGui::CalcTextSize(label);
    ImVec2 center((p_min.x + p_max.x) * 0.5f, (p_min.y + p_max.y) * 0.5f);

    dl->AddText(ImVec2(center.x - text_sz.x * 0.5f, center.y - text_sz.y * 0.5f),
        ImGui::GetColorU32(col_text), label);

    ImGui::PopStyleColor(3);
    return pressed;
}


inline void BeginCard(const char* id, ImVec2 size = ImVec2(0, 0)) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, TuneX::Ink2);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 14));
    ImGui::BeginChild(id, size, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
}

inline void EndCard() {
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

inline void InfoRow(const char* label, const std::string& value) {
    ImFont* bold = (ImGui::GetIO().Fonts->Fonts.Size > 1) ? ImGui::GetIO().Fonts->Fonts[1] : nullptr;
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    if (bold) ImGui::PushFont(bold);
    ImGui::TextColored(TuneX::AccentBlue, "%s", label);
    if (bold) ImGui::PopFont();
    ImGui::TableSetColumnIndex(1);
    ImGui::TextUnformatted(value.c_str());
}

inline void DrawSidebar() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
    ImGui::Begin("Sidebar", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
    ImGui::SetWindowPos(ImVec2(0, kTopBarH));
    ImGui::SetWindowSize(ImVec2(kSidebarW, ImGui::GetIO().DisplaySize.y - kTopBarH));

    struct Entry { const char* label; int page; };
    static const Entry entries[] = {
        { "HOME", 0 },
        { "SYSTEM INFORMATION", 1 },
        { "GPU", -1 },
        { "POWER", -1 },
        { "NETWORK", -1 },
        { "CPU", -1 },
        { "MEMORY", -1 },
        { "SECURITY", -1 },
        { "CLEAN", -1 }
    };

    const int num = IM_ARRAYSIZE(entries);
    const float availH = ImGui::GetContentRegionAvail().y;
    float btnH = (availH / num) - 2.0f;
    btnH = (btnH < 26.0f ? 26.0f : (btnH > 40.0f ? 40.0f : btnH));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));

    for (int i = 0; i < num; ++i) {
        if (ImGui::Button(entries[i].label, ImVec2(-1, btnH))) {
            if (entries[i].page >= 0) activePage = entries[i].page;
        }
    }

    const float iconSize = 32.0f;
    const float spacing = 6.0f;
    const float margin = 12.0f;
    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 winSize = ImGui::GetWindowSize();
    float x = winPos.x + winSize.x - (iconSize * 2 + spacing) - margin;
    float y = winPos.y + winSize.y - iconSize - margin;

    ImGui::SetCursorScreenPos(ImVec2(x, y));
    if (tex_youtube && ImGui::InvisibleButton("##yt_side", ImVec2(iconSize, iconSize)))
        ShellExecuteA(NULL, "open", "https://youtube.com/placeholder", NULL, NULL, SW_SHOWNORMAL);
    if (tex_youtube)
        ImGui::GetWindowDrawList()->AddImage((void*)tex_youtube, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

    ImGui::SameLine(0.0f, spacing);
    if (tex_discord && ImGui::InvisibleButton("##dc_side", ImVec2(iconSize, iconSize)))
        ShellExecuteA(NULL, "open", "https://discord.gg/placeholder", NULL, NULL, SW_SHOWNORMAL);
    if (tex_discord)
        ImGui::GetWindowDrawList()->AddImage((void*)tex_discord, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

    ImGui::PopStyleVar(3);
    ImGui::End();
    ImGui::PopStyleVar();
}

inline void DrawTopBar(HWND hwnd) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
    ImGui::Begin("TopBar", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, kTopBarH));

    if (tex_logo) {
        const float logoH = 200.0f;
        const float aspect = (logo_w > 0 && logo_h > 0) ? (float)logo_w / (float)logo_h : 3.2f;
        const float logoW = logoH * aspect;
        ImGui::SetCursorPos(ImVec2(20, (kTopBarH - logoH) * 0.5f));
        ImGui::Image((void*)tex_logo, ImVec2(logoW, logoH));
    }

    DrawWindowControls(hwnd);

    ImGui::End();
    ImGui::PopStyleVar();
}

inline void DrawHomePage() {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float cardHeight = (avail.y - 12.0f) * 0.5f;

    BeginCard("RestoreCard", ImVec2(-1, cardHeight));
    if (ImGui::BeginTable("restorecols", 2, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableNextColumn();
        DynamicHeader("RESTORE POINT");
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 300);
        ImGui::Text("A backup copy of important Windows operating system files and settings "
            "that can be used to recover the system to an earlier point of time to revert changes");
        ImGui::PopTextWrapPos();
        ImGui::TableNextColumn();
        ImGui::Dummy(ImVec2(0, 8));
        DynamicButton("CREATE A RESTORE POINT", TuneX::AccentGreen, TuneX::TextBright);
        ImGui::Dummy(ImVec2(0, 10));
        DynamicButton("USE A RESTORE POINT", TuneX::AccentRed, TuneX::TextBright);
        ImGui::EndTable();
    }
    EndCard();

    ImGui::Dummy(ImVec2(0, 5));

    BeginCard("SystemHealthCard", ImVec2(-1, cardHeight));
    if (ImGui::BeginTable("healthcols", 2, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableNextColumn();
        DynamicHeader("SYSTEM HEALTH");
        ImGui::BulletText("Windows: %s", GetWindowsVersion().c_str());
        ImGui::BulletText("DirectX: %s", GetDirectXVersion().c_str());
        ImGui::BulletText("Last Update: %s", GetLastUpdateDate().c_str());
        ImGui::BulletText("Uptime: %s", GetUptime().c_str());
        ImGui::Dummy(ImVec2(0, 10));
        DynamicHeader("SECURITY STATUS");
        ImGui::BulletText("Windows Defender: %s", GetDefenderStatus().c_str());
        ImGui::BulletText("Firewall: %s", GetFirewallStatus().c_str());
        ImGui::BulletText("BitLocker: %s", GetBitLockerStatus().c_str());
        ImGui::TableNextColumn();
        DynamicHeader("STORAGE INFO");
        auto drives = GetDrivesUsage();
        for (const auto& d : drives) {
            ImGui::Text("%s  %.1f GB free / %.1f GB total", d.letter.c_str(), d.freeGB, d.totalGB);
            ImGui::ProgressBar(d.usedRatio, ImVec2(-1, 18));
            ImGui::Dummy(ImVec2(0, 6));
        }
        ImGui::EndTable();
    }
    EndCard();
}

inline void DrawSystemInfoPage() {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float cardHeight = (avail.y - 12.0f) * 0.5f;

    BeginCard("SystemSpecs", ImVec2(-1, cardHeight));
    DynamicHeader("SYSTEM SPECIFICATIONS");
    if (ImGui::BeginTable("specs", 2,
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 140.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
        InfoRow("GPU:", GetGPUName());
        InfoRow("CPU:", GetCPUName());
        InfoRow("RAM:", GetRAMInfo());
        InfoRow("Motherboard:", GetMotherboardName());
        InfoRow("BIOS:", GetBIOSVersion());
        ImGui::EndTable();
    }
    EndCard();

    ImGui::Dummy(ImVec2(0, 5));

    BeginCard("SystemSerials", ImVec2(-1, cardHeight));
    DynamicHeader("SYSTEM SERIALS");
    if (ImGui::BeginTable("serials", 2,
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 140.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
        InfoRow("Baseboard Serial:", GetBaseboardSerial());
        InfoRow("BIOS Serial:", GetBIOSSerial());
        InfoRow("CPU ID:", GetCPUSerial());
        InfoRow("System UUID:", GetSystemUUID());
        InfoRow("Disk Serial:", GetDiskSerial());
        InfoRow("MOBO Vendor:", GetMotherboardVendor());
        InfoRow("Primary MAC:", GetMACAddress());
        ImGui::EndTable();
    }
    EndCard();
}

inline void DrawTuneXApp(HWND hwnd) {
    ApplyTuneXTheme();
    DrawTopBar(hwnd);
    DrawSidebar();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
    ImGui::Begin("Content", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetWindowPos(ImVec2(kSidebarW, kTopBarH));
    ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - kSidebarW,
        ImGui::GetIO().DisplaySize.y - kTopBarH));

    if (activePage == 0) DrawHomePage();
    else if (activePage == 1) DrawSystemInfoPage();

    ImGui::End();
    ImGui::PopStyleVar();
    DrawOverlay();
}
