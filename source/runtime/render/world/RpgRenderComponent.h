#pragma once

#include "core/world/RpgComponent.h"
#include "../RpgModel.h"
#include "../RpgSceneViewport.h"
#include "../RpgShadowViewport.h"


class RpgRenderWorldSubsystem;
class RpgRenderTask_CaptureMesh;
class RpgRenderTask_CaptureLight;



RPG_COMPONENT_CLASS_BEGIN(RpgRenderComponent_Mesh, 2, "RpgRenderComponent - Mesh")

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

RPG_COMPONENT_CLASS_END()




RPG_COMPONENT_CLASS_BEGIN(RpgRenderComponent_Light, 3, "RpgRenderComponent - Light")

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

RPG_COMPONENT_CLASS_END()




RPG_COMPONENT_CLASS_BEGIN(RpgRenderComponent_Camera, 4, "RpgRenderComponent - Camera")

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


	friend RpgRenderWorldSubsystem;
	friend RpgRenderTask_CaptureMesh;
	friend RpgRenderTask_CaptureLight;

RPG_COMPONENT_CLASS_END()
