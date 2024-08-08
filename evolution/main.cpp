#include "framework.h"

using namespace std;

DWORD GetProcessID(const std::string processName)
{
	std::wstring wideProcessName(processName.begin(), processName.end());
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	Process32First(processesSnapshot, &processInfo);
	if (!wideProcessName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		Sleep(1);
		if (!wideProcessName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return 0;
}

auto get_process_wnd(uint32_t pid) -> HWND
{
	std::pair<HWND, uint32_t> params = { 0, pid };
	BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
		auto pParams = (std::pair<HWND, uint32_t>*)(lParam);
		uint32_t processId = 0;

		if (GetWindowThreadProcessId(hwnd, reinterpret_cast<LPDWORD>(&processId)) && processId == pParams->second) {
			SetLastError((uint32_t)-1);
			pParams->first = hwnd;
			return FALSE;
		}

		return TRUE;

		}, (LPARAM)&params);

	if (!bResult && GetLastError() == -1 && params.first)
		return params.first;

	return NULL;
}

bool loadDriver() {
	system(OBF("sc stop faceit >> NUL"));
	HANDLE iqvw64e_device_handle = intel_driver::Load();
	if (!iqvw64e_device_handle || iqvw64e_device_handle == INVALID_HANDLE_VALUE)
	{
		Sleep(2000);
		return -1;
	}
	NTSTATUS exitCode = 0;
	if (!kdmapper::MapDriver(iqvw64e_device_handle, driver_shellcode));
	{
		intel_driver::Unload(iqvw64e_device_handle);
		return -1;
	}
}


class SelfDelete
{
public:
	std::string RandString(int len)
	{
		const std::string chars = OBF("ABCDEFGHIJKLMNOPQRSTUVXYZ0123456789");

		std::random_device rd;
		std::mt19937 generator(rd());
		std::uniform_int_distribution<> dis(0, chars.size() - 1);

		std::string rnd;
		for (size_t i = 0; i < len; i++)
		{
			rnd += chars[dis(generator)];
		}

		return rnd;
	}

	BOOL ds_rename_handle(HANDLE hHandle)
	{
		FILE_RENAME_INFO fRename;
		RtlSecureZeroMemory(&fRename, sizeof(fRename));

		LPCWSTR lpwStream = OBF(L":sqnc");
		fRename.FileNameLength = sizeof(lpwStream);
		RtlCopyMemory(fRename.FileName, lpwStream, sizeof(lpwStream));

		return SetFileInformationByHandle(hHandle, FileRenameInfo, &fRename, sizeof(fRename) + sizeof(lpwStream));
	}

	BOOL ds_deposite_handle(HANDLE hHandle)
	{
		FILE_DISPOSITION_INFO fDelete;
		RtlSecureZeroMemory(&fDelete, sizeof(fDelete));
		fDelete.DeleteFile = TRUE;

		return SetFileInformationByHandle(hHandle, FileDispositionInfo, &fDelete, sizeof(fDelete));
	}

	void Dlt() {
		WCHAR wcPath[MAX_PATH + 1];
		RtlSecureZeroMemory(wcPath, sizeof(wcPath));
		GetModuleFileNameW(NULL, wcPath, MAX_PATH);

		auto StringLmfao = (RandString(14) + OBF(".exe"));
		std::wstring wideString(StringLmfao.begin(), StringLmfao.end());

		WCHAR wcNewPath[MAX_PATH + 1];
		RtlSecureZeroMemory(wcNewPath, sizeof(wcNewPath));
		wcscpy_s(wcNewPath, MAX_PATH, wcPath);
		PathRemoveFileSpecW(wcNewPath);
		PathAppendW(wcNewPath, wideString.c_str());
		CopyFileW(wcPath, wcNewPath, FALSE);

		HANDLE hCurrent = CreateFileW(wcPath, DELETE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		ds_rename_handle(hCurrent);
		CloseHandle(hCurrent);

		hCurrent = CreateFileW(wcPath, DELETE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		ds_deposite_handle(hCurrent);
		CloseHandle(hCurrent);
		PathFileExistsW(wcPath);
	}
};

inline SelfDelete* cSelfDelete = new SelfDelete();

int main()
{
	//cSelfDelete->Dlt();

	loadDriver();
	//InitMoveMouse();

	//Wait for game
	while (Entryhwnd == NULL)
	{
		printf(OBF("> Waiting for crossfire...\r"));
		processid = GetProcessID(OBF("crossfire.dat"));
		Entryhwnd = get_process_wnd(processid);
		Sleep(1);
	}

	//Load Driver
	system(OBF("sc stop faceit >> NUL"));

	//Init Driver
	initdriver(processid);
	
	while ((uintptr_t)client_address(OBF("ClientFx_x64.fxd"), true) != 274432) {
		Sleep(1000);
	}

	system(OBF("cls"));

	Beep(1, 1);
	FreeConsole();

	//Main Process
	CShell_x64 = client_address(OBF("CShell_x64.dll"), false);
	LTClientShell = read<uintptr_t>(CShell_x64 + dwLTShell);

	using framerate = std::chrono::duration<int, std::ratio<1, 400>>;
	auto tp = std::chrono::system_clock::now() + framerate{ 1 };
	while (1) {
		CF = FindWindowA(0, OBF("CROSSFIRE"));
		if (CF) {
			Aimbot_Initialize();
		}
		else {
			Beep(1, 1);
			ExitProcess(0);
		}
		std::this_thread::sleep_until(tp);
		tp += framerate{ 1 };
	}
}
