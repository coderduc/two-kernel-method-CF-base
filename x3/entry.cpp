#include "imports.h"
#include "functions.h"

auto driver_entry() -> const NTSTATUS
{

    auto win32k = utils::get_kernel_module(xorstr_("win32k.sys"));
    if (!win32k) {
        return STATUS_FAILED_DRIVER_ENTRY;
    }

    uint64_t NtCompositionInputThread = (uint64_t)RtlFindExportedRoutineByName((PVOID)win32k, xorstr_("NtUserGetPointerProprietaryId"));
    if (!NtCompositionInputThread)
       return STATUS_UNSUCCESSFUL;

    uint64_t NtCompositionInputThreadPtr = NtCompositionInputThread + 0xf;

    const uintptr_t derefrenced_qword = (uintptr_t)NtCompositionInputThreadPtr + *(int*)((BYTE*)NtCompositionInputThreadPtr + 3) + 7;
    if (!derefrenced_qword)
       return  STATUS_UNSUCCESSFUL;

    globals::hook_address = derefrenced_qword;/*win32k + 0x674E8;*/ // change this offset to whatever you want to hook to

    globals::hook_pointer = *reinterpret_cast<uintptr_t*>(globals::hook_address);
    *reinterpret_cast<uintptr_t*>(globals::hook_address) = reinterpret_cast<uintptr_t>(&hooked_function);


    return STATUS_SUCCESS;
}