#pragma once

#include "core/world/RpgComponent.h"
#include "../asset/RpgMesh.h"
#include "../asset/RpgMaterial.h"
#include "../RpgSceneViewport.h"
#include "../RpgShadowViewport.h"


class RpgRenderWorldSubsystem;
class RpgRenderTask_CaptureMesh;
class RpgRenderTask_CaptureLight;



class RpgRenderComponent_Camera
{
	RPG_COMPONENT(RpgRenderComponent_Camera, RPG_COMPONENT_ID_RENDER_0)

public:
	RpgPointInt RenderTargetDimension;
	RpgRenderProjectionMode ProjectionMode;
	float PerspectiveFoVDegree;
	float NearClipZ;
	float FarClipZ;
	bool bActivated;
	bool bFrustumCulling;

	RpgSceneViewport* Viewport;


public:
	RpgRenderComponent_Camera() noexcept
	{
		RenderTargetDimension = RpgPointInt(1600, 900);
		ProjectionMode = RpgRenderProjectionMode::PERSPECTIVE;
		PerspectiveFoVDegree = 60.0f;
		NearClipZ = 10.0f;
		FarClipZ = 10000.0f;
		bActivated = false;
		bFrustumCulling = false;
		Viewport = nullptr;
	}


	inline void Destroy() noexcept
	{
		// Nothing to do
	}


	inline RpgSceneViewport* GetSceneViewport() noexcept
	{
		if (Viewport)
		{
			return Viewport;
		}

		if (!SelfViewport)
		{
			SelfViewport = RpgPointer::MakeUnique<RpgSceneViewport>();
		}

		return SelfViewport.Get();
	}


private:
	RpgUniquePtr<RpgSceneViewport> SelfViewport;

};




class RpgRenderComponent_Mesh
{
	RPG_COMPONENT(RpgRenderComponent_Mesh, RPG_COMPONENT_ID_RENDER_1)

public:
	RpgSharedMesh Mesh;
	RpgSharedMaterial Material;
	bool bIsVisible;


public:
	RpgRenderComponent_Mesh() noexcept
	{
		bIsVisible = false;
		Bound = RpgBoundingAABB(RpgVector3(-32.0f), RpgVector3(32.0f));
	}


	inline void Destroy() noexcept
	{
		// Nothing to do
	}


	inline const RpgBoundingAABB& GetBound() const noexcept
	{
		return Bound;
	}


private:
	RpgBoundingAABB Bound;


	friend RpgRenderWorldSubsystem;

};




class RpgRenderComponent_Light
{
	RPG_COMPONENT(RpgRenderComponent_Light, RPG_COMPONENT_ID_RENDER_2)

public:
	// Light type (point light, spot light, directional light)
	RpgRenderLight::EType Type;

	// Light color and intensity
	// (RGB: color, A: intensity)
	RpgColorLinear ColorIntensity;

	// For point/spot light only, attenuation radius
	float AttenuationRadius;

	// For point/spot light only, attenuation falloff exponent
	float AttenuationFallOffExp;

	// For spotlight only, inner cone (umbra) in degree
	float SpotInnerConeDegree;

	// For spotlight only, outer cone (penumbra) in degree
	float SpotOuterConeDegree;

	// TRUE if light cast shadow
	bool bCastShadow;

	// TRUE if light visible
	bool bIsVisible;


public:
	RpgRenderComponent_Light() noexcept
	{
		Type = RpgRenderLight::TYPE_NONE;
		ColorIntensity = RpgColorLinear(1.0f, 1.0f, 1.0f, 1.0f);
		AttenuationRadius = 800.0f;
		AttenuationFallOffExp = 8.0f;
		SpotInnerConeDegree = 20.0f;
		SpotOuterConeDegree = 40.0f;
		bCastShadow = false;
		bIsVisible = false;
	}


	inline void Destroy() noexcept
	{
		// Nothing to do
	}


	inline RpgShadowViewport* GetShadowViewport() noexcept
	{
		return ShadowViewport.Get();
	}


private:
	RpgUniquePtr<RpgShadowViewport> ShadowViewport;


	friend class RpgRenderWorldSubsystem;

};




class RpgRenderComponent_Terrain
{
	RPG_COMPONENT(RpgRenderComponent_Terrain, RPG_COMPONENT_ID_RENDER_3)

public:
	


public:
	RpgSharedMaterial Material;
	//RpgSharedTexture2D SplatTextures[4];
	bool bIsVisible;


public:
	RpgRenderComponent_Terrain() noexcept
	{
		bIsVisible = false;
		WorldSize = 0.0f;
	}


	inline void Destroy() noexcept
	{
		// Nothing to do
	}


	void Generate(float in_WorldSize) noexcept;
	const RpgVertexIndexArray& GetVertexIndices(const RpgBoundingFrustum* frustum) noexcept;


	inline bool HasGenerated() const noexcept
	{
		return VertexPositions.GetCount() > 0;
	}

	inline RpgVertexMeshPositionArray& GetVertexPositions() noexcept
	{
		return VertexPositions;
	}

	inline const RpgVertexMeshPositionArray& GetVertexPositions() const noexcept
	{
		return VertexPositions;
	}

	inline RpgVertexMeshNormalTangentArray& GetVertexNormalTangents() noexcept
	{
		return VertexNormalTangents;
	}

	inline const RpgVertexMeshNormalTangentArray& GetVertexNormalTangents() const noexcept
	{
		return VertexNormalTangents;
	}

	inline RpgVertexMeshTexCoordArray& GetVertexTexCoords() noexcept
	{
		return VertexTexCoords;
	}

	inline const RpgVertexMeshTexCoordArray& GetVertexTexCoords() const noexcept
	{
		return VertexTexCoords;
	}


private:
	// Vertex position data
	RpgVertexMeshPositionArray VertexPositions;

	// Vertex normal, tangent data
	RpgVertexMeshNormalTangentArray VertexNormalTangents;

	// Vertex texcoord
	RpgVertexMeshTexCoordArray VertexTexCoords;

	// Vertex index
	RpgVertexIndexArray VertexIndices;

	// Terrain tiles
	struct FTile
	{
		RpgArrayInline<RpgVertex::FIndex, 6> VertexIndices;
		RpgBoundingAABB Bound;
	};
	RpgArray<FTile> Tiles;

	// Terrain world size
	float WorldSize;


private:
	void UpdateTileBound(int index) noexcept;

};
