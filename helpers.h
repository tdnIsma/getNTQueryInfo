#pragma once


namespace helpers {

    uintptr_t getModule(const char* module_name, const char* export_name);
    uintptr_t getModuleHandle(const wchar_t* modName);
    LDR_DATA_TABLE_ENTRY* getLdr(const wchar_t* modName);
    PEB* getPeb();



    void PatchInternal(BYTE* dst, BYTE* src, size_t size);
    void PatchisDebuggerPresent();
    void PatchCheckRemoteDebuggerPresent();


}
