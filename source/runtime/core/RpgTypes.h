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


	template<typename T>
	struct IsArithmetic
	{
		static constexpr bool Value = IsIntegral<T>::Value || IsFloat<T>::Value;
	};


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
		RpgRectFloat& bdr_LT = BorderRects[LEFT_TOP];
		bdr_LT.Left = windowRect.Left;
		bdr_LT.Top = windowRect.Top;
		bdr_LT.Right = bdr_LT.Left + BorderThickness;
		bdr_LT.Bottom = bdr_LT.Top + BorderThickness;

		RpgRectFloat& bdr_RT = BorderRects[RIGHT_TOP];
		bdr_RT.Right = windowRect.Right;
		bdr_RT.Top = windowRect.Top;
		bdr_RT.Left = bdr_RT.Right - BorderThickness;
		bdr_RT.Bottom = bdr_RT.Top + BorderThickness;

		RpgRectFloat& bdr_LB = BorderRects[LEFT_BOTTOM];
		bdr_LB.Left = windowRect.Left;
		bdr_LB.Bottom = windowRect.Bottom;
		bdr_LB.Right = bdr_LB.Left + BorderThickness;
		bdr_LB.Top = bdr_LB.Bottom - BorderThickness;

		RpgRectFloat& bdr_RB = BorderRects[RIGHT_BOTTOM];
		bdr_RB.Right = windowRect.Right;
		bdr_RB.Bottom = windowRect.Bottom;
		bdr_RB.Left = bdr_RB.Right - BorderThickness;
		bdr_RB.Top = bdr_RB.Bottom - BorderThickness;

		RpgRectFloat& bdr_L = BorderRects[LEFT];
		bdr_L.Left = bdr_LT.Left;
		bdr_L.Right = bdr_LT.Right;
		bdr_L.Top = bdr_LT.Bottom + SpaceBetweenBorder;
		bdr_L.Bottom = bdr_LB.Top - SpaceBetweenBorder;

		RpgRectFloat& bdr_R = BorderRects[RIGHT];
		bdr_R.Left = bdr_RT.Left;
		bdr_R.Right = bdr_RT.Right;
		bdr_R.Top = bdr_RT.Bottom + SpaceBetweenBorder;
		bdr_R.Bottom = bdr_RB.Top - SpaceBetweenBorder;

		RpgRectFloat& bdr_T = BorderRects[TOP];
		bdr_T.Top = bdr_LT.Top;
		bdr_T.Bottom = bdr_LT.Bottom;
		bdr_T.Left = bdr_LT.Right + SpaceBetweenBorder;
		bdr_T.Right = bdr_RT.Left - SpaceBetweenBorder;

		RpgRectFloat& bdr_B = BorderRects[BOTTOM];
		bdr_B.Top = bdr_LB.Top;
		bdr_B.Bottom = bdr_LB.Bottom;
		bdr_B.Left = bdr_LB.Right + SpaceBetweenBorder;
		bdr_B.Right = bdr_RB.Left - SpaceBetweenBorder;
	}


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



class RpgColorRGBA
{
public:
	uint8_t R, G, B, A;

	static const RpgColorRGBA BLACK;
	static const RpgColorRGBA BLACK_TRANSPARENT;
	static const RpgColorRGBA BLUE;
	static const RpgColorRGBA GREEN;
	static const RpgColorRGBA RED;
	static const RpgColorRGBA WHITE;
	static const RpgColorRGBA WHITE_TRANSPARENT;
	static const RpgColorRGBA YELLOW;


public:
	RpgColorRGBA() noexcept
		: R(0), G(0), B(0), A(0)
	{
	}

	RpgColorRGBA(uint8_t in_R, uint8_t in_G, uint8_t in_B, uint8_t in_A = 255) noexcept
		: R(in_R), G(in_G), B(in_B), A(in_A)
	{
	}

	RpgColorRGBA(uint32_t rgba) noexcept
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

	explicit RpgColorLinear(const RpgColorRGBA& rgba) noexcept
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
