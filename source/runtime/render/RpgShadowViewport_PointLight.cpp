#include "RpgShadowViewport.h"
#include "core/world/RpgWorld.h"
#include "world/RpgRenderComponent.h"



RpgShadowViewport_PointLight::RpgShadowViewport_PointLight() noexcept
{
}


void RpgShadowViewport_PointLight::PreRender(RpgRenderFrameContext& frameContext, RpgWorldResource* worldResource, const RpgWorld* world, RpgWorldResource::FLightID lightId) noexcept
{
	FFrameData& frame = FrameDatas[frameContext.Index];
	frame.DrawMeshes.Clear();
	frame.DrawSkinnedMeshes.Clear();

	const uint16_t shadowTextureDimension = RpgRenderLight::SHADOW_TEXTURE_DIMENSION_POINT_LIGHT[frameContext.ShadowQuality];
	RpgSharedTextureDepthCube& depthTexture = frame.TextureDepthCube;

	if (!depthTexture)
	{
		depthTexture = RpgTextureDepthCube::s_CreateShared(RpgName::Format("TEXDC_SdwVprt_PL_%i", lightId), RpgTextureFormat::TEX_DS_16, shadowTextureDimension, shadowTextureDimension);
	}

	depthTexture->Resize(shadowTextureDimension, shadowTextureDimension);
	depthTexture->GPU_UpdateResource();

	RpgTransform transform = world->GameObject_GetWorldTransform(GameObject);
	const float fovDegree = 91.0f;
	const float nearClipZ = 1.0f;
	const float farClipZ = AttenuationRadius * 1.05f;
	const RpgMatrixProjection projMatrix = RpgMatrixProjection::CreatePerspective(1.0f, fovDegree, nearClipZ, farClipZ);

	// +X
	{
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(0.0f, 90.0f, 0.0f);

		FViewInfo& view = FaceViews[0];
		view.ViewMatrix = transform.ToMatrixTransform().GetInverse();
		view.ViewId = worldResource->AddView(view.ViewMatrix, projMatrix, transform.Position, nearClipZ, farClipZ);
	}
	// -X
	{
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(0.0f, -90.0f, 0.0f);

		FViewInfo& view = FaceViews[1];
		view.ViewMatrix = transform.ToMatrixTransform().GetInverse();
		view.ViewId = worldResource->AddView(view.ViewMatrix, projMatrix, transform.Position, nearClipZ, farClipZ);
	}
	// +Y
	{
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(-90.0f, 0.0f, 0.0f);

		FViewInfo& view = FaceViews[2];
		view.ViewMatrix = transform.ToMatrixTransform().GetInverse();
		view.ViewId = worldResource->AddView(view.ViewMatrix, projMatrix, transform.Position, nearClipZ, farClipZ);
	}
	// -Y
	{
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(90.0f, 0.0f, 0.0f);

		FViewInfo& view = FaceViews[3];
		view.ViewMatrix = transform.ToMatrixTransform().GetInverse();
		view.ViewId = worldResource->AddView(view.ViewMatrix, projMatrix, transform.Position, nearClipZ, farClipZ);
	}
	// +Z
	{
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(0.0f, 0.0f, 0.0f);

		FViewInfo& view = FaceViews[4];
		view.ViewMatrix = transform.ToMatrixTransform().GetInverse();
		view.ViewId = worldResource->AddView(view.ViewMatrix, projMatrix, transform.Position, nearClipZ, farClipZ);
	}
	// -Z
	{
		transform.Rotation = RpgQuaternion::FromPitchYawRollDegree(0.0f, 180.0f, 0.0f);

		FViewInfo& view = FaceViews[5];
		view.ViewMatrix = transform.ToMatrixTransform().GetInverse();
		view.ViewId = worldResource->AddView(view.ViewMatrix, projMatrix, transform.Position, nearClipZ, farClipZ);
	}

	const RpgD3D12::FResourceDescriptor shadowDepthDescriptor = RpgD3D12::AllocateDescriptor_TDI_Cube(frameContext.Index, depthTexture->GPU_GetResource(), DXGI_FORMAT_R16_UNORM);
	worldResource->SetLightShadow(lightId, FaceViews[0].ViewId, shadowDepthDescriptor.Index);


	// build draw calls
	for (auto it = world->Component_CreateConstIterator<RpgRenderComponent_Mesh>(); it; ++it)
	{
		const RpgRenderComponent_Mesh& comp = it.GetValue();
		if (!(comp.Mesh && comp.bIsVisible))
		{
			continue;
		}

		const RpgMatrixTransform worldTransformMatrix = world->GameObject_GetWorldTransformMatrix(comp.GameObject);

		/*
		const RpgSharedModel& model = comp.Model;

		for (int m = 0; m < model->GetMeshCount(); ++m)
		{
			const RpgSharedMesh& mesh = model->GetMeshLod(m, 0);
			RpgDrawIndexedDepth* draw = nullptr;

			if (mesh->HasSkin())
			{
				draw = &frame.DrawSkinnedMeshes.Add();
				frameContext.MeshSkinnedResource->AddMesh(mesh, draw->IndexCount, draw->IndexStart, draw->IndexVertexOffset);
			}
			else
			{
				draw = &frame.DrawMeshes.Add();
				frameContext.MeshResource->AddMesh(mesh, draw->IndexCount, draw->IndexStart, draw->IndexVertexOffset);
			}

			draw->ObjectParam.TransformIndex = worldResource->AddTransform(comp.GameObject.GetIndex(), worldTransformMatrix);
		}
		*/

		const RpgSharedMesh& mesh = comp.Mesh;
		RpgDrawIndexedDepth* draw = nullptr;

		if (mesh->HasSkin())
		{
			draw = &frame.DrawSkinnedMeshes.Add();
			frameContext.MeshSkinnedResource->AddMesh(mesh, draw->IndexCount, draw->IndexStart, draw->IndexVertexOffset);
		}
		else
		{
			draw = &frame.DrawMeshes.Add();
			frameContext.MeshResource->AddMesh(mesh, draw->IndexCount, draw->IndexStart, draw->IndexVertexOffset);
		}

		draw->ObjectParam.TransformIndex = worldResource->AddTransform(comp.GameObject.GetIndex(), worldTransformMatrix);
	}
}


void RpgShadowViewport_PointLight::SetupRenderPasses(const RpgRenderFrameContext& frameContext, const RpgWorldResource* worldResource, const RpgWorld* world, RpgRenderTask_RenderPassShadowArray& out_ShadowPasses) noexcept
{
	FFrameData& frame = FrameDatas[frameContext.Index];

	RpgRenderTask_RenderPassShadow* shadowPass = &frame.TaskRenderPassShadow;
	shadowPass->Reset();
	shadowPass->FrameContext = frameContext;
	shadowPass->WorldResource = worldResource;
	shadowPass->TextureDepth = frame.TextureDepthCube.Get();
	shadowPass->ViewId = FaceViews[0].ViewId;
	shadowPass->DrawMeshData = frame.DrawMeshes.GetData();
	shadowPass->DrawMeshCount = frame.DrawMeshes.GetCount();
	shadowPass->DrawSkinnedMeshData = frame.DrawSkinnedMeshes.GetData();
	shadowPass->DrawSkinnedMeshCount = frame.DrawSkinnedMeshes.GetCount();
	shadowPass->bIsOmniDirectional = true;

	out_ShadowPasses.AddValue(shadowPass);
}
