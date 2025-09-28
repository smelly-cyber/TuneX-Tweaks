#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <iphlpapi.h>
#include <wbemidl.h>
#include <comdef.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "iphlpapi.lib")

// -------- DRIVE USAGE --------
struct DriveInfo {
    std::string letter;
    float usedRatio;
    double freeGB;
    double totalGB;
};

inline std::vector<DriveInfo> GetDrivesUsage()
{
    std::vector<DriveInfo> drives;
    DWORD drivesMask = GetLogicalDrives();
    for (char letter = 'A'; letter <= 'Z'; ++letter)
    {
        if (drivesMask & (1 << (letter - 'A')))
        {
            std::string root = std::string(1, letter) + ":\\";
            ULARGE_INTEGER freeBytes, totalBytes;
            if (GetDiskFreeSpaceExA(root.c_str(), &freeBytes, &totalBytes, NULL))
            {
                DriveInfo di;
                di.letter = root;
                di.totalGB = (double)totalBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
                di.freeGB = (double)freeBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
                di.usedRatio = (float)(1.0 - (double)freeBytes.QuadPart / (double)totalBytes.QuadPart);
                drives.push_back(di);
            }
        }
    }
    return drives;
}

// -------- WINDOWS SYSTEM INFO --------
typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

inline std::string GetWindowsVersion()
{
    HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
    if (hMod)
    {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
        if (fxPtr != nullptr)
        {
            RTL_OSVERSIONINFOW rovi = { 0 };
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (fxPtr(&rovi) == 0) // STATUS_SUCCESS
            {
                std::ostringstream oss;
                oss << "Windows " << rovi.dwMajorVersion
                    << "." << rovi.dwMinorVersion
                    << " (Build " << rovi.dwBuildNumber << ")";
                return oss.str();
            }
        }
    }
    return "Unknown Windows Version";
}

inline std::string GetDirectXVersion()
{
    // Simplified: Normally you'd query via DirectX APIs
    return "DirectX 12";
}

inline std::string GetLastUpdateDate()
{
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\Results\\Install",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD size = 256;
        char buf[256] = { 0 };
        if (RegQueryValueExA(hKey, "LastSuccessTime", 0, NULL, (LPBYTE)buf, &size) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return std::string(buf);
        }
        RegCloseKey(hKey);
    }
    return "Unknown";
}

inline std::string GetUptime()
{
    ULONGLONG ms = GetTickCount64();
    ULONGLONG sec = ms / 1000;
    int days = (int)(sec / 86400);
    int hours = (int)((sec % 86400) / 3600);
    int minutes = (int)((sec % 3600) / 60);

    char buf[128];
    sprintf_s(buf, "%d days, %d hrs, %d min", days, hours, minutes);
    return std::string(buf);
}

// -------- SECURITY / STATUS --------
inline std::string GetFirewallStatus()
{
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\SharedAccess\\Parameters\\FirewallPolicy\\StandardProfile",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD enabled = 0, size = sizeof(enabled);
        if (RegQueryValueExA(hKey, "EnableFirewall", 0, NULL, (LPBYTE)&enabled, &size) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return enabled ? "Enabled" : "Disabled";
        }
        RegCloseKey(hKey);
    }
    return "Unknown";
}

inline std::string GetDefenderStatus()
{
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows Defender\\Real-Time Protection",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD disabled = 0, size = sizeof(disabled);
        if (RegQueryValueExA(hKey, "DisableRealtimeMonitoring", 0, NULL, (LPBYTE)&disabled, &size) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return disabled ? "OFF" : "ON";
        }
        RegCloseKey(hKey);
    }
    return "Unknown";
}

inline std::string GetBitLockerStatus()
{
    // Simplified: checking registry flag
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\BitLocker",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return "Enabled";
    }
    return "Disabled or Not Supported";
}
