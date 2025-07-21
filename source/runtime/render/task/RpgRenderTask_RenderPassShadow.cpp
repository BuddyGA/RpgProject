#include "RpgRenderTask_RenderPass.h"
#include "../RpgRenderPipeline.h"



RpgRenderTask_RenderPassShadow::RpgRenderTask_RenderPassShadow() noexcept
{
	DrawMeshData = nullptr;
	DrawMeshCount = 0;
	DrawSkinnedMeshData = nullptr;
	DrawSkinnedMeshCount = 0;
	bIsOmniDirectional = false;
}


void RpgRenderTask_RenderPassShadow::Reset() noexcept
{
	RpgRenderTask_RenderPass::Reset();

	TextureDepth = nullptr;
	ViewId = RPG_INDEX_INVALID;
	DrawMeshData = nullptr;
	DrawMeshCount = 0;
	DrawSkinnedMeshData = nullptr;
	DrawSkinnedMeshCount = 0;
	bIsOmniDirectional = false;
}


void RpgRenderTask_RenderPassShadow::CommandDraw(ID3D12GraphicsCommandList* cmdList) const noexcept
{
	const RpgPointInt dimension = TextureDepth->GetDimension();

	// Set viewport
	RpgD3D12Command::SetViewport(cmdList, 0, 0, dimension.X, dimension.Y, 0.0f, 1.0f);

	// Set scissor
	RpgD3D12Command::SetScissor(cmdList, 0, 0, dimension.X, dimension.Y);

	// bind root signature
	cmdList->SetGraphicsRootSignature(RpgRenderPipeline::GetRootSignatureGraphics());

	// Bind shader resource world
	WorldResource->CommandBindShaderResources(cmdList);

	ID3D12Resource* depthStencilResource = TextureDepth->GPU_GetResource();
	const RpgD3D12::FResourceDescriptor depthStencilDescriptor = RpgD3D12::AllocateDescriptor_DSV(FrameContext.Index, depthStencilResource);

	// Transition resource to depth-write
	RpgD3D12Command::TransitionAllSubresources(cmdList, depthStencilResource, TextureDepth->GPU_GetState(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	TextureDepth->GPU_SetState(D3D12_RESOURCE_STATE_DEPTH_WRITE);

	// Set and clear render targets
	RpgD3D12Command::SetAndClearRenderTargets(cmdList, nullptr, 0, RpgColorLinear(), &depthStencilDescriptor, 1.0f, 0);

	// bind pipeline state
	ID3D12PipelineState* PSO = bIsOmniDirectional ? RpgRenderPipeline::GetGraphicsPSO_ShadowMapCube() : RpgRenderPipeline::GetGraphicsPSO_ShadowMapDirectional();
	cmdList->SetPipelineState(PSO);

	// Set topology
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw mesh
	if (DrawMeshData)
	{
		RPG_Assert(DrawMeshCount > 0);

		// Bind vertex buffers
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[1] =
		{
			FrameContext.MeshResource->GetVertexBufferView_Position(),
		};
		cmdList->IASetVertexBuffers(0, 1, vertexBufferViews);

		// Bind index buffer
		const D3D12_INDEX_BUFFER_VIEW indexBufferView = FrameContext.MeshResource->GetIndexBufferView();
		cmdList->IASetIndexBuffer(&indexBufferView);

		// Draw calls
		for (int d = 0; d < DrawMeshCount; ++d)
		{
			RpgDrawIndexedDepth draw = DrawMeshData[d];
			draw.ObjectParam.ViewIndex = ViewId;

			cmdList->SetGraphicsRoot32BitConstants(RpgRenderPipeline::GRPI_OBJECT_PARAM, sizeof(RpgShaderObjectParameter) / 4, &draw.ObjectParam, 0);
			cmdList->DrawIndexedInstanced(draw.IndexCount, 1, draw.IndexStart, draw.IndexVertexOffset, 0);
		}
	}


	// Draw mesh skinned
	if (DrawSkinnedMeshData)
	{
		RPG_Assert(DrawSkinnedMeshCount > 0);

		// Bind vertex buffers
		const D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[1] =
		{
			FrameContext.MeshSkinnedResource->GetVertexBufferView_SkinnedPosition(),
		};
		cmdList->IASetVertexBuffers(0, 1, vertexBufferViews);

		// Bind index buffer
		const D3D12_INDEX_BUFFER_VIEW indexBufferView = FrameContext.MeshSkinnedResource->GetIndexBufferView_Skinned();
		cmdList->IASetIndexBuffer(&indexBufferView);

		const RpgArray<RpgShaderSkinnedObjectParameter>& skinnedObjectParams = FrameContext.MeshSkinnedResource->GetObjectParameters();
		RPG_Check(skinnedObjectParams.GetCount() == DrawSkinnedMeshCount);

		// Draw calls
		for (int d = 0; d < DrawSkinnedMeshCount; ++d)
		{
			RpgDrawIndexedDepth draw = DrawMeshData[d];
			draw.ObjectParam.ViewIndex = ViewId;

			const RpgShaderSkinnedObjectParameter& skinnedParam = skinnedObjectParams[d];

			cmdList->SetGraphicsRoot32BitConstants(RpgRenderPipeline::GRPI_OBJECT_PARAM, sizeof(RpgShaderObjectParameter) / 4, &draw.ObjectParam, 0);
			cmdList->DrawIndexedInstanced(skinnedParam.IndexCount, 1, skinnedParam.IndexStart, skinnedParam.VertexStart, 0);
		}
	}


	// Transition resource to pixel shader
	RpgD3D12Command::TransitionAllSubresources(cmdList, depthStencilResource, TextureDepth->GPU_GetState(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	TextureDepth->GPU_SetState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
