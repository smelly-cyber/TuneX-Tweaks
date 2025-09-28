#pragma once
#include "imgui.h"

// Accent palette for TuneX
namespace TuneX
{
    // Background shades
    static ImVec4 Ink0 = ImVec4(0.05f, 0.05f, 0.07f, 1.0f);
    static ImVec4 Ink1 = ImVec4(0.07f, 0.08f, 0.10f, 1.0f);
    static ImVec4 Ink2 = ImVec4(0.12f, 0.13f, 0.15f, 1.0f);
    static ImVec4 Ink3 = ImVec4(0.16f, 0.17f, 0.20f, 1.0f);

    // Accents
    static ImVec4 AccentBlue = ImVec4(0.00f, 0.55f, 0.95f, 1.0f);
    static ImVec4 DeepBlue = ImVec4(0.00f, 0.35f, 0.70f, 1.0f);
    static ImVec4 AccentGreen = ImVec4(0.15f, 0.75f, 0.35f, 1.0f);
    static ImVec4 AccentRed = ImVec4(0.85f, 0.20f, 0.25f, 1.0f);

    // Borders
    static ImVec4 Stroke = ImVec4(0.22f, 0.22f, 0.26f, 1.0f);

    // Text
    static ImVec4 TextBright = ImVec4(0.90f, 0.90f, 0.95f, 1.0f);
    static ImVec4 TextDim = ImVec4(0.60f, 0.65f, 0.70f, 1.0f);

    inline ImU32 GetColorU32(const ImVec4& c) { return ImGui::GetColorU32(c); }
}

// ---------------- GLOBAL THEME ----------------
inline void ApplyTuneXTheme()
{
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = 6.0f;
    s.FrameRounding = 5.0f;
    s.ChildRounding = 6.0f;
    s.ScrollbarRounding = 6.0f;
    s.ItemSpacing = ImVec2(6, 4);
    s.FramePadding = ImVec2(5, 3);
    s.WindowPadding = ImVec2(10, 8);

    ImVec4* c = s.Colors;

    // Text
    c[ImGuiCol_Text] = TuneX::TextBright;
    c[ImGuiCol_TextDisabled] = TuneX::TextDim;

    // Backgrounds
    c[ImGuiCol_WindowBg] = TuneX::Ink0;
    c[ImGuiCol_ChildBg] = TuneX::Ink1;
    c[ImGuiCol_PopupBg] = TuneX::Ink1;

    // Borders
    c[ImGuiCol_Border] = TuneX::Stroke;
    c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

    // Frames
    c[ImGuiCol_FrameBg] = TuneX::Ink2;
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.28f, 0.40f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.55f, 0.40f, 1.00f);

    // Title bars
    c[ImGuiCol_TitleBg] = TuneX::Ink1;
    c[ImGuiCol_TitleBgActive] = TuneX::Ink1;
    c[ImGuiCol_TitleBgCollapsed] = TuneX::Ink1;

    // Menus
    c[ImGuiCol_MenuBarBg] = TuneX::Ink1;

    // Scrollbars
    c[ImGuiCol_ScrollbarBg] = TuneX::Ink1;
    c[ImGuiCol_ScrollbarGrab] = TuneX::Ink2;
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.20f, 0.32f, 0.50f, 1.00f);
    c[ImGuiCol_ScrollbarGrabActive] = TuneX::DeepBlue;

    // Controls
    c[ImGuiCol_CheckMark] = TuneX::AccentBlue;
    c[ImGuiCol_SliderGrab] = TuneX::AccentBlue;
    c[ImGuiCol_SliderGrabActive] = TuneX::DeepBlue;

    // Buttons
    c[ImGuiCol_Button] = TuneX::Ink2;
    c[ImGuiCol_ButtonHovered] = ImVec4(0.21f, 0.33f, 0.50f, 1.00f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.55f, 0.40f, 1.00f);

    // Headers
    c[ImGuiCol_Header] = ImVec4(0.17f, 0.24f, 0.32f, 1.00f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.36f, 0.55f, 1.00f);
    c[ImGuiCol_HeaderActive] = TuneX::DeepBlue;

    // Separators
    c[ImGuiCol_Separator] = TuneX::Stroke;
    c[ImGuiCol_SeparatorHovered] = TuneX::DeepBlue;
    c[ImGuiCol_SeparatorActive] = TuneX::AccentBlue;

    // Resize grips
    c[ImGuiCol_ResizeGrip] = ImVec4(0.45f, 0.45f, 0.50f, 0.30f);
    c[ImGuiCol_ResizeGripHovered] = TuneX::DeepBlue;
    c[ImGuiCol_ResizeGripActive] = TuneX::AccentBlue;

    // Tabs
    c[ImGuiCol_Tab] = TuneX::Ink2;
    c[ImGuiCol_TabHovered] = ImVec4(0.18f, 0.28f, 0.45f, 1.00f);
    c[ImGuiCol_TabActive] = TuneX::DeepBlue;
}

// ---------------- DYNAMIC HELPERS ----------------

// Header bar that grows with text
// ---------------- DYNAMIC HELPERS ----------------

// Header bar that grows with text
inline void DynamicHeader(const char* text)
{
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 sz = ImGui::CalcTextSize(text);

    const float padX = 12.0f, padY = 6.0f, r = 5.0f;

    ImVec2 a(p.x, p.y);
    ImVec2 b(p.x + sz.x + padX * 2.0f, p.y + sz.y + padY * 2.0f);

    dl->AddRectFilled(a, b, ImGui::GetColorU32(TuneX::Ink3), r);
    dl->AddRect(a, b, ImGui::GetColorU32(TuneX::AccentBlue), r);

    ImGui::SetCursorScreenPos(ImVec2(p.x + padX, p.y + padY));
    ImGui::TextColored(TuneX::AccentBlue, "%s", text);

    ImGui::SetCursorScreenPos(ImVec2(a.x, b.y + 6.0f));
}

// Button that sizes dynamically with text and gets much lighter when hovered
inline bool DynamicButton(const char* label, ImVec4 col_border, ImVec4 col_text)
{
    ImVec2 text_sz = ImGui::CalcTextSize(label);
    ImVec2 size(text_sz.x + 40, text_sz.y + 12); // padding

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.35f, 0.8f)); // much lighter hover
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.25f, 1.0f));

    bool pressed = ImGui::Button(label, size);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p_min = ImGui::GetItemRectMin();
    ImVec2 p_max = ImGui::GetItemRectMax();

    // Make border + text lighter when hovered
    bool hovered = ImGui::IsItemHovered();
    ImVec4 border_col = hovered ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : col_border;
    ImVec4 text_col = hovered ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : col_text;

    dl->AddRect(p_min, p_max, ImGui::GetColorU32(border_col), 5.0f, 0, 2.0f);
    dl->AddText(ImVec2(p_min.x + (size.x - text_sz.x) * 0.5f,
        p_min.y + (size.y - text_sz.y) * 0.5f),
        ImGui::GetColorU32(text_col), label);

    ImGui::PopStyleColor(3);
    return pressed;
}

