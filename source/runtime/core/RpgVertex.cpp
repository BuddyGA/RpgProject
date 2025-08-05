#include "RpgVertex.h"



void RpgVertexGeometryFactory::UpdateBatchIndices(RpgVertexIndexArray& out_Indices, uint32_t baseVertex, int baseIndex, int batchIndexCount) noexcept
{
	const __m128i vecVertexOffset = _mm_set_epi32(baseVertex, baseVertex, baseVertex, baseVertex);

	// Get largest count multiple of 4
	const int sseCount = batchIndexCount & ~3;
	RPG_Check(sseCount % 4 == 0);

	// SSE
	for (int i = 0; i < sseCount; i += 4)
	{
		const int dstIndex = baseIndex + i;
		__m128i* vecIndexData = (__m128i*)(out_Indices.GetData(dstIndex));
		__m128i vecIndices = _mm_loadu_si128(vecIndexData);
		__m128i vecIndicesAddedOffsets = _mm_add_epi32(vecIndices, vecVertexOffset);
		_mm_storeu_si128(vecIndexData, vecIndicesAddedOffsets);
	}

	// Remaining
	for (int i = sseCount; i < batchIndexCount; ++i)
	{
		out_Indices[baseIndex + i] += baseVertex;
	}
}


void RpgVertexGeometryFactory::CalculateSmoothTangents(RpgVertexMeshNormalTangentArray& out_NormalTangents, const RpgVertexMeshPositionArray& positions, const RpgVertexMeshTexCoordArray& texCoords, const RpgVertexIndexArray& indices) noexcept
{
	RPG_Check(out_NormalTangents.GetCount() == positions.GetCount());

	const int vertexCount = positions.GetCount();

	// Initialize vertex tangents to zero
	for (int v = 0; v < vertexCount; ++v)
	{
		out_NormalTangents[v].Tangent = RpgVector4();
	}

	const int indexCount = indices.GetCount();

	for (int i = 0; i < indexCount; i += 3)
	{
		const uint32_t idx_A = indices[i];
		const uint32_t idx_B = indices[i + 1];
		const uint32_t idx_C = indices[i + 2];

		const RpgVector3 vtxPos_A(positions[idx_A].Xmm);
		const RpgVector3 vtxPos_B(positions[idx_B].Xmm);
		const RpgVector3 vtxPos_C(positions[idx_C].Xmm);

		const RpgVector2 uv_A(DirectX::XMLoadFloat2(&texCoords[idx_A]));
		const RpgVector2 uv_B(DirectX::XMLoadFloat2(&texCoords[idx_B]));
		const RpgVector2 uv_C(DirectX::XMLoadFloat2(&texCoords[idx_C]));

		const RpgVector3 edge_AB = vtxPos_B - vtxPos_A;
		const RpgVector3 edge_AC = vtxPos_C - vtxPos_A;

		const RpgVector2 deltaUV_AB = uv_B - uv_A;
		const RpgVector2 deltaUV_AC = uv_C - uv_A;

		float det = deltaUV_AB.X * deltaUV_AC.Y - deltaUV_AB.Y * deltaUV_AC.X;
		if (RpgMath::IsZero(det, RPG_MATH_EPS_MP))
		{
			det = 1.0f; // avoid NaN
		}

		const float r = 1.0f / det;

		const RpgVector4 tangent(
			r * (deltaUV_AC.Y * edge_AB.X - deltaUV_AB.Y * edge_AC.X),
			r * (deltaUV_AC.Y * edge_AB.Y - deltaUV_AB.Y * edge_AC.Y),
			r * (deltaUV_AC.Y * edge_AB.Z - deltaUV_AB.Y * edge_AC.Z),
			0.0f
		);

		// Accumulate for shared vertex in many triangles
		for (int j = 0; j < 3; j++)
		{
			out_NormalTangents[indices[i + j]].Tangent += tangent;
		}
	}

	// Normalize vertex tangents
	for (int v = 0; v < vertexCount; ++v)
	{
		out_NormalTangents[v].Tangent.Normalize();
	}
}


void RpgVertexGeometryFactory::CreateMeshBox(RpgVertexMeshPositionArray& out_Positions, RpgVertexMeshNormalTangentArray& out_NormalTangents, RpgVertexMeshTexCoordArray& out_TexCoords, RpgVertexIndexArray& out_Indices, const RpgVector3& boxMin, const RpgVector3& boxMax, float uvScale) noexcept
{
	const int VERTEX_COUNT = 24;
	const int baseVertex = out_Positions.GetCount();

	// Vertex Positions
	int vtxId = baseVertex;
	out_Positions.Resize(baseVertex + VERTEX_COUNT);
	{
		// front
		out_Positions[vtxId++] = RpgVector4(boxMin.X, boxMax.Y, boxMin.Z, 1.0f); // 0
		out_Positions[vtxId++] = RpgVector4(boxMax.X, boxMax.Y, boxMin.Z, 1.0f); // 1
		out_Positions[vtxId++] = RpgVector4(boxMax.X, boxMin.Y, boxMin.Z, 1.0f); // 2
		out_Positions[vtxId++] = RpgVector4(boxMin.X, boxMin.Y, boxMin.Z, 1.0f); // 3
		// back					
		out_Positions[vtxId++] = RpgVector4(boxMax.X, boxMax.Y, boxMax.Z, 1.0f); // 4
		out_Positions[vtxId++] = RpgVector4(boxMin.X, boxMax.Y, boxMax.Z, 1.0f); // 5
		out_Positions[vtxId++] = RpgVector4(boxMin.X, boxMin.Y, boxMax.Z, 1.0f); // 6
		out_Positions[vtxId++] = RpgVector4(boxMax.X, boxMin.Y, boxMax.Z, 1.0f); // 7
		// right				
		out_Positions[vtxId++] = RpgVector4(boxMax.X, boxMax.Y, boxMin.Z, 1.0f); // 8
		out_Positions[vtxId++] = RpgVector4(boxMax.X, boxMax.Y, boxMax.Z, 1.0f); // 9
		out_Positions[vtxId++] = RpgVector4(boxMax.X, boxMin.Y, boxMax.Z, 1.0f); // 10
		out_Positions[vtxId++] = RpgVector4(boxMax.X, boxMin.Y, boxMin.Z, 1.0f); // 11
		// left					
		out_Positions[vtxId++] = RpgVector4(boxMin.X, boxMax.Y, boxMax.Z, 1.0f); // 12
		out_Positions[vtxId++] = RpgVector4(boxMin.X, boxMax.Y, boxMin.Z, 1.0f); // 13
		out_Positions[vtxId++] = RpgVector4(boxMin.X, boxMin.Y, boxMin.Z, 1.0f); // 14
		out_Positions[vtxId++] = RpgVector4(boxMin.X, boxMin.Y, boxMax.Z, 1.0f); // 15
		// top					
		out_Positions[vtxId++] = RpgVector4(boxMin.X, boxMax.Y, boxMax.Z, 1.0f); // 16
		out_Positions[vtxId++] = RpgVector4(boxMax.X, boxMax.Y, boxMax.Z, 1.0f); // 17
		out_Positions[vtxId++] = RpgVector4(boxMax.X, boxMax.Y, boxMin.Z, 1.0f); // 18
		out_Positions[vtxId++] = RpgVector4(boxMin.X, boxMax.Y, boxMin.Z, 1.0f); // 19
		// bottom				
		out_Positions[vtxId++] = RpgVector4(boxMin.X, boxMin.Y, boxMin.Z, 1.0f); // 20
		out_Positions[vtxId++] = RpgVector4(boxMax.X, boxMin.Y, boxMin.Z, 1.0f); // 21
		out_Positions[vtxId++] = RpgVector4(boxMax.X, boxMin.Y, boxMax.Z, 1.0f); // 22
		out_Positions[vtxId++] = RpgVector4(boxMin.X, boxMin.Y, boxMax.Z, 1.0f); // 23
	}

	// Vertex Normals
	vtxId = baseVertex;
	out_NormalTangents.Resize(baseVertex + VERTEX_COUNT);
	{
		// front
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, 0.0f, -1.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, 0.0f, -1.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, 0.0f, -1.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, 0.0f, -1.0f, 0.0f), RpgVector4() };
		// back
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, 0.0f, 1.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, 0.0f, 1.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, 0.0f, 1.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, 0.0f, 1.0f, 0.0f), RpgVector4() };
		// right
		out_NormalTangents[vtxId++] = { RpgVector4(1.0f, 0.0f, 0.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(1.0f, 0.0f, 0.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(1.0f, 0.0f, 0.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(1.0f, 0.0f, 0.0f, 0.0f), RpgVector4() };
		// left
		out_NormalTangents[vtxId++] = { RpgVector4(-1.0f, 0.0f, 0.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(-1.0f, 0.0f, 0.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(-1.0f, 0.0f, 0.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(-1.0f, 0.0f, 0.0f, 0.0f), RpgVector4() };
		// top
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, 1.0f, 0.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, 1.0f, 0.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, 1.0f, 0.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, 1.0f, 0.0f, 0.0f), RpgVector4() };
		// bottom
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, -1.0f, 0.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, -1.0f, 0.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, -1.0f, 0.0f, 0.0f), RpgVector4() };
		out_NormalTangents[vtxId++] = { RpgVector4(0.0f, -1.0f, 0.0f, 0.0f), RpgVector4() };
	}

	// Vertex TexCoords
	vtxId = baseVertex;
	out_TexCoords.Resize(baseVertex + VERTEX_COUNT);
	{
		// front
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(0.0f * uvScale, 0.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(1.0f * uvScale, 0.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(1.0f * uvScale, 1.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(0.0f * uvScale, 1.0f * uvScale);
		// back
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(0.0f * uvScale, 0.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(1.0f * uvScale, 0.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(1.0f * uvScale, 1.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(0.0f * uvScale, 1.0f * uvScale);
		// right
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(0.0f * uvScale, 0.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(1.0f * uvScale, 0.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(1.0f * uvScale, 1.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(0.0f * uvScale, 1.0f * uvScale);
		// left
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(0.0f * uvScale, 0.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(1.0f * uvScale, 0.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(1.0f * uvScale, 1.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(0.0f * uvScale, 1.0f * uvScale);
		// top
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(0.0f * uvScale, 0.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(1.0f * uvScale, 0.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(1.0f * uvScale, 1.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(0.0f * uvScale, 1.0f * uvScale);
		// bottom
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(0.0f * uvScale, 0.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(1.0f * uvScale, 0.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(1.0f * uvScale, 1.0f * uvScale);
		out_TexCoords[vtxId++] = DirectX::XMFLOAT2(0.0f * uvScale, 1.0f * uvScale);
	}


	// Indices
	const int INDEX_COUNT = 36;
	const RpgVertex::FIndex indices[INDEX_COUNT] =
	{
		0, 1, 2, 2, 3, 0, // back
		4, 5, 6, 6, 7, 4, // front
		8, 9, 10, 10, 11, 8, // right
		12, 13, 14, 14, 15, 12, // left
		16, 17, 18, 18, 19, 16, // top
		20, 21, 22, 22, 23, 20 // bottom
	};

	const int baseIndex = out_Indices.GetCount();
	out_Indices.Resize(baseIndex + INDEX_COUNT);

	for (int i = 0; i < INDEX_COUNT; ++i)
	{
		out_Indices[baseIndex + i] = baseVertex + indices[i];
	}

	CalculateSmoothTangents(out_NormalTangents, out_Positions, out_TexCoords, out_Indices);
}




// ============================================================================================================================================= //
// VERTEX PRIMITIVE BATCH LINE
// ============================================================================================================================================= //
void RpgVertexPrimitiveBatchLine::AddLine(const RpgVector3& p0, const RpgVector3& p1, RpgColor color) noexcept
{
	const uint32_t baseVertex = static_cast<uint32_t>(Vertices.GetCount());

	RpgVertex::FPrimitive& v0 = Vertices.Add();
	v0.Position = p0;
	v0.Color = color;

	RpgVertex::FPrimitive& v1 = Vertices.Add();
	v1.Position = p1;
	v1.Color = color;

	Indices.AddValue(baseVertex);
	Indices.AddValue(baseVertex + 1);
}


void RpgVertexPrimitiveBatchLine::AddAABB(const RpgBoundingAABB& aabb, RpgColor color) noexcept
{
	const RpgVector3 vertices[8] =
	{
		RpgVector3(aabb.Min.X, aabb.Min.Y, aabb.Min.Z),
		RpgVector3(aabb.Min.X, aabb.Max.Y, aabb.Min.Z),
		RpgVector3(aabb.Max.X, aabb.Max.Y, aabb.Min.Z),
		RpgVector3(aabb.Max.X, aabb.Min.Y, aabb.Min.Z),
		RpgVector3(aabb.Min.X, aabb.Min.Y, aabb.Max.Z),
		RpgVector3(aabb.Min.X, aabb.Max.Y, aabb.Max.Z),
		RpgVector3(aabb.Max.X, aabb.Max.Y, aabb.Max.Z),
		RpgVector3(aabb.Max.X, aabb.Min.Y, aabb.Max.Z),
	};

	AddLine(vertices[0], vertices[1], color);
	AddLine(vertices[1], vertices[2], color);
	AddLine(vertices[2], vertices[3], color);
	AddLine(vertices[3], vertices[0], color);

	AddLine(vertices[4], vertices[5], color);
	AddLine(vertices[5], vertices[6], color);
	AddLine(vertices[6], vertices[7], color);
	AddLine(vertices[7], vertices[4], color);

	AddLine(vertices[0], vertices[4], color);
	AddLine(vertices[1], vertices[5], color);
	AddLine(vertices[2], vertices[6], color);
	AddLine(vertices[3], vertices[7], color);
}




// ============================================================================================================================================= //
// VERTEX PRIMITIVE BATCH MESH
// ============================================================================================================================================= //
void RpgVertexPrimitiveBatchMesh::AddTriangle(const RpgVector3& p0, const RpgVector3& p1, const RpgVector3& p2, RpgColor color) noexcept
{
	const uint32_t baseVertex = static_cast<uint32_t>(Vertices.GetCount());

	Vertices.AddConstruct(RpgVector4(p0), color);
	Vertices.AddConstruct(RpgVector4(p1), color);
	Vertices.AddConstruct(RpgVector4(p2), color);

	Indices.AddValue(baseVertex);
	Indices.AddValue(baseVertex + 1);
	Indices.AddValue(baseVertex + 2);
}
