#pragma once
#define NOMINMAX
#include <windows.h>
#include <string>

inline std::wstring TxW(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), w.data(), n);
    return w;
}

inline bool SetRegDWORD(const std::string& path, const std::string& name, DWORD val, HKEY root = HKEY_LOCAL_MACHINE) {
    HKEY k{};
    if (RegOpenKeyExW(root, TxW(path).c_str(), 0, KEY_SET_VALUE, &k) != ERROR_SUCCESS) return false;
    auto ok = RegSetValueExW(k, TxW(name).c_str(), 0, REG_DWORD, reinterpret_cast<const BYTE*>(&val), sizeof(val)) == ERROR_SUCCESS;
    RegCloseKey(k);
    return ok;
}
inline bool SetRegSZ(const std::string& path, const std::string& name, const std::string& val, HKEY root = HKEY_CURRENT_USER) {
    HKEY k{};
    if (RegOpenKeyExW(root, TxW(path).c_str(), 0, KEY_SET_VALUE, &k) != ERROR_SUCCESS) return false;
    auto wval = TxW(val);
    auto ok = RegSetValueExW(k, TxW(name).c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(wval.c_str()), (DWORD)((wval.size() + 1) * sizeof(wchar_t))) == ERROR_SUCCESS;
    RegCloseKey(k);
    return ok;
}
