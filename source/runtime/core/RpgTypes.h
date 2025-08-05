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




namespace RpgType
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


	/*
	// A type is trivially copyable if:
	// - It has at least one eligible copy constructor, move constructor, copy assignment operator, or move assignment operator.
	// - Every eligible copy constructor, move constructor, copy assignment operator, and move assignment operator (if any) is trivial.
	// - It has a trivial non - deleted destructor.
	template<typename T>
	struct IsTriviallyCopyable
	{
		static constexpr bool Value = __is_trivially_copyable(T);
	};


	template<typename T>
	struct IsMoveAssignable
	{
		static constexpr bool Value = __is_assignable(T&, T&&);
	};
	*/


	template<typename T>
	constexpr inline void BitSetCondition(T& out_Flags, T bitFlags, bool bCondition) noexcept
	{
		static_assert(IsIntegral<T>::Value, "RpgType::BitSetCondition type of <T> must be integral type!");
		out_Flags = (static_cast<T>(out_Flags) & ~static_cast<T>(bitFlags)) | (-static_cast<T>(bCondition) & static_cast<T>(bitFlags));
	}


	constexpr inline uint32_t Align(uint32_t offset, uint32_t alignment) noexcept
	{
		return (offset + (alignment - 1) & ~(alignment - 1));
	}

	constexpr inline int Align(int offset, int alignment) noexcept
	{
		return (offset + (alignment - 1) & ~(alignment - 1));
	}

};



[[nodiscard]] inline uint64_t Rpg_GetHash(int value) noexcept
{
	return static_cast<uint64_t>(value);
}

template<typename T>
[[nodiscard]] inline uint64_t Rpg_GetHash(T* value) noexcept
{
	return reinterpret_cast<uint64_t>(value);
}




template<typename T>
class RpgPoint
{
	static_assert(RpgType::IsArithmetic<T>::Value, "RpgPoint type of <T> must be arithmetic type!");

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
		static_assert(RpgType::IsArithmetic<T>::Value, "RpgPoint type of <U> must be arithmetic type!");

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
	static_assert(RpgType::IsArithmetic<T>::Value, "RpgRect type of <T> must be arithmetic type!");

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
		static_assert(RpgType::IsArithmetic<T>::Value, "RpgRect type of <U> must be arithmetic type!");

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



enum class RpgAxis : uint8_t
{
	X_AXIS = 0,
	Y_AXIS,
	Z_AXIS
};



namespace RpgTextureFormat
{
	enum EType : uint8_t
	{
		NONE = 0,

		TEX_2D_R,		// Uncompressed, Red8 (Ex: Font, Mask, Specular, AO, Metallic, Roughness)
		TEX_2D_RG,		// Uncompressed, Red8-Green8
		TEX_2D_RGBA,	// Uncompressed, Red8-Green8-Blue8-Alpha8 (Ex: Base color, UI)
		TEX_2D_BC3U,	// Compressed, Red-Green-Blue-(Alpha) (Ex: Base color)
		TEX_2D_BC4U,	// Compressed, Red (Ex: Mask, Specular, AO, Metallic, Roughness)
		TEX_2D_BC5S,	// Compressed, Red-Green (Normal, Motion)
		TEX_2D_BC6H,	// Compressed, HDR
		TEX_2D_BC7U,	// Compressed, Red-Green-Blue-(Alpha)

		TEX_RT_RGBA,
		TEX_RT_BGRA,

		TEX_DS_16,
		TEX_DS_24_8,
		TEX_DS_32,

		MAX_COUNT
	};


	constexpr const char* NAMES[MAX_COUNT] =
	{
		"NONE",
		"TEX_2D_R",
		"TEX_2D_RGBA",
		"TEX_2D_SRGB",
		"TEX_2D_BC3",
		"TEX_2D_BC4",
		"TEX_2D_BC5",
		"TEX_2D_BC6H",
		"TEX_2D_BC7",
		"TEX_RT_RGBA",
		"TEX_RT_BGRA",
		"TEX_DS_16",
		"TEX_DS_24_8",
		"TEX_DS_32",
	};

};
