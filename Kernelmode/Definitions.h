#pragma once
#pragma warning(disable : 4201)
#define PFN_TO_PAGE(pfn) ( pfn << 12 )
#define dereference(ptr) (const uintptr_t)(ptr + *( int * )( ( BYTE * )ptr + 3 ) + 7)
#define in_range(x,a,b)    (x >= a && x <= b) 
#define get_bits( x )    (in_range((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xA) : (in_range(x,'0','9') ? x - '0' : 0))
#define get_byte( x )    (get_bits(x[0]) << 4 | get_bits(x[1]))
#define size_align(Size) ((Size + 0xFFF) & 0xFFFFFFFFFFFFF000)
#define to_lower_i(Char) ((Char >= 'A' && Char <= 'Z') ? (Char + 32) : Char)
#define to_lower_c(Char) ((Char >= (char*)'A' && Char <= (char*)'Z') ? (Char + 32) : Char)
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define PATCOPY             (DWORD)0x00F00021
#define RELATIVE_ADDR(addr, size) ((PVOID)((PBYTE)(addr) + *(PINT)((PBYTE)(addr) + ((size) - (INT)sizeof(INT))) + (size)))
#define KeMRaiseIrql(a, b) *(b) = KfRaiseIrql(a)

typedef HBRUSH(*GdiSelectBrush_t)(_In_ HDC hdc, _In_ HBRUSH  hbr);
typedef HDC(*NtUserGetDC_t)(HWND hWnd);
typedef int(*ReleaseDC_t)(HDC hDC);
typedef BOOL(*DeleteObjectApp_t)(HANDLE hobj);
typedef BOOL(*PatBlt_t)(_In_ HDC, _In_ int x, _In_ int y, _In_ int w, _In_ int h, _In_ DWORD);
typedef HBRUSH(*NtGdiCreateSolidBrush_t)(_In_ COLORREF crColor, _In_opt_ HBRUSH hbr);

__declspec(selectany) PatBlt_t NtGdiPatBlt = NULL;
__declspec(selectany) GdiSelectBrush_t NtGdiSelectBrush = NULL;
__declspec(selectany) NtGdiCreateSolidBrush_t NtGdiCreateSolidBrush = NULL;
__declspec(selectany) NtUserGetDC_t NtUserGetDC = NULL;
__declspec(selectany) ReleaseDC_t NtUserReleaseDC = NULL;
__declspec(selectany) DeleteObjectApp_t NtGdiDeleteObjectApp = NULL;

typedef unsigned int uint32_t;
typedef int BOOL;
typedef ULONG_PTR QWORD;

typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemNextEventIdInformation,
	SystemEventIdsInformation,
	SystemCrashDumpInformation,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemPlugPlayBusInformation,
	SystemDockInformation,
	SystemProcessorSpeedInformation,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation,
	SystemBigPoolInformation = 0x42
} SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

typedef struct _MM_AVL_NODE // Size=24
{
	struct _MM_AVL_NODE* LeftChild; // Size=8 Offset=0
	struct _MM_AVL_NODE* RightChild; // Size=8 Offset=8

	union ___unnamed1666 // Size=8
	{
		struct
		{
			__int64 Balance : 2; // Size=8 Offset=0 BitOffset=0 BitCount=2
		};
		struct _MM_AVL_NODE* Parent; // Size=8 Offset=0
	} u1;
} MM_AVL_NODE, * PMM_AVL_NODE, * PMMADDRESS_NODE;

typedef struct _RTL_AVL_TREE // Size=8
{
	PMM_AVL_NODE BalancedRoot;
	void* NodeHint;
	unsigned __int64 NumberGenericTableElements;
} RTL_AVL_TREE, * PRTL_AVL_TREE, MM_AVL_TABLE, * PMM_AVL_TABLE;

typedef struct _MMVAD_FLAGS
{
	ULONG Lock : 1;                                                           //0x0
	ULONG LockContended : 1;                                                  //0x0
	ULONG DeleteInProgress : 1;                                               //0x0
	ULONG NoChange : 1;                                                       //0x0
	ULONG VadType : 3;                                                        //0x0
	ULONG Protection : 5;                                                     //0x0
	ULONG PreferredNode : 6;                                                  //0x0
	ULONG PageSize : 2;                                                       //0x0
	ULONG PrivateMemory : 1;                                                  //0x0
} MMVAD_FLAGS, * PMMVAD_FLAGS;

struct _EX_PUSH_LOCK
{
	union
	{
		struct
		{
			ULONGLONG Locked : 1;                                             //0x0
			ULONGLONG Waiting : 1;                                            //0x0
			ULONGLONG Waking : 1;                                             //0x0
			ULONGLONG MultipleShared : 1;                                     //0x0
			ULONGLONG Shared : 60;                                            //0x0
		};
		ULONGLONG Value;                                                    //0x0
		VOID* Ptr;                                                          //0x0
	};
};

typedef struct _MMVAD_SHORT {
	union
	{
		struct
		{
			struct _MMVAD_SHORT* NextVad;                                   //0x0
			VOID* ExtraCreateInfo;                                          //0x8
		};
		struct _RTL_BALANCED_NODE VadNode;                                  //0x0
	};
	ULONG StartingVpn;                                                      //0x18
	ULONG EndingVpn;                                                        //0x1c
	UCHAR StartingVpnHigh;                                                  //0x20
	UCHAR EndingVpnHigh;                                                    //0x21
	UCHAR CommitChargeHigh;                                                 //0x22
	UCHAR SpareNT64VadUChar;                                                //0x23
	LONG ReferenceCount;                                                    //0x24
	struct _EX_PUSH_LOCK PushLock;                                          //0x28
	union
	{
		ULONG LongFlags;                                                    //0x30
		struct _MMVAD_FLAGS VadFlags; // Size=4 Offset=0
	} u;                                                                    //0x30
	union
	{
		ULONG LongFlags1;                                                   //0x34
	} u1;                                                                   //0x34
	struct _MI_VAD_EVENT_BLOCK* EventList;                                  //0x38
} MMVAD, * PMMVAD;

typedef struct PiDDBCacheEntry
{
	LIST_ENTRY		List;
	UNICODE_STRING	DriverName;
	ULONG			TimeDateStamp;
	NTSTATUS		LoadStatus;
	char			_0x0028 [ 16 ]; // data from the shim engine, or uninitialized memory for custom drivers
}PIDCacheobj;

typedef struct _PEB_LDR_DATA
{
	ULONG Length;
	UCHAR Initialized;
	PVOID SsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	LIST_ENTRY ModuleListLoadOrder;
	LIST_ENTRY ModuleListMemoryOrder;
	LIST_ENTRY ModuleListInitOrder;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _MM_UNLOADED_DRIVER
{
	UNICODE_STRING 	Name;
	PVOID 			ModuleStart;
	PVOID 			ModuleEnd;
	ULONG64 		UnloadTime;
} MM_UNLOADED_DRIVER, * PMM_UNLOADED_DRIVER;

typedef struct _PEB
{
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	UCHAR BitField;
	PVOID Mutant;
	PVOID ImageBaseAddress;
	PPEB_LDR_DATA Ldr;
	PVOID ProcessParameters;
	PVOID SubSystemData;
	PVOID ProcessHeap;
	PVOID FastPebLock;
	PVOID AtlThunkSListPtr;
	PVOID IFEOKey;
	PVOID CrossProcessFlags;
	PVOID KernelCallbackTable;
	ULONG SystemReserved;
	ULONG AtlThunkSListPtr32;
	PVOID ApiSetMap;
} PEB, * PPEB;

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _KLDR_DATA_TABLE_ENTRY
{
	/* 0x0000 */ struct _LIST_ENTRY InLoadOrderLinks;
	/* 0x0010 */ void* ExceptionTable;
	/* 0x0018 */ unsigned long ExceptionTableSize;
	/* 0x001c */ long Padding_687;
	/* 0x0020 */ void* GpValue;
	/* 0x0028 */ struct _NON_PAGED_DEBUG_INFO* NonPagedDebugInfo;
	/* 0x0030 */ void* DllBase;
	/* 0x0038 */ void* EntryPoint;
	/* 0x0040 */ unsigned long SizeOfImage;
	/* 0x0044 */ long Padding_688;
	/* 0x0048 */ struct _UNICODE_STRING FullDllName;
	/* 0x0058 */ struct _UNICODE_STRING BaseDllName;
	/* 0x0068 */ unsigned long Flags;
	/* 0x006c */ unsigned short LoadCount;
	union
	{
		union
		{
			struct /* bitfield */
			{
				/* 0x006e */ unsigned short SignatureLevel : 4; /* bit position: 0 */
				/* 0x006e */ unsigned short SignatureType : 3; /* bit position: 4 */
				/* 0x006e */ unsigned short Unused : 9; /* bit position: 7 */
			}; /* bitfield */
			/* 0x006e */ unsigned short EntireField;
		}; /* size: 0x0002 */
	} /* size: 0x0002 */ u1;
	/* 0x0070 */ void* SectionPointer;
	/* 0x0078 */ unsigned long CheckSum;
	/* 0x007c */ unsigned long CoverageSectionSize;
	/* 0x0080 */ void* CoverageSection;
	/* 0x0088 */ void* LoadedImports;
	/* 0x0090 */ void* Spare;
	/* 0x0098 */ unsigned long SizeOfImageNotRounded;
	/* 0x009c */ unsigned long TimeDateStamp;
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY; /* size: 0x00a0 */

//0x38 bytes (sizeof)
typedef struct _OBJECT_HEADER
{
	LONGLONG PointerCount;                                                  //0x0
	union
	{
		LONGLONG HandleCount;                                               //0x8
		VOID* NextToFree;                                                   //0x8
	};
	struct _EX_PUSH_LOCK Lock;                                              //0x10
	UCHAR TypeIndex;                                                        //0x18
	union
	{
		UCHAR TraceFlags;                                                   //0x19
		struct
		{
			UCHAR DbgRefTrace : 1;                                            //0x19
			UCHAR DbgTracePermanent : 1;                                      //0x19
		};
	};
	UCHAR InfoMask;                                                         //0x1a
	union
	{
		UCHAR Flags;                                                        //0x1b
		struct
		{
			UCHAR NewObject : 1;                                              //0x1b
			UCHAR KernelObject : 1;                                           //0x1b
			UCHAR KernelOnlyAccess : 1;                                       //0x1b
			UCHAR ExclusiveObject : 1;                                        //0x1b
			UCHAR PermanentObject : 1;                                        //0x1b
			UCHAR DefaultSecurityQuota : 1;                                   //0x1b
			UCHAR SingleHandleEntry : 1;                                      //0x1b
			UCHAR DeletedInline : 1;                                          //0x1b
		};
	};
	ULONG Reserved;                                                         //0x1c
	union
	{
		struct _OBJECT_CREATE_INFORMATION* ObjectCreateInfo;                //0x20
		VOID* QuotaBlockCharged;                                            //0x20
	};
	VOID* SecurityDescriptor;                                               //0x28
	struct _QUAD Body;                                                      //0x30
}OBJECT_HEADER, * POBJECT_HEADER;

typedef struct _PS_PROTECTION
{
	union
	{
		UCHAR Level;                                                        //0x0
		struct
		{
			UCHAR Type : 3;                                                   //0x0
			UCHAR Audit : 1;                                                  //0x0
			UCHAR Signer : 4;                                                 //0x0
		};
	};
} PS_PROTECTION, * PPS_PROTECTION;

typedef struct _KAFFINITY_EX {
	char Affinity [ 0xA8 ];
} KAFFINITY_EX, * PKAFFINITY_EX;

typedef struct _KSTACK_COUNT {
	ULONG State;
	ULONG StackCount;
} KSTACK_COUNT, * PKSTACK_COUNT;

typedef struct _EX_FAST_REF {
	PVOID Object;
} EX_FAST_REF, * PEX_FAST_REF;

typedef struct _JOBOBJECT_WAKE_FILTER {
	UINT HighEdgeFilter;
	UINT LowEdgeFilter;
} JOBOBJECT_WAKE_FILTER, * PJOBOBJECT_WAKE_FILTER;

typedef struct _PS_PROCESS_WAKE_INFORMATION {
	UINT64 NotificationChannel;
	UINT WakeCounters [ 7 ];
	JOBOBJECT_WAKE_FILTER WakeFilter;
	UINT NoWakeCounter;
} PS_PROCESS_WAKE_INFORMATION, * PPS_PROCESS_WAKE_INFORMATION;

typedef struct _MMSUPPORT_FLAGS {
	/*
	0x000 WorkingSetType   : Pos 0, 3 Bits
		+ 0x000 Reserved0 : Pos 3, 3 Bits
		+ 0x000 MaximumWorkingSetHard : Pos 6, 1 Bit
		+ 0x000 MinimumWorkingSetHard : Pos 7, 1 Bit
		+ 0x001 SessionMaster : Pos 0, 1 Bit
		+ 0x001 TrimmerState : Pos 1, 2 Bits
		+ 0x001 Reserved : Pos 3, 1 Bit
		+ 0x001 PageStealers : Pos 4, 4 Bits
		*/
	USHORT u1;
	UCHAR MemoryPriority;
	/*
	+ 0x003 WsleDeleted : Pos 0, 1 Bit
	+ 0x003 SvmEnabled : Pos 1, 1 Bit
	+ 0x003 ForceAge : Pos 2, 1 Bit
	+ 0x003 ForceTrim : Pos 3, 1 Bit
	+ 0x003 NewMaximum : Pos 4, 1 Bit
	+ 0x003 CommitReleaseState : Pos 5, 2 Bits
	*/
	UCHAR u2;
}MMSUPPORT_FLAGS, * PMMSUPPORT_FLAGS;

typedef struct _MMSUPPORT_INSTANCE {
	UINT NextPageColor;
	UINT PageFaultCount;
	UINT64 TrimmedPageCount;
	PVOID VmWorkingSetList;
	LIST_ENTRY WorkingSetExpansionLinks;
	UINT64 AgeDistribution [ 8 ];
	PVOID ExitOutswapGate;
	UINT64 MinimumWorkingSetSize;
	UINT64 WorkingSetLeafSize;
	UINT64 WorkingSetLeafPrivateSize;
	UINT64 WorkingSetSize;
	UINT64 WorkingSetPrivateSize;
	UINT64 MaximumWorkingSetSize;
	UINT64 PeakWorkingSetSize;
	UINT HardFaultCount;
	USHORT LastTrimStamp;
	USHORT PartitionId;
	UINT64 SelfmapLock;
	MMSUPPORT_FLAGS Flags;
} MMSUPPORT_INSTANCE, * PMMSUPPORT_INSTANCE;

typedef struct _MMSUPPORT_SHARED {
	long WorkingSetLock;
	long GoodCitizenWaiting;
	UINT64 ReleasedCommitDebt;
	UINT64 ResetPagesRepurposedCount;
	PVOID WsSwapSupport;
	PVOID CommitReleaseContext;
	PVOID AccessLog;
	UINT64 ChargedWslePages;
	UINT64 ActualWslePages;
	UINT64 WorkingSetCoreLock;
	PVOID ShadowMapping;
} MMSUPPORT_SHARED, * PMMSUPPORT_SHARED;

typedef struct _MMSUPPORT_FULL {
	MMSUPPORT_INSTANCE Instance;
	MMSUPPORT_SHARED Shared;
	UCHAR Padding [ 48 ];
} MMSUPPORT_FULL, * PMMSUPPORT_FULL;

typedef struct _ALPC_PROCESS_CONTEXT {
	char AlpcContext [ 0x20 ];
} ALPC_PROCESS_CONTEXT, * PALPC_PROCESS_CONTEXT;

typedef struct _PS_DYNAMIC_ENFORCED_ADDRESS_RANGES {
	RTL_AVL_TREE Tree;
	EX_PUSH_LOCK Lock;
} PS_DYNAMIC_ENFORCED_ADDRESS_RANGES, * PPS_DYNAMIC_ENFORCED_ADDRESS_RANGES;

typedef struct _MOUSE_INPUT_DATA {
	USHORT UnitId;
	USHORT Flags;
	union {
		ULONG Buttons;
		struct {
			USHORT ButtonFlags;
			USHORT ButtonData;
		};
	};
	ULONG RawButtons;
	LONG LastX;
	LONG LastY;

	ULONG ExtraInformation;
} MOUSE_INPUT_DATA, * PMOUSE_INPUT_DATA;
typedef VOID( *MouseClassServiceCallbackFn )( PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA InputDataStart, PMOUSE_INPUT_DATA InputDataEnd, PULONG InputDataConsumed );

typedef struct _MOUSE_OBJECT {
	PDEVICE_OBJECT mouse_device;
	MouseClassServiceCallbackFn service_callback;
	BOOL use_mouse;
} MOUSE_OBJECT, * PMOUSE_OBJECT;

extern "C"
{
	inline MOUSE_OBJECT gMouseObject;
	inline QWORD _KeAcquireSpinLockAtDpcLevel;
	inline QWORD _KeReleaseSpinLockFromDpcLevel;
	inline QWORD _IofCompleteRequest;
	inline QWORD _IoReleaseRemoveLockEx;

	NTSYSCALLAPI POBJECT_TYPE* IoDriverObjectType;

	NTKERNELAPI NTSTATUS NTAPI ZwQuerySystemInformation(
		_In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
		_Inout_ PVOID SystemInformation,
		_In_ ULONG SystemInformationLength,
		_Out_opt_ PULONG ReturnLength
	);

	NTSTATUS NTAPI MmCopyVirtualMemory(
		PEPROCESS SourceProcess,
		PVOID SourceAddress,
		PEPROCESS TargetProcess,
		PVOID TargetAddress,
		SIZE_T BufferSize,
		KPROCESSOR_MODE PreviousMode,
		PSIZE_T ReturnSize
	);

	NTSYSAPI
		NTSTATUS
		NTAPI
		ZwProtectVirtualMemory(
			__in HANDLE ProcessHandle,
			__inout PVOID* BaseAddress,
			__inout PSIZE_T RegionSize,
			__in ULONG NewProtect,
			__out PULONG OldProtect
		);

	NTKERNELAPI
		PVOID NTAPI RtlFindExportedRoutineByName(
			_In_ PVOID ImageBase,
			_In_ PCCH RoutineName
		);

	NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(PEPROCESS Process);

	NTKERNELAPI PPEB NTAPI PsGetProcessPeb(IN PEPROCESS Process);

	NTSYSAPI PIMAGE_NT_HEADERS NTAPI RtlImageNtHeader( IN PVOID ModuleAddress );

	NTSYSCALLAPI NTSTATUS ObReferenceObjectByName( __in PUNICODE_STRING ObjectName, __in ULONG Attributes, __in_opt PACCESS_STATE AccessState, __in_opt ACCESS_MASK DesiredAccess, __in POBJECT_TYPE ObjectType, __in KPROCESSOR_MODE AccessMode, __inout_opt PVOID ParseContext, __out PVOID* Object );

	VOID MouseClassServiceCallback( PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA InputDataStart, PMOUSE_INPUT_DATA InputDataEnd, PULONG InputDataConsumed );

	PLIST_ENTRY NTKERNELAPI PsLoadedModuleList;
}