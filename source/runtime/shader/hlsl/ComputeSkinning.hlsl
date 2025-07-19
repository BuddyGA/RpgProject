#define RPG_SHADER_HLSL
#include "../RpgShaderTypes.h"



// =============================================================================================== //
// LAYOUT
// =============================================================================================== //
struct RpgVertexMeshNormalTangent
{
    float4 Normal;
    float4 Tangent;
};


struct RpgVertexMeshSkin
{
    float4 BoneWeights0;
    float4 BoneWeights1;
    uint PackedBoneIndices0;
    uint PackedBoneIndices1;
    uint BoneCount;
};



// ================================================================================================================================= //
// BINDING
// ================================================================================================================================= //

// Vertex position
StructuredBuffer<float4> VertexPositions : register(t8, space0);

// Vertex normal-tangent
StructuredBuffer<RpgVertexMeshNormalTangent> VertexNormalTangents : register(t9, space0);

// Vertex skin
StructuredBuffer<RpgVertexMeshSkin> VertexSkins : register(t10, space0);

// Skeleton data
StructuredBuffer<float4x4> SkeletonBoneSkinningTransforms : register(t11, space0);

// Object parameter
ConstantBuffer<RpgShaderSkinnedObjectParameter> ObjectParameter : register(b0, space0);

// Vertex output position
RWStructuredBuffer<float4> SkinnedVertexPositions : register(u0, space0);

// Vertex output normal-tangent
RWStructuredBuffer<RpgVertexMeshNormalTangent> SkinnedVertexNormalTangents : register(u1, space0);



// =============================================================================================== //
// FUNCTION
// =============================================================================================== //
inline uint4 Rpg_UnpackBoneIndices(uint packedBoneIndices)
{
    return uint4(
        packedBoneIndices & 0xFF,
        (packedBoneIndices >> 8) & 0xFF,
        (packedBoneIndices >> 16) & 0xFF,
        (packedBoneIndices >> 24) & 0xFF);
}



// =============================================================================================== //
// MAIN ENTRY POINT
// =============================================================================================== //
[numthreads(64, 1, 1)]
void CS_Main(uint3 GroupID : SV_GroupID,                    // 3D index of the thread group in the dispatch.
             uint3 GroupThreadID : SV_GroupThreadID,        // 3D index of local thread ID in a thread group.
             uint3 DispatchThreadID : SV_DispatchThreadID,  // 3D index of global thread ID in the dispatch.
             uint GroupIndex : SV_GroupIndex                // Flattened local index of the thread within a thread group.
)
{
    const int vtxId = DispatchThreadID.x + ObjectParameter.VertexStart;
    if (vtxId >= (ObjectParameter.VertexStart + ObjectParameter.VertexCount))
    {
        return;
    }
    
    // fetch input vertex
    const float4 vtxPos = VertexPositions[vtxId];
    const RpgVertexMeshNormalTangent vtxNormTan = VertexNormalTangents[vtxId];
    const RpgVertexMeshSkin vtxSkin = VertexSkins[vtxId];
    
    // perform vertex skinning
    const int skelId = ObjectParameter.SkeletonIndex;
    uint4 boneIndices0 = Rpg_UnpackBoneIndices(vtxSkin.PackedBoneIndices0);
    uint4 boneIndices1 = Rpg_UnpackBoneIndices(vtxSkin.PackedBoneIndices1);
    
    float4x4 boneWeightSkinningMatrix = 0;
    
    for (int i = 0; i < vtxSkin.BoneCount; ++i)
    {
        const float boneWeight = (i < 4) ? vtxSkin.BoneWeights0[i] : vtxSkin.BoneWeights1[i - 4];
        const float4x4 boneSkinningTransform = (i < 4) ? SkeletonBoneSkinningTransforms[skelId + boneIndices0[i]] : SkeletonBoneSkinningTransforms[skelId + boneIndices1[i - 4]];
        boneWeightSkinningMatrix += mul(boneWeight, boneSkinningTransform);
    }
    
    // output skinned vertex
    const int skinnedVtxId = DispatchThreadID.x + ObjectParameter.SkinnedVertexStart;
    SkinnedVertexPositions[skinnedVtxId] = mul(vtxPos, boneWeightSkinningMatrix);
    SkinnedVertexNormalTangents[skinnedVtxId].Normal = mul(vtxNormTan.Normal, boneWeightSkinningMatrix);
    SkinnedVertexNormalTangents[skinnedVtxId].Tangent = mul(vtxNormTan.Tangent, boneWeightSkinningMatrix);
}
