#include <windows.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <iostream>
#include <string>
#include <vector>
#include <set>


std::wstring GetProcessPath(DWORD processID) {
    wchar_t path[MAX_PATH] = L"Access Denied (Requires Admin)";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processID);

    if (hProcess) {
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(hProcess, 0, path, &size)) {
            CloseHandle(hProcess);
            return std::wstring(path);
        }
        CloseHandle(hProcess);
    }
    return std::wstring(path);
}

//PID
std::wstring GetProcessNameByPID(DWORD processID) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        if (Process32FirstW(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == processID) {
                    CloseHandle(hSnapshot);
                    return std::wstring(pe32.szExeFile);
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return L"";
}

void KillProcess(DWORD processID) {
    std::wstring processName = GetProcessNameByPID(processID);

    std::set<std::wstring> whiteList = {
        L"System", L"smss.exe", L"csrss.exe", L"wininit.exe",
        L"services.exe", L"lsass.exe", L"winlogon.exe", L"System Idle Process"
    };

    if (processID == 0 || processID == 4 || whiteList.count(processName)) {
        std::cout << "[-] ACCESS DENIED: " << processID << " is a CRITICAL system process!\n";
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
    if (hProcess) {
        if (TerminateProcess(hProcess, 0)) {
            std::cout << "[+] Process " << processID << " successfully terminated!\n";
        } else {
            std::cout << "[-] Failed to terminate process. Error: " << GetLastError() << "\n";
        }
        CloseHandle(hProcess);
    } else {
        std::cout << "[-] Access denied. Run the program as Administrator!\n";
    }
}

int main() {
    std::cout << "=== ADVANCED PROCESS HUNTER ===\n\n";

    if (!IsUserAnAdmin()) {
        std::cout << "##################################################\n";
        std::cout << "[!] RUN AS ADMIN, PLEASE!!!\n";
        std::cout << "[!] Some file paths will not be displayed.\n";
        std::cout << "##################################################\n\n";
    }

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to create system snapshot.\n";
        return 1;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe32)) {
        std::cout << "PID\t| Process Name\t\t| File Path\n";
        std::cout << "----------------------------------------------------------------------\n";
        do {
            std::wstring path = GetProcessPath(pe32.th32ProcessID);

            std::wcout << pe32.th32ProcessID << L"\t| "
                       << pe32.szExeFile << L"\t| "
                       << path << L"\n";

        } while (Process32NextW(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);

    std::cout << "\n----------------------------------------------------------------------\n";
    std::cout << "Enter the PID of the process you want to KILL (or 0 to exit): ";

    DWORD targetPID;
    std::cin >> targetPID;

    if (targetPID != 0) {
        KillProcess(targetPID);
    }

    std::cout << "\nPress Enter to exit the program...";
    std::cin.get();
    std::cin.get();

    return 0;
}
