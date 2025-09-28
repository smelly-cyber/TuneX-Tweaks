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

// ---------------- Globals ----------------
bool loggedIn = false;
static std::string licenseKey;
static std::string loginMessage;
static bool triedAutoLogin = false;
static LPDIRECT3DTEXTURE9 tex_login_logo = nullptr;

static bool showLicense = false;   // toggle show/hide input
static bool rememberMe = false;    // toggle remember license

// ---------------- KeyAuth instance ----------------
std::string name = skCrypt("TuneX Tweaks").decrypt();   // App name
std::string ownerid = skCrypt("mz67HgYGxd").decrypt();     // Owner ID
std::string version = skCrypt("1.0").decrypt();            // Application version
std::string url = skCrypt("https://keyauth.win/api/1.3/").decrypt();
std::string path = ""; // Optional custom path

KeyAuth::api KeyAuthApp(
    name,
    ownerid,
    version,
    url,
    path
);


// ---------------- Persistence ----------------
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

// ---------------- Textures ----------------
void LoadLogoTexture(LPDIRECT3DDEVICE9 device) {
    if (tex_login_logo) return;
    int w, h;
    LoadTextureFromMemory(logo_png, logo_png_len, &tex_login_logo, &w, &h, device);

    // On first load, auto-fill license if remembered
    rememberMe = LoadRememberFlag();
    if (rememberMe) {
        licenseKey = LoadLicense();
    }
}

void ReleaseLogoTexture() {
    if (tex_login_logo) {
        tex_login_logo->Release();
        tex_login_logo = nullptr;
    }
}

// ---------------- Helpers ----------------
bool IsLoggedIn() {
    return loggedIn;
}

// ---------------- UI ----------------
void DrawLoginUI(LPDIRECT3DDEVICE9 device, HWND hwnd)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screen = io.DisplaySize;

    // Fullscreen invisible window (transparent)
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(screen);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::Begin("LoginScreen", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoResize);
    ImGui::PopStyleColor();

    // Top-right chrome buttons (red/green)
    DrawWindowControls(hwnd);

    // --- Logo ---
    if (tex_login_logo) {
        float logoWidth = 300.0f;
        float logoHeight = 300.0f;
        float logoX = (screen.x - logoWidth) * 0.5f;
        float logoY = screen.y * 0.1f;

        ImGui::SetCursorPos(ImVec2(logoX, logoY));
        ImGui::Image((void*)tex_login_logo, ImVec2(logoWidth, logoHeight));
    }

    // --- Login card ---
    float cardWidth = 360.0f;
    float cardHeight = 200.0f;
    float cardX = (screen.x - cardWidth) * 0.5f;
    float cardY = screen.y * 0.25f + 140.0f;

    ImGui::SetCursorPos(ImVec2(cardX, cardY));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, TuneX::Ink2);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));

    ImGui::BeginChild("LoginCard", ImVec2(cardWidth, cardHeight), true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // --- License label ---
    ImGui::SetCursorPosX((cardWidth - ImGui::CalcTextSize("Enter License:").x) * 0.15f - 16);
    ImGui::TextColored(TuneX::AccentBlue, "Enter License:");

    static char licenseBuf[256] = {};
    strcpy_s(licenseBuf, licenseKey.c_str());

    // --- Input + Show on same row ---
    ImGui::PushStyleColor(ImGuiCol_FrameBg, TuneX::Ink1);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, TuneX::Ink3);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, TuneX::Ink3);

    ImGui::SetNextItemWidth(cardWidth - 120);
    ImGui::InputText("##license", licenseBuf, IM_ARRAYSIZE(licenseBuf),
        showLicense ? 0 : ImGuiInputTextFlags_Password);

    ImGui::PopStyleColor(3);

    licenseKey = licenseBuf;

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1, 1, 1, 0.1f));
    ImGui::PushStyleColor(ImGuiCol_Text, TuneX::AccentBlue);
    ImGui::Checkbox("Show", &showLicense);
    ImGui::PopStyleColor(2);

    // --- Remember me below ---
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1, 1, 1, 0.1f));
    ImGui::PushStyleColor(ImGuiCol_Text, TuneX::AccentBlue);
    ImGui::Checkbox("Remember me", &rememberMe);
    ImGui::PopStyleColor(2);

    ImGui::Dummy(ImVec2(0, 8));

    // --- Login button ---
    if (OutlinedButton("LOGIN", ImVec2(-1, 36), TuneX::AccentBlue, TuneX::TextBright)) {
        try {
            KeyAuthApp.init();
            KeyAuthApp.license(licenseKey);
            if (KeyAuthApp.response.success) {
                loginMessage = "Login success!";
                loggedIn = true;
                if (rememberMe) SaveLicense(licenseKey);
                else SaveLicense(""); // clear if not remembering
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
        ImGui::TextColored(col, "%s", loginMessage.c_str());
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2); // padding + rounding
    ImGui::PopStyleColor();

    ImGui::End();
}
