#include "RpgRenderer2D.h"
#include "RpgRenderPipeline.h"
#include "RpgRenderResource.h"



RpgRenderer2D::RpgRenderer2D() noexcept
{
	CurrentClipIndex = RPG_INDEX_INVALID;
	FrameIndex = RPG_INDEX_INVALID;
}


RpgRenderer2D::~RpgRenderer2D() noexcept
{
}


int RpgRenderer2D::GetOrAddMaterialInstanceId(const RpgSharedMaterial& material, const RpgSharedTexture2D& texture, bool bIsText) noexcept
{
	RPG_Assert(material);
	RPG_Assert(texture);

	const RpgSharedMaterial& materialToCheck = material->IsInstance() ? material->GetParentMaterial() : material;
	const RpgSharedTexture2D& textureToCheck = texture ? texture : DefaultTexture;
	int materialInstanceIndex = RPG_INDEX_INVALID;

	for (int i = 0; i < MaterialInstanceTextures.GetCount(); ++i)
	{
		const FMaterialInstanceTexture& check = MaterialInstanceTextures[i];

		if ((check.Material == materialToCheck || check.Material->GetParentMaterial() == materialToCheck) && check.Texture == textureToCheck)
		{
			materialInstanceIndex = i;
			break;
		}
	}

	if (materialInstanceIndex == RPG_INDEX_INVALID)
	{
		materialInstanceIndex = MaterialInstanceTextures.GetCount();

		FMaterialInstanceTexture& instance = MaterialInstanceTextures.Add();
		instance.Material = RpgMaterial::s_CreateSharedInstance(RpgName::Format("%s_inst_%i", *material->GetName(), material.GetRefCount()), material);
		instance.Texture = textureToCheck;
		instance.Material->SetParameterTextureValue(bIsText ? RpgMaterialParameterTexture::OPACITY_MASK : RpgMaterialParameterTexture::BASE_COLOR, instance.Texture);
	}

	return materialInstanceIndex;
}


void RpgRenderer2D::Begin(int frameIndex, RpgPointInt viewportDimension) noexcept
{
	if (!DefaultMaterialMesh)
	{
		DefaultMaterialMesh = RpgMaterial::s_GetDefault(RpgMaterialDefault::GUI);
		DefaultTexture = RpgTexture2D::s_GetDefault_White();

		GetOrAddMaterialInstanceId(DefaultMaterialMesh, DefaultTexture, false);
	}

	if (!DefaultMaterialFont)
	{
		DefaultMaterialFont = RpgMaterial::s_GetDefault(RpgMaterialDefault::FONT);
		DefaultFont = RpgFont::s_GetDefault_Roboto();
		GetOrAddMaterialInstanceId(DefaultMaterialFont, DefaultFont->GetTexture(), true);
	}

	ViewportDimension = viewportDimension;

	Clips.Clear();
	CurrentClipIndex = RPG_INDEX_INVALID;

	MeshVertices.Clear();
	MeshIndices.Clear();

	FrameIndex = frameIndex;

	FFrameData& frame = FrameDatas[FrameIndex];
	frame.BatchMeshVertices.Clear();
	frame.BatchMeshIndices.Clear();
	frame.BatchDrawClipMeshes.Clear();
	frame.LineVertices.Clear();
	frame.LineIndices.Clear();
	frame.BatchDrawLine = FDrawBatchLine();

	PushClipRect(RpgRect(0, 0, ViewportDimension.X, ViewportDimension.Y));
}


void RpgRenderer2D::End(int frameIndex) noexcept
{
	RPG_Assert(FrameIndex == frameIndex);

	PopClipRect();
	RPG_Check(CurrentClipIndex == RPG_INDEX_INVALID);
}


void RpgRenderer2D::AddMeshRect(RpgRectFloat rect, RpgColorRGBA color, const RpgSharedTexture2D& texture, const RpgSharedMaterial& material) noexcept
{
	const RpgPointFloat dimension = rect.GetDimension();
	if (dimension.X == 0 || dimension.Y == 0)
	{
		return;
	}

	FClip& clip = Clips[CurrentClipIndex];

	FMesh& mesh = clip.Shapes.Add();
	
	mesh.MaterialInstanceIndex = GetOrAddMaterialInstanceId(
		material ? material : DefaultMaterialMesh,
		texture ? texture : DefaultTexture,
		false
	);

	mesh.DataVertexStart = MeshVertices.GetCount();
	mesh.DataVertexCount = 4;
	mesh.DataIndexStart = MeshIndices.GetCount();
	mesh.DataIndexCount = 6;

	const RpgVertex::FMesh2D vertices[4] =
	{
		{ DirectX::XMFLOAT2(rect.Left, rect.Top), DirectX::XMFLOAT2(0.0f, 0.0f), color },
		{ DirectX::XMFLOAT2(rect.Right, rect.Top), DirectX::XMFLOAT2(1.0f, 0.0f), color },
		{ DirectX::XMFLOAT2(rect.Right, rect.Bottom), DirectX::XMFLOAT2(1.0f, 1.0f), color },
		{ DirectX::XMFLOAT2(rect.Left, rect.Bottom), DirectX::XMFLOAT2(0.0f, 1.0f), color },
	};

	MeshVertices.InsertAtRange(vertices, 4, RPG_INDEX_LAST);

	const RpgVertex::FIndex indices[6] =
	{
		0, 1, 2,
		2, 3, 0
	};

	MeshIndices.InsertAtRange(indices, 6, RPG_INDEX_LAST);
}


void RpgRenderer2D::AddText(const char* text, int length, RpgPointFloat position, RpgColorRGBA color, const RpgSharedFont& font, const RpgSharedMaterial& material) noexcept
{
	if (text == nullptr || length == 0)
	{
		return;
	}

	const RpgFont* useFont = font ? font.Get() : DefaultFont.Get();

	FClip& clip = Clips[CurrentClipIndex];

	FMesh& mesh = clip.Texts.Add();

	mesh.MaterialInstanceIndex = GetOrAddMaterialInstanceId(
		material ? material : DefaultMaterialFont,
		font ? font->GetTexture() : DefaultFont->GetTexture(),
		true
	);

	mesh.DataVertexStart = MeshVertices.GetCount();
	mesh.DataIndexStart = MeshIndices.GetCount();
	useFont->GenerateTextVertex(text, length, position, color, MeshVertices, MeshIndices, &mesh.DataVertexCount, &mesh.DataIndexCount);
}


void RpgRenderer2D::AddLine(RpgPointFloat p0, RpgPointFloat p1, RpgColorRGBA color) noexcept
{
	FFrameData& frame = FrameDatas[FrameIndex];

	const RpgVertex::FPrimitive2D vertices[2] =
	{
		{ DirectX::XMFLOAT2(p0.X, p0.Y), color },
		{ DirectX::XMFLOAT2(p1.X, p1.Y), color }
	};

	const uint32_t baseVertex = static_cast<uint32_t>(frame.LineVertices.GetCount());
	const RpgVertex::FIndex indices[2] = { baseVertex, baseVertex + 1 };

	frame.LineVertices.InsertAtRange(vertices, 2, RPG_INDEX_LAST);
	frame.LineIndices.InsertAtRange(indices, 2, RPG_INDEX_LAST);
}


void RpgRenderer2D::AddLineRect(RpgRectFloat rect, RpgColorRGBA color) noexcept
{
	AddLine(RpgPointFloat(rect.Left, rect.Top), RpgPointFloat(rect.Right, rect.Top), color);
	AddLine(RpgPointFloat(rect.Right, rect.Top), RpgPointFloat(rect.Right, rect.Bottom), color);
	AddLine(RpgPointFloat(rect.Right, rect.Bottom), RpgPointFloat(rect.Left, rect.Bottom), color);
	AddLine(RpgPointFloat(rect.Left, rect.Bottom), RpgPointFloat(rect.Left, rect.Top), color);
}


void RpgRenderer2D::PreRender(RpgRenderFrameContext& frameContext) noexcept
{
	auto LocalFunc_UpdateMeshBatchDraw = [&](FFrameData& frame, const FMesh& mesh, FDrawBatchArray& batchDraws)
	{
		const RpgSharedMaterial& material = MaterialInstanceTextures[mesh.MaterialInstanceIndex].Material;
		const int shaderMaterialId = frameContext.MaterialResource->AddMaterial(material);

		int batchDrawIndex = RPG_INDEX_INVALID;
		if (batchDraws.AddUnique(FDrawBatch(shaderMaterialId), &batchDrawIndex))
		{
			FDrawBatch& batchDraw = batchDraws[batchDrawIndex];
			batchDraw.IndexStart = frame.BatchMeshIndices.GetCount();
			batchDraw.IndexCount = 0;
		}

		FDrawBatch& batchDraw = batchDraws[batchDrawIndex];
		RPG_Assert(batchDraw.ShaderMaterialId == shaderMaterialId);

		const uint32_t vertexOffset = frame.BatchMeshVertices.GetCount();
		if (vertexOffset > 0)
		{
			RpgVertexGeometryFactory::UpdateBatchIndices(MeshIndices, vertexOffset, mesh.DataIndexStart, mesh.DataIndexCount);
		}

		batchDraw.IndexCount += mesh.DataIndexCount;

		frame.BatchMeshVertices.InsertAtRange(MeshVertices.GetData(mesh.DataVertexStart), mesh.DataVertexCount, RPG_INDEX_LAST);
		frame.BatchMeshIndices.InsertAtRange(MeshIndices.GetData(mesh.DataIndexStart), mesh.DataIndexCount, RPG_INDEX_LAST);
	};


	FFrameData& frame = FrameDatas[frameContext.Index];

	auto& batchClipDrawMeshes = frame.BatchDrawClipMeshes;

	for (int c = 0; c < Clips.GetCount(); ++c)
	{
		const FClip& clip = Clips[c];
		if (clip.Shapes.GetCount() == 0 && clip.Texts.GetCount() == 0)
		{
			continue;
		}

		int batchClipIndex = RPG_INDEX_INVALID;
		if (batchClipDrawMeshes.AddUnique(FDrawBatchClip(clip.Rect), &batchClipIndex))
		{
			FDrawBatchClip& draw = batchClipDrawMeshes[batchClipIndex];
			draw.Rect = clip.Rect;
		}

		FDrawBatchClip& batchClip = batchClipDrawMeshes[batchClipIndex];
		RPG_Assert(batchClip.Rect == clip.Rect);

		for (int s = 0; s < clip.Shapes.GetCount(); ++s)
		{
			LocalFunc_UpdateMeshBatchDraw(frame, clip.Shapes[s], batchClip.BatchDraws);
		}

		for (int t = 0; t < clip.Texts.GetCount(); ++t)
		{
			LocalFunc_UpdateMeshBatchDraw(frame, clip.Texts[t], batchClip.BatchDraws);
		}
	}

	if (!frame.BatchDrawClipMeshes.IsEmpty())
	{
		RpgD3D12::ResizeBuffer(frame.MeshVertexBuffer, frame.BatchMeshVertices.GetMemorySizeBytes_Allocated(), false);
		RPG_D3D12_SetDebugNameAllocation(frame.MeshVertexBuffer, "RES_R2D_MeshVtxBuffer");

		RpgD3D12::ResizeBuffer(frame.MeshIndexBuffer, frame.BatchMeshIndices.GetMemorySizeBytes_Allocated(), false);
		RPG_D3D12_SetDebugNameAllocation(frame.MeshIndexBuffer, "RES_R2D_MeshIdxBuffer");
	}

	if (!frame.LineVertices.IsEmpty())
	{
		RpgD3D12::ResizeBuffer(frame.LineVertexBuffer, frame.LineVertices.GetMemorySizeBytes_Allocated(), false);
		RPG_D3D12_SetDebugNameAllocation(frame.LineVertexBuffer, "RES_R2D_LineVtxBuffer");

		RpgD3D12::ResizeBuffer(frame.LineIndexBuffer, frame.LineIndices.GetMemorySizeBytes_Allocated(), false);
		RPG_D3D12_SetDebugNameAllocation(frame.LineIndexBuffer, "RES_R2D_LineIdxBuffer");

		FDrawBatchLine& batchDrawLine = frame.BatchDrawLine;
		batchDrawLine.MaterialId = frameContext.MaterialResource->AddMaterial(RpgMaterial::s_GetDefault(RpgMaterialDefault::DEBUG_PRIMITIVE_2D_LINE));
		batchDrawLine.IndexCount = frame.LineIndices.GetCount();
		batchDrawLine.IndexStart = 0;
		batchDrawLine.IndexVertexOffset = 0;
	}
}


void RpgRenderer2D::CommandCopy(const RpgRenderFrameContext& frameContext, ID3D12GraphicsCommandList* cmdList) noexcept
{
	FFrameData& frame = FrameDatas[frameContext.Index];

	if (frame.BatchMeshVertices.IsEmpty() && frame.LineVertices.IsEmpty())
	{
		return;
	}

	const size_t meshVtxSizeBytes = frame.BatchMeshVertices.GetMemorySizeBytes_Allocated();
	const size_t meshIdxSizeBytes = frame.BatchMeshIndices.GetMemorySizeBytes_Allocated();
	const size_t lineVtxSizeBytes = frame.LineVertices.GetMemorySizeBytes_Allocated();
	const size_t lineIdxSizeBytes = frame.LineIndices.GetMemorySizeBytes_Allocated();
	const size_t stagingSizeBytes = meshVtxSizeBytes + meshIdxSizeBytes + lineVtxSizeBytes + lineIdxSizeBytes;

	RpgD3D12::ResizeBuffer(frame.StagingBuffer, stagingSizeBytes, true);
	RPG_D3D12_SetDebugNameAllocation(frame.StagingBuffer, "STG_Mesh2D");

	uint8_t* stagingMap = RpgD3D12::MapBuffer<uint8_t>(frame.StagingBuffer.Get());
	{
		size_t stagingOffset = 0;
		ID3D12Resource* stagingResource = frame.StagingBuffer->GetResource();

		if (meshVtxSizeBytes > 0)
		{
			RPG_Assert(meshIdxSizeBytes > 0);

			RpgPlatformMemory::MemCopy(stagingMap + stagingOffset, frame.BatchMeshVertices.GetData(), meshVtxSizeBytes);
			cmdList->CopyBufferRegion(frame.MeshVertexBuffer->GetResource(), 0, stagingResource, stagingOffset, meshVtxSizeBytes);
			stagingOffset += meshVtxSizeBytes;

			RpgPlatformMemory::MemCopy(stagingMap + stagingOffset, frame.BatchMeshIndices.GetData(), meshIdxSizeBytes);
			cmdList->CopyBufferRegion(frame.MeshIndexBuffer->GetResource(), 0, stagingResource, stagingOffset, meshIdxSizeBytes);
			stagingOffset += meshIdxSizeBytes;
		}

		if (lineVtxSizeBytes > 0)
		{
			RPG_Assert(lineIdxSizeBytes > 0);

			RpgPlatformMemory::MemCopy(stagingMap + stagingOffset, frame.LineVertices.GetData(), lineVtxSizeBytes);
			cmdList->CopyBufferRegion(frame.LineVertexBuffer->GetResource(), 0, stagingResource, stagingOffset, lineVtxSizeBytes);
			stagingOffset += lineVtxSizeBytes;

			RpgPlatformMemory::MemCopy(stagingMap + stagingOffset, frame.LineIndices.GetData(), lineIdxSizeBytes);
			cmdList->CopyBufferRegion(frame.LineIndexBuffer->GetResource(), 0, stagingResource, stagingOffset, lineIdxSizeBytes);
			stagingOffset += lineIdxSizeBytes;
		}

		RPG_Check(stagingOffset == stagingSizeBytes);
	}
	RpgD3D12::UnmapBuffer(frame.StagingBuffer.Get());
}


void RpgRenderer2D::CommandDraw(const RpgRenderFrameContext& frameContext, ID3D12GraphicsCommandList* cmdList) noexcept
{
	const FFrameData& frame = FrameDatas[frameContext.Index];

	if (!frame.BatchDrawClipMeshes.IsEmpty())
	{
		// Bind vertex buffer
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		vertexBufferView.BufferLocation = frame.MeshVertexBuffer->GetResource()->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(RpgVertex::FMesh2D);
		vertexBufferView.SizeInBytes = static_cast<UINT>(frame.BatchMeshVertices.GetMemorySizeBytes_Allocated());

		cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);

		// Bind index buffer
		D3D12_INDEX_BUFFER_VIEW indexBufferView{};
		indexBufferView.BufferLocation = frame.MeshIndexBuffer->GetResource()->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		indexBufferView.SizeInBytes = static_cast<UINT>(frame.BatchMeshIndices.GetMemorySizeBytes_Allocated());

		cmdList->IASetIndexBuffer(&indexBufferView);

		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		RpgShaderViewportParameter viewportParam{};
		viewportParam.Width = static_cast<float>(ViewportDimension.X);
		viewportParam.Height = static_cast<float>(ViewportDimension.Y);
		cmdList->SetGraphicsRoot32BitConstants(RpgRenderPipeline::GRPI_VIEWPORT_PARAM, sizeof(RpgShaderViewportParameter) / 4, &viewportParam, 0);

		const RpgMaterialResource* materialResource = frameContext.MaterialResource;

		for (int i = 0; i < frame.BatchDrawClipMeshes.GetCount(); ++i)
		{
			const FDrawBatchClip& batchDrawClip = frame.BatchDrawClipMeshes[i];
			const RpgRect rect = batchDrawClip.Rect;
			RpgD3D12Command::SetScissor(cmdList, rect.Left, rect.Top, rect.Right, rect.Bottom);

			for (int d = 0; d < batchDrawClip.BatchDraws.GetCount(); ++d)
			{
				const FDrawBatch draw = batchDrawClip.BatchDraws[d];
				materialResource->CommandBindMaterial(cmdList, draw.ShaderMaterialId);

				cmdList->DrawIndexedInstanced(draw.IndexCount, 1, draw.IndexStart, 0, 0);
			}
		}
	}
	
	
	if (frame.BatchDrawLine.IndexCount > 0)
	{
		RpgD3D12Command::SetScissor(cmdList, 0, 0, ViewportDimension.X, ViewportDimension.Y);

		// Bind vertex buffer
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		vertexBufferView.BufferLocation = frame.LineVertexBuffer->GetResource()->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(RpgVertex::FPrimitive2D);
		vertexBufferView.SizeInBytes = static_cast<UINT>(frame.LineVertices.GetMemorySizeBytes_Allocated());

		cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);

		// Bind index buffer
		D3D12_INDEX_BUFFER_VIEW indexBufferView{};
		indexBufferView.BufferLocation = frame.LineIndexBuffer->GetResource()->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		indexBufferView.SizeInBytes = static_cast<UINT>(frame.LineIndices.GetMemorySizeBytes_Allocated());

		cmdList->IASetIndexBuffer(&indexBufferView);

		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

		RpgShaderViewportParameter viewportParam{};
		viewportParam.Width = static_cast<float>(ViewportDimension.X);
		viewportParam.Height = static_cast<float>(ViewportDimension.Y);
		cmdList->SetGraphicsRoot32BitConstants(RpgRenderPipeline::GRPI_VIEWPORT_PARAM, sizeof(RpgShaderViewportParameter) / 4, &viewportParam, 0);

		frameContext.MaterialResource->CommandBindMaterial(cmdList, frame.BatchDrawLine.MaterialId);

		cmdList->DrawIndexedInstanced(frame.BatchDrawLine.IndexCount, 1, frame.BatchDrawLine.IndexStart, frame.BatchDrawLine.IndexVertexOffset, 0);
	}
}
