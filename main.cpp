#include <iostream>
#include <windows.h>
#include <sysinfoapi.h>
#include "windows_edition.h"

std::wstring GetUpdateBuildRevision() {
    HKEY hKey;
    DWORD ubr = 0;
    DWORD dataSize = sizeof(ubr);

    // Open the registry key
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        // Query the UBR value
        if (RegQueryValueExW(hKey, L"UBR", nullptr, nullptr, reinterpret_cast<LPBYTE>(&ubr), &dataSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::to_wstring(ubr);
        }
        RegCloseKey(hKey);
                      }
    return L"Unknown"; // Return "Unknown" if the UBR value couldn't be retrieved
}

int main() {
    OSVERSIONINFOEX osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx(reinterpret_cast<OSVERSIONINFO *>(&osvi));

    std::string codebase;
    switch (osvi.dwPlatformId) {
        case VER_PLATFORM_WIN32_NT: codebase = "NT"; break;
        case VER_PLATFORM_WIN32_WINDOWS: codebase = "9x"; break;
        case VER_PLATFORM_WIN32s: codebase = "3.x"; break;
        default: codebase = "Unknown"; break;
    }
    printf("Codebase: %s\n", codebase.c_str());

    // get the architecture from SYSTEM_INFO
    std::string architecture;
    SYSTEM_INFO systemInfo;
    GetNativeSystemInfo(&systemInfo);  // Use GetNativeSystemInfo to get the system's architecture

    // Determine the system architecture
    switch (systemInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            architecture = "x64";
        break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            architecture = "x86";
        break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            architecture = "ARM64";
        break;
        case PROCESSOR_ARCHITECTURE_ARM:
            architecture = "ARM";
        break;
        case PROCESSOR_ARCHITECTURE_IA64:
            architecture = "Itanium";
        break;
        default:
            architecture = "N/A (no match)";
        break;
    }

    printf("Architecture: %s\n", architecture.c_str());

    std::string friendlyName;
    std::string editionName;

    if (codebase == "9x") {
        if (osvi.dwMajorVersion == 4) {
            if (osvi.dwMinorVersion == 0) friendlyName = "Windows 95";
            else if (osvi.dwMinorVersion == 10) friendlyName = (osvi.szCSDVersion[1] == 'A') ? "Windows 98 SE" : "Windows 98";
            else if (osvi.dwMinorVersion == 90) friendlyName = "Windows ME";
        }
    } else if (codebase == "3.x") {
        if (osvi.dwMinorVersion == 0) friendlyName = "Windows 3.0";
        else if (osvi.dwMinorVersion == 10) friendlyName = "Windows 3.1";
        else if (osvi.dwMinorVersion == 11) friendlyName = "Windows 3.11";
    } else if (codebase == "NT") {
        if (osvi.dwMajorVersion == 3) {
            if (osvi.dwMinorVersion == 10) friendlyName = "Windows NT 3.1";
            else if (osvi.dwMinorVersion == 50) friendlyName = "Windows NT 3.5";
            else if (osvi.dwMinorVersion == 51) friendlyName = "Windows NT 3.51";
        } else if (osvi.dwMajorVersion == 4) {
            friendlyName = "Windows NT 4.0";
        } else if (osvi.dwMajorVersion == 5) {
            if (osvi.dwMinorVersion == 0) friendlyName = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows 2000 Professional" : "Windows 2000 Server";
            else if (osvi.dwMinorVersion == 1) friendlyName = "Windows XP";
            else if (osvi.dwMinorVersion == 2) {
                if (GetSystemMetrics(SM_SERVERR2) != 0) friendlyName = "Windows Server 2003 R2";
                else friendlyName = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows XP" : "Windows Server 2003";
            }
        } else if (osvi.dwMajorVersion == 6) {
            if (osvi.dwMinorVersion == 0) friendlyName = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows Vista" : "Windows Server 2008";
            else if (osvi.dwMinorVersion == 1) friendlyName = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows 7" : "Windows Server 2008 R2";
            else if (osvi.dwMinorVersion == 2) friendlyName = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows 8" : "Windows Server 2012";
            else if (osvi.dwMinorVersion == 3) friendlyName = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows 8.1" : "Windows Server 2012 R2";
            else if (osvi.dwMinorVersion == 4) friendlyName = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows 10" : "Windows Server 2016"; // Technical Preview
        } else if (osvi.dwMajorVersion == 10) {
            if (osvi.dwMinorVersion != 0) friendlyName = "Unknown (NT 10)";
            else if (osvi.dwBuildNumber < 21296) friendlyName = "Windows 10";
            else friendlyName = "Windows 11";
        }
    } else {
        friendlyName = "Unknown";
    }

    printf("Friendly name: %s\n", friendlyName.c_str());

    if (osvi.dwMajorVersion >= 10) {
        // we can be more accurate
        if (auto RtlGetVersion = reinterpret_cast<void(*)(OSVERSIONINFOEXW*)>(GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlGetVersion"))) {
            OSVERSIONINFOEXW osviw;
            osviw.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
            RtlGetVersion(&osviw);
            printf("Version: %d.%d.%d.%d\n", osviw.dwMajorVersion, osviw.dwMinorVersion, osviw.dwBuildNumber, std::stoi(GetUpdateBuildRevision()));
        }
    }
    else {
        printf("Version: %d.%d.%d\n", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
    }

    // check if service pack is empty
    if (char* servicePack = osvi.szCSDVersion; servicePack[0] == 0) {
        printf("Service pack: N/A (probably RTM)\n");
    } else {
        printf("Service pack: %s\n", servicePack);
    }

    // how I check editions depends on the version
    // Vista+ is easy, just check the product type
    // I gotta check if I'm on Vista+ first
    if (osvi.dwMajorVersion >= 6) {
        // we can LoadLibrary what sysinfoapi.h uses
        if (HMODULE kernel32 = LoadLibrary("kernel32.dll")) {
            // we can get the function pointer
            typedef BOOL (WINAPI *GetProductInfo_t)(DWORD, DWORD, DWORD, DWORD, PDWORD);
            if (auto GetProductInfo = reinterpret_cast<GetProductInfo_t>(GetProcAddress(kernel32, "GetProductInfo"))) {
                // we can check the product type
                DWORD productType;
                if (GetProductInfo(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &productType)) {
                    editionName = getWindowsEdition(productType);
                }
                FreeLibrary(kernel32);
            }
        }
    }
    else {
        // This is for Windows XP and below
        // we can deduct some editions from GetSystemMetrics

        // get current console color
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        // set the color to yellow
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN);
        printf("Warning - This is a heuristic method and may not be accurate\n");
        // restore the color
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), csbi.wAttributes);


        if (GetSystemMetrics(SM_MEDIACENTER) != 0) editionName = "Media Center";
        else if (GetSystemMetrics(SM_STARTER) != 0) editionName = "Starter";
        else if (GetSystemMetrics(SM_TABLETPC) != 0) editionName = "Tablet PC";

        if (editionName.empty()) {
            // well that didn't work
            if (osvi.wProductType == VER_NT_WORKSTATION) {
                if (osvi.wSuiteMask & VER_SUITE_PERSONAL) editionName = "Home";
                else editionName = "Professional";
            }
        }
    }

    printf("Edition: %s\n", editionName.c_str());

    return 0;
}