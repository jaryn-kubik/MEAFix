#include "stdafx.h"
#include "MinHook.h"
#include "Unknwnbase.h"
#include <fstream>

typedef HRESULT (WINAPI *tDirectInput8Create)(HINSTANCE inst_handle, DWORD version, const IID& r_iid, LPVOID* out_wrapper, LPUNKNOWN p_unk);
tDirectInput8Create oDirectInput8Create = nullptr;
std::ofstream logFile;

typedef BOOL (WINAPI *tSetCursorPos)(int x, int y);
tSetCursorPos oSetCursorPos = nullptr;
BOOL WINAPI HSetCursorPos(int x, int y)
{
	CURSORINFO cursor_info = {};
	cursor_info.cbSize = sizeof(CURSORINFO);
	GetCursorInfo(&cursor_info);

	BOOL result = oSetCursorPos(x, y);
	if (cursor_info.flags & CURSOR_SHOWING)
	{
		ShowCursor(FALSE);
	}
	return result;
}

void CreateHook(LPCSTR name, LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID* ppOriginal)
{
	LPVOID ppTarget;
	logFile << "MH_CreateHookApiEx - " << name << ": ";
	logFile << MH_StatusToString(MH_CreateHookApiEx(pszModule, pszProcName, pDetour, ppOriginal, &ppTarget)) << ", ";
	logFile << MH_StatusToString(MH_EnableHook(ppTarget)) << std::endl;
}

void Initialize()
{
	logFile.rdbuf()->pubsetbuf(nullptr, 0);
	logFile.open("dinput8.log", std::ios_base::out);

	logFile << "MH_Initialize: " << MH_StatusToString(MH_Initialize()) << std::endl;

	CreateHook("SetCursorPos", L"user32.dll", "SetCursorPos", HSetCursorPos, (LPVOID*)&oSetCursorPos);

	WCHAR syspath[MAX_PATH];
	GetSystemDirectory(syspath, MAX_PATH);
	wcscat_s(syspath, L"\\dinput8.dll");
	HMODULE hMod = LoadLibrary(syspath);
	oDirectInput8Create = (tDirectInput8Create)GetProcAddress(hMod, "DirectInput8Create");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		Initialize();
	}
	return TRUE;
}

HRESULT WINAPI DirectInput8Create(HINSTANCE inst_handle, DWORD version, const IID& r_iid, LPVOID* out_wrapper, LPUNKNOWN p_unk)
{
	return oDirectInput8Create(inst_handle, version, r_iid, out_wrapper, p_unk);
}
