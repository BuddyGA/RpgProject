#pragma once

#include "core/world/RpgGameObject.h"
#include "shader/RpgShaderTypes.h"
#include "RpgMesh.h"
#include "RpgMaterial.h"



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


public:
	RpgMeshResource() noexcept;
	FMeshID AddMesh(const RpgSharedMesh& mesh, int& out_IndexCount, int& out_IndexStart, int& out_IndexVertexOffset) noexcept;
	void UpdateResources() noexcept;
	void CommandCopy(ID3D12GraphicsCommandList* cmdList) noexcept;


	inline void Reset() noexcept
	{
		TotalVertexCount = 0;
		TotalIndexCount = 0;
		MeshDatas.Clear();
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_Position() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshPosition>(VertexPositionBuffer->GetResource());
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_NormalTangent() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshNormalTangent>(VertexNormalTangentBuffer->GetResource());
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_TexCoord() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshTexCoord>(VertexTexCoordBuffer->GetResource());
	}

	inline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const noexcept
	{
		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = IndexBuffer->GetResource()->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = static_cast<UINT>(sizeof(RpgVertex::FIndex) * TotalIndexCount);

		return view;
	}


private:
	template<typename TVertex>
	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(ID3D12Resource* vertexBufferResource) const noexcept
	{
		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = vertexBufferResource->GetGPUVirtualAddress();
		view.StrideInBytes = sizeof(TVertex);
		view.SizeInBytes = static_cast<UINT>(sizeof(TVertex) * TotalVertexCount);

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

	// Total vertex count
	int TotalVertexCount;

	// Total index count
	int TotalIndexCount;

	// (VBO) Input vertex position
	ComPtr<D3D12MA::Allocation> VertexPositionBuffer;

	// (VBO) Input vertex normal-tangent
	ComPtr<D3D12MA::Allocation> VertexNormalTangentBuffer;

	// (VBO) Input vertex texcoord
	ComPtr<D3D12MA::Allocation> VertexTexCoordBuffer;

	// (IBO) Input index
	ComPtr<D3D12MA::Allocation> IndexBuffer;

	// Staging buffer
	ComPtr<D3D12MA::Allocation> StagingBuffer;

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
		VertexCount = 0;
		IndexCount = 0;
		SkinnedVertexCount = 0;
		SkinnedIndexCount = 0;
	}

	inline int GetVertexCount() const noexcept
	{
		return VertexCount;
	}

	inline int GetIndexCount() const noexcept
	{
		return IndexCount;
	}

	inline const RpgArray<RpgShaderSkinnedObjectParameter>& GetObjectParameters() const noexcept
	{
		return ObjectParameters;
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_Position() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshPosition>(VertexPositionBuffer->GetResource(), VertexCount);
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_NormalTangent() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshNormalTangent>(VertexNormalTangentBuffer->GetResource(), VertexCount);
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_TexCoord() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshTexCoord>(VertexTexCoordBuffer->GetResource(), VertexCount);
	}

	inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView_Skin() const noexcept
	{
		return GetVertexBufferView<RpgVertex::FMeshSkin>(VertexSkinBuffer->GetResource(), VertexCount);
	}

	inline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const noexcept
	{
		D3D12_INDEX_BUFFER_VIEW view{};
		view.BufferLocation = IndexBuffer->GetResource()->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = static_cast<UINT>(sizeof(RpgVertex::FIndex) * IndexCount);

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
		return VertexPositionBuffer->GetResource();
	}

	inline ID3D12Resource* GetResourceVertexNormalTangent() const noexcept
	{
		return VertexNormalTangentBuffer->GetResource();
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
		int VertexCount{ 0 };
		int IndexStart{ 0 };
		int IndexCount{ 0 };
		int InstanceCount{ 0 };
	};

	// Per mesh data
	RpgArray<FMeshData> MeshDatas;

	// Skeleton bone skinning transforms
	RpgArray<RpgMatrixTransform> SkeletonBoneSkinningTransforms;

	// Per object parameter
	RpgArray<RpgShaderSkinnedObjectParameter> ObjectParameters;

	// Vertex count
	int VertexCount;

	// Index count
	int IndexCount;

	// Skinned vertex count
	int SkinnedVertexCount;

	// Skinned index count
	int SkinnedIndexCount;


	// (VBO) Vertex position
	ComPtr<D3D12MA::Allocation> VertexPositionBuffer;

	// (VBO) Vertex normal-tangent
	ComPtr<D3D12MA::Allocation> VertexNormalTangentBuffer;

	// (VBO) Vertex texcoord
	ComPtr<D3D12MA::Allocation> VertexTexCoordBuffer;

	// (VBO) Vertex skin (bone, weight)
	ComPtr<D3D12MA::Allocation> VertexSkinBuffer;

	// (IBO) Vertex index
	ComPtr<D3D12MA::Allocation> IndexBuffer;


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
	ComPtr<D3D12MA::Allocation> StagingBuffer;

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
		RpgPlatformMemory::MemCopy(&WorldData.AmbientColorStrength, &color, sizeof(RpgColorLinear));
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
	// @param uniqueTagId - Unique tag identifier. Normally the value is game object id but could be anything as long as it is to prevent adding the same transform object multiple times
	// @param worldTransformMatrix - World transformation matrix
	// @returns Transform id in this world resource
	FTransformID AddTransform(int uniqueTagId, const RpgMatrixTransform& worldTransformMatrix) noexcept;


	// Add point light
	// @param uniqueTagId - Unique tag identifier. Normally the value is game object id but could be anything as long as it is to prevent adding the same light object multiple times
	// @param worldPosition - World position
	// @param colorIntensity - Light color (RGB), and light intensity (A)
	// @param attRadius - Attenuation radius factor
	// @param attFallOffExp - Attenuation falloff exponential factor
	// @returns Light id in this world resource
	FLightID AddLight_Point(int uniqueTagId, RpgVector3 worldPosition, RpgColorLinear colorIntensity, float attRadius, float attFallOffExp) noexcept;
	
	// Add spot light
	// @param uniqueTagId - Unique tag identifier. Normally the value is game object id but could be anything as long as it is to prevent adding the same light object multiple times
	// @param worldPosition - World position
	// @param worldDirection - World direction 
	// @param colorIntensity - Light color (RGB), and light intensity (A)
	// @param attRadius - Attenuation radius factor
	// @param attFallOffExp - Attenuation falloff exponential factor
	// @param innerConeDegree - Inner cone (umbra) in degree
	// @param outerConeDegree - Outer cone (penumbra) in degree
	// @returns Light id in this world resource
	FLightID AddLight_Spot(int uniqueTagId, RpgVector3 worldPosition, RpgVector3 worldDirection, RpgColorLinear colorIntensity, float attRadius, float attFallOffExp, float innerConeDegree, float outerConeDegree) noexcept;


	inline void SetLightShadow(FLightID lightId, FViewID shadowCameraId, int shadowTextureDescriptorIndex) noexcept
	{
		RpgShaderLight& data = WorldData.Lights[lightId];
		data.ShadowViewIndex = shadowCameraId;
		data.ShadowTextureDescriptorIndex = shadowTextureDescriptorIndex;
	}


private:
	struct FTagLightID
	{
		int TagId{ 0 };
		int LightId{ 0 };

		inline bool operator==(int rhs) const noexcept
		{
			return TagId == rhs;
		}
	};

	RpgArrayInline<FTagLightID, RPG_SHADER_MAX_LIGHT> CachedTagLights;

	RpgShaderWorldData WorldData;
	ComPtr<D3D12MA::Allocation> WorldConstantBuffer;


	struct FTagTransformID
	{
		int TagId{ 0 };
		int TransformId{ 0 };

		inline bool operator==(int rhs) const noexcept
		{
			return TagId == rhs;
		}
	};

	RpgArray<FTagTransformID> CachedTagTransforms;
	RpgArray<RpgMatrixTransform> TransformDatas;
	ComPtr<D3D12MA::Allocation> TransformStructBuffer;

	ComPtr<D3D12MA::Allocation> StagingBuffer;



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
	RpgGameObjectID GameObject;
	RpgMatrixTransform WorldTransformMatrix;
	RpgSharedMaterial Material;
	RpgSharedMesh Mesh;
	int Lod{ 0 };
};


struct RpgSceneLight
{
	RpgGameObjectID GameObject;
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
