#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <utility>


#define RPG_INDEX_INVALID			-1
#define RPG_INDEX_FIRST				0
#define RPG_INDEX_LAST				(INT32_MAX - 1)
#define RPG_MAX_COUNT				INT32_MAX

#define RPG_MEMORY_SIZE_KiB(n)		(n * 1024)
#define RPG_MEMORY_SIZE_MiB(n)		(RPG_MEMORY_SIZE_KiB(n) * 1024)
#define RPG_MEMORY_SIZE_GiB(n)		(RPG_MEMORY_SIZE_MiB(n) * 1024)

#define RPG_FRAME_BUFFERING			3



#define RPG_NOCOPY(type)					\
private:									\
	type(const type&) = delete;				\
	type& operator=(const type&) = delete;	


#define RPG_NOCOPYMOVE(type)				\
private:									\
	type(const type&) = delete;				\
	type(type&&) = delete;					\
	type& operator=(const type&) = delete;	\
	type& operator=(type&&) = delete;


#define RPG_SINGLETON(type)							\
RPG_NOCOPYMOVE(type)								\
type() noexcept;									\
public:												\
[[nodiscard]] static inline type& Get() noexcept	\
{													\
	static type s_instance;							\
	return s_instance;								\
}




class RpgType
{
public:
	constexpr RpgType(const char* in_Name, uint32_t in_SizeBytes, const RpgType* in_ArrayType, const RpgType* in_PointerType, bool in_bIsIntegral, bool in_bIsFloat, bool in_bIsString) noexcept
		: Name(in_Name)
		, SizeBytes(in_SizeBytes)
		, ArrayType(in_ArrayType)
		, PointerType(in_PointerType)
		, bIsIntegral(in_bIsIntegral)
		, bIsFloat(in_bIsFloat)
		, bIsString(in_bIsString)
	{
	}


	constexpr RpgType(const char* in_Name, uint32_t in_SizeBytes) noexcept
		: Name(in_Name)
		, SizeBytes(in_SizeBytes)
		, ArrayType(nullptr)
		, PointerType(nullptr)
		, bIsIntegral(false)
		, bIsFloat(false)
		, bIsString(false)
	{
	}


public:
	constexpr inline bool operator==(const RpgType& rhs) const noexcept
	{
		return Name == rhs.Name && SizeBytes == rhs.SizeBytes && ArrayType == rhs.ArrayType && PointerType == rhs.PointerType &&
			bIsIntegral == rhs.bIsIntegral && bIsFloat == rhs.bIsFloat && bIsString == rhs.bIsString;
	}


	constexpr inline bool operator!=(const RpgType& rhs) const noexcept
	{
		return !(*this == rhs);
	}


public:
	constexpr inline const char* GetName() const noexcept
	{
		return Name;
	}


	constexpr inline uint32_t GetSizeBytes() const noexcept
	{
		return SizeBytes;
	}


	constexpr inline const RpgType* GetArrayType() const noexcept
	{
		return ArrayType;
	}


	constexpr inline const RpgType* GetPointerType() const noexcept
	{
		return PointerType;
	}


	constexpr inline bool IsIntegral() const noexcept
	{
		return bIsIntegral;
	}


	constexpr inline bool IsFloat() const noexcept
	{
		return bIsFloat;
	}


	constexpr inline bool IsString() const noexcept
	{
		return bIsString;
	}


	constexpr inline bool IsArray() const noexcept
	{
		return ArrayType != nullptr;
	}


	constexpr inline bool IsPointer() const noexcept
	{
		return PointerType != nullptr;
	}


	constexpr inline bool IsNumeric() const noexcept
	{
		return bIsIntegral || bIsFloat;
	}


private:
	const char* Name;
	uint32_t SizeBytes;
	const RpgType* ArrayType;
	const RpgType* PointerType;
	bool bIsIntegral;
	bool bIsFloat;
	bool bIsString;

};



namespace RpgTypeTraits
{
	template<typename T> struct IsIntegral { static constexpr bool Value = false; };
	template<> struct IsIntegral<int8_t> { static constexpr bool Value = true; };
	template<> struct IsIntegral<int16_t> { static constexpr bool Value = true; };
	template<> struct IsIntegral<int32_t> { static constexpr bool Value = true; };
	template<> struct IsIntegral<int64_t> { static constexpr bool Value = true; };
	template<> struct IsIntegral<uint8_t> { static constexpr bool Value = true; };
	template<> struct IsIntegral<uint16_t> { static constexpr bool Value = true; };
	template<> struct IsIntegral<uint32_t> { static constexpr bool Value = true; };
	template<> struct IsIntegral<uint64_t> { static constexpr bool Value = true; };


	template<typename T> struct IsFloat { static constexpr bool Value = false; };
	template<> struct IsFloat<float> { static constexpr bool Value = true; };
	template<> struct IsFloat<double> { static constexpr bool Value = true; };


	// A type is arithmetic if type is integral or float
	template<typename T>
	struct IsArithmetic
	{
		static constexpr bool Value = IsIntegral<T>::Value || IsFloat<T>::Value;
	};

}; // RpgTypeTraits





template<typename T>
class RpgPoint
{
	static_assert(RpgTypeTraits::IsArithmetic<T>::Value, "RpgPoint type of <T> must be arithmetic type!");

public:
	T X;
	T Y;


public:
	RpgPoint() noexcept
		: X(0), Y(0)
	{
	}

	RpgPoint(T in_Value) noexcept
		: X(in_Value), Y(in_Value)
	{
	}

	RpgPoint(T in_X, T in_Y) noexcept
		: X(in_X), Y(in_Y)
	{
	}

	template<typename U>
	explicit RpgPoint(const RpgPoint<U>& other) noexcept
	{
		static_assert(RpgTypeTraits::IsArithmetic<T>::Value, "RpgPoint type of <U> must be arithmetic type!");

		X = static_cast<T>(other.X);
		Y = static_cast<T>(other.Y);
	}

public:
	inline RpgPoint operator+(const RpgPoint& rhs) const noexcept
	{
		return RpgPoint(X + rhs.X, Y + rhs.Y);
	}

	inline RpgPoint& operator+=(const RpgPoint& rhs) noexcept
	{
		X += rhs.X;
		Y += rhs.Y;
		return *this;
	}

	inline RpgPoint operator-(const RpgPoint& rhs) const noexcept
	{
		return RpgPoint(X - rhs.X, Y - rhs.Y);
	}

	inline RpgPoint& operator-=(const RpgPoint& rhs) noexcept
	{
		X -= rhs.X;
		Y -= rhs.Y;
		return *this;
	}

	inline RpgPoint operator*(T rhs) const noexcept
	{
		return RpgPoint(X * rhs, Y * rhs);
	}

	inline RpgPoint& operator*=(T rhs) noexcept
	{
		X *= rhs;
		Y *= rhs;
		return *this;
	}

	inline RpgPoint operator/(T rhs) const noexcept
	{
		return RpgPoint(X / rhs, Y / rhs);
	}

	inline RpgPoint& operator/=(T rhs) noexcept
	{
		X /= rhs;
		Y /= rhs;
		return *this;
	}

	inline bool operator==(const RpgPoint& rhs) const noexcept
	{
		return X == rhs.X && Y == rhs.Y;
	}

	inline bool operator!=(const RpgPoint& rhs) const noexcept
	{
		return !(*this == rhs);
	}

};


typedef RpgPoint<int> RpgPointInt;
typedef RpgPoint<float> RpgPointFloat;




template<typename T>
class RpgRect
{
	static_assert(RpgTypeTraits::IsArithmetic<T>::Value, "RpgRect type of <T> must be arithmetic type!");

public:
	T Left;
	T Top;
	T Right;
	T Bottom;


public:
	RpgRect() noexcept
		: Left(0), Top(0), Right(0), Bottom(0)
	{
	}

	RpgRect(T in_Value) noexcept
		: Left(in_Value), Top(in_Value), Right(in_Value), Bottom(in_Value)
	{
	}

	RpgRect(T in_Left, T in_Top, T in_Right, T in_Bottom) noexcept
		: Left(in_Left), Top(in_Top), Right(in_Right), Bottom(in_Bottom)
	{
	}


	template<typename U>
	explicit RpgRect(const RpgRect<U>& other) noexcept
	{
		static_assert(RpgTypeTraits::IsArithmetic<T>::Value, "RpgRect type of <U> must be arithmetic type!");

		Left = static_cast<T>(other.Left);
		Top = static_cast<T>(other.Top);
		Right = static_cast<T>(other.Right);
		Bottom = static_cast<T>(other.Bottom);
	}


public:
	inline bool operator==(const RpgRect& rhs) const noexcept
	{
		return Left == rhs.Left && Top == rhs.Top && Right == rhs.Right && Bottom == rhs.Bottom;
	}

	inline bool operator!=(const RpgRect& rhs) const noexcept
	{
		return !(*this == rhs);
	}


public:
	inline RpgPoint<T> GetPosition() const noexcept
	{
		return RpgPoint<T>(Left, Top);
	}

	inline T GetWidth() const noexcept
	{
		return Right - Left;
	}

	inline T GetHeight() const noexcept
	{
		return Bottom - Top;
	}

	inline RpgPoint<T> GetDimension() const noexcept
	{
		return RpgPoint<T>(Right - Left, Bottom - Top);
	}

	inline bool IsPointIntersect(const RpgPoint<T>& p) const noexcept
	{
		return (p.X >= Left && p.X <= Right && p.Y >= Top && p.Y <= Bottom);
	}

	inline bool IsPointInside(const RpgPoint<T>& p, int margin = 0) const noexcept
	{
		return (p.X + margin > Left) && (p.X - margin < Right) && (p.Y + margin > Top) && (p.Y - margin < Bottom);
	}

	inline bool IsRectIntersect(const RpgRect& r) const noexcept
	{
		return !(r.Right < Left || r.Left > Right || r.Top > Bottom || r.Bottom < Top);
	}

	inline bool IsRectInside(const RpgRect& r, T margin = 0) const noexcept
	{
		return (r.Left + margin > Left) && (r.Top + margin > Top) && (r.Right - margin < Right) && (r.Bottom - margin < Bottom);
	}

};


typedef RpgRect<int> RpgRectInt;
typedef RpgRect<float> RpgRectFloat;




class RpgRectBorders
{
public:
	enum EBorder : uint8_t
	{
		NONE = 0,
		LEFT_TOP,
		RIGHT_TOP,
		LEFT_BOTTOM,
		RIGHT_BOTTOM,
		LEFT,
		RIGHT,
		TOP,
		BOTTOM,
		MAX_COUNT
	};

	RpgRectFloat BorderRects[MAX_COUNT];
	float BorderThickness;
	float SpaceBetweenBorder;


public:
	RpgRectBorders() noexcept
		: BorderThickness(2.0f)
		, SpaceBetweenBorder(0.0f)
	{
	}

	RpgRectBorders(const RpgRectFloat& windowRect, float borderThickness, float spaceBetweenBorder) noexcept
		: BorderThickness(borderThickness)
		, SpaceBetweenBorder(spaceBetweenBorder)
	{
		UpdateRects(windowRect);
	}

	inline void UpdateRects(const RpgRectFloat& windowRect) noexcept
	{
		RpgRectFloat& borderLeftTop = BorderRects[LEFT_TOP];
		borderLeftTop.Left = windowRect.Left;
		borderLeftTop.Top = windowRect.Top;
		borderLeftTop.Right = borderLeftTop.Left + BorderThickness;
		borderLeftTop.Bottom = borderLeftTop.Top + BorderThickness;

		RpgRectFloat& borderRightTop = BorderRects[RIGHT_TOP];
		borderRightTop.Right = windowRect.Right;
		borderRightTop.Top = windowRect.Top;
		borderRightTop.Left = borderRightTop.Right - BorderThickness;
		borderRightTop.Bottom = borderRightTop.Top + BorderThickness;

		RpgRectFloat& borderLeftBottom = BorderRects[LEFT_BOTTOM];
		borderLeftBottom.Left = windowRect.Left;
		borderLeftBottom.Bottom = windowRect.Bottom;
		borderLeftBottom.Right = borderLeftBottom.Left + BorderThickness;
		borderLeftBottom.Top = borderLeftBottom.Bottom - BorderThickness;

		RpgRectFloat& borderRightBottom = BorderRects[RIGHT_BOTTOM];
		borderRightBottom.Right = windowRect.Right;
		borderRightBottom.Bottom = windowRect.Bottom;
		borderRightBottom.Left = borderRightBottom.Right - BorderThickness;
		borderRightBottom.Top = borderRightBottom.Bottom - BorderThickness;

		RpgRectFloat& borderLeft = BorderRects[LEFT];
		borderLeft.Left = borderLeftTop.Left;
		borderLeft.Right = borderLeftTop.Right;
		borderLeft.Top = borderLeftTop.Bottom + SpaceBetweenBorder;
		borderLeft.Bottom = borderLeftBottom.Top - SpaceBetweenBorder;

		RpgRectFloat& borderRight = BorderRects[RIGHT];
		borderRight.Left = borderRightTop.Left;
		borderRight.Right = borderRightTop.Right;
		borderRight.Top = borderRightTop.Bottom + SpaceBetweenBorder;
		borderRight.Bottom = borderRightBottom.Top - SpaceBetweenBorder;

		RpgRectFloat& borderTop = BorderRects[TOP];
		borderTop.Top = borderLeftTop.Top;
		borderTop.Bottom = borderLeftTop.Bottom;
		borderTop.Left = borderLeftTop.Right + SpaceBetweenBorder;
		borderTop.Right = borderRightTop.Left - SpaceBetweenBorder;

		RpgRectFloat& borderBottom = BorderRects[BOTTOM];
		borderBottom.Top = borderLeftBottom.Top;
		borderBottom.Bottom = borderLeftBottom.Bottom;
		borderBottom.Left = borderLeftBottom.Right + SpaceBetweenBorder;
		borderBottom.Right = borderRightBottom.Left - SpaceBetweenBorder;
	}


	// Get rect area inside border
	inline RpgRectFloat GetInnerRect() const noexcept
	{
		return RpgRectFloat(
			BorderRects[RpgRectBorders::LEFT].Right,
			BorderRects[RpgRectBorders::TOP].Bottom,
			BorderRects[RpgRectBorders::RIGHT].Left,
			BorderRects[RpgRectBorders::BOTTOM].Top
		);
	}


	inline EBorder TestIntersectBorder(const RpgPointFloat& p) const noexcept
	{
		for (int i = 1; i < MAX_COUNT; ++i)
		{
			if (BorderRects[i].IsPointIntersect(p))
			{
				return static_cast<EBorder>(i);
			}
		}

		return NONE;
	}

};



class RpgColor
{
public:
	uint8_t R, G, B, A;

	static const RpgColor BLACK;
	static const RpgColor BLACK_TRANSPARENT;
	static const RpgColor BLUE;
	static const RpgColor GREEN;
	static const RpgColor RED;
	static const RpgColor WHITE;
	static const RpgColor WHITE_TRANSPARENT;
	static const RpgColor YELLOW;


public:
	RpgColor() noexcept
		: R(0), G(0), B(0), A(0)
	{
	}

	RpgColor(uint8_t in_R, uint8_t in_G, uint8_t in_B, uint8_t in_A = 255) noexcept
		: R(in_R), G(in_G), B(in_B), A(in_A)
	{
	}

	RpgColor(uint32_t rgba) noexcept
	{
		R = (rgba & 0x000000FF);
		G = (rgba & 0x0000FF00) >> 8;
		B = (rgba & 0x00FF0000) >> 16;
		A = (rgba & 0xFF000000) >> 24;
	}

};



class RpgColorLinear
{
public:
	float R, G, B, A;

	static const RpgColorLinear BLACK;
	static const RpgColorLinear BLACK_TRANSPARENT;
	static const RpgColorLinear BLUE;
	static const RpgColorLinear GREEN;
	static const RpgColorLinear RED;
	static const RpgColorLinear WHITE;
	static const RpgColorLinear WHITE_TRANSPARENT;
	static const RpgColorLinear YELLOW;


public:
	RpgColorLinear() noexcept
		: R(0), G(0), B(0), A(0)
	{
	}

	RpgColorLinear(float in_R, float in_G, float in_B, float in_A = 1.0f) noexcept
		: R(in_R), G(in_G), B(in_B), A(in_A)
	{
	}

	explicit RpgColorLinear(const RpgColor& rgba) noexcept
	{
		R = rgba.R / 255.0f;
		G = rgba.G / 255.0f;
		B = rgba.B / 255.0f;
		A = rgba.A / 255.0f;
	}


public:
	inline void Saturate() noexcept
	{
		R = R < 0.0f ? 0.0f : R > 1.0f ? 1.0f : R;
		G = G < 0.0f ? 0.0f : G > 1.0f ? 1.0f : G;
		B = B < 0.0f ? 0.0f : B > 1.0f ? 1.0f : B;
		A = A < 0.0f ? 0.0f : A > 1.0f ? 1.0f : A;
	}

};



namespace Rpg
{
	template<typename T>
	inline uint64_t GetHash(const T& value) noexcept
	{
		return 0;
	}


	template<typename T>
	constexpr inline void BitSetCondition(T& out_Flags, T bitFlags, bool bCondition) noexcept
	{
		static_assert(RpgTypeTraits::IsIntegral<T>::Value, "Rpg::BitSetCondition type of <T> must be integral type!");
		out_Flags = (static_cast<T>(out_Flags) & ~static_cast<T>(bitFlags)) | (-static_cast<T>(bCondition) & static_cast<T>(bitFlags));
	}


	template<typename T>
	constexpr inline bool IsPowerOfTwo(T value) noexcept
	{
		static_assert(RpgTypeTraits::IsIntegral<T>::Value, "RpgAlgorithm IsPowerOfTwo type of <T> must be integral type!");

		return (value > 0) && !(value & (value - 1));
	}


	constexpr inline uint32_t Align(uint32_t offset, uint32_t alignment) noexcept
	{
		return (offset + (alignment - 1) & ~(alignment - 1));
	}

	constexpr inline int Align(int offset, int alignment) noexcept
	{
		return (offset + (alignment - 1) & ~(alignment - 1));
	}


	template<typename T>
	struct Type
	{
		static constexpr RpgType Value = RpgType("void", 0);
	};

	template<typename T>
	struct Type<T*>
	{
		static constexpr RpgType Value = RpgType("pointer", sizeof(T*), nullptr, &Type<T>::Value, false, false, false);
	};

	template<>
	struct Type<bool>
	{
		static constexpr RpgType Value = RpgType("bool", sizeof(bool));
	};

	template<>
	struct Type<float>
	{
		static constexpr RpgType Value = RpgType("float", sizeof(float), nullptr, nullptr, false, true, false);
	};

	template<>
	struct Type<double>
	{
		static constexpr RpgType Value = RpgType("double", sizeof(double), nullptr, nullptr, false, true, false);
	};

	template<>
	struct Type<int8_t>
	{
		static constexpr RpgType Value = RpgType("int8_t", sizeof(int8_t), nullptr, nullptr, true, false, false);
	};

	template<>
	struct Type<int16_t>
	{
		static constexpr RpgType Value = RpgType("int16_t", sizeof(int16_t), nullptr, nullptr, true, false, false);
	};

	template<>
	struct Type<int32_t>
	{
		static constexpr RpgType Value = RpgType("int32_t", sizeof(int32_t), nullptr, nullptr, true, false, false);
	};

	template<>
	struct Type<int64_t>
	{
		static constexpr RpgType Value = RpgType("int64_t", sizeof(int64_t), nullptr, nullptr, true, false, false);
	};

	template<>
	struct Type<uint8_t>
	{
		static constexpr RpgType Value = RpgType("uint8_t", sizeof(uint8_t), nullptr, nullptr, true, false, false);
	};

	template<>
	struct Type<uint16_t>
	{
		static constexpr RpgType Value = RpgType("uint16_t", sizeof(uint16_t), nullptr, nullptr, true, false, false);
	};

	template<>
	struct Type<uint32_t>
	{
		static constexpr RpgType Value = RpgType("uint32_t", sizeof(uint32_t), nullptr, nullptr, true, false, false);
	};

	template<>
	struct Type<uint64_t>
	{
		static constexpr RpgType Value = RpgType("uint64_t", sizeof(uint64_t), nullptr, nullptr, true, false, false);
	};

	template<>
	struct Type<RpgColor>
	{
		static constexpr RpgType Value = RpgType("RpgColor", sizeof(RpgColor));
	};

	template<>
	struct Type<RpgColorLinear>
	{
		static constexpr RpgType Value = RpgType("RpgColorLinear", sizeof(RpgColorLinear));
	};

}; // Rpg



enum class RpgAxis : uint8_t
{
	X_AXIS = 0,
	Y_AXIS,
	Z_AXIS
};
