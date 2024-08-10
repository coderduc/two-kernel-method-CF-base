#pragma once
#define ENUM_STRING_CASE(en) case en: return #en;
#define MM_UNLOADED_DRIVERS_SIZE 50
#define WIN_1507 10240
#define WIN_1511 10586
#define WIN_1607 14393
#define WIN_1703 15063
#define WIN_1709 16299
#define WIN_1803 17134
#define WIN_1809 17763
#define WIN_1903 18362
#define WIN_1909 18363
#define WIN_2004 19041
#define WIN_20H2 19042
#define WIN_21H1 19043
#define WIN_21H2 19044
#define WIN_22H2 19045
#define WIN_1121H2 22000
#define WIN_1122H2 22621

class Utils
{
public:
	UINT64 ntoskrnlBase = 0, ntoskrnlSize = 0;

	__int64(__fastcall* orig_NtGdiEngPaint)(void* a1);
	NTSTATUS PatternScan( IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, IN const VOID* base, IN ULONG_PTR size, OUT PVOID* ppFound );
	NTSTATUS ScanSection( IN PCCHAR section, IN PCUCHAR pattern, IN UCHAR wildcard, IN ULONG_PTR len, OUT PVOID* ppFound );
	BOOLEAN bDataCompare( const BYTE* pData, const BYTE* bMask, const char* szMask );
	UINT64 FindPattern( UINT64 dwAddress, UINT64 dwLen, BYTE* bMask, char* szMask );
	uintptr_t FindPattern( uintptr_t base, size_t range, const char* pattern, const char* mask );
	uintptr_t FindPattern( uintptr_t base, const char* pattern, const char* mask );
	uintptr_t FindPatternPageKernelMode( const char* szmodule, const char* szsection, const char* bmask, const char* szmask );
	uintptr_t CalcRelative( uintptr_t current, uint32_t relative );
	uintptr_t SwapProcess( uintptr_t NewProcess );
	PVOID ResolveRelativeAddress( PVOID Instruction, ULONG OffsetOffset, ULONG InstructionSize );
	bool KernelCopy( void* dst, void* src, size_t size );
	DWORD GetHandleTableOffset( );
	PKLDR_DATA_TABLE_ENTRY GetLdrDataByName( const char* szmodule );
	PIMAGE_SECTION_HEADER GetSectionHeader( const uintptr_t image_base, const char* section_name );
	uintptr_t GetKernelModule( const char* name );
	void* GetSystemInformation( SYSTEM_INFORMATION_CLASS information_class );
	uintptr_t GetModuleHandle( uintptr_t pid, LPCWSTR module_name );
	UNICODE_STRING AnsiToUnicode( const char* str );
	PVOID get_sys_routine_address(PCWSTR routine_name);
	PVOID get_sys_module_export(LPCWSTR module_name, LPCSTR routine_name);
	INT FrameRect(HDC hDC, CONST RECT* lprc, HBRUSH hbr, int thickness);
	bool draw_box(HWND hWnd, int x, int y, int w, int h, int t, int r, int g, int b);
	NTSTATUS find_process(char* process_name, PEPROCESS* process);
};
inline Utils pUtils;