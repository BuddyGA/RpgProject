#include "RpgSceneViewport.h"



RpgSceneViewport::RpgSceneViewport() noexcept
{
	bFrustumCulling = false;
	bWireframeMode = false;

	for (int f = 0; f < RPG_FRAME_BUFFERING; ++f)
	{
		FFrameData& frame = FrameDatas[f];
		frame.RenderTargetDimension = RpgPointInt(1600, 900);
		frame.FovDegree = 60.0f;
		frame.NearClipZ = 10.0f;
		frame.FarClipZ = 10000.0f;
		frame.bOrthographicProjection = false;
	}
}


void RpgSceneViewport::UpdateFrame(RpgRenderFrameContext& context) noexcept
{
	FFrameData& frame = FrameDatas[context.Index];
	frame.CapturedTerrains.Clear();
	
	const RpgPointInt renderTargetDimension = frame.RenderTargetDimension;

	// Update view-projection
	const RpgMatrixTransform worldMatrixTransform(frame.ViewPosition, frame.ViewRotation);

	frame.ViewMatrix = worldMatrixTransform.GetInverse();

	frame.ProjectionMatrix = frame.bOrthographicProjection ?
		RpgMatrixProjection::CreateOrthographic(0.0f, static_cast<float>(renderTargetDimension.X), 0.0f, static_cast<float>(renderTargetDimension.Y), frame.NearClipZ, frame.FarClipZ) :
		RpgMatrixProjection::CreatePerspective(static_cast<float>(renderTargetDimension.X) / static_cast<float>(renderTargetDimension.Y), frame.FovDegree, frame.NearClipZ, frame.FarClipZ);

	frame.ViewFrustum.CreateFromMatrix(worldMatrixTransform, frame.ProjectionMatrix);


	// Resize render target
	if (!frame.TextureRenderTarget)
	{
		frame.TextureRenderTarget = RpgPointer::MakeShared<RpgTextureRenderTarget>("texrt_scn_vprt", RpgTextureFormat::TEX_RT_RGBA, renderTargetDimension.X, renderTargetDimension.Y);
	}

	frame.TextureRenderTarget->Resize(renderTargetDimension.X, renderTargetDimension.Y);
	frame.TextureRenderTarget->GPU_UpdateResource();


	// Resize depth-stencil
	if (!frame.TextureDepthStencil)
	{
		frame.TextureDepthStencil = RpgPointer::MakeShared<RpgTextureDepthStencil>("texds_scn_vprt", RpgTextureFormat::TEX_DS_32, renderTargetDimension.X, renderTargetDimension.Y);
	}

	frame.TextureDepthStencil->Resize(renderTargetDimension.X, renderTargetDimension.Y);
	frame.TextureDepthStencil->GPU_UpdateResource();
}


void RpgSceneViewport::SetupRenderPasses(const RpgRenderFrameContext& context, RpgRenderTask_RenderPass_Forward_Array& out_ForwardPasses) noexcept
{
	FFrameData& frame = FrameDatas[context.Index];

	RpgRenderTask_RenderPass_Forward* forwardPass = &frame.TaskRenderPassForward;
	forwardPass->Reset();
	forwardPass->FrameContext = &context;
	forwardPass->TextureRenderTarget = frame.TextureRenderTarget.Get();
	forwardPass->TextureDepthStencil = frame.TextureDepthStencil.Get();
	forwardPass->DrawTerrains = frame.CapturedTerrains;
}
