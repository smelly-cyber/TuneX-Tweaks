#include "login.h"
#include "ui.h"
#include "imgui.h"
#include "theme.h"
#include "icons.h"
#include "icon-loader.h"
#include "auth.hpp"
#include "utils.hpp"
#include "skStr.h"
#include "json.hpp"
#include <fstream>
#include <string>
#include <d3d9.h>

bool loggedIn = false;
static std::string licenseKey;
static std::string loginMessage;
static bool triedAutoLogin = false;
static LPDIRECT3DTEXTURE9 tex_login_logo = nullptr;

static bool showLicense = false;
static bool rememberMe = false;

std::string name = skCrypt("TuneX Tweaks").decrypt();
std::string ownerid = skCrypt("mz67HgYGxd").decrypt();
std::string version = skCrypt("1.0").decrypt();
std::string url = skCrypt("https://keyauth.win/api/1.3/").decrypt();
std::string path = "";

KeyAuth::api KeyAuthApp(name, ownerid, version, url, path);

static void SaveLicense(const std::string& key) {
    std::ofstream("license.dat") << key;
    std::ofstream("remember.dat") << (rememberMe ? "1" : "0");
}

static std::string LoadLicense() {
    std::ifstream in("license.dat");
    std::string key;
    std::getline(in, key);
    return key;
}

static bool LoadRememberFlag() {
    std::ifstream in("remember.dat");
    std::string val;
    std::getline(in, val);
    return (val == "1");
}

void LoadLogoTexture(LPDIRECT3DDEVICE9 device) {
    if (tex_login_logo) return;
    int w, h;
    LoadTextureFromMemory(logo_png, logo_png_len, &tex_login_logo, &w, &h, device);
    rememberMe = LoadRememberFlag();
    if (rememberMe) licenseKey = LoadLicense();
}

void ReleaseLogoTexture() {
    if (tex_login_logo) {
        tex_login_logo->Release();
        tex_login_logo = nullptr;
    }
}

bool IsLoggedIn() {
    return loggedIn;
}

void DrawLoginUI(LPDIRECT3DDEVICE9 device, HWND hwnd) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screen = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(screen);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::Begin("LoginScreen", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoResize);
    ImGui::PopStyleColor();

    DrawWindowControls(hwnd);

    if (tex_login_logo) {
        float logoW = 220.0f, logoH = 220.0f;
        ImGui::SetCursorPos(ImVec2((screen.x - logoW) * 0.5f, screen.y * 0.12f));
        ImGui::Image((void*)tex_login_logo, ImVec2(logoW, logoH));
    }

    float cardW = 420.0f, cardH = 280.0f;
    ImGui::SetCursorPos(ImVec2((screen.x - cardW) * 0.5f, screen.y * 0.4f));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, TuneX::Ink2);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));

    ImGui::BeginChild("LoginCard", ImVec2(cardW, cardH), true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (g_FontBold) ImGui::PushFont(g_FontBold);
    ImGui::SetCursorPosX((cardW - ImGui::CalcTextSize("LICENSE LOGIN").x) * 0.5f);
    ImGui::TextColored(TuneX::AccentBlue, "LICENSE LOGIN");
    if (g_FontBold) ImGui::PopFont();

    ImGui::Dummy(ImVec2(0, 12));

    static char licenseBuf[256] = {};
    strcpy_s(licenseBuf, licenseKey.c_str());

    ImGui::PushStyleColor(ImGuiCol_FrameBg, TuneX::Ink1);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, TuneX::Ink3);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, TuneX::Ink3);
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##license", licenseBuf, IM_ARRAYSIZE(licenseBuf),
        showLicense ? 0 : ImGuiInputTextFlags_Password);
    ImGui::PopStyleColor(3);

    licenseKey = licenseBuf;

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::Columns(2, nullptr, false);

    // Make checkboxes slightly visible when inactive
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1, 1, 1, 0.08f));       // faint background
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(1, 1, 1, 0.20f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, TuneX::AccentBlue);

    ImGui::Checkbox("Show", &showLicense);

    ImGui::NextColumn();
    ImGui::Checkbox("Remember me", &rememberMe);

    ImGui::PopStyleColor(3);

    ImGui::Columns(1);


    ImGui::Dummy(ImVec2(0, 16));

    if (OutlinedButton("LOGIN", ImVec2(-1, 42), TuneX::AccentBlue, TuneX::TextBright)) {
        try {
            KeyAuthApp.init();
            KeyAuthApp.license(licenseKey);
            if (KeyAuthApp.response.success) {
                loginMessage = "Login successful!";
                loggedIn = true;
                if (rememberMe) SaveLicense(licenseKey);
                else SaveLicense("");
            }
            else {
                loginMessage = KeyAuthApp.response.message;
            }
        }
        catch (const std::exception& e) {
            loginMessage = std::string("Error: ") + e.what();
        }
    }

    if (!loginMessage.empty()) {
        ImVec4 col = loggedIn ? TuneX::AccentGreen : TuneX::AccentRed;
        ImGui::SetCursorPosX((cardW - ImGui::CalcTextSize(loginMessage.c_str()).x) * 0.5f);
        ImGui::TextColored(col, "%s", loginMessage.c_str());
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    ImGui::End();
}
