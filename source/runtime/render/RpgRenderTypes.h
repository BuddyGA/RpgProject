#pragma once

#include "core/RpgString.h"
#include "core/RpgD3D12.h"
#include "core/world/RpgGameObject.h"


// Use dedicated render thread
#define RPG_RENDER_MULTITHREADED		1

// All copy, compute, render execute in async task threadpool
#define RPG_RENDER_ASYNC_TASK			1



class RpgRenderer;
class RpgRenderer2D;

class RpgMaterialResource;
class RpgMeshResource;
class RpgMeshSkinnedResource;
class RpgWorldResource;

struct RpgDrawIndexed;

class RpgSceneViewport;

class RpgShadowViewport;
class RpgShadowViewport_PointLight;
class RpgShadowViewport_SpotLight;
class RpgShadowViewport_Cascade;

class RpgRenderTask_CompilePSO;
class RpgRenderTask_Compute;
class RpgRenderTask_Copy;
class RpgRenderTask_RenderPass;
class RpgAsyncTask_RenderPass_Depth;
class RpgRenderTask_RenderPassShadow;
class RpgRenderTask_RenderPassForward;
class RpgAsyncTask_RenderPass_Transparency;

typedef RpgArrayInline<RpgRenderTask_RenderPassShadow*, 32> RpgRenderTask_RenderPassShadowArray;
typedef RpgArrayInline<RpgRenderTask_RenderPassForward*, 32> RpgRenderTask_RenderPassForwardArray;


class RpgRenderTask_Compute;



enum class RpgRenderProjectionMode : uint8_t
{
	PERSPECTIVE = 0,
	ORTHOGRAPHIC
};



enum class RpgRenderVertexMode : uint8_t
{
	NONE = 0,

	PRIMITIVE_2D,
	GUI,

	PRIMITIVE,
	MESH,

	POST_PROCESS,

	MAX_COUNT
};


enum class RpgRenderRasterMode : uint8_t
{
	NONE = 0,
	LINE,
	SOLID,
	WIREFRAME,
	MAX_COUNT
};


enum class RpgRenderColorBlendMode : uint8_t
{
	NONE = 0,
	ADDITIVE,
	OPACITY_MASK,
	FADE,
	TRANSPARENCY,
	MAX_COUNT
};



struct RpgRenderPipelineState
{
	RpgName VertexShaderName;
	RpgName PixelShaderName;
	RpgName GeometryShaderName;

	RpgRenderVertexMode VertexMode{ RpgRenderVertexMode::NONE };
	RpgRenderRasterMode RasterMode{ RpgRenderRasterMode::NONE };
	RpgRenderColorBlendMode BlendMode{ RpgRenderColorBlendMode::NONE };
	DXGI_FORMAT RenderTargetFormat{ DXGI_FORMAT_UNKNOWN };
	int RenderTargetCount{ 1 };

	DXGI_FORMAT DepthStencilFormat{ DXGI_FORMAT_UNKNOWN };
	int DepthBias{ 0 };
	float DepthBiasSlope{ 0.0f };
	float DepthBiasClamp{ 0.0f };
	bool bDepthTest{ false };
	bool bDepthWrite{ false };
	bool bStencilTest{ false };

	bool bTwoSides{ false };
	bool bConservativeRasterization{ false };
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



struct RpgRenderFrameContext
{
	uint64_t Counter{ 0 };
	int Index{ 0 };
	float DeltaTime{ 0.0f };
	RpgMaterialResource* MaterialResource{ nullptr };
	RpgMeshResource* MeshResource{ nullptr };
	RpgMeshSkinnedResource* MeshSkinnedResource{ nullptr };
	RpgRenderLight::EShadowQuality ShadowQuality{ RpgRenderLight::SHADOW_QUALITY_NONE };
	RpgRenderAntiAliasing::EMode AntiAliasingMode{ RpgRenderAntiAliasing::MODE_NONE };
};



namespace RpgRender
{
	constexpr const DXGI_FORMAT DEFAULT_FORMAT_SCENE_RENDER_TARGET = DXGI_FORMAT_R8G8B8A8_UNORM;
	constexpr const DXGI_FORMAT DEFAULT_FORMAT_SCENE_DEPTH_STENCIL = DXGI_FORMAT_D24_UNORM_S8_UINT;
	constexpr const DXGI_FORMAT DEFAULT_FORMAT_SHADOW_DEPTH = DXGI_FORMAT_D16_UNORM;

};
