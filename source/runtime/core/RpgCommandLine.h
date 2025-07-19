#pragma once

#include "RpgPlatform.h"



namespace RpgCommandLine
{
	extern void Initialize(const char* commandArgs) noexcept;

	extern bool HasCommand(const char* command) noexcept;
	extern const char* GetCommandValue(const char* command) noexcept;


	inline int GetCommandValueInt(const char* command) noexcept
	{
		return RpgPlatformString::CStringToInt(GetCommandValue(command));
	}

};
