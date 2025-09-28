#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <intrin.h>
#include <iphlpapi.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <algorithm>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "iphlpapi.lib")

// Utility: query WMI for a single property
inline std::string QueryWMI(const std::wstring& wql, const std::wstring& property)
{
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    IEnumWbemClassObject* pEnumerator = nullptr;
    HRESULT hres;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) return "COM Init Failed";

    hres = CoInitializeSecurity(
        NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);

    if (FAILED(hres) && hres != RPC_E_TOO_LATE) {
        CoUninitialize();
        return "Security Init Failed";
    }

    hres = CoCreateInstance(CLSID_WbemLocator, 0,
        CLSCTX_INPROC_SERVER, IID_IWbemLocator,
        (LPVOID*)&pLoc);

    if (FAILED(hres)) {
        CoUninitialize();
        return "WbemLocator Failed";
    }

    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL, NULL, 0, NULL, 0, 0, &pSvc);

    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return "ConnectServer Failed";
    }

    hres = CoSetProxyBlanket(
        pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE);

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return "Proxy Failed";
    }

    hres = pSvc->ExecQuery(
        bstr_t("WQL"), bstr_t(wql.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnumerator);

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return "Query Failed";
    }

    IWbemClassObject* pObj = nullptr;
    ULONG uReturn = 0;
    std::string result = "Unknown";

    if (pEnumerator) {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &uReturn);
        if (uReturn) {
            VARIANT vtProp;
            VariantInit(&vtProp);
            if (SUCCEEDED(pObj->Get(property.c_str(), 0, &vtProp, 0, 0))) {
                if ((vtProp.vt == VT_BSTR) && vtProp.bstrVal != nullptr)
                    result = _bstr_t(vtProp.bstrVal);
            }
            VariantClear(&vtProp);
            pObj->Release();
        }
    }

    if (pSvc) pSvc->Release();
    if (pLoc) pLoc->Release();
    if (pEnumerator) pEnumerator->Release();
    CoUninitialize();
    return result;
}

// ================= ORIGINAL FUNCTIONS =================
inline std::string GetCPUName()
{
    int cpuInfo[4] = { -1 };
    char name[0x40] = {};
    __cpuid(cpuInfo, 0x80000000);
    unsigned int nExIds = cpuInfo[0];
    if (nExIds >= 0x80000004) {
        __cpuid(cpuInfo, 0x80000002);
        memcpy(name, cpuInfo, sizeof(cpuInfo));
        __cpuid(cpuInfo, 0x80000003);
        memcpy(name + 16, cpuInfo, sizeof(cpuInfo));
        __cpuid(cpuInfo, 0x80000004);
        memcpy(name + 32, cpuInfo, sizeof(cpuInfo));
    }
    return std::string(name);
}

inline std::string GetRAMInfo()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    DWORDLONG totalPhys = status.ullTotalPhys / (1024 * 1024 * 1024);

    char buf[64];
    sprintf_s(buf, "%llu GB RAM", totalPhys);
    return std::string(buf);
}

inline std::string GetGPUName()
{
    DISPLAY_DEVICEA dd;
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(dd);

    if (EnumDisplayDevicesA(NULL, 0, &dd, 0))
        return std::string(dd.DeviceString);

    return "Unknown GPU";
}

inline std::string GetMotherboardName()
{
    HKEY hKey;
    char buf[256];
    DWORD bufSize = sizeof(buf);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\BIOS",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueExA(hKey, "BaseBoardProduct", 0, NULL, (LPBYTE)buf, &bufSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::string(buf);
        }
        RegCloseKey(hKey);
    }
    return "Unknown Motherboard";
}

inline std::string GetBIOSVersion()
{
    HKEY hKey;
    char buf[256];
    DWORD bufSize = sizeof(buf);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\BIOS",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueExA(hKey, "BIOSVersion", 0, NULL, (LPBYTE)buf, &bufSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::string(buf);
        }
        RegCloseKey(hKey);
    }
    return "Unknown BIOS";
}

// ================= NEW SERIAL FUNCTIONS =================
inline std::string GetBaseboardSerial() {
    return QueryWMI(L"SELECT SerialNumber FROM Win32_BaseBoard", L"SerialNumber");
}

inline bool IsInvalidSerial(const std::string& s)
{
    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    return (lower.find("default") != std::string::npos ||
        lower.find("system") != std::string::npos ||
        lower.find("o.e.m") != std::string::npos ||
        lower.find("filled") != std::string::npos ||
        lower.empty());
}

inline std::string ExtractString(BYTE* data, BYTE length, BYTE index, BYTE* end)
{
    if (index == 0) return "";
    const char* str = (const char*)(data + length);
    int idx = 1;
    while (str < (char*)end && *str) {
        if (idx == index) return std::string(str);
        str += strlen(str) + 1;
        idx++;
    }
    return "";
}

inline std::string GetBIOSSerial()
{
    // Read SMBIOS/RSMB table to attempt to extract the serial string reliably.
    UINT size = GetSystemFirmwareTable('RSMB', 0, nullptr, 0);
    if (size == 0) return "No BIOS Serial";

    std::vector<BYTE> buffer(size);
    if (GetSystemFirmwareTable('RSMB', 0, buffer.data(), size) != size)
        return "SMBIOS Read Failed";

    BYTE* data = buffer.data();
    BYTE* end = data + size;

    std::string serial = "";

    while (data + 4 < end) {
        BYTE type = data[0];
        BYTE length = data[1];
        if (length == 0) break; // safety

        // Type 1 = System Information (contains chassis/serial fields)
        if (type == 1 && length >= 0x08) {
            BYTE serialIndex = data[0x07];
            if (serialIndex != 0) {
                serial = ExtractString(data, length, serialIndex, end);
                if (!IsInvalidSerial(serial)) return serial;
            }
        }
        // Type 0 = BIOS Information (vendor/version strings)
        if (type == 0 && length >= 0x06) {
            BYTE vendorIndex = data[0x04];
            BYTE versionIndex = data[0x05];
            std::string vendor = vendorIndex ? ExtractString(data, length, vendorIndex, end) : "";
            std::string version = versionIndex ? ExtractString(data, length, versionIndex, end) : "";
            if (!vendor.empty() || !version.empty()) {
                std::string combined = vendor + (vendor.empty() || version.empty() ? "" : " ") + version;
                if (!IsInvalidSerial(combined)) return combined;
            }
        }

        // advance to next structure: length + string-set (terminated by double 0)
        BYTE hdrLen = length;
        BYTE* p = data + hdrLen;
        // advance past strings until double null
        while (p < end && !(p[0] == 0 && (p + 1 >= end || p[1] == 0))) {
            // move to next string
            size_t slen = strlen((const char*)p);
            p += slen + 1;
            if (p >= end) break;
        }
        // skip the double null terminator
        if (p < end && p[0] == 0) p++;
        if (p < end && p[0] == 0) p++;

        data = p;
    }

    return "No valid BIOS Serial";
}

inline std::string GetCPUSerial() {
    return QueryWMI(L"SELECT ProcessorId FROM Win32_Processor", L"ProcessorId");
}

inline std::string GetSystemUUID() {
    return QueryWMI(L"SELECT UUID FROM Win32_ComputerSystemProduct", L"UUID");
}

inline std::string GetDiskSerial() {
    return QueryWMI(L"SELECT SerialNumber FROM Win32_DiskDrive", L"SerialNumber");
}

inline std::string GetMotherboardVendor() {
    return QueryWMI(L"SELECT Manufacturer FROM Win32_BaseBoard", L"Manufacturer");
}

// ================= FIXED MAC ADDRESS =================
inline std::string GetMACAddress()
{
    PIP_ADAPTER_INFO AdapterInfo = nullptr;
    DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
    AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
    if (!AdapterInfo) return "Unknown";

    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(AdapterInfo);
        AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
        if (!AdapterInfo) return "Unknown";
    }

    std::string mac = "Unknown";
    if (AdapterInfo && GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
        if (pAdapterInfo) {
            char buf[64];
            sprintf_s(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
                pAdapterInfo->Address[0], pAdapterInfo->Address[1], pAdapterInfo->Address[2],
                pAdapterInfo->Address[3], pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
            mac = buf;
        }
    }
    if (AdapterInfo) free(AdapterInfo);
    return mac;
}
