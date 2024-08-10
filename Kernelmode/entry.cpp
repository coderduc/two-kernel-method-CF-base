#include "includes.h"

#define NT_QWORD_SIG _X("\x48\x8B\x00\x00\x00\x00\x00\x48\x85\xC0\x74\x00\x44\x8B\x54\x24\x60\x44\x89\x54\x24\x20\xFF\x15\x08\xAF\x06\x00")
#define NT_QWORD_MASK _X("xx?????xxxx?xxxxxxxxxxxxxxxx")

__int64 __fastcall hkNtGdiEngPaint(void* a1)
{
	request_data data { 0 };
	if ( ExGetPreviousMode( ) != UserMode || !pUtils.KernelCopy( &data, a1, sizeof( request_data ) ) || data.unique != request_unique ) {
		return pUtils.orig_NtGdiEngPaint( a1 );
	}

	const auto request = reinterpret_cast< request_data* >( a1 );
	switch ( request->code )
	{
	case request_drv:
	{
		drv_request data { 0 };

		if ( !pUtils.KernelCopy( &data, request->data, sizeof( drv_request ) ) ) {
			return STATUS_UNSUCCESSFUL;
		}

		reinterpret_cast< drv_request* >( request->data )->secret_code = 0x1337;

		return request_success;
	}

	case request_read:
	{
		read_request data { 0 };
		NTSTATUS Status = STATUS_UNSUCCESSFUL;
		PEPROCESS process;

		if ( pUtils.KernelCopy( &data, request->data, sizeof( read_request ) ) )
		{
			if ( data.address && data.pid && data.buffer && data.size )
			{
				Status = PsLookupProcessByProcessId( ( HANDLE ) data.pid, &process );
				if ( Status == STATUS_SUCCESS )
				{
					size_t bytes = 0;

					if ( data.mmcopy )
					{
						Status = MmCopyVirtualMemory( process, ( void* ) data.address, PsGetCurrentProcess( ), ( void* ) data.buffer, data.size, UserMode, &bytes );
					}
					else
					{
						Status = pPhysMemory.ReadProcessMemory( process, ( void* ) data.address, ( void* ) data.buffer, data.size, &bytes );
					}

					if ( Status == STATUS_SUCCESS )
					{
						reinterpret_cast< pread_request >( request->data )->ret_size = bytes;
					}

					ObDereferenceObject( process );
				}
			}
			else
			{
				Status = STATUS_INVALID_PARAMETER_2;
			}
		}
		else
		{
			Status = STATUS_INVALID_PARAMETER_1;
		}

		request->status = Status;

		return request_success;
	}

	case request_write:
	{
		write_request data { 0 };

		if ( !pUtils.KernelCopy( &data, request->data, sizeof( write_request ) ) ) {
			return STATUS_UNSUCCESSFUL;
		}

		if ( !data.address || !data.pid || !data.buffer || !data.size ) {
			return STATUS_UNSUCCESSFUL;
		}

		PEPROCESS process;
		if ( PsLookupProcessByProcessId( ( HANDLE ) data.pid, &process ) == STATUS_SUCCESS ) {
			size_t bytes = 0;
			if ( pPhysMemory.WriteProcessMemory( process, ( void* ) data.address, ( void* ) data.buffer, data.size, &bytes ) != STATUS_SUCCESS || bytes != data.size ) {
				ObDereferenceObject( process );
				return STATUS_UNSUCCESSFUL;
			}
			ObDereferenceObject( process );
		}
		else {
			return STATUS_UNSUCCESSFUL;
		}

		return request_success;
	}

	case request_procbase:
	{
		proc_request data { 0 };

		if ( !pUtils.KernelCopy( &data, request->data, sizeof( proc_request ) ) ) {
			return STATUS_UNSUCCESSFUL;
		}

		if ( !data.pid ) {
			return STATUS_UNSUCCESSFUL;
		}

		PEPROCESS process;
		data.ProcessBase = -1;

		if ( PsLookupProcessByProcessId( ( HANDLE ) data.pid, &process ) == STATUS_SUCCESS ) {
			reinterpret_cast< proc_request* >( request->data )->ProcessBase = ( uint64_t ) PsGetProcessSectionBaseAddress( process );
			ObfDereferenceObject( process );
		}
		else {
			return STATUS_UNSUCCESSFUL;
		}

		return request_success;
	}

	case request_modbase:
	{
		proc_request data { 0 };

		if ( !pUtils.KernelCopy( &data, request->data, sizeof( proc_request ) ) ) {
			return STATUS_UNSUCCESSFUL;
		}

		if ( !data.pid || !data.lpModName ) {
			return STATUS_UNSUCCESSFUL;
		}

		const auto base = pUtils.GetModuleHandle( data.pid, data.lpModName );
		if ( !base ) {
			return STATUS_UNSUCCESSFUL;
		}

		reinterpret_cast< proc_request* >( request->data )->ModuleBase = base;

		return request_success;
	}

	case request_movemouse:
	{
		k_mousemove data { 0 };
		if ( !pUtils.KernelCopy( &data, request->data, sizeof( k_mousemove ) ) ) {
			return STATUS_UNSUCCESSFUL;
		}

		mouse_move( data.LastX, data.LastY, data.ButtonFlags );

		return request_success;
	}
	case request_drawbox:
	{
		k_draw data { 0 };
		if (!pUtils.KernelCopy(&data, request->data, sizeof(k_draw))) {
			return STATUS_UNSUCCESSFUL;
		}
		pUtils.draw_box(data.hWnd, data.x, data.y, data.w, data.h, data.t, data.r, data.g, data.b);
		
	}
	}

	return 0;
}

NTSTATUS DriverEntry( ) {

	NtGdiSelectBrush = (GdiSelectBrush_t)pUtils.get_sys_module_export(_X(L"win32kfull.sys"), _X("NtGdiSelectBrush"));
	//dbg("[+] SysCall: NtGdiSelectBrush module_export: 0x%p \n", NtGdiSelectBrush);
	NtGdiCreateSolidBrush = (NtGdiCreateSolidBrush_t)pUtils.get_sys_module_export(_X(L"win32kfull.sys"), _X("NtGdiCreateSolidBrush"));
	//dbg("[+] SysCall: NtGdiCreateSolidBrush module_export: 0x%p \n", NtGdiCreateSolidBrush);
	NtUserGetDC = (NtUserGetDC_t)pUtils.get_sys_module_export(_X(L"win32kbase.sys"), _X("NtUserGetDC"));
	//dbg("[+] SysCall: NtUserGetDC module_export: 0x%p \n", NtUserGetDC);
	NtUserReleaseDC = (ReleaseDC_t)pUtils.get_sys_module_export(_X(L"win32kbase.sys"), _X("NtUserReleaseDC"));
	//dbg("[+] SysCall: NtUserReleaseDC module_export: 0x%p \n", NtUserReleaseDC);
	NtGdiDeleteObjectApp = (DeleteObjectApp_t)pUtils.get_sys_module_export(_X(L"win32kbase.sys"), _X("NtGdiDeleteObjectApp"));
	//dbg("[+] SysCall: NtGdiDeleteObjectApp module_export: 0x%p \n", NtGdiDeleteObjectApp);
	NtGdiPatBlt = (PatBlt_t)pUtils.get_sys_module_export(_X(L"win32kfull.sys"), _X("NtGdiPatBlt"));
	//dbg("[+] SysCall: NtGdiPatBlt module_export: 0x%p \n", NtGdiPatBlt);

	uintptr_t win32kb = pUtils.GetKernelModule( _X( "win32k.sys" ) );
	uintptr_t nt_qword{};

	if (win32kb) {
		nt_qword = pUtils.FindPattern(win32kb, NT_QWORD_SIG, NT_QWORD_MASK);
	}else {	
		printf("[-] win32k.sys not found");
		return STATUS_UNSUCCESSFUL;
	}

	PEPROCESS process_target{};

	if (pUtils.find_process(_X("explorer.exe"), &process_target) == STATUS_SUCCESS && process_target) {
		const uintptr_t nt_qword_deref = (uintptr_t)nt_qword + *(int*)((BYTE*)nt_qword + 3) + 7;

		printf("[+] *nt_qword @ 0x%p", nt_qword_deref);

		KeAttachProcess(process_target);
		*(void**)&pUtils.orig_NtGdiEngPaint = _InterlockedExchangePointer((void**)nt_qword_deref, (void*)hkNtGdiEngPaint);
		KeDetachProcess();
	}
	else {
		printf("[-] Can't find explorer.exe");
		return STATUS_UNSUCCESSFUL;
	}
	printf("[+] Driver loaded");
	return STATUS_SUCCESS;
}