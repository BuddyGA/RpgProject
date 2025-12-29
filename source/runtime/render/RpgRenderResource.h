#pragma once

#include "core/world/RpgGameObject.h"
#include "shader/RpgShaderTypes.h"
#include "asset/RpgMesh.h"
#include "asset/RpgMaterial.h"



// Global material resource
// Texture descriptor dynamic indexing
// Parameter vector and scalar
class RpgMaterialResource
{
public:
	typedef int FMaterialID;


public:
	RpgMaterialResource() noexcept;
	void UpdateResources(int frameIndex) noexcept;
	void CommandCopy(ID3D12GraphicsCommandList* cmdList) noexcept;
	void CommandBindShaderResources(ID3D12GraphicsCommandList* cmdList) const noexcept;
	void CommandBindMaterial(ID3D12GraphicsCommandList* cmdList, FMaterialID materialId) const noexcept;


	inline void Reset() noexcept
	{
		Materials.Clear();
		TextureDescriptors.Clear();
		VectorScalarData.Clear();
		ParameterRootConstants.Clear();
		UploadTextureIndices.Clear(true);
	}

	inline FMaterialID AddMaterial(const RpgSharedMaterial& material) noexcept
	{
		FMaterialID id = Materials.FindIndexByValue(material);

		if (id == RPG_INDEX_INVALID)
		{
			id = Materials.GetCount();
			Materials.AddValue(material);
		}

		return id;
	}


private:
	RpgArray<RpgSharedMaterial, 8> Materials;


	struct FTextureDescriptor
	{
		RpgWeakPtr<RpgTexture2D> WeakTexture;
		RpgD3D12::FResourceDescriptor Descriptor;


		FTextureDescriptor(RpgSharedTexture2D in_Texture = RpgSharedTexture2D()) noexcept
			: WeakTexture(in_Texture)
		{
		}

		inline bool operator==(const FTextureDescriptor& rhs) const noexcept
		{
			return WeakTexture == rhs.WeakTexture;
		}

	};
	RpgArray<FTextureDescriptor, 8> TextureDescriptors;


	RpgArray<RpgShaderMaterialVectorScalarData, 16> VectorScalarData;
	ComPtr<D3D12MA::Allocation> VectorScalarStructBuffer;

	RpgArray<RpgShaderMaterialParameter, 16> ParameterRootConstants;

	// Material vector-scalar data staging buffer
	ComPtr<D3D12MA::Allocation> MaterialStagingBuffer;

	RpgArray<int, 16> UploadTextureIndices;

};




// Global mesh resource
class RpgMeshResource
{
public:
	typedef int FMeshID;
	typedef int FTerrainID;


public:
	RpgMeshResource() noexcept;
	FMeshID AddMesh(const RpgSharedMesh& mesh, int& out_IndexCount, int& out_IndexStart, int& out_IndexVertexOffset) noexcept;
	FTerrainID AddTerrain(const RpgVertexMeshPositionArray* vertexPositions, const RpgVertexMeshNormalTangentArray* vertexNormalTangents, const RpgVertexMeshTexCoordArray* vertexTexCoords, const RpgVertexIndexArray* indices, int& out_IndexCount, int& out_IndexStart, int& out_IndexVertexOffset) noexcept;
	void UpdateResources() noexcept;
	void CommandCopy(ID3D12GraphicsCommandList* cmdList) noexcept;


	inline void Reset() noexcept
	{
		MeshDatas.Clear();
		MeshVertexCount = 0;
		MeshIndexCount = 0;

		TerrainDatas.Clear();
		TerrainVertexCount = 0;
		TerrainIndexCount = 0;
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetMeshVertexBufferView_Position() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshPosition>(MeshVertexPositionBuffer->GetResource(), MeshVertexCount);
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetMeshVertexBufferView_NormalTangent() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshNormalTangent>(MeshVertexNormalTangentBuffer->GetResource(), MeshVertexCount);
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetMeshVertexBufferView_TexCoord() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshTexCoord>(MeshVertexTexCoordBuffer->GetResource(), MeshVertexCount);
	}

	inline D3D12_INDEX_BUFFER_VIEW GetMeshIndexBufferView() const noexcept
	{
		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = MeshIndexBuffer->GetResource()->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = static_cast<UINT>(sizeof(RpgVertex::FIndex) * MeshIndexCount);

		return view;
	}


	inline D3D12_VERTEX_BUFFER_VIEW GetTerrainVertexBufferView_Position() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshPosition>(TerrainVertexPositionBuffer->GetResource(), TerrainVertexCount);
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetTerrainVertexBufferView_NormalTangent() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshNormalTangent>(TerrainVertexNormalTangentBuffer->GetResource(), TerrainVertexCount);
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetTerrainVertexBufferView_TexCoord() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshTexCoord>(TerrainVertexTexCoordBuffer->GetResource(), TerrainVertexCount);
	}

	inline D3D12_INDEX_BUFFER_VIEW GetTerrainIndexBufferView() const noexcept
	{
		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = TerrainIndexBuffer->GetResource()->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = static_cast<UINT>(sizeof(RpgVertex::FIndex) * TerrainIndexCount);

		return view;
	}


private:
	template<typename TVertex>
	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(ID3D12Resource* vertexBufferResource, int vertexCount) const noexcept
	{
		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = vertexBufferResource->GetGPUVirtualAddress();
		view.StrideInBytes = sizeof(TVertex);
		view.SizeInBytes = static_cast<UINT>(sizeof(TVertex) * vertexCount);

		return view;
	}


private:
	struct FMeshData
	{
		RpgSharedMesh Mesh;
		int VertexStart{ 0 };
		int VertexCount{ 0 };
		int IndexStart{ 0 };
		int IndexCount{ 0 };
	};
	RpgArray<FMeshData, 16> MeshDatas;

	// (VBO) Mesh input vertex position
	ComPtr<D3D12MA::Allocation> MeshVertexPositionBuffer;

	// (VBO) Mesh input vertex normal-tangent
	ComPtr<D3D12MA::Allocation> MeshVertexNormalTangentBuffer;

	// (VBO) Mesh input vertex texcoord
	ComPtr<D3D12MA::Allocation> MeshVertexTexCoordBuffer;

	// (IBO) Mesh input index
	ComPtr<D3D12MA::Allocation> MeshIndexBuffer;

	// Mesh staging buffer
	ComPtr<D3D12MA::Allocation> MeshStagingBuffer;

	// Mesh vertex count
	int MeshVertexCount;

	// Mesh index count
	int MeshIndexCount;


	struct FTerrainData
	{
		const RpgVertexMeshPositionArray* VertexPositions{ nullptr };
		const RpgVertexMeshNormalTangentArray* VertexNormalTangents{ nullptr };
		const RpgVertexMeshTexCoordArray* VertexTexCoords{ nullptr };
		const RpgVertexIndexArray* VertexIndices{ nullptr };
		int VertexStart{ 0 };
		int VertexCount{ 0 };
		int IndexStart{ 0 };
		int IndexCount{ 0 };
	};
	RpgArray<FTerrainData> TerrainDatas;

	// (VBO) Terrain input vertex position
	ComPtr<D3D12MA::Allocation> TerrainVertexPositionBuffer;

	// (VBO) Terrain input vertex normal-tangent
	ComPtr<D3D12MA::Allocation> TerrainVertexNormalTangentBuffer;

	// (VBO) Terrain input vertex texcoord
	ComPtr<D3D12MA::Allocation> TerrainVertexTexCoordBuffer;

	// (IBO) Terrain input index
	ComPtr<D3D12MA::Allocation> TerrainIndexBuffer;

	// Terrain staging buffer
	ComPtr<D3D12MA::Allocation> TerrainStagingBuffer;

	// Terrain vertex count
	int TerrainVertexCount;

	// Terrain index count
	int TerrainIndexCount;

};




// Global mesh skinned (skeletal mesh) resource
class RpgMeshSkinnedResource
{
public:
	typedef int FMeshID;
	typedef int FSkeletonID;


public:
	RpgMeshSkinnedResource() noexcept;
	
	FMeshID AddMesh(const RpgSharedMesh& mesh, int& out_IndexCount, int& out_IndexStart, int& out_IndexVertexOffset) noexcept;
	FSkeletonID AddObjectBoneSkinningTransforms(FMeshID meshId, const RpgArray<RpgMatrixTransform>& boneSkinningTransforms) noexcept;

	void UpdateResources() noexcept;
	void CommandCopy(ID3D12GraphicsCommandList* cmdList) noexcept;


	inline void Reset() noexcept
	{
		MeshDatas.Clear();
		SkeletonBoneSkinningTransforms.Clear();
		ObjectParameters.Clear();
		MeshVertexCount = 0;
		MeshIndexCount = 0;
		SkinnedVertexCount = 0;
		SkinnedIndexCount = 0;
	}

	inline int GetVertexCount() const noexcept
	{
		return MeshVertexCount;
	}

	inline int GetIndexCount() const noexcept
	{
		return MeshIndexCount;
	}

	inline const RpgArray<RpgShaderSkinnedObjectParameter>& GetObjectParameters() const noexcept
	{
		return ObjectParameters;
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_Position() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshPosition>(MeshVertexPositionBuffer->GetResource(), MeshVertexCount);
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_NormalTangent() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshNormalTangent>(MeshVertexNormalTangentBuffer->GetResource(), MeshVertexCount);
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_TexCoord() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshTexCoord>(MeshVertexTexCoordBuffer->GetResource(), MeshVertexCount);
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_Skin() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshSkin>(VertexSkinBuffer->GetResource(), MeshVertexCount);
	}

	inline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const noexcept
	{
		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = MeshIndexBuffer->GetResource()->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = static_cast<UINT>(sizeof(RpgVertex::FIndex) * MeshIndexCount);

		return view;
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_SkinnedPosition() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshPosition>(SkinnedVertexPositionBuffer->GetResource(), SkinnedVertexCount);
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_SkinnedNormalTangent() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshNormalTangent>(SkinnedVertexNormalTangentBuffer->GetResource(), SkinnedVertexCount);
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_SkinnedTexCoord() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshTexCoord>(SkinnedVertexTexCoordBuffer->GetResource(), SkinnedVertexCount);
	}

	inline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView_Skinned() const noexcept
	{
		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = SkinnedIndexBuffer->GetResource()->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = static_cast<UINT>(sizeof(RpgVertex::FIndex) * SkinnedIndexCount);

		return view;
	}

	inline ID3D12Resource* GetResourceVertexPosition() const noexcept
	{
		return MeshVertexPositionBuffer->GetResource();
	}

	inline ID3D12Resource* GetResourceVertexNormalTangent() const noexcept
	{
		return MeshVertexNormalTangentBuffer->GetResource();
	}

	inline ID3D12Resource* GetResourceVertexSkin() const noexcept
	{
		return VertexSkinBuffer->GetResource();
	}

	inline ID3D12Resource* GetResourceSkeletonBoneSkinning() const noexcept
	{
		return SkeletonBoneSkinningBuffer->GetResource();
	}

	inline ID3D12Resource* GetResourceSkinnedVertexPosition() const noexcept
	{
		return SkinnedVertexPositionBuffer->GetResource();
	}

	inline ID3D12Resource* GetResourceSkinnedVertexNormalTangent() const noexcept
	{
		return SkinnedVertexNormalTangentBuffer->GetResource();
	}


private:
	template<typename TVertex>
	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(ID3D12Resource* vertexBufferResource, int vertexCount) const noexcept
	{
		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = vertexBufferResource->GetGPUVirtualAddress();
		view.StrideInBytes = sizeof(TVertex);
		view.SizeInBytes = static_cast<UINT>(sizeof(TVertex) * vertexCount);

		return view;
	}


private:
	struct FMeshData
	{
		RpgSharedMesh Mesh;
		int VertexStart{ 0 };
		int MeshVertexCount{ 0 };
		int IndexStart{ 0 };
		int MeshIndexCount{ 0 };
		int InstanceCount{ 0 };
	};

	// Per mesh data
	RpgArray<FMeshData> MeshDatas;

	// Skeleton bone skinning transforms
	RpgArray<RpgMatrixTransform> SkeletonBoneSkinningTransforms;

	// Per object parameter
	RpgArray<RpgShaderSkinnedObjectParameter> ObjectParameters;

	// Vertex count
	int MeshVertexCount;

	// Index count
	int MeshIndexCount;

	// Skinned vertex count
	int SkinnedVertexCount;

	// Skinned index count
	int SkinnedIndexCount;


	// (VBO) Vertex position
	ComPtr<D3D12MA::Allocation> MeshVertexPositionBuffer;

	// (VBO) Vertex normal-tangent
	ComPtr<D3D12MA::Allocation> MeshVertexNormalTangentBuffer;

	// (VBO) Vertex texcoord
	ComPtr<D3D12MA::Allocation> MeshVertexTexCoordBuffer;

	// (VBO) Vertex skin (bone, weight)
	ComPtr<D3D12MA::Allocation> VertexSkinBuffer;

	// (IBO) Vertex index
	ComPtr<D3D12MA::Allocation> MeshIndexBuffer;


	// (SRV) Skeleton bone skinning buffer
	ComPtr<D3D12MA::Allocation> SkeletonBoneSkinningBuffer;


	// (UAV->VBO) Skinned vertex position from compute
	ComPtr<D3D12MA::Allocation> SkinnedVertexPositionBuffer;

	// (UAV->VBO) Skinned vertex normal-tangent from compute
	ComPtr<D3D12MA::Allocation> SkinnedVertexNormalTangentBuffer;

	// (VBO) Skinned vertex texcoord
	ComPtr<D3D12MA::Allocation> SkinnedVertexTexCoordBuffer;

	// (IBO) Skinned vertex index
	ComPtr<D3D12MA::Allocation> SkinnedIndexBuffer;


	// Staging buffer
	ComPtr<D3D12MA::Allocation> MeshStagingBuffer;

};




// Per-world resource
class RpgWorldResource
{
public:
	typedef int FViewID;
	typedef int FLightID;
	typedef int FTransformID;


public:
	RpgWorldResource() noexcept;
	void Reset() noexcept;
	void UpdateResources() noexcept;
	void CommandCopy(ID3D12GraphicsCommandList* cmdList) noexcept;
	void CommandBindShaderResources(ID3D12GraphicsCommandList* cmdList) const noexcept;


	inline void SetDeltaTime(float deltaTime) noexcept
	{
		WorldData.DeltaTime = deltaTime;
	}

	inline void SetAmbientColorStrength(RpgColorLinear color) noexcept
	{
		static_assert(sizeof(RpgShaderFloat4) == sizeof(RpgColorLinear), "Size not equals!");
		RpgPlatformMemory::Copy(&WorldData.AmbientColorStrength, &color, sizeof(RpgColorLinear));
	}


	inline FViewID AddView(const RpgMatrixTransform& viewMatrix, const RpgMatrixProjection& projMatrix, const RpgVector3& worldPosition, float nearClipZ, float farClipZ) noexcept
	{
		const FViewID id = WorldData.ViewCount++;
		RPG_Check(WorldData.ViewCount <= RPG_SHADER_MAX_VIEW);

		RpgShaderView& camera = WorldData.Views[id];
		camera.ViewMatrix = viewMatrix.Xmm;
		camera.ViewProjectionMatrix = RpgMatrixViewProjection(viewMatrix, projMatrix).Xmm;
		camera.WorldPosition = worldPosition.Xmm;
		camera.NearClipZ = nearClipZ;
		camera.FarClipZ = farClipZ;

		return id;
	}


	// Add transform
	// @param gameObject - Gameobject for unique identifier to prevent adding the same transform object multiple times
	// @param worldTransformMatrix - World transformation matrix
	// @returns Transform id in this world resource
	FTransformID AddTransform(RpgGameObject gameObject, const RpgMatrixTransform& worldTransformMatrix) noexcept;


	// Add point light
	// @param gameObject - Gameobject for unique identifier to prevent adding the same light multiple times
	// @param worldPosition - World position
	// @param colorIntensity - Light color (RGB), and light intensity (A)
	// @param attRadius - Attenuation radius factor
	// @param attFallOffExp - Attenuation falloff exponential factor
	// @returns Light id in this world resource
	FLightID AddLight_Point(RpgGameObject gameObject, RpgVector3 worldPosition, RpgColorLinear colorIntensity, float attRadius, float attFallOffExp) noexcept;
	
	// Add spot light
	// @param gameObject - Gameobject for unique identifier to prevent adding the same light multiple times
	// @param worldPosition - World position
	// @param worldDirection - World direction 
	// @param colorIntensity - Light color (RGB), and light intensity (A)
	// @param attRadius - Attenuation radius factor
	// @param attFallOffExp - Attenuation falloff exponential factor
	// @param innerConeDegree - Inner cone (umbra) in degree
	// @param outerConeDegree - Outer cone (penumbra) in degree
	// @returns Light id in this world resource
	FLightID AddLight_Spot(RpgGameObject gameObject, RpgVector3 worldPosition, RpgVector3 worldDirection, RpgColorLinear colorIntensity, float attRadius, float attFallOffExp, float innerConeDegree, float outerConeDegree) noexcept;


	inline void SetLightShadow(FLightID lightId, FViewID shadowCameraId, int shadowTextureDescriptorIndex) noexcept
	{
		RpgShaderLight& data = WorldData.Lights[lightId];
		data.ShadowViewIndex = shadowCameraId;
		data.ShadowTextureDescriptorIndex = shadowTextureDescriptorIndex;
	}


private:
	struct FGameObjectLightID
	{
		RpgGameObject GameObject;
		int LightId{ 0 };

		inline bool operator==(RpgGameObject rhs) const noexcept
		{
			return GameObject == rhs;
		}
	};

	RpgArrayInline<FGameObjectLightID, RPG_SHADER_MAX_LIGHT> CachedGameObjectLights;

	RpgShaderWorldData WorldData;
	ComPtr<D3D12MA::Allocation> WorldConstantBuffer;


	struct FGameObjectTransformID
	{
		RpgGameObject GameObject;
		int TransformId{ 0 };

		inline bool operator==(const RpgGameObject& rhs) const noexcept
		{
			return GameObject == rhs;
		}
	};

	RpgArray<FGameObjectTransformID> CachedGameObjectTransforms;
	RpgArray<RpgMatrixTransform> TransformDatas;
	ComPtr<D3D12MA::Allocation> TransformStructBuffer;

	ComPtr<D3D12MA::Allocation> MeshStagingBuffer;



#ifndef RPG_BUILD_SHIPPING
private:
	size_t DebugLineVertexSizeBytes;
	size_t DebugLineIndexSizeBytes;
	ComPtr<D3D12MA::Allocation> DebugLineVertexBuffer;
	ComPtr<D3D12MA::Allocation> DebugLineIndexBuffer;

public:
	RpgVertexPrimitiveBatchLine DebugLine;
	RpgVertexPrimitiveBatchLine DebugLineNoDepth;


public:
	void Debug_CommandDrawIndexed_Line(ID3D12GraphicsCommandList* cmdList, const RpgMaterialResource* materialResource, RpgMaterialResource::FMaterialID materialId, RpgMaterialResource::FMaterialID noDepthMaterialId, FViewID cameraId) const noexcept;
#endif // !RPG_BUILD_SHIPPING

};



struct RpgSceneMesh
{
	RpgGameObject GameObject;
	RpgMatrixTransform WorldTransformMatrix;
	RpgSharedMaterial Material;
	RpgSharedMesh Mesh;
	int Lod{ 0 };
};


struct RpgSceneTerrain
{
	RpgGameObject GameObject;
	RpgMatrixTransform WorldTransformMatrix;
	RpgSharedMaterial Material;
	const RpgVertexMeshPositionArray* VertexPositions{ nullptr };
	const RpgVertexMeshNormalTangentArray* VertexNormalTangents{ nullptr };
	const RpgVertexMeshTexCoordArray* VertexTexCoords{ nullptr };
	const RpgVertexIndexArray* VertexIndices{ nullptr };
};


struct RpgSceneLight
{
	RpgGameObject GameObject;
	RpgTransform WorldTransform;
	RpgRenderLight::EType Type{ RpgRenderLight::TYPE_NONE };
	RpgColorLinear ColorIntensity;
	float AttenuationRadius{ 0.0f };
	float AttenuationFallOffExp{ 0.0f };
	float SpotInnerConeDegree{ 0.0f };
	float SpotOuterConeDegree{ 0.0f };
	RpgShadowViewport* ShadowViewport{ nullptr };
};



struct RpgDrawIndexed
{
	RpgMaterialResource::FMaterialID Material;
	RpgShaderObjectParameter ObjectParam;
	int IndexCount{ 0 };
	int IndexStart{ 0 };
	int IndexVertexOffset{ 0 };
};


struct RpgDrawIndexedDepth
{
	RpgShaderObjectParameter ObjectParam;
	int IndexCount{ 0 };
	int IndexStart{ 0 };
	int IndexVertexOffset{ 0 };
};
