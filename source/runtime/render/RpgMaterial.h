#pragma once

#include "core/RpgMath.h"
#include "shader/RpgShaderTypes.h"
#include "RpgTexture.h"



namespace RpgMaterialParameterTexture
{
	enum ESlot : uint16_t
	{
		BASE_COLOR = 0,
		NORMAL,
		METALNESS,
		SPECULAR,
		ROUGHNESS,
		AMBIENT_OCCLUSION,
		OPACITY_MASK,
		CUSTOM_0,
		CUSTOM_1,
		CUSTOM_2,
		CUSTOM_3,
		CUSTOM_4,
		CUSTOM_5,
		CUSTOM_6,
		CUSTOM_7,

		MAX_COUNT
	};

};
typedef RpgArray<RpgSharedTexture2D> RpgMaterialParameterTextureArray;



struct RpgMaterialParameterVector
{
	RpgName Name;
	RpgVector4 Value;

	inline bool operator==(const RpgName& rhs) const noexcept
	{
		return Name == rhs;
	}
};
typedef RpgArrayInline<RpgMaterialParameterVector, RPG_SHADER_MATERIAL_PARAM_VECTOR_COUNT> RpgMaterialParameterVectorArray;



struct RpgMaterialParameterScalar
{
	RpgName Name;
	float Value = 0.0f;

	inline bool operator==(const RpgName& rhs) const noexcept
	{
		return Name == rhs;
	}
};
typedef RpgArrayInline<RpgMaterialParameterScalar, RPG_SHADER_MATERIAL_PARAM_SCALAR_COUNT> RpgMaterialParameterScalarArray;




struct RpgMaterialParameterLayout
{
public:
	RpgMaterialParameterLayout() noexcept
	{
		Textures.Resize(RpgMaterialParameterTexture::MAX_COUNT);
	}


	inline void SetTextureValue(RpgMaterialParameterTexture::ESlot slot, const RpgSharedTexture2D& in_Value) noexcept
	{
		RPG_Check(slot < RpgMaterialParameterTexture::MAX_COUNT);
		Textures[slot] = in_Value;
	}

	inline RpgMaterialParameterTextureArray& GetTextures() noexcept
	{
		return Textures;
	}

	inline const RpgMaterialParameterTextureArray& GetTextures() const noexcept
	{
		return Textures;
	}


	inline void AddVector(RpgName in_ParamName, RpgVector4 in_DefaultValue) noexcept
	{
		RpgMaterialParameterVector& vector = Vectors.Add();
		vector.Name = in_ParamName;
		vector.Value = in_DefaultValue;
	}

	inline void SetVectorValue(const RpgName& paramName, RpgVector4 in_Value) noexcept
	{
		const int index = Vectors.FindIndexByCompare(paramName);
		RPG_CheckV(index != RPG_INDEX_INVALID, "Vector param %s not found!", *paramName);
		Vectors[index].Value = in_Value;
	}

	inline RpgVector4 GetVectorValue(const RpgName& paramName) const noexcept
	{
		const int index = Vectors.FindIndexByCompare(paramName);
		RPG_CheckV(index != RPG_INDEX_INVALID, "Vector param %s not found!", *paramName);

		return Vectors[index].Value;
	}

	inline RpgMaterialParameterVectorArray& GetVectors() noexcept
	{
		return Vectors;
	}

	inline const RpgMaterialParameterVectorArray& GetVectors() const noexcept
	{
		return Vectors;
	}


	inline void AddScalar(RpgName in_ParamName, float in_DefaultValue) noexcept
	{
		RpgMaterialParameterScalar& scalar = Scalars.Add();
		scalar.Name = in_ParamName;
		scalar.Value = in_DefaultValue;
	}

	inline void SetScalarValue(const RpgName& paramName, float in_Value) noexcept
	{
		const int index = Scalars.FindIndexByCompare(paramName);
		RPG_CheckV(index != RPG_INDEX_INVALID, "Scalar param %s not found!", *paramName);
		Scalars[index].Value = in_Value;
	}

	inline float GetScalarValue(const RpgName& paramName) const noexcept
	{
		const int index = Scalars.FindIndexByCompare(paramName);
		RPG_CheckV(index != RPG_INDEX_INVALID, "Scalar param %s not found!", *paramName);

		return Scalars[index].Value;
	}

	inline RpgMaterialParameterScalarArray& GetScalars() noexcept
	{
		return Scalars;
	}

	inline const RpgMaterialParameterScalarArray& GetScalars() const noexcept
	{
		return Scalars;
	}


private:
	RpgMaterialParameterTextureArray Textures;
	RpgMaterialParameterVectorArray Vectors;
	RpgMaterialParameterScalarArray Scalars;

};



namespace RpgMaterialDefault
{
	enum EType : uint8_t
	{
		GUI = 0,
		FONT,

		MESH_PHONG,
		FULLSCREEN,

		DEBUG_PRIMITIVE_2D_LINE,
		DEBUG_PRIMITIVE_2D_MESH,
		DEBUG_PRIMITIVE_LINE,
		DEBUG_PRIMITIVE_LINE_NO_DEPTH,
		DEBUG_PRIMITIVE_MESH,
		DEBUG_PRIMITIVE_MESH_NO_DEPTH,

		MAX_COUNT
	};

};


typedef RpgSharedPtr<class RpgMaterial> RpgSharedMaterial;

class RpgMaterial
{
	RPG_NOCOPY(RpgMaterial)

public:
	RpgMaterial(const RpgName& in_Name, const RpgRenderPipelineState& in_RenderState, const RpgMaterialParameterLayout& in_ParameterLayout) noexcept;
	RpgMaterial(const RpgName& in_Name, const RpgSharedMaterial& in_ParentMaterial) noexcept;
	~RpgMaterial() noexcept;


	inline const RpgName& GetName() const noexcept
	{
		return Name;
	}

	inline const RpgSharedMaterial& GetParentMaterial() const noexcept
	{
		return ParentMaterial;
	}

	inline const RpgRenderPipelineState& GetRenderState() const noexcept
	{
		return RenderState;
	}

	inline bool IsInstance() const noexcept
	{
		return ParentMaterial.IsValid();
	}

	inline bool IsTransparency() const noexcept
	{
		return RenderState.BlendMode == RpgRenderColorBlendMode::TRANSPARENCY;
	}


	inline void SetParameterTextureValue(RpgMaterialParameterTexture::ESlot slot, const RpgSharedTexture2D& in_Value) noexcept
	{
		AcquireSRWLockExclusive(&ParameterTextureLock);
		ParameterLayout.SetTextureValue(slot, in_Value);
		ReleaseSRWLockExclusive(&ParameterTextureLock);
	}

	inline void SetParameterVectorValue(const RpgName& paramName, RpgVector4 in_Value) noexcept
	{
		AcquireSRWLockExclusive(&ParameterVectorLock);
		ParameterLayout.SetVectorValue(paramName, in_Value);
		ReleaseSRWLockExclusive(&ParameterVectorLock);
	}

	inline void SetParameterScalarValue(const RpgName& paramName, float in_Value) noexcept
	{
		AcquireSRWLockExclusive(&ParameterScalarLock);
		ParameterLayout.SetScalarValue(paramName, in_Value);
		ReleaseSRWLockExclusive(&ParameterScalarLock);
	}

	inline RpgSharedTexture2D GetParameterTextureValue(RpgMaterialParameterTexture::ESlot slot) const noexcept
	{
		AcquireSRWLockShared(&ParameterTextureLock);
		const RpgSharedTexture2D textureValue = ParameterLayout.GetTextures()[slot];
		ReleaseSRWLockShared(&ParameterTextureLock);

		return textureValue;
	}

	inline RpgVector4 GetParameterVectorValue(const RpgName& paramName) const noexcept
	{
		AcquireSRWLockShared(&ParameterVectorLock);
		const RpgVector4 vectorValue = ParameterLayout.GetVectorValue(paramName);
		ReleaseSRWLockShared(&ParameterVectorLock);

		return vectorValue;
	}

	inline float GetParameterScalarValue(const RpgName& paramName) const noexcept
	{
		AcquireSRWLockShared(&ParameterScalarLock);
		const float scalarValue = ParameterLayout.GetScalarValue(paramName);
		ReleaseSRWLockShared(&ParameterScalarLock);

		return scalarValue;
	}


	inline RpgMaterialParameterTextureArray& ParameterTexturesReadLock() noexcept
	{
		AcquireSRWLockShared(&ParameterTextureLock);
		return ParameterLayout.GetTextures();
	}

	inline void ParameterTexturesReadUnlock() noexcept
	{
		ReleaseSRWLockShared(&ParameterTextureLock);
	}


	inline const RpgMaterialParameterVectorArray& ParameterVectorsReadLock() const noexcept
	{
		AcquireSRWLockShared(&ParameterVectorLock);
		return ParameterLayout.GetVectors();
	}

	inline void ParameterVectorsReadUnlock() noexcept
	{
		ReleaseSRWLockShared(&ParameterVectorLock);
	}


	inline const RpgMaterialParameterScalarArray& ParameterScalarsReadLock() const noexcept
	{
		AcquireSRWLockShared(&ParameterScalarLock);
		return ParameterLayout.GetScalars();
	}

	inline void ParameterScalarsReadUnlock() noexcept
	{
		ReleaseSRWLockShared(&ParameterScalarLock);
	}


	inline void MarkPipelinePending() noexcept
	{
		RPG_Assert((Flags & (FLAG_PSO_Compiling | FLAG_PSO_Compiled)) == 0);
		Flags |= FLAG_PSO_Pending;
	}

	inline void MarkPipelineCompiling() noexcept
	{
		RPG_Assert((Flags & FLAG_PSO_Pending) && (Flags & FLAG_PSO_Compiled) == 0);
		Flags &= ~FLAG_PSO_Pending;
		Flags |= FLAG_PSO_Compiling;
	}

	inline void MarkPipelineCompiled() noexcept
	{
		RPG_Assert((Flags & FLAG_PSO_Compiling) && (Flags & FLAG_PSO_Pending) == 0);
		Flags &= ~FLAG_PSO_Compiling;
		Flags |= FLAG_PSO_Compiled;
	}

	inline bool IsPipelinePending() const noexcept
	{
		return (Flags & FLAG_PSO_Pending);
	}

	inline bool IsPipelineCompiling() const noexcept
	{
		return (Flags & FLAG_PSO_Compiling);
	}

	inline bool IsPipelineCompiled() const noexcept
	{
		return (Flags & FLAG_PSO_Compiled);
	}


private:
	RpgName Name;
	RpgSharedMaterial ParentMaterial;
	RpgRenderPipelineState RenderState;
	RpgMaterialParameterLayout ParameterLayout;
	mutable SRWLOCK ParameterTextureLock;
	mutable SRWLOCK ParameterVectorLock;
	mutable SRWLOCK ParameterScalarLock;


	enum EFlag : uint8_t
	{
		FLAG_None			= (0),
		FLAG_Loading		= (1 << 0),
		FLAG_Loaded			= (1 << 1),
		FLAG_PendingDestroy = (1 << 2),
		FLAG_PSO_Pending	= (1 << 3),
		FLAG_PSO_Compiling	= (1 << 4),
		FLAG_PSO_Compiled	= (1 << 5),
	};
	uint8_t Flags;


public:
	[[nodiscard]] static RpgSharedMaterial s_CreateShared(const RpgName& name, const RpgRenderPipelineState& renderState, const RpgMaterialParameterLayout& parameterLayout = RpgMaterialParameterLayout()) noexcept;
	[[nodiscard]] static RpgSharedMaterial s_CreateSharedInstance(const RpgName& name, const RpgSharedMaterial& parentMaterial) noexcept;

	static void s_CreateDefaults() noexcept;
	static void s_DestroyDefaults() noexcept;
	static const RpgSharedMaterial& s_GetDefault(RpgMaterialDefault::EType type) noexcept;

};
