#pragma once

#include "RpgString.h"



class RpgStreamWriter
{
	RPG_NOCOPY(RpgStreamWriter)

public:
	RpgStreamWriter() noexcept = default;
	virtual ~RpgStreamWriter() noexcept = default;
	virtual void Reset() noexcept = 0;
	virtual void WriteData(const void* data, uint32_t dataSizeBytes) noexcept = 0;


	template<typename T>
	inline void Write(const T& rhs) noexcept
	{
		WriteData(&rhs, sizeof(T));
	}


	template<typename T, int N>
	inline void Write(const RpgArray<T, N>& dataArray) noexcept
	{
		int count = dataArray.GetCount();
		WriteData(&count, sizeof(int));

		for (int i = 0; i < dataArray.GetCount(); ++i)
		{
			WriteData(&dataArray[i], sizeof(T));
		}
	}


	template<typename T, int N>
	inline void Write(const RpgArrayInline<T, N>& dataArray) noexcept
	{
		int count = dataArray.GetCount();
		WriteData(&count, sizeof(int));

		for (int i = 0; i < dataArray.GetCount(); ++i)
		{
			WriteData(&dataArray[i], sizeof(T));
		}
	}


	inline void Write(const RpgString& str) noexcept
	{
		int length = str.GetLength();
		WriteData(&length, sizeof(int));

		if (length > 0)
		{
			WriteData(str.GetData(), length);
		}
	}

};



class RpgStreamReader
{
	RPG_NOCOPY(RpgStreamReader)

public:
	RpgStreamReader() noexcept = default;
	virtual ~RpgStreamReader() noexcept = default;
	virtual void Reset() noexcept = 0;
	virtual void ReadData(void* outData, uint32_t dataSizeBytes) noexcept = 0;


	template<typename T>
	inline void Read(T& data) noexcept
	{
		ReadData(&data, sizeof(T));
	}


	template<typename T, int N>
	inline void Read(RpgArray<T, N>& dataArray) noexcept
	{
		int count = 0;
		ReadData(&count, sizeof(int));

		if (count > 0)
		{
			const int index = dataArray.GetCount();
			dataArray.Resize(index + count);

			for (int i = 0; i < count; ++i)
			{
				ReadData(&dataArray[index + i], sizeof(T));
			}
		}
	}


	template<typename T, int N>
	inline void Read(RpgArrayInline<T, N>& dataArray) noexcept
	{
		int count = 0;
		ReadData(&count, sizeof(int));

		if (count > 0)
		{
			const int index = dataArray.GetCount();
			dataArray.Resize(index + count);

			for (int i = 0; i < count; ++i)
			{
				ReadData(&dataArray[index + i], sizeof(T));
			}
		}
	}


	inline void Read(RpgString& str) noexcept
	{
		int length = 0;
		ReadData(&length, sizeof(int));

		if (length > 0)
		{
			RPG_Check(length < RPG_STRING_FORMAT_BUFFER_COUNT);

			char temp[RPG_STRING_FORMAT_BUFFER_COUNT];
			RpgPlatformMemory::MemZero(temp, RPG_STRING_FORMAT_BUFFER_COUNT);
			ReadData(temp, length);
			temp[length] = '\0';

			str = temp;
		}
	}

};



class RpgBinaryStreamWriter : public RpgStreamWriter
{

public:
	RpgBinaryStreamWriter() noexcept
	{
		Bytes.Reserve(RPG_MEMORY_SIZE_KiB(8));
	}


	virtual void Reset() noexcept override
	{
		Bytes.Clear();
	}

	virtual void WriteData(const void* data, uint32_t dataSizeBytes) noexcept override
	{
		const uint32_t offset = Bytes.GetCount();
		Bytes.Resize(offset + dataSizeBytes);
		RpgPlatformMemory::MemCopy(Bytes.GetData() + offset, data, dataSizeBytes);
	}


	inline const uint8_t* GetByteData() const noexcept
	{
		return Bytes.GetData();
	}

	inline size_t GetByteSize() const noexcept
	{
		return Bytes.GetCount();
	}


private:
	RpgArray<uint8_t> Bytes;

};



class RpgBinaryStreamReader : public RpgStreamReader
{

public:
	RpgBinaryStreamReader(RpgArray<uint8_t>& inBytes) noexcept
		: Bytes(std::move(inBytes))
		, Offset(0)
	{
	}

	RpgBinaryStreamReader(RpgBinaryStreamReader&& other) noexcept
		: Bytes(std::move(other.Bytes))
		, Offset(other.Offset)
	{
	}


	virtual void Reset() noexcept override
	{
		Bytes.Clear();
		Offset = 0;
	}

	virtual void ReadData(void* outData, uint32_t dataSizeBytes) noexcept override
	{
		RpgPlatformMemory::MemCopy(outData, Bytes.GetData() + Offset, dataSizeBytes);
		Offset += dataSizeBytes;
	}


	inline const uint8_t* GetByteData() const noexcept
	{
		return Bytes.GetData();
	}

	inline size_t GetByteSize() const noexcept
	{
		return Bytes.GetCount();
	}


private:
	RpgArray<uint8_t> Bytes;
	size_t Offset;

};
