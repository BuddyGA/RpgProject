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
		PerspectiveFoVDegree = 75.0f;
		NearClipZ = 0.1f;
		FarClipZ = 1000.0f;
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
		AttenuationRadius = 8.0f;
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
