#pragma once
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
	wchar_t lpModName [ 32 ];
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