#include "RpgRenderer2D.h"
#include "RpgRenderPipeline.h"
#include "RpgRenderResource.h"



RpgRenderer2D::RpgRenderer2D() noexcept
{
	CurrentOrderIndex = RPG_INDEX_INVALID;
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

	FrameIndex = frameIndex;
	CurrentOrderIndex = RPG_INDEX_INVALID;
	
	FFrameData& frame = FrameDatas[FrameIndex];
	frame.Orders.Clear();
	frame.MeshVertices.Clear();
	frame.MeshIndices.Clear();
	frame.BatchMeshVertices.Clear();
	frame.BatchMeshIndices.Clear();
	frame.BatchDrawOrders.Clear();
	frame.LineVertices.Clear();
	frame.LineIndices.Clear();
	frame.BatchDrawLine = FDrawBatchLine();

	SetOrder(255);
	SetClipRect(RpgRectInt(0, 0, ViewportDimension.X, ViewportDimension.Y));
}


void RpgRenderer2D::End(int frameIndex) noexcept
{
	RPG_Assert(FrameIndex == frameIndex);
}


void RpgRenderer2D::SetOrder(uint8_t value) noexcept
{
	FFrameData& frame = FrameDatas[FrameIndex];

	if (CurrentOrderIndex == RPG_INDEX_INVALID)
	{
		RPG_Check(frame.Orders.IsEmpty());

		CurrentOrderIndex = 0;

		FOrder& order = frame.Orders.Add();
		order.Value = value;
		order.CurrentClipIndex = RPG_INDEX_INVALID;

		return;
	}

	if (frame.Orders[CurrentOrderIndex].Value == value)
	{
		return;
	}

	bool bAddNewOrder = true;
	int insertIndex = RPG_INDEX_INVALID;
	for (int o = 0; o < frame.Orders.GetCount(); ++o)
	{
		const FOrder& check = frame.Orders[o];
		if (value == check.Value)
		{
			CurrentOrderIndex = o;
			bAddNewOrder = false;
			break;
		}

		if (value > check.Value)
		{
			insertIndex = o;
			break;
		}
	}

	if (!bAddNewOrder)
	{
		return;
	}

	FOrder* newOrder = nullptr;

	if (insertIndex == RPG_INDEX_INVALID)
	{
		CurrentOrderIndex = frame.Orders.GetCount();
		newOrder = &frame.Orders.Add();
	}
	else
	{
		CurrentOrderIndex = insertIndex;
		frame.Orders.InsertAt(FOrder(), insertIndex);
		newOrder = &frame.Orders[insertIndex];
	}

	newOrder->Value = value;
	newOrder->CurrentClipIndex = RPG_INDEX_INVALID;
}


void RpgRenderer2D::SetClipRect(RpgRectInt rect) noexcept
{
	FOrder& order = FrameDatas[FrameIndex].Orders[CurrentOrderIndex];

	if (order.CurrentClipIndex == RPG_INDEX_INVALID)
	{
		RPG_Check(order.Clips.IsEmpty());
		order.CurrentClipIndex = 0;

		FClip& clip = order.Clips.Add();
		clip.Rect = rect;

		return;
	}

	FClip& currentClip = order.Clips[order.CurrentClipIndex];
	if (currentClip.Rect == rect)
	{
		return;
	}

	bool bAddNewClip = true;

	for (int c = 0; c < order.Clips.GetCount(); ++c)
	{
		const FClip& check = order.Clips[c];
		if (check.Rect == rect)
		{
			order.CurrentClipIndex = c;
			bAddNewClip = false;
			break;
		}
	}

	if (bAddNewClip)
	{
		order.CurrentClipIndex = order.Clips.GetCount();
		FClip& newClip = order.Clips.Add();
		newClip.Rect = rect;
	}
}


void RpgRenderer2D::AddMeshRect(RpgRectFloat rect, RpgColor color, const RpgSharedTexture2D& texture, const RpgSharedMaterial& material) noexcept
{
	const RpgPointFloat dimension = rect.GetDimension();
	if (dimension.X == 0 || dimension.Y == 0)
	{
		return;
	}

	FFrameData& frame = FrameDatas[FrameIndex];
	FOrder& order = frame.Orders[CurrentOrderIndex];
	FClip& clip = order.Clips[order.CurrentClipIndex];

	FMesh& mesh = clip.Shapes.Add();
	
	mesh.MaterialInstanceIndex = GetOrAddMaterialInstanceId(
		material ? material : DefaultMaterialMesh,
		texture ? texture : DefaultTexture,
		false
	);

	mesh.DataVertexStart = frame.MeshVertices.GetCount();
	mesh.DataVertexCount = 4;
	mesh.DataIndexStart = frame.MeshIndices.GetCount();
	mesh.DataIndexCount = 6;

	const RpgVertex::FMesh2D vertices[4] =
	{
		{ DirectX::XMFLOAT2(rect.Left, rect.Top), DirectX::XMFLOAT2(0.0f, 0.0f), color },
		{ DirectX::XMFLOAT2(rect.Right, rect.Top), DirectX::XMFLOAT2(1.0f, 0.0f), color },
		{ DirectX::XMFLOAT2(rect.Right, rect.Bottom), DirectX::XMFLOAT2(1.0f, 1.0f), color },
		{ DirectX::XMFLOAT2(rect.Left, rect.Bottom), DirectX::XMFLOAT2(0.0f, 1.0f), color },
	};

	frame.MeshVertices.InsertAtRange(vertices, 4, RPG_INDEX_LAST);

	const RpgVertex::FIndex indices[6] =
	{
		0, 1, 2,
		2, 3, 0
	};

	frame.MeshIndices.InsertAtRange(indices, 6, RPG_INDEX_LAST);
}


void RpgRenderer2D::AddText(const char* text, int length, RpgPointFloat position, RpgColor color, const RpgSharedFont& font, const RpgSharedMaterial& material) noexcept
{
	if (text == nullptr || length == 0)
	{
		return;
	}

	const RpgFont* useFont = font ? font.Get() : DefaultFont.Get();

	FFrameData& frame = FrameDatas[FrameIndex];
	FOrder& order = frame.Orders[CurrentOrderIndex];
	FClip& clip = order.Clips[order.CurrentClipIndex];

	FMesh& mesh = clip.Texts.Add();

	mesh.MaterialInstanceIndex = GetOrAddMaterialInstanceId(
		material ? material : DefaultMaterialFont,
		useFont->GetTexture(),
		true
	);

	mesh.DataVertexStart = frame.MeshVertices.GetCount();
	mesh.DataIndexStart = frame.MeshIndices.GetCount();

	useFont->GenerateTextVertex(text, length, position, color, frame.MeshVertices, frame.MeshIndices, &mesh.DataVertexCount, &mesh.DataIndexCount);
}


void RpgRenderer2D::AddLine(RpgPointFloat p0, RpgPointFloat p1, RpgColor color) noexcept
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


void RpgRenderer2D::AddLineRect(RpgRectFloat rect, RpgColor color) noexcept
{
	AddLine(RpgPointFloat(rect.Left, rect.Top), RpgPointFloat(rect.Right, rect.Top), color);
	AddLine(RpgPointFloat(rect.Right, rect.Top), RpgPointFloat(rect.Right, rect.Bottom), color);
	AddLine(RpgPointFloat(rect.Right, rect.Bottom), RpgPointFloat(rect.Left, rect.Bottom), color);
	AddLine(RpgPointFloat(rect.Left, rect.Bottom), RpgPointFloat(rect.Left, rect.Top), color);
}


void RpgRenderer2D::PreRender(RpgRenderFrameContext& frameContext) noexcept
{
	auto LocalFunc_UpdateMeshBatchDraw = [&](FFrameData& frame, const FMesh& mesh, FDrawBatchArray& drawBatchArray)
	{
		const RpgSharedMaterial& material = MaterialInstanceTextures[mesh.MaterialInstanceIndex].Material;
		const int shaderMaterialId = frameContext.MaterialResource->AddMaterial(material);

		int batchDrawIndex = RPG_INDEX_INVALID;
		if (drawBatchArray.AddUnique(FDrawBatch(shaderMaterialId), &batchDrawIndex))
		{
			FDrawBatch& draw = drawBatchArray[batchDrawIndex];
			draw.VertexStart = frame.BatchMeshVertices.GetCount();
			draw.VertexCount = 0;
			draw.IndexStart = frame.BatchMeshIndices.GetCount();
			draw.IndexCount = 0;
		}

		FDrawBatch& draw = drawBatchArray[batchDrawIndex];
		RPG_Assert(draw.ShaderMaterialId == shaderMaterialId);

		const uint32_t vertexOffset = static_cast<uint32_t>(draw.VertexStart + draw.VertexCount);
		if (vertexOffset > 0)
		{
			RpgVertexGeometryFactory::UpdateBatchIndices(frame.MeshIndices, vertexOffset, mesh.DataIndexStart, mesh.DataIndexCount);
		}

		draw.VertexCount += mesh.DataVertexCount;
		draw.IndexCount += mesh.DataIndexCount;

		if (draw.IndexCount == 120)
		{
			//RPG_DebugBreak();
		}

		frame.BatchMeshVertices.InsertAtRange(frame.MeshVertices.GetData(mesh.DataVertexStart), mesh.DataVertexCount, RPG_INDEX_LAST);
		frame.BatchMeshIndices.InsertAtRange(frame.MeshIndices.GetData(mesh.DataIndexStart), mesh.DataIndexCount, RPG_INDEX_LAST);
	};


	FFrameData& frame = FrameDatas[frameContext.Index];
	uint8_t prevOrder = 255;

	for (int i = 0; i < frame.Orders.GetCount(); ++i)
	{
		const FOrder& order = frame.Orders[i];
		RPG_Check(order.Value <= prevOrder);
		prevOrder = order.Value;

		FDrawBatchOrder& batchOrder = frame.BatchDrawOrders.Add();
		RPG_Check(batchOrder.Clips.IsEmpty());

		for (int c = 0; c < order.Clips.GetCount(); ++c)
		{
			const FClip& clip = order.Clips[c];
			if (clip.Shapes.IsEmpty() && clip.Texts.IsEmpty())
			{
				continue;
			}

			FDrawBatchClip& batchClip = batchOrder.Clips.Add();
			batchClip.Rect = clip.Rect;
			RPG_Check(batchClip.BatchDraws.IsEmpty());

			for (int s = 0; s < clip.Shapes.GetCount(); ++s)
			{
				LocalFunc_UpdateMeshBatchDraw(frame, clip.Shapes[s], batchClip.BatchDraws);
			}

			for (int t = 0; t < clip.Texts.GetCount(); ++t)
			{
				LocalFunc_UpdateMeshBatchDraw(frame, clip.Texts[t], batchClip.BatchDraws);
			}
		}
	}
	

	if (!frame.BatchMeshVertices.IsEmpty())
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

	// Set root signature and global texture descriptor table (dynamic indexing)
	cmdList->SetGraphicsRootSignature(RpgRenderPipeline::GetRootSignatureGraphics());

	// Set descriptor table (texture dynamic indexing)
	ID3D12DescriptorHeap* textureDescriptorHeap = RpgD3D12::GetDescriptorHeap_TDI(frameContext.Index);
	cmdList->SetDescriptorHeaps(1, &textureDescriptorHeap);
	cmdList->SetGraphicsRootDescriptorTable(RpgRenderPipeline::GRPI_TEXTURES, textureDescriptorHeap->GetGPUDescriptorHandleForHeapStart());


	if (!frame.BatchMeshVertices.IsEmpty())
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

		const RpgArray<FDrawBatchOrder>& batchOrders = frame.BatchDrawOrders;
		for (int i = 0; i < batchOrders.GetCount(); ++i)
		{
			for (int c = 0; c < batchOrders[i].Clips.GetCount(); ++c)
			{
				const FDrawBatchClip& batchDrawClip = batchOrders[i].Clips[c];
				const RpgRect rect = batchDrawClip.Rect;
				RpgD3D12Command::SetScissor(cmdList, rect.Left, rect.Top, rect.Right, rect.Bottom);

				RPG_Check(!batchDrawClip.BatchDraws.IsEmpty());

				for (int d = 0; d < batchDrawClip.BatchDraws.GetCount(); ++d)
				{
					const FDrawBatch draw = batchDrawClip.BatchDraws[d];
					materialResource->CommandBindMaterial(cmdList, draw.ShaderMaterialId);

					cmdList->DrawIndexedInstanced(draw.IndexCount, 1, draw.IndexStart, 0, 0);
				}
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
