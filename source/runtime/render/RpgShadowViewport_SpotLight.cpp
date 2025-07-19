#include "RpgShadowViewport.h"
#include "core/world/RpgWorld.h"
#include "world/RpgRenderComponent.h"



RpgShadowViewport_SpotLight::RpgShadowViewport_SpotLight() noexcept
{
	SpotInnerConeDegree = 20.0f;
	SpotOuterConeDegree = 40.0f;
}


void RpgShadowViewport_SpotLight::PreRender(RpgRenderFrameContext& frameContext, RpgWorldResource* worldResource, const RpgWorld* world, RpgWorldResource::FLightID lightId) noexcept
{
	FFrameData& frame = FrameDatas[frameContext.Index];
	frame.DrawMeshes.Clear();
	frame.DrawSkinnedMeshes.Clear();

	const uint16_t shadowTextureDimension = RpgRenderLight::SHADOW_TEXTURE_DIMENSION_SPOT_LIGHT[frameContext.ShadowQuality];
	RpgSharedTexture2D& depthTexture = frame.TextureDepth;

	if (!depthTexture)
	{
		depthTexture = RpgTexture2D::s_CreateSharedDepthStencil(RpgName::Format("TEXD_SdwVprt_SL_%i", lightId), RpgTextureFormat::TEX_DS_16, shadowTextureDimension, shadowTextureDimension);
	}

	depthTexture->Resize(shadowTextureDimension, shadowTextureDimension);
	depthTexture->GPU_UpdateResource();

	const RpgTransform transform = world->GameObject_GetWorldTransform(GameObject);
	const RpgMatrixTransform viewMatrix = transform.ToMatrixTransform().GetInverse();
	const float fovDegree = SpotInnerConeDegree * 2.0f;
	const float nearClipZ = 1.0f;
	const float farClipZ = AttenuationRadius * 1.05f;
	const RpgMatrixProjection projMatrix = RpgMatrixProjection::CreatePerspective(1.0f, fovDegree, nearClipZ, farClipZ);

	ViewId = worldResource->AddView(viewMatrix, projMatrix, transform.Position, nearClipZ, farClipZ);

	const RpgD3D12::FResourceDescriptor shadowDepthDescriptor = RpgD3D12::AllocateDescriptor_TDI(depthTexture->GPU_GetResource(), DXGI_FORMAT_R16_UNORM);
	worldResource->SetLightShadow(lightId, ViewId, shadowDepthDescriptor.Index);


	// build draw calls
	for (auto it = world->Component_CreateConstIterator<RpgRenderComponent_Mesh>(); it; ++it)
	{
		const RpgRenderComponent_Mesh& comp = it.GetValue();
		if (!(comp.Model && comp.bIsVisible))
		{
			continue;
		}

		const RpgSharedModel& model = comp.Model;
		const RpgMatrixTransform worldTransformMatrix = world->GameObject_GetWorldTransformMatrix(comp.GameObject);

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
	}
}


void RpgShadowViewport_SpotLight::SetupRenderPasses(const RpgRenderFrameContext& frameContext, const RpgWorldResource* worldResource, const RpgWorld* world, RpgRenderTask_RenderPassShadowArray& out_ShadowPasses) noexcept
{
	FFrameData& frame = FrameDatas[frameContext.Index];

	RpgRenderTask_RenderPassShadow* shadowPass = &frame.TaskRenderPassShadow;
	shadowPass->Reset();
	shadowPass->FrameContext = frameContext;
	shadowPass->WorldResource = worldResource;
	shadowPass->TextureDepth = frame.TextureDepth.Get();
	shadowPass->ViewId = ViewId;
	shadowPass->DrawMeshData = frame.DrawMeshes.GetData();
	shadowPass->DrawMeshCount = frame.DrawMeshes.GetCount();
	shadowPass->DrawSkinnedMeshData = frame.DrawSkinnedMeshes.GetData();
	shadowPass->DrawSkinnedMeshCount = frame.DrawSkinnedMeshes.GetCount();
	shadowPass->bIsOmniDirectional = false;

	out_ShadowPasses.AddValue(shadowPass);
}
