#pragma once

#include "core/RpgString.h"
#include "core/RpgD3D12.h"


// Use dedicated render thread
#define RPG_RENDER_MULTITHREADED		1

// All copy/compute/render execute in async task threadpool
#define RPG_RENDER_ASYNC_TASK			1



enum class RpgRenderProjectionMode : uint8_t
{
	PERSPECTIVE = 0,
	ORTHOGRAPHIC
};



namespace RpgRenderLight
{
	enum EType : uint8_t
	{
		TYPE_NONE = 0,
		TYPE_POINT_LIGHT,
		TYPE_SPOT_LIGHT,
		TYPE_DIRECTIONAL_LIGHT
	};


	enum EShadowQuality : uint8_t
	{
		SHADOW_QUALITY_NONE = 0,
		SHADOW_QUALITY_LOW,
		SHADOW_QUALITY_MEDIUM,
		SHADOW_QUALITY_HIGH,
		SHADOW_QUALITY_MAX_COUNT
	};


	constexpr uint16_t SHADOW_TEXTURE_DIMENSION_POINT_LIGHT[SHADOW_QUALITY_MAX_COUNT] =
	{
		0,
		256,	// LOW
		512,	// MEDIUM
		1024	// HIGH
	};


	constexpr uint16_t SHADOW_TEXTURE_DIMENSION_SPOT_LIGHT[SHADOW_QUALITY_MAX_COUNT] =
	{
		0,
		256,	// LOW
		512,	// MEDIUM
		1024	// HIGH
	};


	constexpr uint16_t SHADOW_TEXTURE_DIMENSION_DIRECTIONAL_LIGHT[SHADOW_QUALITY_MAX_COUNT] =
	{
		0,
		512,	// LOW
		1024,	// MEDIUM
		2048	// HIGH
	};

};



namespace RpgRenderAntiAliasing
{
	enum EMode : uint8_t
	{
		MODE_NONE = 0,
		MODE_FXAA,
		MODE_SMAA,
		MODE_MAX_COUNT
	};


	constexpr const char* NAMES[MODE_MAX_COUNT] =
	{
		"None",
		"FXAA",
		"SMAA"
	};

}



namespace RpgRenderFormat
{
	constexpr const DXGI_FORMAT SCENE_RENDER_TARGET = DXGI_FORMAT_R8G8B8A8_UNORM;
	constexpr const DXGI_FORMAT SCENE_DEPTH_STENCIL = DXGI_FORMAT_D24_UNORM_S8_UINT;
	constexpr const DXGI_FORMAT SHADOW_DEPTH = DXGI_FORMAT_D16_UNORM;

};
