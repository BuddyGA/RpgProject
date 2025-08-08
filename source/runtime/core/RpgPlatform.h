#pragma once

#include "RpgTypes.h"

#include <mimalloc-override.h>

#include <winsdkver.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <sdkddkver.h>

#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#define RPG_DebugBreak()	if (IsDebuggerPresent()) __debugbreak()


typedef volatile LONG	RpgAtomicInt;



// ========================================================================================================================= //
// PLATFORM - MEMORY
// ========================================================================================================================= //
namespace RpgPlatformMemory
{
	[[nodiscard]] extern void* MemMalloc(size_t sizeBytes) noexcept;
	[[nodiscard]] extern void* MemMallocAligned(size_t sizeBytes, size_t alignmentBytes) noexcept;
	[[nodiscard]] extern void* MemRealloc(void* prevAlloc, size_t newSizeBytes) noexcept;
	[[nodiscard]] extern void* MemRecalloc(void* prevAlloc, int count, size_t sizeBytes) noexcept;
	extern void MemFree(void* alloc) noexcept;
	extern void MemCopy(void* dst, const void* src, size_t sizeBytes) noexcept;
	extern void MemMove(void* dst, const void* src, size_t sizeBytes) noexcept;
	extern void MemSet(void* data, int value, size_t sizeBytes) noexcept;
	extern void MemZero(void* data, size_t sizeBytes) noexcept;

}; // RpgPlatformMemory




// ========================================================================================================================= //
// PLATFORM - STRING
// ========================================================================================================================= //
namespace RpgPlatformString
{
	extern int CStringLength(const char* cstr) noexcept;
	extern bool CStringCompare(const char* cstrA, const char* cstrB, bool bIgnoreCase) noexcept;
	extern void CStringCopy(char* dst, const char* src) noexcept;
	extern void CStringToWide(wchar_t* dst, const char* src, size_t maxBufferCount) noexcept;
	extern void CStringToLower(char* cstr, int len) noexcept;
	extern int CStringToInt(const char* cstr) noexcept;
	extern float CStringToFloat(const char* cstr) noexcept;
	extern uint64_t CStringHash(const char* cstr) noexcept;

	extern int WStringLength(const wchar_t* wstr) noexcept;
	extern void WStringToMultibyte(char* dst, const wchar_t* wstr, size_t maxBufferCount) noexcept;

}; // RpgPlatformString




// ========================================================================================================================= //
// PLATFORM - CONSOLE
// ========================================================================================================================= //
namespace RpgPlatformConsole
{
	enum EOutputColor : uint8_t
	{
		OUTPUT_COLOR_DEFAULT = 0,
		OUTPUT_COLOR_GREEN,
		OUTPUT_COLOR_YELLOW,
		OUTPUT_COLOR_RED
	};


	// Initialize and allocate console window
	// @returns True on success false otherwise
	extern void Initialize() noexcept;


	// Output message to console
	// @param message - Message to output
	// @param color - Message color
	// @returns None
	extern void OutputMessage(const char* message, int messageLength, EOutputColor color) noexcept;

}; // RpgPlatformConsole




// ========================================================================================================================= //
// PLATFORM - LOG
// ========================================================================================================================= //
#define RPG_LOG_MAX_OUTPUT_BUFFER		2048
#define RPG_LOG_CATEGORY_NAME_LENGTH	32	// Includes null terminator
#define RPG_LOG_CATEGORY_MAX_FORMAT		(RPG_LOG_MAX_OUTPUT_BUFFER - RPG_LOG_CATEGORY_NAME_LENGTH)


namespace RpgPlatformLog
{
	enum EVerbosity : uint8_t
	{
		VERBOSITY_NONE = 0,
		VERBOSITY_DEBUG,
		VERBOSITY_LOG,
		VERBOSITY_WARN,
		VERBOSITY_ERROR
	};


	struct FCategory
	{
		const char* Name{ nullptr };
		EVerbosity Verbosity{ VERBOSITY_NONE };
	};


	extern void Initialize(EVerbosity in_GlobalVerbosity, const char* opt_OutputLogFilePath = nullptr) noexcept;
	extern void Shutdown() noexcept;
	extern void SetGlobalVerbosity(EVerbosity in_Verbosity) noexcept;
	extern EVerbosity GetGlobalVerbosity() noexcept;
	extern void OutputMessage(RpgPlatformConsole::EOutputColor consoleOutputColor, const char* message) noexcept;
	extern void OutputMessageFormat(RpgPlatformConsole::EOutputColor consoleOutputColor, const char* format, ...) noexcept;
	extern void OutputMessageLogCategoryFormat(const FCategory& category, EVerbosity verbosity, const char* format, ...) noexcept;

}; // RpgPlatformLog


#define RPG_LOG_DECLARE_CATEGORY_EXTERN(catName)	extern RpgPlatformLog::FCategory catName;


#define RPG_LOG_DECLARE_CATEGORY_STATIC(catName, defaultVerbosity)												\
static RpgPlatformLog::FCategory catName{#catName, RpgPlatformLog::defaultVerbosity};							\
static_assert(sizeof(#catName) <= RPG_LOG_CATEGORY_NAME_LENGTH, "Exceeds maximum log category name length!");


#define RPG_LOG_DEFINE_CATEGORY(catName, defaultVerbosity)														\
RpgPlatformLog::FCategory catName{#catName, RpgPlatformLog::defaultVerbosity};									\
static_assert(sizeof(#catName) <= RPG_LOG_CATEGORY_NAME_LENGTH, "Exceeds maximum log category name length!");


#ifndef RPG_BUILD_SHIPPING
	#define RPG_LogDebug(category, format, ...)	RpgPlatformLog::OutputMessageLogCategoryFormat(category, RpgPlatformLog::VERBOSITY_DEBUG, format, __VA_ARGS__)
#else
	#define RPG_LogDebug(category, format, ...)
#endif // !RPG_BUILD_SHIPPING

#define RPG_Log(category, format, ...)		RpgPlatformLog::OutputMessageLogCategoryFormat(category, RpgPlatformLog::VERBOSITY_LOG, format, __VA_ARGS__)
#define RPG_LogWarn(category, format, ...)	RpgPlatformLog::OutputMessageLogCategoryFormat(category, RpgPlatformLog::VERBOSITY_WARN, format, __VA_ARGS__)
#define RPG_LogError(category, format, ...)	RpgPlatformLog::OutputMessageLogCategoryFormat(category, RpgPlatformLog::VERBOSITY_ERROR, format, __VA_ARGS__)


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogTemp)
RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogSystem)




// ========================================================================================================================= //
// PLATFORM - PROCESS
// ========================================================================================================================= //
namespace RpgPlatformProcess
{
	extern void Initialize() noexcept;
	extern void Shutdown() noexcept;
	extern void Exit(uint32_t code) noexcept;
	extern void ShowMessageBoxError(const char* title, const char* message) noexcept;
	extern void SetMainWindowHandle(HWND handle) noexcept;
	extern HWND GetMainWindowHandle() noexcept;
	extern uint32_t GetMainThreadId() noexcept;


	// Check if function running on main thread
	// @returns True if running on main thread
	extern bool IsMainThread() noexcept;

}; // RpgPlatformProcess




// ========================================================================================================================= //
// PLATFORM - ASSERT
// ========================================================================================================================= //
#ifdef RPG_BUILD_DEBUG
#define RPG_ASSERT_LEVEL	0
#elif RPG_BUILD_DEVELOPMENT
#define RPG_ASSERT_LEVEL	1
#else
#define RPG_ASSERT_LEVEL	2
#endif // RPG_BUILD_DEBUG


#define RPG_ASSERT_FORMAT_BUFFER_COUNT	256



#define RPG_ASSERT_MESSAGE_EXIT(message)										\
RpgPlatformLog::OutputMessage(RpgPlatformConsole::OUTPUT_COLOR_RED, message);	\
RpgPlatformProcess::ShowMessageBoxError("ERROR (333)", message);				\
RPG_DebugBreak();																\
RpgPlatformProcess::Exit(333);		


#define RPG_AssertMessageV(cond, format, ...)																													\
if (!(cond))																																					\
{																																								\
	char message[RPG_ASSERT_FORMAT_BUFFER_COUNT];																												\
	snprintf(message, RPG_ASSERT_FORMAT_BUFFER_COUNT, format, __VA_ARGS__);																						\
	char assertMessage[RPG_ASSERT_FORMAT_BUFFER_COUNT];																											\
	snprintf(assertMessage, RPG_ASSERT_FORMAT_BUFFER_COUNT, "AssertionFailed: (%s)\nMessage: %s\nFile: %s\nLine: %i\n", #cond, message, __FILE__, __LINE__);	\
	RPG_ASSERT_MESSAGE_EXIT(assertMessage);																							\
}


#define RPG_AssertMessage(cond)																											\
if (!(cond))																															\
{																																		\
	char assertMessage[RPG_ASSERT_FORMAT_BUFFER_COUNT];																					\
	snprintf(assertMessage, RPG_ASSERT_FORMAT_BUFFER_COUNT, "AssertionFailed: (%s)\nFile: %s\nLine: %i\n", #cond, __FILE__, __LINE__);	\
	RPG_ASSERT_MESSAGE_EXIT(assertMessage);																								\
}


#if RPG_ASSERT_LEVEL < 1
#define RPG_AssertV(cond, format, ...)	RPG_AssertMessageV(cond, format, __VA_ARGS__)
#define RPG_Assert(cond)				RPG_AssertMessage(cond)
#else
#define RPG_AssertV(cond, format, ...)
#define RPG_Assert(cond)
#endif // RPG_PLATFORM_ASSERT_LEVEL < 1


#if RPG_ASSERT_LEVEL < 2
#define RPG_CheckV(cond, format, ...)	RPG_AssertMessageV(cond, format, __VA_ARGS__)
#define RPG_Check(cond)					RPG_AssertMessage(cond)
#else
#define RPG_CheckV(cond, format, ...)
#define RPG_Check(cond)
#endif // RPG_PLATFORM_ASSERT_LEVEL < 2


#define RPG_ValidateV(cond, format, ...)	RPG_AssertMessageV(cond, format, __VA_ARGS__)
#define RPG_Validate(cond)					RPG_AssertMessage(cond)


#define RPG_RuntimeErrorCheck(cond, message)															\
if (!(cond))																							\
{																										\
	RpgPlatformLog::OutputMessageFormat(RpgPlatformConsole::OUTPUT_COLOR_RED, "ERROR: %s\n", message);	\
	RpgPlatformProcess::ShowMessageBoxError("Runtime Error", message);									\
	RPG_DebugBreak();																					\
	RpgPlatformProcess::Exit(111);																		\
}


#define RPG_NotImplementedYet()		RPG_RuntimeErrorCheck(false, "Function or scope not implemented yet!");

#define RPG_IsMainThread()			RPG_Check(RpgPlatformProcess::IsMainThread())




// =============================================================================================================================================================== //
// PLATFORM - FILE 
// =============================================================================================================================================================== //
namespace RpgPlatformFile
{
	enum EOpenMode : uint8_t
	{
		OPEN_MODE_READ = 0,
		OPEN_MODE_WRITE_OVERWRITE,
		OPEN_MODE_WRITE_APPEND
	};

	enum ESeekMode : uint8_t
	{
		SEEK_MODE_BEGIN = 0,
		SEEK_MODE_CURRENT,
		SEEK_MODE_END
	};


	extern bool FolderExists(const char* folderPath) noexcept;
	extern bool FolderCreate(const char* folderPath) noexcept;
	extern bool FolderDelete(const char* folderPath) noexcept;

	extern bool FileExists(const char* absoluteFilePath) noexcept;
	extern HANDLE FileOpen(const char* absoluteFilePath, EOpenMode mode) noexcept;
	extern size_t FileGetSize(HANDLE fileHandle) noexcept;
	extern bool FileSeek(HANDLE fileHandle, size_t byteOffset, ESeekMode mode = SEEK_MODE_BEGIN) noexcept;
	extern bool FileRead(HANDLE fileHandle, void* outData, size_t sizeBytes) noexcept;
	extern bool FileWrite(HANDLE fileHandle, const void* data, size_t dataSizeBytes) noexcept;
	extern void FileClose(HANDLE& fileHandle) noexcept;
	extern bool FileDelete(const char* absoluteFilePath) noexcept;

};



// =============================================================================================================================================================== //
// PLATFORM - MOUSE
// =============================================================================================================================================================== //
namespace RpgPlatformMouse
{
	enum ECursorMode : uint8_t
	{
		CURSOR_MODE_ARROW = 0,
		CURSOR_MODE_IBEAM,
		CURSOR_MODE_BUSY,
		CURSOR_MODE_HAND,
		CURSOR_MODE_MOVE,
		CURSOR_MODE_RESIZE_HORIZONTAL,
		CURSOR_MODE_RESIZE_VERTICAL,
		CURSOR_MODE_RESIZE_DIAGONAL_1,
		CURSOR_MODE_RESIZE_DIAGONAL_2,
		CURSOR_MODE_MAX_COUNT
	};


	extern void Capture(HWND windowHandle, bool bCapture) noexcept;
	extern void SetCursorMode(ECursorMode mode) noexcept;
	extern void SetCursorPosition(HWND windowHandle, RpgPointInt position) noexcept;
	extern RpgPointInt GetCursorPosition(HWND windowHandle) noexcept;
	extern void SetEnableClipCursor(HWND windowHandle, RpgRectInt rect) noexcept;
	extern void SetDisableClipCursor() noexcept;
	extern void SetCursorHidden(bool bHidden) noexcept;

};



enum class RpgPlatformWindowDisplayMode : uint8_t
{
	WINDOWED = 0,
	FULLSCREEN
};



enum class RpgPlatformWindowSizeState : uint8_t
{
	DEFAULT = 0,
	MINIMIZED,
	MAXIMIZED
};



struct RpgPlatformWindowEvent
{
	RpgPointInt Size;
	RpgPlatformWindowSizeState State{ RpgPlatformWindowSizeState::DEFAULT };
};


struct RpgPlatformMouseMoveEvent
{
	RpgPointInt Position;
	RpgPointInt DeltaPosition;
};


struct RpgPlatformMouseWheelEvent
{
	RpgPointInt Position;
	RpgPointInt ScrollValue;
};


struct RpgPlatformMouseButtonEvent
{
	RpgPointInt Position;
	uint8_t Button{ 0 };
	bool bIsDown{ false };
	bool bIsDoubleClick{ false };
};


struct RpgPlatformKeyboardEvent
{
	uint8_t KeyCode{ 0 };
	uint8_t ScanCode{ 0 };
	uint16_t RepeatCount{ 0 };
	bool bIsDown{ false };
};
