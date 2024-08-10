#pragma once
class kinterface_t {
private:
	__int64(__fastcall* NtGdiEngPaint)(void*) = nullptr;

	typedef enum _request_codes
	{
		request_drv = 0xFF << 1,
		request_read = 0xFF << 2,
		request_write = 0xFF << 3,
		request_procbase = 0xFF << 4,
		request_modbase = 0xFF << 5,
		request_movemouse = 0xFF << 6,
		request_drawbox = 0xFF << 7,

		request_success = 0xFF << 200,
		request_unique = 0xFF << 202,
	}request_codes, * prequest_codes;

	typedef struct _drv_request {
		uintptr_t secret_code;
	} drv_request, * pdrv_request;

	typedef struct _read_request {
		uint32_t pid;
		uint32_t mmcopy;
		uintptr_t address;
		void* buffer;
		size_t size;
		size_t ret_size;
	} read_request, * pread_request;

	typedef struct _write_request {
		uint32_t pid;
		uintptr_t address;
		void* buffer;
		size_t size;
	} write_request, * pwrite_request;

	typedef struct _proc_request {
		uint32_t pid;
		uintptr_t ProcessBase;
		uintptr_t ModuleBase;
		wchar_t lpModName[32];
	} proc_request, * pproc_request;

	typedef struct mousemove_t {
		LONG LastX;
		LONG LastY;
		USHORT ButtonFlags;
	} k_mousemove, * p_mousemove;

	typedef struct draw_t {
		HWND hWnd;
		int r, g, b, x, y, w, h, t;
		int     dx, dy;
	} k_draw, * p_draw;

	typedef struct _request_data
	{
		uint32_t unique;
		request_codes code;
		uint32_t status;
		void* data;
	}request_data, * prequest_data;

public:
	int _processid;
	uintptr_t ProcessBase;
	uintptr_t ModuleBase;

	bool Initialize();
	bool SendCMD(void* data, request_codes code);

	uintptr_t GetProcessBase(int PID);
	uintptr_t GetModuleBase(int PID, LPCWSTR ModName);

	bool ReadPhysMemory(const int pid, const std::uintptr_t address, void* buffer, const std::size_t size, bool mmcopy = false, PDWORD_PTR num_bytes = 0);
	bool WritePhysMemory(const int pid, const std::uintptr_t address, void* buffer, const std::size_t size);
	bool move_mouse(long x, long y, unsigned short button_flags);
	bool draw_box(HWND hWnd, int x, int y, int w, int h, int t, int r, int g, int b);
};
inline kinterface_t* kinterface = new kinterface_t();

bool kinterface_t::SendCMD(void* data, request_codes code) {
	if (!data || !code) {
		return false;
	}

	request_data request{ 0 };
	request.unique = request_unique;
	request.data = data;
	request.code = code;

	const auto result = NtGdiEngPaint(&request);
	if (result != request_success) {
		return false;
	}

	return true;
}
bool kinterface_t::Initialize() {
	LoadLibraryA(xorstr_("user32.dll"));
	LoadLibraryA(xorstr_("win32u.dll"));
	HMODULE win32u = LoadLibraryA(xorstr_("win32u.dll"));
	if (!win32u) {
		return false;
	}

	*(void**)&NtGdiEngPaint = GetProcAddress(win32u, xorstr_("NtGdiEngPaint"));
	if (!NtGdiEngPaint) {
		return false;
	}

	drv_request drv;
	drv.secret_code = 0x1000;
	SendCMD(&drv, request_drv);
	if (drv.secret_code == 0x1337) {
		//_processid = processid;
		return true;
	}

	return false;
}

uintptr_t kinterface_t::GetProcessBase(int PID) {
	proc_request data{ 0 };

	data.pid = PID;
	SendCMD(&data, request_procbase);
	return data.ProcessBase;
}

uintptr_t kinterface_t::GetModuleBase(int PID, LPCWSTR ModName) {
	proc_request data{ 0 };

	data.pid = PID;
	lstrcpyW(data.lpModName, ModName);
	SendCMD(&data, request_modbase);
	return data.ModuleBase;
}

bool kinterface_t::ReadPhysMemory(const int pid, const std::uintptr_t address, void* buffer, const std::size_t size, bool mmcopy, PDWORD_PTR num_bytes) {
	read_request data{ 0 };

	data.pid = pid;
	data.address = address;
	data.buffer = buffer;
	data.mmcopy = mmcopy;
	data.size = size;

	if (num_bytes)
		*num_bytes = data.ret_size;
	return SendCMD(&data, request_read);
}

bool kinterface_t::WritePhysMemory(const int pid, const std::uintptr_t address, void* buffer, const std::size_t size) {
	read_request data{ 0 };

	data.pid = pid;
	data.address = address;
	data.buffer = buffer;
	data.size = size;

	return SendCMD(&data, request_write);
}

bool kinterface_t::move_mouse(long x, long y, unsigned short button_flags) {
	k_mousemove data{ 0 };

	data.LastX = x;
	data.LastY = y;
	data.ButtonFlags = button_flags;

	return SendCMD(&data, request_movemouse);
}

bool kinterface_t::draw_box(HWND hWnd, int x, int y, int w, int h, int t, int r, int g, int b) {
	k_draw data{ 0 };

	data.hWnd = hWnd;
	data.x = x;
	data.y = y;
	data.w = w;
	data.h = h;
	data.t = t;
	data.r = r;
	data.g = g;
	data.b = b;
	return SendCMD(&data, request_drawbox);
}


template <typename T>
inline T read(const std::uintptr_t address)
{
	T response{ };
	kinterface->ReadPhysMemory(kinterface->_processid, address, &response, sizeof(T));
	return response;
}

template <class T>
bool read_raw(UINT_PTR addr, T* buffer, size_t size) {
	kinterface->ReadPhysMemory(kinterface->_processid, addr, &buffer, sizeof(T) * size);
}

template <typename T>
inline T write(const std::uintptr_t address, T value)
{
	return kinterface->WritePhysMemory(kinterface->_processid, address, &value, sizeof(T));
}

class FindPattern
{
public:
	FindPattern(DWORD64 start, DWORD64 len, BYTE* pMask, char* szMask) : Base(0), Offset(0)
	{
		BYTE* data = new BYTE[len];
		if (read_raw<BYTE>(start, data, len))
		{
			for (DWORD i = 0; i < len; i++)
			{
				if (DataCompare((const BYTE*)(data + i), (const BYTE*)pMask, szMask))
				{
					Base = (DWORD64)(start + i);
					Offset = i;
					break;
				}
			}
		}
		delete[] data;
	}
	static HANDLE pHandle;
	DWORD64 Base;
	DWORD Offset;
private:
	bool DataCompare(const BYTE* pData, const BYTE* pMask, const char* pszMask)
	{
		for (; *pszMask; ++pData, ++pMask, ++pszMask)
		{
			if (*pszMask == '0' && *pData != *pMask)
				return false;
		}
		return (*pszMask == NULL);
	}
};

	//typedef INT64(*Nt_UserGetPointerProprietaryId)(uintptr_t);
	//Nt_UserGetPointerProprietaryId NtUserGetPointerProprietaryId = nullptr;

	//int _processid;
	//ULONG64 _clientaddress;


	////request codes
	//#define DRIVER_READVM				0x80000001
	//#define CLIENT_BASE					0x80000002
	//#define WRITE						0x80000003

	//struct _requests
	//{
	//	//rw
	//	uint32_t    src_pid;
	//	uint64_t    src_addr;
	//	void*		dst_addr;
	//	size_t        size;

	//	//function requests
	//	int request_key;

	//	ULONG64 client_base;
	//	const char* modName;
	//	bool isgetsize;
	//};

	//auto readvm(uint32_t src_pid, uint64_t src_addr, void* dst_addr, size_t size) -> void
	//{
	//	if (src_pid == 0 || src_addr == 0) return;

	//	_requests out = { src_pid, src_addr, dst_addr, size, DRIVER_READVM };
	//	NtUserGetPointerProprietaryId(reinterpret_cast<uintptr_t>(&out));
	//}
	//auto writevm(uint32_t src_pid, uint64_t src_addr, void* dst_addr, size_t size) -> void
	//{
	//	if (src_pid == 0 || dst_addr == 0) return;

	//	_requests out = { src_pid, src_addr, dst_addr, size, WRITE };
	//	NtUserGetPointerProprietaryId(reinterpret_cast<uintptr_t>(&out));
	//}

	//auto initdriver(int processid) -> void
	//{
	//	NtUserGetPointerProprietaryId = (Nt_UserGetPointerProprietaryId)GetProcAddress(LoadLibraryA(OBF("win32u.dll")), OBF("NtUserGetPointerProprietaryId"));
	//	if (NtUserGetPointerProprietaryId != 0)
	//	{
	//		_processid = processid;
	//	}
	//}

	//void readsize(const uintptr_t address, void* buffer, const size_t size)
	//{
	//	readvm(_processid, address, buffer, size);
	//}


	//std::string read_str(std::uintptr_t dst)
	//{
	//	char buf[256];
	//	readsize(dst, &buf, sizeof(buf));
	//	return buf;
	//}

	//template <typename T>
	//T read(uintptr_t src, size_t size = sizeof(T))
	//{
	//	T buffer;
	//	readvm(_processid, src, &buffer, size);
	//	return buffer;
	//}

	//template <typename T>
	//void write(const uintptr_t address, T buffer)
	//{
	//	writevm(_processid, address, &buffer, sizeof(T));
	//}

	//void writeBytes(const uintptr_t address, PVOID buffer, SIZE_T buffer_size) {
	//	writevm(_processid, address, buffer, buffer_size);
	//}

	//auto client_address(const char* moduleName, bool isgetsize) -> ULONG64
	//{
	//	_requests out = { 0 };
	//	out.request_key = CLIENT_BASE;
	//	out.src_pid = _processid;
	//	out.modName = moduleName;
	//	out.isgetsize = isgetsize;
	//	NtUserGetPointerProprietaryId(reinterpret_cast<uintptr_t>(&out));
	//	_clientaddress = out.client_base;
	//	return out.client_base;
	//}
