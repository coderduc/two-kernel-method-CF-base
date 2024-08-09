#include "includes.h"
#include "Utils.h"

template <typename str_type, typename str_type_2>
__forceinline bool crt_strcmp( str_type str, str_type_2 in_str, bool two )
{
#define to_lower(c_char) ((c_char >= 'A' && c_char <= 'Z') ? (c_char + 32) : c_char)

	if ( !str || !in_str )
		return false;

	wchar_t c1, c2;
	do
	{
		c1 = *str++; c2 = *in_str++;
		c1 = to_lower( c1 ); c2 = to_lower( c2 );

		if ( !c1 && ( two ? !c2 : 1 ) )
			return true;

	} while ( c1 == c2 );

	return false;
}

void* Utils::GetSystemInformation( SYSTEM_INFORMATION_CLASS information_class ) {
	unsigned long size = 32;
	char buffer [ 32 ];

	ZwQuerySystemInformation( information_class, buffer, size, &size );

	void* info = ExAllocatePoolZero( NonPagedPool, size, 7265746172 );

	if ( !info )
		return nullptr;

	if ( !NT_SUCCESS( ZwQuerySystemInformation( information_class, info, size, &size ) ) )
	{
		ExFreePool( info );
		return nullptr;
	}

	return info;
}

uintptr_t Utils::GetKernelModule( const char* name ) {
	const auto to_lower = []( char* string ) -> const char*
		{
			for ( char* pointer = string; *pointer != '\0'; ++pointer )
			{
				*pointer = ( char ) ( short ) tolower( *pointer );
			}

			return string;
		};

	const PRTL_PROCESS_MODULES info = ( PRTL_PROCESS_MODULES ) GetSystemInformation( SystemModuleInformation );

	if ( !info )
		return NULL;

	for ( size_t i = 0; i < info->NumberOfModules; ++i )
	{
		const auto& mod = info->Modules [ i ];

		if ( strcmp( to_lower_c( ( char* ) mod.FullPathName + mod.OffsetToFileName ), name ) == 0 || strcmp( to_lower_c( ( char* ) mod.FullPathName ), name ) == 0 )
		{
			const void* address = mod.ImageBase;
			ExFreePool( info );
			return ( uintptr_t ) address;
		}
	}

	ExFreePool( info );
	return NULL;
}

uintptr_t Utils::FindPattern( uintptr_t base, size_t range, const char* pattern, const char* mask ) {
	const auto check_mask = []( const char* base, const char* pattern, const char* mask ) -> bool
		{
			for ( ; *mask; ++base, ++pattern, ++mask )
			{
				if ( *mask == 'x' && *base != *pattern )
				{
					return false;
				}
			}

			return true;
		};

	range = range - strlen( mask );

	for ( size_t i = 0; i < range; ++i )
	{
		if ( check_mask( ( const char* ) base + i, pattern, mask ) )
		{
			return base + i;
		}
	}

	return NULL;
}

uintptr_t Utils::FindPattern( uintptr_t base, const char* pattern, const char* mask ) {
	const PIMAGE_NT_HEADERS headers = ( PIMAGE_NT_HEADERS ) ( base + ( ( PIMAGE_DOS_HEADER ) base )->e_lfanew );
	const PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION( headers );

	for ( size_t i = 0; i < headers->FileHeader.NumberOfSections; i++ )
	{
		const PIMAGE_SECTION_HEADER section = &sections [ i ];

		if ( section->Characteristics & IMAGE_SCN_MEM_EXECUTE )
		{
			const auto match = FindPattern( base + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask );

			if ( match )
			{
				return match;
			}
		}
	}

	return 0;
}

uintptr_t Utils::CalcRelative( uintptr_t current, uint32_t relative ) {
	return current + *( uint32_t* ) ( current ) +relative;
}

uintptr_t Utils::SwapProcess( uintptr_t NewProcess ) {
	auto CurrentThread = ( uintptr_t ) KeGetCurrentThread( );

	auto APCState = *( uintptr_t* ) ( CurrentThread + 0x98 );
	auto OldProcess = *( uintptr_t* ) ( APCState + 0x20 );
	*( uintptr_t* ) ( APCState + 0x20 ) = NewProcess;

	auto dir_table_base = *( uintptr_t* ) ( NewProcess + 0x28 );
	__writecr3( dir_table_base );

	return OldProcess;
}

UNICODE_STRING Utils::AnsiToUnicode( const char* str )
{
	UNICODE_STRING unicode;
	ANSI_STRING ansi_str;

	RtlInitAnsiString( &ansi_str, str );
	RtlAnsiStringToUnicodeString( &unicode, &ansi_str, TRUE );

	return unicode;
}

PKLDR_DATA_TABLE_ENTRY Utils::GetLdrDataByName( const char* szmodule )
{
	PKLDR_DATA_TABLE_ENTRY ldr_entry = nullptr;
	UNICODE_STRING mod = AnsiToUnicode( szmodule );

	PLIST_ENTRY ps_loaded_module_list = PsLoadedModuleList;
	if ( !ps_loaded_module_list )
		return ldr_entry;

	auto current_ldr_entry = reinterpret_cast< PKLDR_DATA_TABLE_ENTRY >( ps_loaded_module_list->Flink );

	while ( reinterpret_cast< PLIST_ENTRY >( current_ldr_entry ) != ps_loaded_module_list )
	{
		if ( !RtlCompareUnicodeString( &current_ldr_entry->BaseDllName, &mod, TRUE ) )
		{
			ldr_entry = current_ldr_entry;
			break;
		}

		current_ldr_entry = reinterpret_cast< PKLDR_DATA_TABLE_ENTRY >( current_ldr_entry->InLoadOrderLinks.Flink );
	}

	return ldr_entry;
}

PIMAGE_SECTION_HEADER Utils::GetSectionHeader( const uintptr_t image_base, const char* section_name )
{
	if ( !image_base || !section_name )
		return nullptr;

	const auto pimage_dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( image_base );
	const auto pimage_nt_headers = reinterpret_cast< PIMAGE_NT_HEADERS64 >( image_base + pimage_dos_header->e_lfanew );

	auto psection = reinterpret_cast< PIMAGE_SECTION_HEADER >( pimage_nt_headers + 1 );

	PIMAGE_SECTION_HEADER psection_hdr = nullptr;

	const auto number_of_sections = pimage_nt_headers->FileHeader.NumberOfSections;

	for ( auto i = 0; i < number_of_sections; ++i )
	{
		if ( crt_strcmp( reinterpret_cast< const char* >( psection->Name ), section_name, false ) )
		{
			psection_hdr = psection;
			break;
		}

		++psection;
	}

	return psection_hdr;
}

uintptr_t Utils::FindPatternPageKernelMode( const char* szmodule, const char* szsection, const char* bmask, const char* szmask )
{
	if ( !szmodule || !szsection || !bmask || !szmask )
		return 0;

	const auto* pldr_entry = GetLdrDataByName( szmodule );

	if ( !pldr_entry )
		return 0;

	const auto  module_base = reinterpret_cast< uintptr_t >( pldr_entry->DllBase );
	const auto* psection = GetSectionHeader( reinterpret_cast< uintptr_t >( pldr_entry->DllBase ), szsection );

	return psection ? FindPattern( module_base + psection->VirtualAddress, psection->Misc.VirtualSize, bmask, szmask ) : 0;
}

uintptr_t Utils::GetModuleHandle( uintptr_t pid, LPCWSTR module_name )
{
	PEPROCESS target_proc;
	uintptr_t base = 0;
	if ( !NT_SUCCESS( PsLookupProcessByProcessId( ( HANDLE ) pid, &target_proc ) ) )
		return 0;

	const auto o_process = SwapProcess( ( uintptr_t ) target_proc );

	PPEB peb = PsGetProcessPeb( target_proc );
	if ( !peb )
		goto end;

	if ( !peb->Ldr || !peb->Ldr->Initialized )
		goto end;

	UNICODE_STRING module_name_unicode;
	RtlInitUnicodeString( &module_name_unicode, module_name );
	for ( PLIST_ENTRY list = peb->Ldr->InLoadOrderModuleList.Flink;
		list != &peb->Ldr->InLoadOrderModuleList;
		list = list->Flink ) {
		PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD( list, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks );
		if ( RtlCompareUnicodeString( &entry->BaseDllName, &module_name_unicode, TRUE ) == 0 ) {
			base = ( uintptr_t ) entry->DllBase;
			goto end;
		}
	}

end:
	SwapProcess( ( uintptr_t ) o_process );
	ObDereferenceObject( target_proc );
	return base;
}

BOOLEAN Utils::bDataCompare( const BYTE* pData, const BYTE* bMask, const char* szMask )
{
	for ( ; *szMask; ++szMask, ++pData, ++bMask )
		if ( *szMask == 'x' && *pData != *bMask )
			return 0;

	return ( *szMask ) == 0;
}

UINT64 Utils::FindPattern( UINT64 dwAddress, UINT64 dwLen, BYTE* bMask, char* szMask )
{
	for ( UINT64 i = 0; i < dwLen; i++ )
		if ( bDataCompare( ( BYTE* ) ( dwAddress + i ), bMask, szMask ) )
			return ( UINT64 ) ( dwAddress + i );

	return 0;
}

PVOID Utils::ResolveRelativeAddress( PVOID Instruction, ULONG OffsetOffset, ULONG InstructionSize )
{
	ULONG_PTR Instr = ( ULONG_PTR ) Instruction;
	LONG RipOffset = *( PLONG ) ( Instr + OffsetOffset );
	PVOID ResolvedAddr = ( PVOID ) ( Instr + InstructionSize + RipOffset );
	return ResolvedAddr;
}

NTSTATUS Utils::PatternScan( IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, IN const VOID* base, IN ULONG_PTR size, OUT PVOID* ppFound )
{
	ASSERT( ppFound != NULL && pattern != NULL && base != NULL );
	if ( ppFound == NULL || pattern == NULL || base == NULL ) return STATUS_INVALID_PARAMETER;

	for ( ULONG_PTR i = 0; i < size - len; i++ )
	{
		BOOLEAN found = TRUE;
		for ( ULONG_PTR j = 0; j < len; j++ )
		{
			if ( pattern [ j ] != wildcard && pattern [ j ] != ( ( PCUCHAR ) base ) [ i + j ] )
			{
				found = FALSE;
				break;
			}
		}
		if ( found != FALSE )
		{
			*ppFound = ( PUCHAR ) base + i;
			return STATUS_SUCCESS;
		}
	}

	return STATUS_NOT_FOUND;
}

NTSTATUS Utils::ScanSection( IN PCCHAR section, IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, OUT PVOID* ppFound )
{
	ASSERT( ppFound != NULL );
	if ( ppFound == NULL ) return STATUS_INVALID_PARAMETER;

	PVOID base = ( PVOID ) ntoskrnlBase;
	if ( !base ) return STATUS_NOT_FOUND;

	PIMAGE_NT_HEADERS64 pHdr = RtlImageNtHeader( base );
	if ( !pHdr ) return STATUS_INVALID_IMAGE_FORMAT;

	PIMAGE_SECTION_HEADER pFirstSection = ( PIMAGE_SECTION_HEADER ) ( pHdr + 1 );
	for ( PIMAGE_SECTION_HEADER pSection = pFirstSection; pSection < pFirstSection + pHdr->FileHeader.NumberOfSections; pSection++ )
	{
		ANSI_STRING s1, s2;
		RtlInitAnsiString( &s1, section );
		RtlInitAnsiString( &s2, ( PCCHAR ) pSection->Name );
		if ( RtlCompareString( &s1, &s2, TRUE ) == 0 )
		{
			PVOID ptr = NULL;
			NTSTATUS status = PatternScan( pattern, wildcard, len, ( PUCHAR ) base + pSection->VirtualAddress, pSection->Misc.VirtualSize, &ptr );
			if ( NT_SUCCESS( status ) ) *( PULONG ) ppFound = ( ULONG ) ( ( PUCHAR ) ptr - ( PUCHAR ) base );
			return status;
		}
	}
	return STATUS_NOT_FOUND;
}

DWORD Utils::GetHandleTableOffset( )
{
	RTL_OSVERSIONINFOW ver = { 0 };
	RtlGetVersion( &ver );

	switch ( ver.dwBuildNumber )
	{
	case WINDOWS_1803:
		return 0x418;
		break;
	case WINDOWS_1809:
		return 0x418;
		break;
	case WINDOWS_1903:
		return 0x418;
		break;
	case WINDOWS_1909:
		return 0x418;
		break;
	case WINDOWS_2004:
		return 0x570;
		break;
	case WINDOWS_20H2:
		return 0x570;
		break;
	case WINDOWS_21H1:
		return 0x570;
		break;
	default:
		return 0x570;
	}
}

bool Utils::KernelCopy( void* dst, void* src, size_t size ) {
	SIZE_T bytes = 0;

	if ( MmCopyVirtualMemory( IoGetCurrentProcess( ), src, IoGetCurrentProcess( ), dst, size, KernelMode, &bytes ) == STATUS_SUCCESS && bytes == size ) {
		return true;
	}

	return false;
}

PVOID Utils::get_sys_routine_address(PCWSTR routine_name) {
	UNICODE_STRING name;
	RtlInitUnicodeString(&name, routine_name);
	return MmGetSystemRoutineAddress(&name);
}

PVOID Utils::get_sys_module_export(LPCWSTR module_name, LPCSTR routine_name) {
	PLIST_ENTRY module_list = reinterpret_cast<PLIST_ENTRY>(get_sys_routine_address(L"PsLoadedModuleList"));

	if (!module_list) {
		return NULL;
	}

	for (PLIST_ENTRY link = module_list; link != module_list->Blink; link = link->Flink) {
		LDR_DATA_TABLE_ENTRY* entry = CONTAINING_RECORD(link, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

		UNICODE_STRING name;
		RtlInitUnicodeString(&name, module_name);
		if (RtlEqualUnicodeString(&entry->BaseDllName, &name, TRUE)) {
			return (entry->DllBase) ? RtlFindExportedRoutineByName(entry->DllBase, routine_name) : NULL;
		}
	}
}

INT Utils::FrameRect(HDC hDC, CONST RECT* lprc, HBRUSH hbr, int thickness) {
	HBRUSH oldbrush;
	RECT r = *lprc;

	if (!(oldbrush = NtGdiSelectBrush(hDC, hbr))) return 0;
	NtGdiPatBlt(hDC, r.left, r.top, thickness, r.bottom - r.top, PATCOPY);
	NtGdiPatBlt(hDC, r.right - thickness, r.top, thickness, r.bottom - r.top, PATCOPY);
	NtGdiPatBlt(hDC, r.left, r.top, r.right - r.left, thickness, PATCOPY);
	NtGdiPatBlt(hDC, r.left, r.bottom - thickness, r.right - r.left, thickness, PATCOPY);

	NtGdiSelectBrush(hDC, oldbrush);
	return TRUE;
}

bool Utils::draw_box(HWND hWnd, int x, int y, int w, int h, int t, int r, int g, int b)
{
	HDC hdc = NtUserGetDC(hWnd);
	if (!hdc)
		return STATUS_UNSUCCESSFUL;

	HBRUSH brush = NtGdiCreateSolidBrush(RGB(r, g, b), NULL);
	if (!brush)
		return STATUS_UNSUCCESSFUL;

	RECT rect = { x, y, x + w, y + h };
	FrameRect(hdc, &rect, brush, t);
	NtUserReleaseDC(hdc);
	NtGdiDeleteObjectApp(brush);
}


NTSTATUS Utils::find_process(char* process_name, PEPROCESS* process)
{
	PEPROCESS sys_process = PsInitialSystemProcess;
	PEPROCESS curr_entry = sys_process;

	char image_name[15];

	do {
		RtlCopyMemory((PVOID)(&image_name), (PVOID)((uintptr_t)curr_entry + 0x5a8), sizeof(image_name));

		if (strstr(image_name, process_name)) {
			DWORD active_threads;
			RtlCopyMemory((PVOID)&active_threads, (PVOID)((uintptr_t)curr_entry + 0x5f0), sizeof(active_threads));
			if (active_threads) {
				*process = curr_entry;
				return STATUS_SUCCESS;
			}
		}

		PLIST_ENTRY list = (PLIST_ENTRY)((uintptr_t)(curr_entry)+0x448);
		curr_entry = (PEPROCESS)((uintptr_t)list->Flink - 0x448);

	} while (curr_entry != sys_process);

	return STATUS_NOT_FOUND;
}

