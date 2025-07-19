#pragma once

#include "RpgMath.h"
#include "dsa/RpgArray.h"



namespace RpgVertex
{
	struct FPrimitive2D
	{
		DirectX::XMFLOAT2 Position;
		RpgColorRGBA Color;
	};

	struct FMesh2D
	{
		DirectX::XMFLOAT2 Position;
		DirectX::XMFLOAT2 TexCoord;
		RpgColorRGBA Color;
	};


	struct FPrimitive
	{
		RpgVector4 Position;
		RpgColorRGBA Color;
	};


	typedef RpgVector4 FMeshPosition;
	

	struct FMeshNormalTangent
	{
		RpgVector4 Normal;
		RpgVector4 Tangent;
	};


	typedef DirectX::XMFLOAT2 FMeshTexCoord;


	struct FMeshSkin
	{
		float BoneWeights0[4];
		float BoneWeights1[4];
		uint8_t BoneIndices0[4];
		uint8_t BoneIndices1[4];
		uint8_t BoneCount;
	};


	struct FMeshInstance
	{
		int CameraIndex;
		int TransformIndex;
	};


	typedef uint32_t FIndex;

}; // RpgVertex


typedef RpgArray<RpgVertex::FPrimitive2D, 64> RpgVertexPrimitive2DArray;
typedef RpgArray<RpgVertex::FMesh2D, 64> RpgVertexMesh2DArray;

typedef RpgArray<RpgVertex::FPrimitive, 64> RpgVertexPrimitiveArray;

typedef RpgArray<RpgVertex::FMeshPosition, 64> RpgVertexMeshPositionArray;
typedef RpgArray<RpgVertex::FMeshNormalTangent, 64> RpgVertexMeshNormalTangentArray;
typedef RpgArray<RpgVertex::FMeshTexCoord, 64> RpgVertexMeshTexCoordArray;
typedef RpgArray<RpgVertex::FMeshSkin, 64> RpgVertexMeshSkinArray;
typedef RpgArray<RpgVertex::FMeshInstance, 64> RpgVertexMeshInstanceArray;

typedef RpgArray<RpgVertex::FIndex, 64> RpgVertexIndexArray;



namespace RpgVertexGeometryFactory
{
	extern void UpdateBatchIndices(RpgVertexIndexArray& out_Indices, uint32_t baseVertex, int baseIndex, int batchIndexCount) noexcept;

	extern void CalculateSmoothTangents(RpgVertexMeshNormalTangentArray& out_normalTangents, const RpgVertexMeshPositionArray& positions, const RpgVertexMeshTexCoordArray& texCoords, const RpgVertexIndexArray& indices) noexcept;

	extern void CreateMeshBox(RpgVertexMeshPositionArray& out_Positions, RpgVertexMeshNormalTangentArray& out_NormalTangents, RpgVertexMeshTexCoordArray& out_TexCoords, RpgVertexIndexArray& out_Indices, const RpgVector3& boxMin, const RpgVector3& boxMax, float uvScale = 1.0f) noexcept;

};




class RpgVertexPrimitiveBatchLine
{

public:
	RpgVertexPrimitiveBatchLine() noexcept = default;

	void AddLine(const RpgVector3& p0, const RpgVector3& p1, RpgColorRGBA color) noexcept;
	void AddAABB(const RpgBoundingAABB& aabb, RpgColorRGBA color) noexcept;


	inline void Clear(bool bFreeMemory = false) noexcept
	{
		Vertices.Clear(bFreeMemory);
		Indices.Clear(bFreeMemory);
	}

	inline bool IsEmpty() const noexcept
	{
		return Vertices.IsEmpty();
	}

	inline int GetVertexCount() const noexcept
	{
		return Vertices.GetCount();
	}

	inline const RpgVertex::FPrimitive* GetVertexData() const noexcept
	{
		return Vertices.GetData();
	}

	inline size_t GetVertexSizeBytes() const noexcept
	{
		return Vertices.GetMemorySizeBytes_Allocated();
	}

	inline int GetIndexCount() const noexcept
	{
		return Indices.GetCount();
	}

	inline const RpgVertex::FIndex* GetIndexData() const noexcept
	{
		return Indices.GetData();
	}

	inline size_t GetIndexSizeBytes() const noexcept
	{
		return Indices.GetMemorySizeBytes_Allocated();
	}


private:
	RpgVertexPrimitiveArray Vertices;
	RpgVertexIndexArray Indices;

};




class RpgVertexPrimitiveBatchMesh
{

public:
	RpgVertexPrimitiveBatchMesh() noexcept = default;

	void AddTriangle(const RpgVector3& p0, const RpgVector3& p1, const RpgVector3& p2, RpgColorRGBA color) noexcept;
	

	inline void Clear(bool bFreeMemory = false) noexcept
	{
		Vertices.Clear(bFreeMemory);
		Indices.Clear(bFreeMemory);
	}

	inline int GetVertexCount() const noexcept
	{
		return Vertices.GetCount();
	}

	inline const RpgVertex::FPrimitive* GetVertexData() const noexcept
	{
		return Vertices.GetData();
	}

	inline size_t GetVertexSizeBytes() const noexcept
	{
		return Vertices.GetMemorySizeBytes_Allocated();
	}

	inline int GetIndexCount() const noexcept
	{
		return Indices.GetCount();
	}

	inline const RpgVertex::FIndex* GetIndexData() const noexcept
	{
		return Indices.GetData();
	}

	inline size_t GetIndexSizeBytes() const noexcept
	{
		return Indices.GetMemorySizeBytes_Allocated();
	}


private:
	RpgVertexPrimitiveArray Vertices;
	RpgVertexIndexArray Indices;

};
