#pragma once

#include "RpgString.h"



class RpgFilePath
{
public:
	RpgFilePath() noexcept = default;
	~RpgFilePath() noexcept = default;


	RpgFilePath(const RpgFilePath& other) noexcept
		: FullPath(other.FullPath)
	{
		InitializeInternal();
	}

	RpgFilePath(RpgFilePath&& other) noexcept
		: FullPath(std::move(other.FullPath))
		, DirectoryPath(other.DirectoryPath)
		, DirectoryName(other.DirectoryName)
		, FileName(other.FileName)
		, FileExt(other.FileExt)
	{
		other.DirectoryPath = RpgStringView();
		other.DirectoryName = RpgStringView();
		other.FileName = RpgStringView();
		other.FileExt = RpgStringView();
	}

	RpgFilePath(const RpgString& in_Path) noexcept
		: FullPath(in_Path)
	{
		FullPath.ReplaceInPlace('\\', '/');
		InitializeInternal();
	}

	RpgFilePath(RpgString&& in_Path) noexcept
		: FullPath(std::move(in_Path))
	{
		FullPath.ReplaceInPlace('\\', '/');
		InitializeInternal();
	}


public:
	inline RpgFilePath& operator=(const RpgFilePath& rhs) noexcept
	{
		if (this != &rhs)
		{
			FullPath = rhs.FullPath;
			InitializeInternal();
		}

		return *this;
	}

	inline RpgFilePath& operator=(RpgFilePath&& rhs) noexcept
	{
		if (this != &rhs)
		{
			FullPath = std::move(rhs.FullPath);
			DirectoryPath = rhs.DirectoryPath;
			DirectoryName = rhs.DirectoryName;
			FileName = rhs.FileName;
			FileExt = rhs.FileExt;

			rhs.DirectoryPath = RpgStringView();
			rhs.DirectoryName = RpgStringView();
			rhs.FileName = RpgStringView();
			rhs.FileExt = RpgStringView();
		}

		return *this;
	}

	inline RpgFilePath& operator=(const RpgString& rhs) noexcept
	{
		FullPath = rhs;
		FullPath.ReplaceInPlace('\\', '/');
		InitializeInternal();

		return *this;
	}

	inline RpgFilePath& operator=(RpgString&& rhs) noexcept
	{
		if (&FullPath != &rhs)
		{
			FullPath = std::move(rhs);
			FullPath.ReplaceInPlace('\\', '/');
			InitializeInternal();
		}

		return *this;
	}


	inline RpgFilePath operator+(const RpgString& rhs) const noexcept
	{
		return RpgFilePath(FullPath + rhs);
	}

	inline RpgFilePath operator+(const char* rhs) const noexcept
	{
		return RpgFilePath(FullPath + rhs);
	}


	inline const char* operator*() const noexcept
	{
		return *FullPath;
	}


	inline bool operator==(const RpgFilePath& rhs) const noexcept
	{
		return FullPath.Equals(rhs.FullPath, true);
	}

	inline bool operator!=(const RpgFilePath& rhs) const noexcept
	{
		return !FullPath.Equals(rhs.FullPath, true);
	}


public:
	RpgFilePath GetParentDirectoryPath() const noexcept;
	RpgString GetDirectoryPath() const noexcept;
	RpgName GetDirectoryName() const noexcept;
	RpgName GetFileName() const noexcept;
	RpgName GetFileExtension() const noexcept;


	inline void Clear(bool bFreeMemory = false) noexcept
	{
		FullPath.Clear(bFreeMemory);
		InitializeInternal();
	}


	inline bool IsDirectoryPath() const noexcept
	{
		return !DirectoryPath.IsEmpty() && !DirectoryName.IsEmpty() && FileName.IsEmpty() && FileExt.IsEmpty();
	}

	inline bool IsFilePath() const noexcept
	{
		return !FileName.IsEmpty();
	}

	inline bool HasFileExtension() const noexcept
	{
		return !FileExt.IsEmpty();
	}

	inline const RpgString& ToString() const noexcept
	{
		return FullPath;
	}


private:
	void InitializeInternal();
	bool IsPathValid() const noexcept;


private:
	RpgString FullPath;
	RpgStringView DirectoryPath;
	RpgStringView DirectoryName;
	RpgStringView FileName;
	RpgStringView FileExt;

};



namespace RpgFileSystem
{
	extern void Initialize() noexcept;

	extern const RpgString& GetExecutableDirPath() noexcept;
	extern const RpgString& GetUserAppDataLocalDirPath() noexcept;
	extern const RpgString& GetUserTempDirPath() noexcept;
	extern const RpgString& GetProjectDirPath() noexcept;
	extern const RpgString& GetSourceDirPath() noexcept;
	extern const RpgString& GetAssetDirPath() noexcept;
	extern const RpgString& GetAssetRawDirPath() noexcept;

};
