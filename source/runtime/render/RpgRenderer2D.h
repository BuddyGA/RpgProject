#pragma once

#include "RpgFont.h"
#include "RpgRenderResource.h"



class RpgRenderer2D
{
	RPG_NOCOPY(RpgRenderer2D)

public:
	RpgRenderer2D() noexcept;
	~RpgRenderer2D() noexcept;

private:
	int GetOrAddMaterialInstanceId(const RpgSharedMaterial& material, const RpgSharedTexture2D& texture, bool bIsText) noexcept;

public:
	void Begin(int frameIndex, RpgPointInt viewportDimension) noexcept;
	void End(int frameIndex) noexcept;


	inline RpgPointInt GetViewportDimension() const noexcept
	{
		return ViewportDimension;
	}


	inline void PushOrder(uint8_t value) noexcept
	{
		const int prevOrderIndex = CurrentOrderIndex;

		FFrameData& frame = FrameDatas[FrameIndex];
		CurrentOrderIndex = frame.Orders.GetCount();

		FOrder& order = frame.Orders.Add();
		order.PrevOrderIndex = prevOrderIndex;
	}

	inline void PopOrder() noexcept
	{
		FFrameData& frame = FrameDatas[FrameIndex];
		
		FOrder& order = frame.Orders[CurrentOrderIndex];
		RPG_Check(order.CurrentClipIndex == RPG_INDEX_INVALID);

		CurrentOrderIndex = order.PrevOrderIndex;
	}


	inline void PushClipRect(RpgRectInt rect) noexcept
	{
		FFrameData& frame = FrameDatas[FrameIndex];
		FOrder& order = frame.Orders[CurrentOrderIndex];

		const int parentClipIndex = order.CurrentClipIndex;
		order.CurrentClipIndex = order.Clips.GetCount();

		FClip& clip = order.Clips.Add();
		clip.Rect = rect;
		clip.ParentClipIndex = parentClipIndex;
	}

	inline void PopClipRect() noexcept
	{
		FFrameData& frame = FrameDatas[FrameIndex];
		FOrder& order = frame.Orders[CurrentOrderIndex];
		order.CurrentClipIndex = order.Clips[order.CurrentClipIndex].ParentClipIndex;
	}


	void AddMeshRect(RpgRectFloat rect, RpgColorRGBA color, const RpgSharedTexture2D& texture = RpgSharedTexture2D(), const RpgSharedMaterial& material = RpgSharedMaterial()) noexcept;
	void AddText(const char* text, int length, RpgPointFloat position, RpgColorRGBA color, const RpgSharedFont& font = RpgSharedFont(), const RpgSharedMaterial& material = RpgSharedMaterial()) noexcept;
	void AddLine(RpgPointFloat p0, RpgPointFloat p1, RpgColorRGBA color) noexcept;
	void AddLineRect(RpgRectFloat rect, RpgColorRGBA color) noexcept;

	void PreRender(RpgRenderFrameContext& frameContext) noexcept;
	void CommandCopy(const RpgRenderFrameContext& frameContext, ID3D12GraphicsCommandList* cmdList) noexcept;
	void CommandDraw(const RpgRenderFrameContext& frameContext, ID3D12GraphicsCommandList* cmdList) noexcept;


private:
	RpgSharedMaterial DefaultMaterialMesh;
	RpgSharedMaterial DefaultMaterialFont;
	RpgSharedTexture2D DefaultTexture;
	RpgSharedFont DefaultFont;
	RpgPointInt ViewportDimension;


	struct FMaterialInstanceTexture
	{
		RpgSharedMaterial Material;
		RpgSharedTexture2D Texture;
	};
	RpgArray<FMaterialInstanceTexture> MaterialInstanceTextures;


	struct FMesh
	{
		int MaterialInstanceIndex{ RPG_INDEX_INVALID };
		int DataVertexStart{ 0 };
		int DataVertexCount{ 0 };
		int DataIndexStart{ 0 };
		int DataIndexCount{ 0 };
	};


	struct FClip
	{
		RpgRectInt Rect;
		RpgArrayInline<FMesh, 64> Shapes;
		RpgArrayInline<FMesh, 32> Texts;
		int ParentClipIndex{ RPG_INDEX_INVALID };
	};


	struct FOrder
	{
		RpgArrayInline<FClip, 16> Clips;
		int CurrentClipIndex{ RPG_INDEX_INVALID };
		int PrevOrderIndex{ RPG_INDEX_INVALID };
	};

	int CurrentOrderIndex;
	

	struct FDrawBatch
	{
		int ShaderMaterialId;
		int IndexStart;
		int IndexCount;


		FDrawBatch(int in_MaterialId = RPG_INDEX_INVALID) noexcept
			: ShaderMaterialId(in_MaterialId)
			, IndexStart(0)
			, IndexCount(0)
		{
		}

		inline bool operator==(const FDrawBatch& rhs) const noexcept
		{
			return ShaderMaterialId == rhs.ShaderMaterialId;
		}
	};
	typedef RpgArrayInline<FDrawBatch, 16> FDrawBatchArray;


	struct FDrawBatchClip
	{
		RpgRectInt Rect;
		FDrawBatchArray BatchDraws;


		FDrawBatchClip(const RpgRectInt& in_Rect = RpgRectInt()) noexcept
			: Rect(in_Rect)
		{
		}

		inline bool operator==(const FDrawBatchClip& rhs) const noexcept
		{
			return Rect == rhs.Rect;
		}
	};


	struct FDrawBatchOrder
	{
		RpgArrayInline<FDrawBatchClip, 8> Clips;
	};


	struct FDrawBatchLine
	{
		RpgMaterialResource::FMaterialID MaterialId{ RPG_INDEX_INVALID };
		int IndexCount{ 0 };
		int IndexStart{ 0 };
		int IndexVertexOffset{ 0 };
	};


	struct FFrameData
	{
		RpgArray<FOrder> Orders;
		RpgVertexMesh2DArray MeshVertices;
		RpgVertexIndexArray MeshIndices;

		RpgVertexMesh2DArray BatchMeshVertices;
		RpgVertexIndexArray BatchMeshIndices;
		RpgArray<FDrawBatchOrder> BatchDrawOrders;

		RpgVertexPrimitive2DArray LineVertices;
		RpgVertexIndexArray LineIndices;
		FDrawBatchLine BatchDrawLine;

		ComPtr<D3D12MA::Allocation> StagingBuffer;
		ComPtr<D3D12MA::Allocation> MeshVertexBuffer;
		ComPtr<D3D12MA::Allocation> MeshIndexBuffer;
		ComPtr<D3D12MA::Allocation> LineVertexBuffer;
		ComPtr<D3D12MA::Allocation> LineIndexBuffer;
	};
	
	FFrameData FrameDatas[RPG_FRAME_BUFFERING];
	int FrameIndex;

};
