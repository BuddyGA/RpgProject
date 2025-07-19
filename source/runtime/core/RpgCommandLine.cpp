#include "RpgCommandLine.h"



namespace RpgCommandLine
{
	struct FArgument
	{
		char Name[32];
		char Value[32];
	};

	static FArgument ArgumentArray[16];
	static int ArgumentCount;
	static bool bInitialized;
};


void RpgCommandLine::Initialize(const char* commandArgs) noexcept
{
	if (bInitialized)
	{
		return;
	}

	const int len = RpgPlatformString::CStringLength(commandArgs);
	if (len == 0)
	{
		bInitialized = true;
		return;
	}

	int nameIndex = -1;
	int valueIndex = -1;

	for (int i = 0; i <= len; ++i)
	{
		if (commandArgs[i] == '-')
		{
			if (i + 1 < len)
			{
				nameIndex = i + 1;
			}
		}
		else if (commandArgs[i] == '=')
		{
			if (i + 1 < len)
			{
				valueIndex = i + 1;
			}
		}
		else if (commandArgs[i] == ' ' || commandArgs[i] == '\0')
		{
			if (nameIndex != -1)
			{
				int nameLength = 0;
				int valueLength = 0;

				if (valueIndex != -1)
				{
					nameLength = (valueIndex - 1) - nameIndex;
					valueLength = i - valueIndex;
				}
				else
				{
					nameLength = i - nameIndex;
				}

				if (nameLength > 0)
				{
					FArgument& arg = ArgumentArray[ArgumentCount++];
					RpgPlatformMemory::MemZero(&arg, sizeof(FArgument));
					RpgPlatformMemory::MemCopy(&arg.Name, commandArgs + nameIndex, nameLength);

					if (valueLength > 0)
					{
						RpgPlatformMemory::MemCopy(&arg.Value, commandArgs + valueIndex, valueLength);
					}
					else
					{
						arg.Value[0] = '1';
					}
				}
			}

			nameIndex = -1;
			valueIndex = -1;
		}
	}

	bInitialized = true;
}


bool RpgCommandLine::HasCommand(const char* command) noexcept
{
	if (command == nullptr)
	{
		return false;
	}

	for (int i = 0; i < ArgumentCount; ++i)
	{
		if (RpgPlatformString::CStringCompare(ArgumentArray[i].Name, command, true))
		{
			return true;
		}
	}

	return false;
}


const char* RpgCommandLine::GetCommandValue(const char* command) noexcept
{
	if (command == nullptr)
	{
		return nullptr;
	}

	for (int i = 0; i < ArgumentCount; ++i)
	{
		if (RpgPlatformString::CStringCompare(ArgumentArray[i].Name, command, true))
		{
			return ArgumentArray[i].Value;
		}
	}

	return nullptr;
}
