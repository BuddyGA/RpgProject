#include "RpgRenderTask_RenderPass.h"
#include "../RpgRenderPipeline.h"



RpgRenderTask_RenderPass_Forward::RpgRenderTask_RenderPass_Forward() noexcept
{
	TextureRenderTarget = nullptr;
	TextureDepthStencil = nullptr;

	/*
	DrawMeshData = nullptr;
	DrawMeshCount = 0;
	DrawSkinnedMeshData = nullptr;
	DrawSkinnedMeshCount = 0;
	DrawTerrainData = nullptr;
	DrawTerrainCount = 0;
	*/
}


void RpgRenderTask_RenderPass_Forward::Reset() noexcept
{
	RpgThreadTask::Reset();

	TextureRenderTarget = nullptr;
	TextureDepthStencil = nullptr;

	/*
	DrawMeshData = nullptr;
	DrawMeshCount = 0;
	DrawSkinnedMeshData = nullptr;
	DrawSkinnedMeshCount = 0;
	DrawTerrainData = nullptr;
	DrawTerrainCount = 0;
	*/

	DrawTerrains.Clear();
}


void RpgRenderTask_RenderPass_Forward::CommandDraw(ID3D12GraphicsCommandList* cmdList) const noexcept
{
	RPG_Assert(TextureRenderTarget);
	RPG_Assert(TextureDepthStencil);

	const int frameIndex = FrameContext->Index;

	const RpgPointInt renderTargetDimension = TextureRenderTarget->GetDimension();

	// Set viewport
	RpgD3D12Command::SetViewport(cmdList, 0, 0, renderTargetDimension.X, renderTargetDimension.Y, 0.0f, 1.0f);

	// Set scissor
	RpgD3D12Command::SetScissor(cmdList, 0, 0, renderTargetDimension.X, renderTargetDimension.Y);

	ID3D12Resource* renderTargetResource = TextureRenderTarget->GPU_GetResource();
	const RpgD3D12::FResourceDescriptor renderTargetDescriptor = RpgD3D12::AllocateDescriptor_RTV(frameIndex, renderTargetResource);

	// Transition resource to render target
	RpgD3D12Command::TransitionAllSubresources(cmdList, renderTargetResource, TextureRenderTarget->GPU_GetState(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	TextureRenderTarget->GPU_SetState(D3D12_RESOURCE_STATE_RENDER_TARGET);

	ID3D12Resource* depthStencilResource = TextureDepthStencil->GPU_GetResource();
	const RpgD3D12::FResourceDescriptor depthStencilDescriptor = RpgD3D12::AllocateDescriptor_DSV(frameIndex, depthStencilResource);

	// Transition resource to depth-write
	RpgD3D12Command::TransitionAllSubresources(cmdList, depthStencilResource, TextureDepthStencil->GPU_GetState(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	TextureDepthStencil->GPU_SetState(D3D12_RESOURCE_STATE_DEPTH_WRITE);

	// Set and clear render targets
	RpgD3D12Command::SetAndClearRenderTargets(cmdList, &renderTargetDescriptor, 1, RpgColorLinear(0.0f, 0.0f, 0.0f), &depthStencilDescriptor, 1.0f, 0);

	// Set root signature and global texture descriptor table (dynamic indexing)
	cmdList->SetGraphicsRootSignature(RpgRenderPipeline::GetRootSignatureGraphics());

	// Set descriptor table (texture dynamic indexing)
	ID3D12DescriptorHeap* textureDescriptorHeap = RpgD3D12::GetDescriptorHeap_TDI(frameIndex);
	cmdList->SetDescriptorHeaps(1, &textureDescriptorHeap);
	cmdList->SetGraphicsRootDescriptorTable(RpgRenderPipeline::GRPI_TEXTURES, textureDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	// Bind shader resource material
	FrameContext->ResourceMaterial.CommandBindShaderResources(cmdList);

	// Set topology
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// Draw terrains
	for (const RpgTerrain* terrain : DrawTerrains)
	{

	}
}
