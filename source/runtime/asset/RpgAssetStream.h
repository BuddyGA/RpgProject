#pragma once

#include "core/RpgStream.h"
#include "render/RpgMesh.h"
#include "render/RpgMaterial.h"



class RpgAssetStreamWriter : public RpgBinaryStreamWriter
{
public:
	RpgAssetStreamWriter() noexcept;

	uint32_t GetMeshSizeBytes(const RpgSharedMesh& mesh) const noexcept;
	void WriteMesh(const RpgSharedMesh& mesh) noexcept;

	uint32_t GetTextureSizeBytes(const RpgSharedTexture2D& texture) const noexcept;
	void WriteTexture(const RpgSharedTexture2D& texture) noexcept;

	uint32_t GetMaterialSizeBytes(const RpgSharedMaterial& material) const noexcept;
	void WriteMaterial(const RpgSharedMaterial& material) noexcept;

};



class RpgAssetStreamReader : public RpgBinaryStreamReader
{
public:
	RpgAssetStreamReader(RpgArray<uint8_t>& in_Bytes) noexcept;

	void ReadMesh(RpgSharedMesh& mesh) noexcept;
	void ReadTexture(RpgSharedTexture2D& texture) noexcept;
	void ReadMaterial(RpgSharedMaterial& material) noexcept;

};
