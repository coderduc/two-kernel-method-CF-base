#include "includes.h"

__int64 __fastcall hk_NtCompositionInputThread( void* a1, void* a2, void* a3 )
{
	request_data data { 0 };
	if ( ExGetPreviousMode( ) != UserMode || !pUtils.KernelCopy( &data, a1, sizeof( request_data ) ) || data.unique != request_unique ) {
		return pUtils.orig_NtCompositionInputThread( a1, a2, a3 );
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

	NtGdiSelectBrush = (GdiSelectBrush_t)pUtils.get_sys_module_export(L"win32kfull.sys", "NtGdiSelectBrush");
	//dbg("[+] SysCall: NtGdiSelectBrush module_export: 0x%p \n", NtGdiSelectBrush);
	NtGdiCreateSolidBrush = (NtGdiCreateSolidBrush_t)pUtils.get_sys_module_export(L"win32kfull.sys", "NtGdiCreateSolidBrush");
	//dbg("[+] SysCall: NtGdiCreateSolidBrush module_export: 0x%p \n", NtGdiCreateSolidBrush);
	NtUserGetDC = (NtUserGetDC_t)pUtils.get_sys_module_export(L"win32kbase.sys", "NtUserGetDC");
	//dbg("[+] SysCall: NtUserGetDC module_export: 0x%p \n", NtUserGetDC);
	NtUserReleaseDC = (ReleaseDC_t)pUtils.get_sys_module_export(L"win32kbase.sys", "NtUserReleaseDC");
	//dbg("[+] SysCall: NtUserReleaseDC module_export: 0x%p \n", NtUserReleaseDC);
	NtGdiDeleteObjectApp = (DeleteObjectApp_t)pUtils.get_sys_module_export(L"win32kbase.sys", "NtGdiDeleteObjectApp");
	//dbg("[+] SysCall: NtGdiDeleteObjectApp module_export: 0x%p \n", NtGdiDeleteObjectApp);
	NtGdiPatBlt = (PatBlt_t)pUtils.get_sys_module_export(L"win32kfull.sys", "NtGdiPatBlt");
	//dbg("[+] SysCall: NtGdiPatBlt module_export: 0x%p \n", NtGdiPatBlt);

	NTSTATUS status = STATUS_SUCCESS;
	uintptr_t win32kb = pUtils.GetKernelModule( _X( "win32kbase.sys" ) );
	if ( !win32kb )
		status = STATUS_UNSUCCESSFUL;

	uint64_t NtCompositionInputThread = ( uint64_t ) RtlFindExportedRoutineByName( ( PVOID ) win32kb, _X( "NtCompositionInputThread" ) );
	if ( !NtCompositionInputThread )
		status = STATUS_UNSUCCESSFUL;

	uint64_t NtCompositionInputThreadPtr = NtCompositionInputThread + 0xf;

	const uintptr_t derefrenced_qword = ( uintptr_t ) NtCompositionInputThreadPtr + *( int* ) ( ( BYTE* ) NtCompositionInputThreadPtr + 3 ) + 7;
	if ( !derefrenced_qword )
		status = STATUS_UNSUCCESSFUL;

	*( void** ) &pUtils.orig_NtCompositionInputThread = _InterlockedExchangePointer( ( void** ) derefrenced_qword, ( void* ) hk_NtCompositionInputThread );
	return status;
}