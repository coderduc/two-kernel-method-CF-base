#include "includes.h"
#include "PhysicalMemory.h"

DWORD PhysMemory::GetWindowsOffset()
{
    RTL_OSVERSIONINFOW ver = { 0 };
    RtlGetVersion(&ver);

    switch (ver.dwBuildNumber)
    {
    case WINDOWS_1803:
        return 0x0278;
        break;
    case WINDOWS_1809:
        return 0x0278;
        break;
    case WINDOWS_1903:
        return 0x0280;
        break;
    case WINDOWS_1909:
        return 0x0280;
        break;
    case WINDOWS_2004:
        return 0x0388;
        break;
    case WINDOWS_20H2:
        return 0x0388;
        break;
    case WINDOWS_21H1:
        return 0x0388;
        break;
    default:
        return 0x0388;
    }
}

ULONG_PTR PhysMemory::GetProcessDirBase(PEPROCESS targetprocess)
{
    if (!targetprocess)
        return 0;

    PUCHAR process = (PUCHAR)targetprocess;
    ULONG_PTR process_dirbase = *(PULONG_PTR)(process + 0x28);
    if (process_dirbase == 0) {
        auto userdiroffset = GetWindowsOffset();
        ULONG_PTR process_userdirbase = *(PULONG_PTR)(process + userdiroffset);
        return process_userdirbase;
    }

    return process_dirbase;
}

NTSTATUS PhysMemory::ReadPhysAddress(PVOID address, PVOID buffer, SIZE_T size, SIZE_T* read) {
    if (!address)
        return STATUS_UNSUCCESSFUL;

    MM_COPY_ADDRESS addr = { 0 };
    addr.PhysicalAddress.QuadPart = (LONGLONG)address;
    return MmCopyMemory(buffer, addr, size, MM_COPY_MEMORY_PHYSICAL, read);
}

NTSTATUS PhysMemory::WritePhysAddress(PVOID address, PVOID buffer, SIZE_T size, SIZE_T* written)
{
    if (!address)
        return STATUS_UNSUCCESSFUL;

    PHYSICAL_ADDRESS addr = { 0 };
    addr.QuadPart = (LONGLONG)address;

    auto mapped_mem = MmMapIoSpaceEx(addr, size, PAGE_READWRITE);

    if (!mapped_mem)
        return STATUS_UNSUCCESSFUL;

    memcpy(mapped_mem, buffer, size);

    *written = size;
    MmUnmapIoSpace(mapped_mem, size);
    return STATUS_SUCCESS;
}

uint64_t PhysMemory::translateaddress(uint64_t processdirbase, uint64_t address) {
    processdirbase &= ~0xf;

    uint64_t pageoffset = address & ~(~0ul << PAGE_OFFSET_SIZE);
    uint64_t pte = ((address >> 12) & (0x1ffll));
    uint64_t pt = ((address >> 21) & (0x1ffll));
    uint64_t pd = ((address >> 30) & (0x1ffll));
    uint64_t pdp = ((address >> 39) & (0x1ffll));

    SIZE_T readsize = 0;
    uint64_t pdpe = 0;
    ReadPhysAddress((void*)(processdirbase + 8 * pdp), &pdpe, sizeof(pdpe), &readsize);
    if (~pdpe & 1)
        return 0;

    uint64_t pde = 0;
    ReadPhysAddress((void*)((pdpe & mask) + 8 * pd), &pde, sizeof(pde), &readsize);
    if (~pde & 1)
        return 0;

    if (pde & 0x80)
        return (pde & (~0ull << 42 >> 12)) + (address & ~(~0ull << 30));

    uint64_t ptraddr = 0;
    ReadPhysAddress((void*)((pde & mask) + 8 * pt), &ptraddr, sizeof(ptraddr), &readsize);
    if (~ptraddr & 1)
        return 0;

    if (ptraddr & 0x80)
        return (ptraddr & mask) + (address & ~(~0ull << 21));

    address = 0;
    ReadPhysAddress((void*)((ptraddr & mask) + 8 * pte), &address, sizeof(address), &readsize);
    address &= mask;

    if (!address)
        return 0;

    return address + pageoffset;
}

NTSTATUS PhysMemory::ReadProcessMemory(PEPROCESS process, PVOID address, PVOID buffer, SIZE_T size, SIZE_T* read)
{
    auto process_dirbase = GetProcessDirBase(process);

    SIZE_T curoffset = 0;
    while (size)
    {
        auto addr = translateaddress(process_dirbase, (ULONG64)address + curoffset);
        if (!addr) return STATUS_UNSUCCESSFUL;

        ULONG64 readsize = min(PAGE_SIZE - (addr & 0xFFF), size);
        SIZE_T readreturn = 0;
        auto readstatus = ReadPhysAddress((void*)addr, (PVOID)((ULONG64)buffer + curoffset), readsize, &readreturn);
        size -= readreturn;
        curoffset += readreturn;
        if (readstatus != STATUS_SUCCESS) break;
        if (readreturn == 0) break;
    }

    *read = curoffset;
    return STATUS_SUCCESS;
}

NTSTATUS PhysMemory::WriteProcessMemory(PEPROCESS process, PVOID address, PVOID buffer, SIZE_T size, SIZE_T* written)
{
    auto process_dirbase = GetProcessDirBase(process);

    SIZE_T curoffset = 0;
    while (size)
    {
        auto addr = translateaddress(process_dirbase, (ULONG64)address + curoffset);
        if (!addr) return STATUS_UNSUCCESSFUL;

        ULONG64 writesize = min(PAGE_SIZE - (addr & 0xFFF), size);
        SIZE_T written = 0;
        auto writestatus = WritePhysAddress((void*)addr, (PVOID)((ULONG64)buffer + curoffset), writesize, &written);
        size -= written;
        curoffset += written;
        if (writestatus != STATUS_SUCCESS) break;
        if (written == 0) break;
    }

    *written = curoffset;
    return STATUS_SUCCESS;
}