#include "imports.h"
#include "functions.h"

auto driver_entry() -> const NTSTATUS
{
    auto win32k = utils::get_kernel_module(xorstr_("win32k.sys"));
    if (!win32k) {
        return STATUS_FAILED_DRIVER_ENTRY;
    }
    //22H2 - 19045.3930  ==> 0x674E8
    //22H2 - 19045.4717  ==> 0x67538
    globals::hook_address = win32k + 0x4A0DC;
    globals::hook_pointer = *reinterpret_cast<uintptr_t*>(globals::hook_address);
    *reinterpret_cast<uintptr_t*>(globals::hook_address) = reinterpret_cast<uintptr_t>(&hooked_function);

    return STATUS_SUCCESS;
}