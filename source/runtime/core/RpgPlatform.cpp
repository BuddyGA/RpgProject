#include "RpgPlatform.h"
#include <mimalloc-new-delete.h>



// ========================================================================================================================= //
// PLATFORM - MEMORY
// ========================================================================================================================= //
void* RpgPlatformMemory::MemMalloc(size_t sizeBytes) noexcept
{
	return mi_malloc(sizeBytes);
}


void* RpgPlatformMemory::MemMallocAligned(size_t sizeBytes, size_t alignmentBytes) noexcept
{
	return mi_malloc_aligned(sizeBytes, alignmentBytes);
}


void* RpgPlatformMemory::MemRealloc(void* prevAlloc, size_t newSizeBytes) noexcept
{
	return mi_realloc(prevAlloc, newSizeBytes);
}


void* RpgPlatformMemory::MemRecalloc(void* prevAlloc, int count, size_t sizeBytes) noexcept
{
	return mi_recalloc(prevAlloc, count, sizeBytes);
}


void RpgPlatformMemory::MemFree(void* alloc) noexcept
{
	mi_free(alloc);
}


void RpgPlatformMemory::MemCopy(void* dst, const void* src, size_t sizeBytes) noexcept
{
	memcpy(dst, src, sizeBytes);
}


void RpgPlatformMemory::MemMove(void* dst, const void* src, size_t sizeBytes) noexcept
{
	memmove(dst, src, sizeBytes);
}


void RpgPlatformMemory::MemSet(void* data, int value, size_t sizeBytes) noexcept
{
	memset(data, value, sizeBytes);
}


void RpgPlatformMemory::MemZero(void* data, size_t sizeBytes) noexcept
{
	memset(data, 0, sizeBytes);
}




// ========================================================================================================================= //
// PLATFORM - STRING
// ========================================================================================================================= //
int RpgPlatformString::CStringLength(const char* cstr) noexcept
{
	return static_cast<int>(strlen(cstr));
}


bool RpgPlatformString::CStringCompare(const char* cstrA, const char* cstrB, bool bIgnoreCase) noexcept
{
	if (cstrA == nullptr && cstrB == nullptr)
	{
		return true;
	}

	if ((cstrA && cstrB == nullptr) || (cstrA == nullptr && cstrB))
	{
		return false;
	}

	const int lenA = CStringLength(cstrA);
	const int lenB = CStringLength(cstrB);

	if (lenA != lenB)
	{
		return false;
	}

	if (bIgnoreCase)
	{
		for (int i = 0; i < lenA; ++i)
		{
			if (tolower(cstrA[i]) != tolower(cstrB[i]))
			{
				return false;
			}
		}
	}

	return strcmp(cstrA, cstrB) == 0;
}


void RpgPlatformString::CStringCopy(char* dst, const char* src) noexcept
{
	strcpy(dst, src);
}


void RpgPlatformString::CStringToWide(wchar_t* dst, const char* src, size_t maxBufferCount) noexcept
{
	mbstowcs(dst, src, maxBufferCount);
}


void RpgPlatformString::CStringToLower(char* cstr, int len) noexcept
{
	RPG_Assert(cstr);
	RPG_Assert(len > 0);

	for (int i = 0; i < len; ++i)
	{
		cstr[i] = static_cast<char>(tolower(cstr[i]));
	}
}


int RpgPlatformString::CStringToInt(const char* cstr) noexcept
{
	return cstr ? atoi(cstr) : 0;
}


float RpgPlatformString::CStringToFloat(const char* cstr) noexcept
{
	return cstr ? static_cast<float>(atof(cstr)) : 0.0f;
}


uint64_t RpgPlatformString::CStringHash(const char* cstr) noexcept
{
	const int len = CStringLength(cstr);
	if (len == 0)
	{
		return 0;
	}

	uint64_t hash = 0;

	for (int i = 0; i < len; ++i)
	{
		hash += cstr[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}


int RpgPlatformString::WStringLength(const wchar_t* wstr) noexcept
{
	return static_cast<int>(wcslen(wstr));
}


void RpgPlatformString::WStringToMultibyte(char* dst, const wchar_t* wstr, size_t maxBufferCount) noexcept
{
	wcstombs(dst, wstr, maxBufferCount);
}




// ========================================================================================================================= //
// PLATFORM - CONSOLE
// ========================================================================================================================= //
#define RPG_WINDOWS_CONSOLE_DEFAULT_ATTRIBUTES		(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)


namespace RpgPlatformConsole
{
	static bool bInitialized = false;

};


void RpgPlatformConsole::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

	// Check if console already exists
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (consoleHandle == NULL || consoleHandle == INVALID_HANDLE_VALUE)
	{
		if (AllocConsole())
		{
			consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		}
	}

	// Double check console handle
	if (consoleHandle == NULL || consoleHandle == INVALID_HANDLE_VALUE)
	{
		OutputDebugStringA("ERROR: Initialize console failed!\n");
		return;
	}

	HWND consoleWindow = GetConsoleWindow();
	if (consoleWindow == NULL || consoleWindow == INVALID_HANDLE_VALUE)
	{
		OutputDebugStringA("ERROR: Fail to get console window!\n");
		return;
	}

	SetWindowPos(consoleWindow, NULL, 16, 16, 1280, 480, SWP_SHOWWINDOW);

	bInitialized = true;
}


void RpgPlatformConsole::OutputMessage(const char* message, int messageLength, EOutputColor color) noexcept
{
	if (!bInitialized || message == nullptr || messageLength == 0)
	{
		return;
	}

	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (consoleHandle == NULL || consoleHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	WORD attributes = RPG_WINDOWS_CONSOLE_DEFAULT_ATTRIBUTES;

	switch (color)
	{
		case OUTPUT_COLOR_GREEN: attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
		case OUTPUT_COLOR_YELLOW: attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
		case OUTPUT_COLOR_RED: attributes = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
		default: break;
	}

	SetConsoleTextAttribute(consoleHandle, attributes);
	WriteConsoleA(consoleHandle, message, static_cast<DWORD>(messageLength), NULL, NULL);
	SetConsoleTextAttribute(consoleHandle, RPG_WINDOWS_CONSOLE_DEFAULT_ATTRIBUTES);
}




// ========================================================================================================================= //
// PLATFORM - LOG
// ========================================================================================================================= //
namespace RpgPlatformLog
{
	static EVerbosity GlobalVerbosity;
	static HANDLE OutputFileHandle;
	static CRITICAL_SECTION OutputFileCS;
	static bool bInitialized;
};


void RpgPlatformLog::Initialize(EVerbosity in_GlobalVerbosity, const char* opt_OutputLogFilePath) noexcept
{
	if (bInitialized)
	{
		return;
	}

	GlobalVerbosity = in_GlobalVerbosity;

	if (opt_OutputLogFilePath)
	{
		OutputFileHandle = CreateFileA(opt_OutputLogFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		InitializeCriticalSection(&OutputFileCS);
	}

	bInitialized = true;
}


void RpgPlatformLog::Shutdown() noexcept
{
	if (!bInitialized)
	{
		return;
	}

	if (OutputFileHandle)
	{
		CloseHandle(OutputFileHandle);
		OutputFileHandle = NULL;

		DeleteCriticalSection(&OutputFileCS);
	}

	bInitialized = false;
}


void RpgPlatformLog::SetGlobalVerbosity(EVerbosity in_Verbosity) noexcept
{
	GlobalVerbosity = in_Verbosity;
}


RpgPlatformLog::EVerbosity RpgPlatformLog::GetGlobalVerbosity() noexcept
{
	return GlobalVerbosity;
}


void RpgPlatformLog::OutputMessage(RpgPlatformConsole::EOutputColor consoleOutputColor, const char* message) noexcept
{
	const int len = RpgPlatformString::CStringLength(message);
	if (len == 0)
	{
		return;
	}

	OutputDebugString(message);

	EnterCriticalSection(&OutputFileCS);

	RpgPlatformConsole::OutputMessage(message, len, consoleOutputColor);

	if (OutputFileHandle && OutputFileHandle != INVALID_HANDLE_VALUE)
	{
		WriteFile(OutputFileHandle, message, len, NULL, NULL);
	}

	LeaveCriticalSection(&OutputFileCS);
}


void RpgPlatformLog::OutputMessageFormat(RpgPlatformConsole::EOutputColor consoleOutputColor, const char* format, ...) noexcept
{
	char message[RPG_LOG_MAX_OUTPUT_BUFFER]{};
	
	va_list args;
	va_start(args, format);
	vsnprintf(message, RPG_LOG_MAX_OUTPUT_BUFFER, format, args);
	va_end(args);

	OutputMessage(consoleOutputColor, message);
}


void RpgPlatformLog::OutputMessageLogCategoryFormat(const FCategory& category, EVerbosity verbosity, const char* format, ...) noexcept
{
	if (verbosity < category.Verbosity || category.Verbosity < GlobalVerbosity)
	{
		return;
	}

	char message[RPG_LOG_CATEGORY_MAX_FORMAT]{};

	va_list args;
	va_start(args, format);
	vsnprintf(message, RPG_LOG_CATEGORY_MAX_FORMAT, format, args);
	va_end(args);

	RpgPlatformConsole::EOutputColor consoleOutputColor = RpgPlatformConsole::OUTPUT_COLOR_DEFAULT;

	switch (verbosity)
	{
		case VERBOSITY_DEBUG:
		{
			consoleOutputColor = RpgPlatformConsole::OUTPUT_COLOR_GREEN;
			break;
		}

		case VERBOSITY_LOG:
		{
			consoleOutputColor = RpgPlatformConsole::OUTPUT_COLOR_DEFAULT;
			break;
		}

		case VERBOSITY_WARN:
		{
			consoleOutputColor = RpgPlatformConsole::OUTPUT_COLOR_YELLOW;
			break;
		}

		case VERBOSITY_ERROR:
		{
			consoleOutputColor = RpgPlatformConsole::OUTPUT_COLOR_RED;
			break;
		}

		default:
			break;
	}

	OutputMessageFormat(consoleOutputColor, "<%s>: %s\n", category.Name, message);
}


RPG_LOG_DEFINE_CATEGORY(RpgLogTemp, VERBOSITY_DEBUG)
RPG_LOG_DEFINE_CATEGORY(RpgLogSystem, VERBOSITY_LOG)





// ========================================================================================================================= //
// PLATFORM - PROCESS
// ========================================================================================================================= //
static void Rpg_MimallocOutput(const char* msg, void* arg)
{
	RpgPlatformLog::OutputMessage(RpgPlatformConsole::OUTPUT_COLOR_GREEN, msg);
}



namespace RpgPlatformProcess
{
	static uint32_t MainThreadId;
	static HWND MainWindowHandle;
	static bool bInitialized;
};


void RpgPlatformProcess::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

	RPG_Log(RpgLogSystem, "Initialize platform process [WINDOWS]");

	mi_option_enable(mi_option_allow_large_os_pages);


#ifndef RPG_BUILD_SHIPPING
	mi_register_output(Rpg_MimallocOutput, nullptr);
	mi_version();
	mi_stats_reset();
#endif // !RPG_BUILD_SHIPPING


	MainThreadId = GetCurrentThreadId();
	SetThreadAffinityMask(GetCurrentThread(), 0);

	bInitialized = true;
}


void RpgPlatformProcess::Shutdown() noexcept
{
	if (!bInitialized)
	{
		return;
	}

	RPG_Log(RpgLogSystem, "Shutdown platform process [WINDOWS]");

	bInitialized = false;
}


void RpgPlatformProcess::Exit(uint32_t code) noexcept
{
	ExitProcess(code);
}


void RpgPlatformProcess::ShowMessageBoxError(const char* title, const char* message) noexcept
{
	MessageBoxA(IsMainThread() ? MainWindowHandle : NULL, message, title, MB_ICONERROR | MB_OK);
}


void RpgPlatformProcess::SetMainWindowHandle(HWND handle) noexcept
{
	RPG_Assert(handle);
	MainWindowHandle = handle;
}


HWND RpgPlatformProcess::GetMainWindowHandle() noexcept
{
	return MainWindowHandle;
}


uint32_t RpgPlatformProcess::GetMainThreadId() noexcept
{
	return MainThreadId;
}


bool RpgPlatformProcess::IsMainThread() noexcept
{
	return GetCurrentThreadId() == MainThreadId;
}




// ========================================================================================================================= //
// PLATFORM - FILE
// ========================================================================================================================= //
bool RpgPlatformFile::FolderExists(const char* folderPath) noexcept
{
	const int len = RpgPlatformString::CStringLength(folderPath);
	if (len == 0)
	{
		return false;
	}

	const DWORD dwAttrib = GetFileAttributes(folderPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


bool RpgPlatformFile::FolderCreate(const char* folderPath) noexcept
{
	const int len = RpgPlatformString::CStringLength(folderPath);
	if (len == 0)
	{
		return false;
	}

	if (FolderExists(folderPath))
	{
		return true;
	}

	const int error = CreateDirectory(folderPath, NULL);
	return error != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
}


bool RpgPlatformFile::FolderDelete(const char* folderPath) noexcept
{
	const int len = RpgPlatformString::CStringLength(folderPath);
	if (len == 0)
	{
		return false;
	}

	if (!FolderExists(folderPath))
	{
		return false;
	}

	char search[MAX_PATH];
	snprintf(search, MAX_PATH, "%s/*", folderPath);

	WIN32_FIND_DATA fileData{};
	HANDLE fileHandle = FindFirstFile(search, &fileData);

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			char fileName[MAX_PATH];
			RpgPlatformMemory::MemZero(fileName, MAX_PATH);
			strcpy(fileName, fileData.cFileName);

			if (fileName[0] == '.' || fileName[1] == '.')
			{
				continue;
			}

			char filePath[MAX_PATH];
			RpgPlatformMemory::MemZero(filePath, MAX_PATH);
			snprintf(filePath, MAX_PATH, "%s/%s", folderPath, fileName);

			if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				FolderDelete(filePath);
				RemoveDirectory(filePath);
			}
			else
			{
				DeleteFile(filePath);
			}
		}
		while (FindNextFile(fileHandle, &fileData));

		RemoveDirectory(folderPath);
	}

	FindClose(fileHandle);

	return true;
}


bool RpgPlatformFile::FileExists(const char* absoluteFilePath) noexcept
{
	const int len = RpgPlatformString::CStringLength(absoluteFilePath);
	if (len == 0)
	{
		return false;
	}

	RPG_Validate(len <= MAX_PATH);
	const DWORD dwAttrib = GetFileAttributes(absoluteFilePath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


HANDLE RpgPlatformFile::FileOpen(const char* absoluteFilePath, EOpenMode mode) noexcept
{
	const int len = RpgPlatformString::CStringLength(absoluteFilePath);
	if (len == 0)
	{
		return NULL;
	}

	DWORD desiredAccess = 0;
	DWORD createDisposition = 0;

	if (mode == OPEN_MODE_READ)
	{
		desiredAccess = GENERIC_READ;
		createDisposition = OPEN_ALWAYS;

	}
	else if (mode == OPEN_MODE_WRITE_OVERWRITE)
	{
		desiredAccess = GENERIC_WRITE;
		createDisposition = CREATE_ALWAYS;
	}
	else // mode == OPEN_MODE_WRITE_APPEND
	{
		desiredAccess = FILE_APPEND_DATA;
		createDisposition = OPEN_ALWAYS;
	}

	HANDLE handle = CreateFileA(absoluteFilePath, desiredAccess, FILE_SHARE_READ, NULL, createDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
	const DWORD error = GetLastError();

	if (handle == INVALID_HANDLE_VALUE)
	{
		if (error == ERROR_SHARING_VIOLATION)
		{
			RPG_LogError(RpgLogSystem, "Open file [%s] failed. Error sharing violation!\n", absoluteFilePath);
		}
		else if (error == ERROR_PATH_NOT_FOUND)
		{
			RPG_LogError(RpgLogSystem, "Open file [%s] failed. Error path not found!\n", absoluteFilePath);
		}
	}

	return handle;
}


size_t RpgPlatformFile::FileGetSize(HANDLE fileHandle) noexcept
{
	if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
	{
		RPG_LogError(RpgLogSystem, "Fail to get file size. Invalid file handle!");
		return 0;
	}

	return GetFileSize(fileHandle, NULL);
}


bool RpgPlatformFile::FileSeek(HANDLE fileHandle, size_t byteOffset, ESeekMode mode) noexcept
{
	if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
	{
		RPG_LogError(RpgLogSystem, "Fail to set file pointer (seek). Invalid file handle!\n");
		return false;
	}

	DWORD seekWord = FILE_BEGIN;

	if (mode == SEEK_MODE_CURRENT)
	{
		seekWord = FILE_CURRENT;
	}
	else if (mode == SEEK_MODE_END)
	{
		seekWord = FILE_END;
	}

	return SetFilePointer(fileHandle, static_cast<LONG>(byteOffset), NULL, seekWord) != INVALID_SET_FILE_POINTER;
}


bool RpgPlatformFile::FileRead(HANDLE fileHandle, void* outData, size_t sizeBytes) noexcept
{
	if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
	{
		RPG_LogError(RpgLogSystem, "Fail to read file. Invalid file handle!\n");
		return FALSE;
	}

	return ReadFile(fileHandle, outData, static_cast<DWORD>(sizeBytes), NULL, NULL);
}


bool RpgPlatformFile::FileWrite(HANDLE fileHandle, const void* data, size_t dataSizeBytes) noexcept
{
	if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
	{
		RPG_LogError(RpgLogSystem, "Fail write to file. Invalid file handle!\n");
		return false;
	}

	return WriteFile(fileHandle, data, static_cast<DWORD>(dataSizeBytes), NULL, NULL);
}


void RpgPlatformFile::FileClose(HANDLE& fileHandle) noexcept
{
	if (fileHandle && fileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(fileHandle);
		fileHandle = NULL;
	}
}


bool RpgPlatformFile::FileDelete(const char* absoluteFilePath) noexcept
{
	return DeleteFile(absoluteFilePath);
}




// =============================================================================================================================================================== //
// PLATFORM - MOUSE
// =============================================================================================================================================================== //
namespace RpgPlatformMouse
{
	const LPSTR IDC_CURSORS[] =
	{
		IDC_ARROW,		// CURSOR_MODE_ARROW,
		IDC_IBEAM,		// CURSOR_MODE_IBEAM,
		IDC_WAIT,		// CURSOR_MODE_BUSY,
		IDC_HAND,		// CURSOR_MODE_HAND,
		IDC_SIZEALL,	// CURSOR_MODE_MOVE,
		IDC_SIZEWE,		// CURSOR_MODE_RESIZE_HORIZONTAL,
		IDC_SIZENS,		// CURSOR_MODE_RESIZE_VERTICAL,
		IDC_SIZENWSE,	// CURSOR_MODE_RESIZE_DIAGONAL_1,
		IDC_SIZENESW,	// CURSOR_MODE_RESIZE_DIAGONAL_2,
	};
	static_assert(sizeof(IDC_CURSORS) / sizeof(LPSTR) == CURSOR_MODE_MAX_COUNT, "Enum count not equals!");


	static HCURSOR DefaultCursors[CURSOR_MODE_MAX_COUNT];
	static RpgPointInt RelativeModeCursorPosition;
	static bool bIsRelativeMode;

};


void RpgPlatformMouse::Capture(HWND windowHandle, bool bCapture) noexcept
{
	if (bCapture)
	{
		SetCapture(windowHandle);
	}
	else
	{
		ReleaseCapture();
	}
}


void RpgPlatformMouse::SetCursorMode(ECursorMode mode) noexcept
{
	if (mode < CURSOR_MODE_ARROW || mode >= CURSOR_MODE_MAX_COUNT)
	{
		return;
	}

	if (DefaultCursors[mode] == NULL)
	{
		DefaultCursors[mode] = LoadCursorA(NULL, IDC_CURSORS[mode]);
	}

	SetCursor(DefaultCursors[mode]);
}


void RpgPlatformMouse::SetCursorPosition(HWND windowHandle, RpgPointInt position) noexcept
{
	POINT cursorPos{ position.X, position.Y };
	ClientToScreen(windowHandle, &cursorPos);

	SetCursorPos(cursorPos.x, cursorPos.y);
}


RpgPointInt RpgPlatformMouse::GetCursorPosition(HWND windowHandle) noexcept
{
	POINT mouseCursorPosition{};
	GetCursorPos(&mouseCursorPosition);
	ScreenToClient(windowHandle ? windowHandle : RpgPlatformProcess::GetMainWindowHandle(), &mouseCursorPosition);

	return RpgPointInt(mouseCursorPosition.x, mouseCursorPosition.y);
}


void RpgPlatformMouse::SetEnableClipCursor(HWND windowHandle, RpgRectInt rect) noexcept
{
	POINT clipPos{ rect.Left, rect.Top };
	ClientToScreen(windowHandle ? windowHandle : RpgPlatformProcess::GetMainWindowHandle(), &clipPos);

	RECT clipRect;
	clipRect.left = clipPos.x;
	clipRect.top = clipPos.y;
	clipRect.right = clipRect.left + rect.GetWidth();
	clipRect.bottom = clipRect.top + rect.GetHeight();

	ClipCursor(&clipRect);
}


void RpgPlatformMouse::SetDisableClipCursor() noexcept
{
	ClipCursor(nullptr);
}


void RpgPlatformMouse::SetCursorHidden(bool bHidden) noexcept
{
	ShowCursor(!bHidden);
}


void RpgPlatformMouse::SetEnableRelativeMode(HWND windowHandle, bool bEnable) noexcept
{
	if (bIsRelativeMode == bEnable)
	{
		return;
	}

	bIsRelativeMode = bEnable;

	if (bIsRelativeMode)
	{
		RpgPlatformMouse::Capture(windowHandle, true);
		RpgPlatformMouse::SetCursorHidden(true);
	}
	else
	{
		RpgPlatformMouse::SetCursorHidden(false);
		RpgPlatformMouse::Capture(windowHandle, false);
	}
}
