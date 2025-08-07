#pragma once

#include "RpgString.h"
#include "RpgDelegate.h"


#define RPG_CONSOLE_MESSAGE_FORMAT_MAX_COUNT	1024


typedef RpgArrayInline<RpgName, 8> RpgConsoleCommandParams;



class RpgConsoleSystem
{
	RPG_NOCOPYMOVE(RpgConsoleSystem)

private:
	struct FLogInfo
	{
		int BufferIndex{ RPG_INDEX_INVALID };
		int BufferCount{ 0 };
		RpgColor Color;
	};


public:
	RpgConsoleSystem() noexcept;

	void AddLogMessage(const char* message, RpgColor color) noexcept;
	void AddLogCategoryMessage(const RpgPlatformLog::FCategory& category, RpgPlatformLog::EVerbosity verbosity, const char* message) noexcept;


	template<typename...TVarArgs>
	inline void AddLogMessageFormat(RpgColor color, const char* format, TVarArgs&&... args) noexcept
	{
		char message[RPG_CONSOLE_MESSAGE_FORMAT_MAX_COUNT];
		RpgPlatformMemory::MemZero(message, RPG_CONSOLE_MESSAGE_FORMAT_MAX_COUNT);
		snprintf(message, RPG_CONSOLE_MESSAGE_FORMAT_MAX_COUNT, format, std::forward<TVarArgs>(args)...);

		AddLogMessage(message, color);
	}


	template<typename...TVarArgs>
	inline void AddLogCategoryMessageFormat(const RpgPlatformLog::FCategory& category, RpgPlatformLog::EVerbosity verbosity, const char* format, TVarArgs&&... args) noexcept
	{
		char message[RPG_CONSOLE_MESSAGE_FORMAT_MAX_COUNT];
		RpgPlatformMemory::MemZero(message, RPG_CONSOLE_MESSAGE_FORMAT_MAX_COUNT);

		snprintf(message, RPG_CONSOLE_MESSAGE_FORMAT_MAX_COUNT, format, std::forward<TVarArgs>(args)...);

		AddLogCategoryMessage(category, verbosity, message);
	}


	inline int GetLogCount() const noexcept
	{
		return LogInfos.GetCount();
	}


	inline const char* GetLogMessage(int index, int* optOut_CharLength = nullptr, RpgColor* optOut_Color = nullptr) const noexcept
	{
		const FLogInfo& info = LogInfos[index];

		if (optOut_CharLength)
		{
			*optOut_CharLength = info.BufferCount;
		}

		if (optOut_Color)
		{
			*optOut_Color = info.Color;
		}

		return LogBuffer.GetData(info.BufferIndex);
	}


	inline void RegisterCommand(const RpgName& command) noexcept
	{
		if (RegisteredCommands.FindIndexByValue(command) == RPG_INDEX_INVALID)
		{
			RegisteredCommands.AddValue(command);
		}
	}


	template<typename TFunction>
	inline void RegisterStaticCommandListener(TFunction callback) noexcept
	{
		RegisteredListeners.AddStaticFunction(callback);
	}


	template<typename TObject, typename TFunction>
	inline void RegisterObjectCommandListener(TObject* obj, TFunction callback) noexcept
	{
		RegisteredListeners.AddObjectFunction(obj, callback);
	}


	void ExecuteCommand(const char* commandArgs) noexcept;


private:
	RpgArray<char, 128> LogBuffer;
	RpgArray<FLogInfo> LogInfos;

	RpgArray<RpgName> RegisteredCommands;

	RPG_DELEGATE_DECLARE_TwoParams(FCommandDelegate, const RpgName&, command, const RpgConsoleCommandParams&, params)
	FCommandDelegate RegisteredListeners;

};


extern RpgConsoleSystem* g_ConsoleSystem;



#define RPG_CONSOLE_Log(category, format, ...)																		\
{																													\
	g_ConsoleSystem->AddLogCategoryMessageFormat(category, RpgPlatformLog::VERBOSITY_LOG, format, __VA_ARGS__);		\
	RPG_Log(category, format, __VA_ARGS__);																			\
}


#define RPG_CONSOLE_Warn(category, format, ...)																		\
{																													\
	g_ConsoleSystem->AddLogCategoryMessageFormat(category, RpgPlatformLog::VERBOSITY_WARN, format, __VA_ARGS__);	\
	RPG_LogWarn(category, format, __VA_ARGS__);																		\
}


#define RPG_CONSOLE_Error(category, format, ...)																	\
{																													\
	g_ConsoleSystem->AddLogCategoryMessageFormat(category, RpgPlatformLog::VERBOSITY_ERROR, format, __VA_ARGS__);	\
	RPG_LogError(category, format, __VA_ARGS__);																	\
}
