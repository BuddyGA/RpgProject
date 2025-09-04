#include "RpgAssetStream.h"
#include "RpgAssetManager.h"



RpgAssetStreamWriter::RpgAssetStreamWriter() noexcept
{
}


uint32_t RpgAssetStreamWriter::GetMeshSizeBytes(const RpgSharedMesh& mesh) const noexcept
{
	return GetSizeBytes(g_AssetManager->GetMeshAssetPath(mesh));
}

void RpgAssetStreamWriter::WriteMesh(const RpgSharedMesh& mesh) noexcept
{
	Write(g_AssetManager->GetMeshAssetPath(mesh));
}


uint32_t RpgAssetStreamWriter::GetTextureSizeBytes(const RpgSharedTexture2D& texture) const noexcept
{
	return GetSizeBytes(g_AssetManager->GetTextureAssetPath(texture));
}

void RpgAssetStreamWriter::WriteTexture(const RpgSharedTexture2D& texture) noexcept
{
	Write(g_AssetManager->GetTextureAssetPath(texture));
}


uint32_t RpgAssetStreamWriter::GetMaterialSizeBytes(const RpgSharedMaterial& material) const noexcept
{
	return GetSizeBytes(g_AssetManager->GetMaterialAssetPath(material));
}

void RpgAssetStreamWriter::WriteMaterial(const RpgSharedMaterial& material) noexcept
{
	Write(g_AssetManager->GetMaterialAssetPath(material));
}



RpgAssetStreamReader::RpgAssetStreamReader(RpgArray<uint8_t>& in_Bytes) noexcept
	: RpgBinaryStreamReader(in_Bytes)
{
}


void RpgAssetStreamReader::ReadMesh(RpgSharedMesh& mesh) noexcept
{
	RpgString assetPath;
	Read(assetPath);

	if (!assetPath.IsEmpty())
	{
		mesh = g_AssetManager->LoadMesh(assetPath);
	}
}


void RpgAssetStreamReader::ReadTexture(RpgSharedTexture2D& texture) noexcept
{
	RpgString assetPath;
	Read(assetPath);

	if (!assetPath.IsEmpty())
	{
		texture = g_AssetManager->LoadTexture(assetPath);
	}
}


void RpgAssetStreamReader::ReadMaterial(RpgSharedMaterial& material) noexcept
{
	RpgString assetPath;
	Read(assetPath);

	if (!assetPath.IsEmpty())
	{
		material = g_AssetManager->LoadMaterial(assetPath);
	}
}
