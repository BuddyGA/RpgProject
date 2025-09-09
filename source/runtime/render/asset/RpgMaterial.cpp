#include "RpgMaterial.h"
#include "core/asset/RpgAssetSystem.h"
#include "shader/RpgShaderTypes.h"


RPG_LOG_DECLARE_CATEGORY_STATIC(RpgLogMaterial, VERBOSITY_DEBUG)



RpgMaterial::RpgMaterial(const RpgName& in_Name) noexcept
	: RpgAssetObject(in_Name)
{
	Flags = FLAG_None;
	InitializeSRWLock(&ParameterTextureLock);
	InitializeSRWLock(&ParameterVectorLock);
	InitializeSRWLock(&ParameterScalarLock);
}


RpgMaterial::RpgMaterial(const RpgName& in_Name, const RpgRenderPipelineState& in_RenderState, const RpgMaterialParameterLayout& in_ParameterLayout) noexcept
	: RpgMaterial(in_Name)
{
	Name = in_Name;
	RenderState = in_RenderState;
	ParameterLayout = in_ParameterLayout;
}


RpgMaterial::RpgMaterial(const RpgName& in_Name, const RpgSharedMaterial& in_ParentMaterial) noexcept
	: RpgMaterial(in_Name, in_ParentMaterial->RenderState, in_ParentMaterial->ParameterLayout)
{
	Name = in_Name;
	Flags = FLAG_Instance;
	ParentMaterial = in_ParentMaterial;
}


RpgMaterial::~RpgMaterial() noexcept
{
	RPG_LogDebug(RpgLogTemp, "Destroy material [%s]", *Name);
}


void RpgMaterial::AssetStreamWrite(RpgStreamWriter& writer) noexcept
{
	// only save non-runtime flags
	const uint16_t savedFlags = (Flags & ~RUNTIME_FLAGS);
	writer.Write(savedFlags);

	if (ParentMaterial)
	{
		RPG_Check(ParentMaterial->IsAssetLoaded());
		writer.Write(ParentMaterial->GetAssetPath());
	}
	else
	{
		writer.Write(RpgString());
	}

	writer.Write(RenderState);

	for (RpgSharedTexture2D& texture : ParameterLayout.GetTextures())
	{
		if (texture)
		{
			RPG_Check(texture->IsAssetLoaded());
			writer.Write(texture->GetAssetPath());
		}
		else
		{
			writer.Write(RpgString());
		}
	}

	writer.Write(ParameterLayout.GetVectors());
	writer.Write(ParameterLayout.GetScalars());
}


void RpgMaterial::AssetStreamRead(RpgStreamReader& reader, uint16_t version) noexcept
{
	RpgString tempAssetPath;

	reader.Read(Name);
	
	reader.Read(Flags);
	Flags |= FLAG_Runtime_Loading;

	reader.Read(tempAssetPath);

	if (!tempAssetPath.IsEmpty())
	{
		ParentMaterial = g_AssetSystem->LoadAsset<RpgMaterial>(tempAssetPath);
	}

	reader.Read(RenderState);

	RpgMaterialParameterTextureArray& paramTextures = ParameterLayout.GetTextures();
	for (int i = 0; i < paramTextures.GetCount(); ++i)
	{
		reader.Read(tempAssetPath);

		if (!tempAssetPath.IsEmpty())
		{
			paramTextures[i] = g_AssetSystem->LoadAsset<RpgTexture2D>(tempAssetPath);
		}
	}

	reader.Read(ParameterLayout.GetVectors());
	reader.Read(ParameterLayout.GetScalars());

	IsAssetLoaded();
}


bool RpgMaterial::IsAssetLoaded() noexcept
{
	if (Flags & FLAG_Runtime_Loaded)
	{
		return true;
	}

	// if not loaded check external asset refs
	RPG_Check(Flags & FLAG_Runtime_Loading);

	// check parent material
	if (ParentMaterial && !ParentMaterial->IsAssetLoaded())
	{
		return false;
	}

	// check textures
	for (RpgSharedTexture2D& texture : ParameterLayout.GetTextures())
	{
		if (texture && !texture->IsAssetLoaded())
		{
			return false;
		}
	}

	Flags = (Flags & ~FLAG_Runtime_Loading) | FLAG_Runtime_Loaded;

	return true;
}


void RpgMaterial::GetExternalAssetReferences(RpgAssetReferences& out_AssetRefs) noexcept
{
	if (ParentMaterial)
	{
		RPG_Check(ParentMaterial->IsAssetLoaded());
		ParentMaterial->GetExternalAssetReferences(out_AssetRefs);
		out_AssetRefs.Add(ParentMaterial->GetAssetPath());
	}

	for (RpgSharedTexture2D& texture : ParameterLayout.GetTextures())
	{
		if (texture)
		{
			RPG_Check(texture->IsAssetLoaded());
			out_AssetRefs.Add(texture->GetAssetPath());
		}
	}
}


void RpgMaterial::SetAssetLoading() noexcept
{
	Flags |= FLAG_Runtime_Loading;
}


RpgSharedMaterial RpgMaterial::s_CreateShared(const RpgName& name, const RpgRenderPipelineState& renderState, const RpgMaterialParameterLayout& parameterLayout) noexcept
{
	return RpgSharedMaterial(new RpgMaterial(name, renderState, parameterLayout));
}


RpgSharedMaterial RpgMaterial::s_CreateSharedInstance(const RpgName& name, const RpgSharedMaterial& parentMaterial) noexcept
{
	return RpgSharedMaterial(new RpgMaterial(name, parentMaterial));
}



static RpgArray<RpgSharedMaterial> DefaultMaterials;


void RpgMaterial::s_CreateDefaults() noexcept
{
	RPG_LogDebug(RpgLogMaterial, "Create default materials...");

	DefaultMaterials.Resize(RpgMaterialDefault::MAX_COUNT);

	// mesh2d
	{
		RpgRenderPipelineState renderState{};
		renderState.PixelShaderName = RPG_SHADER_NAME_GUI_PS;
		renderState.VertexMode = RpgRenderVertexMode::GUI;
		renderState.BlendMode = RpgRenderColorBlendMode::TRANSPARENCY;
		renderState.RasterMode = RpgRenderRasterMode::SOLID;

		DefaultMaterials[RpgMaterialDefault::GUI] = s_CreateShared("mat_def_gui", renderState);
		DefaultMaterials[RpgMaterialDefault::GUI]->Flags |= FLAG_Default | FLAG_Runtime_Loaded;
	}

	// font2d
	{
		RpgRenderPipelineState renderState{};
		renderState.PixelShaderName = RPG_SHADER_NAME_GUI_Font_PS;
		renderState.VertexMode = RpgRenderVertexMode::GUI;
		renderState.BlendMode = RpgRenderColorBlendMode::TRANSPARENCY;
		renderState.RasterMode = RpgRenderRasterMode::SOLID;

		DefaultMaterials[RpgMaterialDefault::FONT] = s_CreateShared("mat_def_font", renderState);
		DefaultMaterials[RpgMaterialDefault::FONT]->Flags |= FLAG_Default | FLAG_Runtime_Loaded;
	}

	// mesh phong
	{
		RpgRenderPipelineState renderState{};
		renderState.PixelShaderName = RPG_SHADER_NAME_PixelForwardPhong;
		renderState.VertexMode = RpgRenderVertexMode::MESH;
		renderState.RasterMode = RpgRenderRasterMode::SOLID;
		renderState.BlendMode = RpgRenderColorBlendMode::NONE;
		renderState.bDepthTest = true;
		renderState.bDepthWrite = true;

		RpgMaterialParameterLayout paramLayout{};
		paramLayout.AddVector("base_color", RpgVector4(1.0f));
		paramLayout.AddVector("specular_color", RpgVector4(1.0f));
		paramLayout.AddScalar("shininess", 32.0f);
		paramLayout.AddScalar("opacity", 1.0f);

		DefaultMaterials[RpgMaterialDefault::MESH_PHONG] = s_CreateShared("mat_def_mesh_phong", renderState, paramLayout);
		DefaultMaterials[RpgMaterialDefault::MESH_PHONG]->Flags |= FLAG_Default | FLAG_Runtime_Loaded;
	}


	// fullscreen
	{
		RpgRenderPipelineState renderState{};
		renderState.VertexShaderName = RPG_SHADER_NAME_PostProcessFullscreen_VS;
		renderState.PixelShaderName = RPG_SHADER_NAME_PostProcessFullscreen_PS;
		renderState.VertexMode = RpgRenderVertexMode::NONE;
		renderState.BlendMode = RpgRenderColorBlendMode::NONE;
		renderState.RasterMode = RpgRenderRasterMode::SOLID;

		RpgMaterialParameterLayout paramLayout{};
		paramLayout.AddScalar("gamma", 2.2f);

		DefaultMaterials[RpgMaterialDefault::POSTPROCESS_FULLSCREEN] = s_CreateShared("mat_def_postprocess_fullscreen", renderState, paramLayout);
		DefaultMaterials[RpgMaterialDefault::POSTPROCESS_FULLSCREEN]->Flags |= FLAG_Default | FLAG_Runtime_Loaded;
	}


	// Debug primitive2d line
	{
		RpgRenderPipelineState renderState{};
		renderState.PixelShaderName = RPG_SHADER_NAME_PixelColor;
		renderState.VertexMode = RpgRenderVertexMode::PRIMITIVE_2D;
		renderState.BlendMode = RpgRenderColorBlendMode::NONE;
		renderState.RasterMode = RpgRenderRasterMode::LINE;

		DefaultMaterials[RpgMaterialDefault::DEBUG_PRIMITIVE_LINE_2D] = s_CreateShared("mat_def_dbg_prim_line_2d", renderState);
		DefaultMaterials[RpgMaterialDefault::DEBUG_PRIMITIVE_LINE_2D]->Flags |= FLAG_Default | FLAG_Runtime_Loaded;
	}

	// Debug primitive2d mesh
	{
		RpgRenderPipelineState renderState{};
		renderState.PixelShaderName = RPG_SHADER_NAME_PixelColor;
		renderState.VertexMode = RpgRenderVertexMode::PRIMITIVE_2D;
		renderState.BlendMode = RpgRenderColorBlendMode::NONE;
		renderState.RasterMode = RpgRenderRasterMode::SOLID;

		DefaultMaterials[RpgMaterialDefault::DEBUG_PRIMITIVE_MESH_2D] = s_CreateShared("mat_def_dbg_prim_mesh_2d", renderState);
		DefaultMaterials[RpgMaterialDefault::DEBUG_PRIMITIVE_MESH_2D]->Flags |= FLAG_Default | FLAG_Runtime_Loaded;
	}

	// Debug primitive line
	{
		RpgRenderPipelineState renderState{};
		renderState.PixelShaderName = RPG_SHADER_NAME_PixelColor;
		renderState.VertexMode = RpgRenderVertexMode::PRIMITIVE;
		renderState.BlendMode = RpgRenderColorBlendMode::NONE;
		renderState.RasterMode = RpgRenderRasterMode::LINE;
		renderState.bDepthTest = true;
		renderState.bDepthWrite = false;

		DefaultMaterials[RpgMaterialDefault::DEBUG_PRIMITIVE_LINE] = s_CreateShared("mat_def_dbg_prim_line", renderState);
		DefaultMaterials[RpgMaterialDefault::DEBUG_PRIMITIVE_LINE]->Flags |= FLAG_Default | FLAG_Runtime_Loaded;

		// no-depth
		renderState.BlendMode = RpgRenderColorBlendMode::FADE;
		renderState.bDepthTest = false;

		DefaultMaterials[RpgMaterialDefault::DEBUG_PRIMITIVE_LINE_NO_DEPTH] = s_CreateShared("mat_def_dbg_prim_line_no_depth", renderState);
		DefaultMaterials[RpgMaterialDefault::DEBUG_PRIMITIVE_LINE_NO_DEPTH]->Flags |= FLAG_Default | FLAG_Runtime_Loaded;
	}

	// Debug primitive mesh
	{
		RpgRenderPipelineState renderState{};
		renderState.PixelShaderName = RPG_SHADER_NAME_PixelColor;
		renderState.VertexMode = RpgRenderVertexMode::PRIMITIVE;
		renderState.BlendMode = RpgRenderColorBlendMode::NONE;
		renderState.RasterMode = RpgRenderRasterMode::SOLID;
		renderState.bDepthTest = true;
		renderState.bDepthWrite = false;

		DefaultMaterials[RpgMaterialDefault::DEBUG_PRIMITIVE_MESH] = s_CreateShared("mat_def_dbg_prim_mesh", renderState);
		DefaultMaterials[RpgMaterialDefault::DEBUG_PRIMITIVE_MESH]->Flags |= FLAG_Default | FLAG_Runtime_Loaded;

		// no-depth
		renderState.BlendMode = RpgRenderColorBlendMode::FADE;
		renderState.bDepthTest = false;

		DefaultMaterials[RpgMaterialDefault::DEBUG_PRIMITIVE_MESH_NO_DEPTH] = s_CreateShared("mat_def_dbg_prim_mesh_no_depth", renderState);
		DefaultMaterials[RpgMaterialDefault::DEBUG_PRIMITIVE_MESH_NO_DEPTH]->Flags |= FLAG_Default | FLAG_Runtime_Loaded;
	}
}


void RpgMaterial::s_DestroyDefaults() noexcept
{
	DefaultMaterials.Clear(true);
}


const RpgSharedMaterial& RpgMaterial::s_GetDefault(RpgMaterialDefault::EType type) noexcept
{
	return DefaultMaterials[type];
}
