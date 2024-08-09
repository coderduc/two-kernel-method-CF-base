#pragma once
#define WINDOWS_1803 17134
#define WINDOWS_1809 17763
#define WINDOWS_1903 18362
#define WINDOWS_1909 18363
#define WINDOWS_2004 19041
#define WINDOWS_20H2 19569
#define WINDOWS_21H1 20180
#define PAGE_OFFSET_SIZE 12
static const uint64_t mask = (~0xfull << 8) & 0xfffffffffull;

class PhysMemory
{
public:
    DWORD GetWindowsOffset();
    ULONG_PTR GetProcessDirBase(PEPROCESS targetprocess);
    NTSTATUS ReadPhysAddress(PVOID address, PVOID buffer, SIZE_T size, SIZE_T* read);
    NTSTATUS WritePhysAddress(PVOID address, PVOID buffer, SIZE_T size, SIZE_T* written);
    uint64_t translateaddress(uint64_t processdirbase, uint64_t address);
    NTSTATUS ReadProcessMemory(PEPROCESS process, PVOID address, PVOID buffer, SIZE_T size, SIZE_T* read);
    NTSTATUS WriteProcessMemory(PEPROCESS process, PVOID address, PVOID buffer, SIZE_T size, SIZE_T* written);
};
inline PhysMemory pPhysMemory;