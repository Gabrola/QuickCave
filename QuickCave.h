#pragma once

class QuickCave
{
private:
	static void JmpRedirect(LPVOID location, UINT replaceLength);
	static LPVOID BackupInstructions(LPVOID location, UINT length);

public:
	struct ModuleInfo
	{
		LPVOID baseAddress;
		int moduleSize;
	};

	static LPVOID CodeCaveAddress;
	static LPVOID ReplacedInstructionsAddress;
	static LPVOID ReturnAddress;

	static LPVOID ReturnAddressWrapper;

	static LPVOID FindPattern(HANDLE hProcess, LPVOID searchLocation, SIZE_T searchArea, const BYTE* patternBuffer, UINT patternSize, int occurence = 1);
	static ModuleInfo GetModuleInfo(const char* moduleName);

	static void FindPatternAndRedirect(HANDLE hProcess, const char* moduleName, LPVOID CodeCave, int patternRedirectOffset, UINT replaceLength, const BYTE* patternBuffer, UINT patternSize, int occurence = 1);
	static void Redirect(HANDLE hProcess, LPVOID CodeCave, LPVOID callLocation, UINT replaceLength);
};

