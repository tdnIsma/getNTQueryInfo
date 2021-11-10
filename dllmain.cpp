#include "includes.h"



typedef NTSTATUS(__stdcall* tNtQueryInformationProcess)
(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
    );

tNtQueryInformationProcess NtQueryInfoProc = (tNtQueryInformationProcess)(helpers::getModule("ntdll.dll", "NtQueryInformationProcess"));

typedef NTSTATUS(__stdcall* tNtQueryInformationToken)
(
   HANDLE               TokenHandle,
   TOKEN_INFORMATION_CLASS TokenInformationClass,
   PVOID               TokenInformation,
   ULONG                TokenInformationLength,
   PULONG              ReturnLength
   );

tNtQueryInformationToken NtQueryToken = (tNtQueryInformationToken)(helpers::getModule("ntdll.dll", "NtQueryInformationToken"));


DWORD WINAPI InternalMain(HMODULE hMod) {
#ifdef _DEBUG
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
#endif

    HANDLE prochandle = reinterpret_cast<HANDLE>(helpers::getModuleHandle(L"your_process.exe"));

    DWORD_PTR buffer;

    NTSTATUS status = NtQueryInfoProc(prochandle, ProcessDebugPort, &buffer, sizeof(buffer), 0);


    while (!GetAsyncKeyState(VK_END) & 1) {

        if (NT_SUCCESS(status))
        {
            std::cout << std::hex << buffer << std::endl;
            Sleep(100);
        }


    }


#ifdef _DEBUG
   if (f != nullptr) fclose(f);
    FreeConsole();
#endif 

    FreeLibraryAndExitThread(hMod, 0);
    return 0;
}






BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        HANDLE tHndl = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)InternalMain, hModule, 0, 0);
        if (tHndl) CloseHandle(tHndl);
        else return FALSE;
        break;
    }
    return TRUE;
}

