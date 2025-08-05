#include "RpgConsoleSystem.h"
#include "RpgCommandLine.h"



RpgConsoleSystem* g_ConsoleSystem = nullptr;



RpgConsoleSystem::RpgConsoleSystem() noexcept
{

#ifdef RPG_BUILD_DEBUG
	RpgPlatformConsole::Initialize();

#else
	if (RpgCommandLine::HasCommand("console"))
	{
		RpgPlatformConsole::Initialize();
	}
#endif // RPG_BUILD_DEBUG

	RegisteredCommands.AddConstruct("exit");
}


void RpgConsoleSystem::AddLogMessage(const char* message, RpgColor color) noexcept
{
	if (message == nullptr)
	{
		return;
	}

	const int length = RpgPlatformString::CStringLength(message);
	if (length == 0)
	{
		return;
	}

	FLogInfo& info = LogInfos.Add();
	info.BufferIndex = LogBuffer.GetCount();
	info.BufferCount = length + 1; // with null terminator
	info.Color = color;

	LogBuffer.InsertAtRange(message, length, RPG_INDEX_LAST);
	LogBuffer.AddValue('\0');
}


void RpgConsoleSystem::AddLogCategoryMessage(const RpgPlatformLog::FCategory& category, RpgPlatformLog::EVerbosity verbosity, const char* message) noexcept
{
	if (verbosity < category.Verbosity || category.Verbosity < RpgPlatformLog::GetGlobalVerbosity())
	{
		return;
	}

	RpgPlatformConsole::EOutputColor consoleOutputColor = RpgPlatformConsole::OUTPUT_COLOR_DEFAULT;
	RpgColor color = RpgColor::WHITE;

	switch (verbosity)
	{
		case RpgPlatformLog::VERBOSITY_DEBUG:
		{
			consoleOutputColor = RpgPlatformConsole::OUTPUT_COLOR_GREEN;
			color = RpgColor::GREEN;
			break;
		}

		case RpgPlatformLog::VERBOSITY_WARN:
		{
			consoleOutputColor = RpgPlatformConsole::OUTPUT_COLOR_YELLOW;
			color = RpgColor::YELLOW;
			break;
		}

		case RpgPlatformLog::VERBOSITY_ERROR:
		{
			consoleOutputColor = RpgPlatformConsole::OUTPUT_COLOR_RED;
			color = RpgColor::RED;
			break;
		}

		default:
			break;
	}

	char output[RPG_CONSOLE_MESSAGE_FORMAT_MAX_COUNT];
	RpgPlatformMemory::MemZero(output, RPG_CONSOLE_MESSAGE_FORMAT_MAX_COUNT);
	snprintf(output, RPG_CONSOLE_MESSAGE_FORMAT_MAX_COUNT, "<%s>: %s\n", category.Name, message);

	AddLogMessage(output, color);

	RpgPlatformLog::OutputMessage(consoleOutputColor, output);
}


void RpgConsoleSystem::ExecuteCommand(const char* commandArgs) noexcept
{
	if (commandArgs == nullptr)
	{
		return;
	}

	const int length = RpgPlatformString::CStringLength(commandArgs);
	if (length == 0)
	{
		return;
	}


	RpgName name;
	RpgConsoleCommandParams params;

	auto LocalFunc_CopyToNameOrParams = [&](const char* src, int count)
	{
		if (count == 0)
		{
			return;
		}

		RPG_Check(count < RPG_NAME_MAX_COUNT);

		if (name.IsEmpty())
		{
			RpgPlatformMemory::MemCopy(name.GetData(), src, sizeof(char) * count);
		}
		else
		{
			RpgName& param = params.Add();
			RpgPlatformMemory::MemCopy(param.GetData(), src, sizeof(char) * count);
		}
	};
	

	int index = 0;
	int count = 0;

	for (int i = 0; i < length; ++i)
	{
		char c = commandArgs[i];

		if (c == ' ')
		{
			LocalFunc_CopyToNameOrParams(commandArgs + index, count);
			index = i + 1;
			count = 0;
		}
		else
		{
			++count;
		}
	}

	LocalFunc_CopyToNameOrParams(commandArgs + index, count);

	if (name.IsEmpty())
	{
		return;
	}

	if (RegisteredCommands.FindIndexByValue(name) == RPG_INDEX_INVALID)
	{
		AddLogMessageFormat(RpgColor::YELLOW, "Invalid command: %s", *name);
	}
	else
	{
		AddLogMessageFormat(RpgColor::WHITE, "Command: %s", commandArgs);
		RegisteredListeners.Broadcast(name, params);
	}
}
