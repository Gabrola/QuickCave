# QuickCave
A library for quickly creating code caves in C++ on Win32

It will easily call your code cave from any location you specify or search for a code pattern and call your code cave from there.

The library will also automatically call any instructions replaced by the redirection code after your codecave has been called.

![Code cave call location](http://i.imgur.com/HaUbTqR.png)

Example Code:
```cpp
#include <windows.h>
#include "QuickCave.h"

void __declspec(naked) MyCodeCave(void);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		BYTE patternCode[] =
		{
			0x68, 0xE0, 0x67, 0x41, 0x00		//PUSH 0x4167E0
		};

		QuickCave::FindPatternAndRedirect(GetCurrentProcess(), "Test.exe", MyCodeCave, 0, 5, patternCode, sizeof(patternCode));
	}

	return (TRUE);
}

void __stdcall hookedFunction2()
{
	MessageBoxA(0, "It works!", "Code Cave Example", 0);
	return;
}

void __declspec(naked) MyCodeCave(void)
{
	__asm 
	{
		call hookedFunction

		//Code cave is called using a CALL instruction so we must return using RET
		ret
	}
}
```
```cpp
//************************************
// Method:    FindPatternAndRedirect
// FullName:  QuickCave::FindPatternAndRedirect
// Access:    public static 
// Returns:   void
// Qualifier:
// Parameter: HANDLE hProcess Process handle
// Parameter: const char * moduleName Module name to search for pattern in
// Parameter: LPVOID CodeCave Pointer to code cave
// Parameter: int patternRedirectOffset Offset from pattern location to call your code cave
// Parameter: UINT replaceLength Number of bytes to be replaced at the calling location, this must be at least 5
// Parameter: const BYTE * patternBuffer
// Parameter: UINT patternSize
// Parameter: int occurence Which occurence of the pattern to choose in case your pattern occurs more than once
//************************************
```
