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
	inline uint32_t GetSizeBytes(const T& data) const noexcept
	{
		static_assert(std::is_trivially_copyable<T>::value, "RpgStreamWriter::GetSizeBytes type of <T> must trivially copyable!");
		return sizeof(T);
	}


	template<typename T>
	inline void Write(const T& rhs) noexcept
	{
		static_assert(std::is_trivially_copyable<T>::value, "RpgStreamWriter::Write type of <T> must trivially copyable!");
		WriteData(&rhs, sizeof(T));
	}


	template<typename T, int N>
	inline uint32_t GetSizeBytes(const RpgArray<T, N>& data) const noexcept
	{
		// count
		uint32_t sizeBytes = sizeof(int);

		// data
		for (int i = 0; i < data.GetCount(); ++i)
		{
			sizeBytes += GetSizeBytes(data[i]);
		}

		return sizeBytes;
	}

	template<typename T, int N>
	inline void Write(const RpgArray<T, N>& dataArray) noexcept
	{
		Write(dataArray.GetCount());

		for (int i = 0; i < dataArray.GetCount(); ++i)
		{
			Write(dataArray[i]);
		}
	}


	template<typename T, int N>
	inline uint32_t GetSizeBytes(const RpgArrayInline<T, N>& data) const noexcept
	{
		// count
		uint32_t sizeBytes = sizeof(int);

		// data
		for (int i = 0; i < data.GetCount(); ++i)
		{
			sizeBytes += GetSizeBytes(data[i]);
		}

		return sizeBytes;
	}

	template<typename T, int N>
	inline void Write(const RpgArrayInline<T, N>& dataArray) noexcept
	{
		Write(dataArray.GetCount());

		for (int i = 0; i < dataArray.GetCount(); ++i)
		{
			Write(dataArray[i]);
		}
	}


	inline uint32_t GetSizeBytes(const RpgString& data) const noexcept
	{
		// length
		uint32_t sizeBytes = sizeof(int);
		
		// data
		sizeBytes += data.GetLength();

		return sizeBytes;
	}

	inline void Write(const RpgString& str) noexcept
	{
		int length = str.GetLength();
		Write(length);

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
		static_assert(std::is_trivially_copyable<T>::value, "RpgStreamReader::Read type of <T> must trivially copyable!");
		ReadData(&data, sizeof(T));
	}


	template<typename T, int N>
	inline void Read(RpgArray<T, N>& dataArray) noexcept
	{
		int count = 0;
		Read(count);

		if (count > 0)
		{
			const int index = dataArray.GetCount();
			dataArray.Resize(index + count);

			for (int i = 0; i < count; ++i)
			{
				Read(dataArray[index + i]);
			}
		}
	}


	template<typename T, int N>
	inline void Read(RpgArrayInline<T, N>& dataArray) noexcept
	{
		int count = 0;
		Read(count);

		if (count > 0)
		{
			const int index = dataArray.GetCount();
			dataArray.Resize(index + count);

			for (int i = 0; i < count; ++i)
			{
				Read(dataArray[index + i]);
			}
		}
	}


	inline void Read(RpgString& str) noexcept
	{
		int length = 0;
		Read(length);

		if (length > 0)
		{
			RPG_Check(length < RPG_STRING_FORMAT_BUFFER_COUNT);
			str.Resize(length);
			ReadData(str.GetData(), length);
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


	inline const uint8_t* GetByteArrayData() const noexcept
	{
		return Bytes.GetData();
	}

	inline size_t GetByteArraySize() const noexcept
	{
		return Bytes.GetCount();
	}


private:
	RpgArray<uint8_t> Bytes;

};



class RpgBinaryStreamReader : public RpgStreamReader
{
public:
	RpgBinaryStreamReader(RpgArray<uint8_t>& in_Bytes) noexcept
		: Bytes(std::move(in_Bytes))
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


	inline const uint8_t* GetByteArrayData() const noexcept
	{
		return Bytes.GetData();
	}

	inline size_t GetByteArraySize() const noexcept
	{
		return Bytes.GetCount();
	}

	inline size_t GetOffset() const noexcept
	{
		return Offset;
	}


private:
	RpgArray<uint8_t> Bytes;
	size_t Offset;

};
