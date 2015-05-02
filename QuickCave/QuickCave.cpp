#include <windows.h>
#include <Psapi.h>
#include <stdio.h>
#include "QuickCave.h"

void _declspec(naked) CodeCaveWrapper()
{
	__asm
	{
		pushad;
		call QuickCave::CodeCaveAddress;
		popad;

		call next;

		//EIP is now in stack, so pop it
	next:
		pop QuickCave::ReturnAddressWrapper;

		//Add 19 bytes to EIP because POP instruction is 6 bytes, add = 7 bytes, jmp = 6 bytes
		add QuickCave::ReturnAddressWrapper, 19;
		jmp QuickCave::ReplacedInstructionsAddress;
		jmp QuickCave::ReturnAddress;
	}
}

void Debug(int output)
{
	char strBuf[50];
	sprintf_s(strBuf, "Output: %x", output);

	MessageBoxA(0, strBuf, "Test", 0);
	return;
}

LPVOID QuickCave::FindPattern(HANDLE hProcess, LPVOID searchLocation, SIZE_T searchArea, const BYTE* patternBuffer, UINT patternSize, int occurence)
{
	BYTE* moduleData = new BYTE[searchArea];
	SIZE_T lpNumberOfBytesRead = 0;
	int cur = 0;

	ReadProcessMemory(hProcess, searchLocation, moduleData, searchArea, &lpNumberOfBytesRead);

	while (cur < searchArea)
	{
		bool foundFlag = false;
		for (int i = cur; i < searchArea - patternSize + 1; i++)
		{
			if (moduleData[i] == patternBuffer[0])
			{
				foundFlag = true;
				cur = i;
				break;
			}
		}

		if (!foundFlag)
			break;

		if (memcmp(moduleData + cur, patternBuffer, patternSize) == 0 && --occurence == 0){
			delete[] moduleData;
			return (char*)searchLocation + cur;
		}

		cur++;
	}

	delete[] moduleData;

	return NULL;
}

QuickCave::ModuleInfo QuickCave::GetModuleInfo(const char* moduleName)
{
	HANDLE hProcess = GetCurrentProcess();
	HMODULE hModule = GetModuleHandleA(moduleName);
	MODULEINFO modInfo;

	GetModuleInformation(hProcess, hModule, &modInfo, sizeof(modInfo));

	return{ modInfo.lpBaseOfDll, modInfo.SizeOfImage };
}

void QuickCave::JmpRedirect(LPVOID location, UINT replaceLength)
{
	ReturnAddress = (char*)location + replaceLength;

	int currentIP = (int)location + 5;

	DWORD pfdwOldProtect = 0;
	HANDLE hProcess = GetCurrentProcess();
	VirtualProtectEx(hProcess, location, replaceLength, PAGE_EXECUTE_READWRITE, &pfdwOldProtect);

	int relativeAddress = (int)CodeCaveWrapper - currentIP;
	BYTE* locationPointer = (BYTE*)location;

	//JMP to code cave
	*(locationPointer) = 0xE9;
	*(int*)(locationPointer + 1) = relativeAddress;

	//NOP the rest
	for (UINT i = 5; i < replaceLength; i++)
		*(locationPointer + i) = 0x90;

	VirtualProtectEx(hProcess, (void*)location, replaceLength, pfdwOldProtect, &pfdwOldProtect);

	FlushInstructionCache(hProcess, location, replaceLength);
}

LPVOID QuickCave::BackupInstructions(LPVOID location, UINT length)
{
	LPVOID instructionCaveAddress = VirtualAllocEx(GetCurrentProcess(), NULL, length + 6, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	memcpy(instructionCaveAddress, location, length);

	//JMP [QuickCave::ReturnAddressWrapper]
	*((unsigned char*)instructionCaveAddress + length) = 0xFF;
	*((unsigned char*)instructionCaveAddress + length + 1) = 0x25;
	*(int*)((unsigned char*)instructionCaveAddress + length + 2) = (int)&QuickCave::ReturnAddressWrapper;

	return instructionCaveAddress;
}

void QuickCave::FindPatternAndRedirect(HANDLE hProcess, const char* moduleName, LPVOID CodeCave, int patternRedirectOffset, UINT replaceLength, const BYTE* patternBuffer, UINT patternSize, int occurence)
{
	ModuleInfo modInfo = GetModuleInfo(moduleName);
	LPVOID codeLocation = FindPattern(hProcess, modInfo.baseAddress, modInfo.moduleSize, patternBuffer, patternSize, occurence);

	Debug((int)codeLocation);

	if (codeLocation == NULL)
		return;

	CodeCaveAddress = CodeCave;

	LPVOID redirectAddress = (char*)codeLocation + patternRedirectOffset;
	ReplacedInstructionsAddress = BackupInstructions(redirectAddress, replaceLength);

	JmpRedirect(redirectAddress, replaceLength);
}

void QuickCave::Redirect(HANDLE hProcess, LPVOID CodeCave, LPVOID callLocation, UINT replaceLength)
{
	CodeCaveAddress = CodeCave;
	ReturnAddress = (char*)callLocation + replaceLength;

	ReplacedInstructionsAddress = BackupInstructions(callLocation, replaceLength);

	JmpRedirect(callLocation, replaceLength);
}

LPVOID QuickCave::CodeCaveAddress;
LPVOID QuickCave::ReplacedInstructionsAddress;
LPVOID QuickCave::ReturnAddress;
LPVOID QuickCave::ReturnAddressWrapper;
