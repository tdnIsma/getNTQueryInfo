#include "includes.h"


PEB* helpers::getPeb()
{
#ifdef _WIN64
    PEB* peb = (PEB*)__readgsqword(0x60);

#else
    PEB* peb = (PEB*)__readfsdword(0x30);
#endif

    return peb;
}


LDR_DATA_TABLE_ENTRY* helpers::getLdr(const wchar_t* modName)
{
    LDR_DATA_TABLE_ENTRY* modEntry = nullptr;

    PEB* peb = helpers::getPeb();

    LIST_ENTRY head = peb->Ldr->InMemoryOrderModuleList;

    LIST_ENTRY curr = head;

    for (auto curr = head; curr.Flink != &peb->Ldr->InMemoryOrderModuleList; curr = *curr.Flink)
    {
        LDR_DATA_TABLE_ENTRY* mod = (LDR_DATA_TABLE_ENTRY*)CONTAINING_RECORD(curr.Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        if (mod->BaseDllName.Buffer)
        {
            if (_wcsicmp(modName, mod->BaseDllName.Buffer) == 0)
            {
                modEntry = mod;
                break;
            }
        }
    }
    return modEntry;
}


uintptr_t helpers::getModuleHandle(const wchar_t* modName)
{
    LDR_DATA_TABLE_ENTRY* mod = helpers::getLdr(modName);

    return (uintptr_t)mod->DllBase;

}


uintptr_t helpers::getModule(const char* module_name, const char* export_name)
{
    if (!module_name || !export_name)
        return 0;

    auto module_base = reinterpret_cast<uintptr_t>(GetModuleHandleA(module_name));

    if (!module_base)
        return 0;

    auto pimage_dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_base);

    if (pimage_dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        return 0;

    auto pimage_nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(module_base + pimage_dos_header->e_lfanew);

    if (pimage_nt_headers->Signature != IMAGE_NT_SIGNATURE)
        return 0;

    auto export_directory_rva = pimage_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

    if (!export_directory_rva)
        return 0;

    auto pimage_export_directory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(module_base + export_directory_rva);
    auto peat = reinterpret_cast<PDWORD>(module_base + pimage_export_directory->AddressOfFunctions);
    auto pent = reinterpret_cast<PDWORD>(module_base + pimage_export_directory->AddressOfNames);
    auto peot = reinterpret_cast<PWORD>(module_base + pimage_export_directory->AddressOfNameOrdinals);

    unsigned short ordinal = 0;

    for (DWORD i = 0; i < pimage_export_directory->NumberOfNames; ++i)
    {
        if (!lstrcmpiA(export_name, reinterpret_cast<const char*>(module_base + pent[i])))
        {
            ordinal = peot[i];
            break;
        }
    }
    return ordinal ? module_base + peat[ordinal] : 0;
}


void helpers::PatchInternal(BYTE* dst, BYTE* src, size_t size) {
    DWORD oldprotect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect); 
    memcpy(dst, src, size); 
    VirtualProtect(dst, size, oldprotect, &oldprotect); 
}



void helpers::PatchisDebuggerPresent()
{

    uintptr_t debuggerAddr = helpers::getModule("kernelbase.dll", "isDebuggerPresent");
    helpers::PatchInternal((BYTE*)debuggerAddr, (BYTE*)"\xB8\x00\x00\x00\x00\x90\x90\x90\x90\x90", 10);


}


void helpers::PatchCheckRemoteDebuggerPresent()
{
    uintptr_t debuggerAddr = helpers::getModule("kernelbase.dll", "CheckRemoteDebuggerPresent");
    helpers::PatchInternal((BYTE*)debuggerAddr, (BYTE*)"\xC2\x08\x00", 3);
}