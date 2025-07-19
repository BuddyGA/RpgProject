#include "Common.hlsli"


#define RPG_FORWARD_PHONG_SPECULAR  1



// =============================================================================================== //
// PIXEL SHADER
// =============================================================================================== //
#define RPG_MATERIAL_PARAM_VECTOR_INDEX_base_color              0
#define RPG_MATERIAL_PARAM_VECTOR_INDEX_specular_color          1

#define RPG_MATERIAL_PARAM_SCALAR_INDEX_shininess               0
#define RPG_MATERIAL_PARAM_SCALAR_INDEX_opacity                 1


struct PixelShaderInput
{
    float4 SvPosition : SV_Position;
    float4 WsFragPosition : WORLD_POSITION;
    float4 WsFragNormal : WORLD_NORMAL;
    float4 WsFragTangent : WORLD_TANGENT;
    float4 WsCameraPosition : CAMERA_WORLD_POSITION;
    float2 TexCoord : TEXCOORD;
};



// =============================================================================================== //
// FUNCTIONS
// =============================================================================================== //

float Rpg_PhongLightShadowFactor_Directional(float3 wsFragPosition, RpgShaderLight light, RpgShaderView shadowView)
{
    const float4 lsFragPosition = mul(float4(wsFragPosition, 1.0f), shadowView.ViewProjectionMatrix);

    float2 shadowProjCoords = (lsFragPosition.xy / lsFragPosition.w);
    shadowProjCoords.x = 0.5f + shadowProjCoords.x * 0.5f;
    shadowProjCoords.y = 0.5f - shadowProjCoords.y * 0.5f;
    
    const float depthValue = lsFragPosition.z / shadowView.FarClipZ;
    
    return DynamicIndexingTexture2Ds[light.ShadowTextureDescriptorIndex].SampleCmpLevelZero(SamplerShadow, shadowProjCoords.xy, depthValue);
}


float Rpg_PhongLightShadowFactor_OmniDirectional(float3 wsFragPosition, RpgShaderLight light, RpgShaderView shadowView)
{
    const float3 shadowProjCoords = wsFragPosition - light.WorldPosition.xyz;
    const float depthValue = length(shadowProjCoords) / shadowView.FarClipZ;
   
    return DynamicIndexingTextureCubes[light.ShadowTextureDescriptorIndex].SampleCmpLevelZero(SamplerShadow, shadowProjCoords, depthValue);
}


float Rpg_PhongLightDistanceAttenuation(float3 lightVector, float lightRadius, float lightFallOffExponent)
{
    const float distanceSqr = dot(lightVector, lightVector);
    const float invRadius = 1.0f / lightRadius;
    const float normalizedDistanceSqr = distanceSqr * invRadius * invRadius;
    const float edgeFalloff = RPG_SQR(saturate(1.0f - RPG_SQR(normalizedDistanceSqr)));
    float distanceAttenuation = (1.0f / (distanceSqr + 1.0f)) * edgeFalloff;
            
    if (lightFallOffExponent > 0.0f)
    {
        const float distanceAttenuationMask = 1.0f - saturate(normalizedDistanceSqr);
        distanceAttenuation = pow(distanceAttenuationMask, lightFallOffExponent);
    }
    
    return distanceAttenuation;
}


float3 Rpg_PhongLightContributionColor(float3 surfaceNormal, float3 toViewDirection, float3 toLightDirection, float4 lightColorIntensity, float lightAttenuation, float3 diffuseColor, float specularColor, float shininess)
{
    const float3 baseLightColor = (lightColorIntensity.rgb * lightColorIntensity.a);
    
    const float diffuseFactor = saturate(dot(surfaceNormal, toLightDirection));
    float3 lightContributionColor = baseLightColor * diffuseColor * diffuseFactor * lightAttenuation;

    
#if RPG_FORWARD_PHONG_SPECULAR
    // Blinn-Phong
    const float3 halfReflectDirection = normalize(toViewDirection + toLightDirection);
    
    const float specularFactor = pow(saturate(dot(surfaceNormal, halfReflectDirection)), shininess);
    lightContributionColor += baseLightColor * specularColor * specularFactor * lightAttenuation;
#endif // 
    
    
    return lightContributionColor;
}



// =============================================================================================== //
// MAIN ENTRY POINT
// =============================================================================================== //

float4 PS_Main(PixelShaderInput input) : SV_TARGET
{
    float finalAlpha = 1.0f;
    
#ifdef MASK
    if (MaterialParameter.TextureDescriptorIndex_OpacityMask != -1)
    {
        finalAlpha = Rpg_GetMaterialParameterTextureColor(MaterialParameter.TextureDescriptorIndex_OpacityMask, SamplerMipMapLinear, input.TexCoord).r;
    }
    
    if (finalAlpha <= 0.0f)
    {
        discard;
    }
#endif // MASK
    
    
    float3 wsFragPosition = input.WsFragPosition.xyz;
    float3 wsFragNormal = normalize(input.WsFragNormal).xyz;
    const float3 toViewDir = normalize(input.WsCameraPosition.xyz - wsFragPosition);
    
    
    // Default values
    float specularColor = 0.5f;
    float shininess = 32.0f;
    
    
    // Diffuse
    float3 diffuseColor = Rpg_GetMaterialParameterVectorValue(RPG_MATERIAL_PARAM_VECTOR_INDEX_base_color).rgb;
    if (MaterialParameter.TextureDescriptorIndex_BaseColor != -1)
    {
        diffuseColor = Rpg_GetMaterialParameterTextureColor(MaterialParameter.TextureDescriptorIndex_BaseColor, SamplerMipMapLinear, input.TexCoord).rgb;
    }
    else
    {
        const float2 scaledUV = input.TexCoord;
        const float2 checker = floor(scaledUV) % 2.0f; //fmod(floor(scaledUV), 2.0f);
        diffuseColor = (checker.x + checker.y) % 2.0f < 1.0f ? 0.5f : 0.25f; // fmod(checker.x + checker.y, 2.0f) < 1.0f ? 0.5f : 0.25f;
    }
    
    
    // Normal
    if (MaterialParameter.TextureDescriptorIndex_Normal != -1)
    {
        float3 wsFragTangent = normalize(input.WsFragTangent).xyz;
        wsFragTangent = normalize(wsFragTangent - dot(wsFragTangent, wsFragNormal) * wsFragNormal);
    
        const float3 worldBitangent = normalize(cross(wsFragNormal, wsFragTangent));
        const float3x3 worldTangentMatrix = float3x3(wsFragTangent, worldBitangent, wsFragNormal);
        
        wsFragNormal = Rpg_GetMaterialParameterTextureColor(MaterialParameter.TextureDescriptorIndex_Normal, SamplerMipMapLinear, input.TexCoord).rgb;
        //wsFragNormal = wsFragNormal * 2.0f - 1.0f; // Using BC5_SNORM
        wsFragNormal = normalize(mul(wsFragNormal, worldTangentMatrix));
    }
    
    
    // Specular
    specularColor = Rpg_GetMaterialParameterVectorValue(RPG_MATERIAL_PARAM_VECTOR_INDEX_specular_color).r;
    if (MaterialParameter.TextureDescriptorIndex_Specular != -1)
    {
        specularColor = Rpg_GetMaterialParameterTextureColor(MaterialParameter.TextureDescriptorIndex_Specular, SamplerMipMapLinear, input.TexCoord).r;
    }
    
    shininess = Rpg_GetMaterialParameterScalarValue(RPG_MATERIAL_PARAM_SCALAR_INDEX_shininess);
    
    
    // Ambient light
    const float3 ambientColor = (diffuseColor * WorldData.AmbientColorStrength.rgb * WorldData.AmbientColorStrength.a);
    
    
    float3 lightContribColor = 0.0f;

    
    // Point lights
    for (int pl = RPG_SHADER_LIGHT_POINT_INDEX; pl < (RPG_SHADER_LIGHT_POINT_INDEX + WorldData.PointLightCount); ++pl)
    {
        const RpgShaderLight pointLight = WorldData.Lights[pl];
        const float3 lightPosition = pointLight.WorldPosition.xyz;
        const float3 lightVector = lightPosition - wsFragPosition;
        const float distanceToLight = length(lightVector);
        
        if (distanceToLight <= pointLight.AttenuationRadius)
        {
            float shadowFactor = 1.0f;
                
            // if has shadow camera and shadow texture
            if (pointLight.ShadowViewIndex != -1 && pointLight.ShadowTextureDescriptorIndex != -1)
            {
                const RpgShaderView shadowView = WorldData.Views[pointLight.ShadowViewIndex];
                shadowFactor = Rpg_PhongLightShadowFactor_OmniDirectional(wsFragPosition, pointLight, shadowView);
            }
            
            // final point light attenuation factor
            float attenuationFactor = Rpg_PhongLightDistanceAttenuation(lightVector, pointLight.AttenuationRadius, pointLight.AttenuationFallOffExp);
            //attenuationFactor = 1.0f / (0.0f + 0.0f * (distanceToLight / 100.0f) + 0.06f * RPG_SQR(distanceToLight / 100.0f));
            lightContribColor += Rpg_PhongLightContributionColor(wsFragNormal, toViewDir, normalize(lightVector), pointLight.ColorIntensity, attenuationFactor, diffuseColor, specularColor, shininess) * shadowFactor;
        }
    }
    
    
    // Spot lights
    for (int sl = RPG_SHADER_LIGHT_SPOT_INDEX; sl < (RPG_SHADER_LIGHT_SPOT_INDEX + WorldData.SpotLightCount); ++sl)
    {
        const RpgShaderLight spotLight = WorldData.Lights[sl];
        const float3 lightVector = spotLight.WorldPosition.xyz - wsFragPosition;
        const float distanceToLight = length(lightVector);
        
        if (distanceToLight <= spotLight.AttenuationRadius)
        {
            const float3 normalizedLightVector = normalize(lightVector);
            const float cosLS = dot(normalizedLightVector, normalize(-spotLight.WorldDirection.xyz));
            const float cosOuterCutOff = cos(spotLight.SpotLightOuterConeRadian);
            
            if (cosLS > cosOuterCutOff)
            {
                float shadowFactor = 1.0f;
                
                // if has shadow camera and shadow texture
                if (spotLight.ShadowViewIndex != -1 && spotLight.ShadowTextureDescriptorIndex != -1)
                {
                    const RpgShaderView shadowView = WorldData.Views[spotLight.ShadowViewIndex];
                    shadowFactor = Rpg_PhongLightShadowFactor_Directional(wsFragPosition, spotLight, shadowView);
                }
                
                // distance attenuation
                const float distanceAttenuation = Rpg_PhongLightDistanceAttenuation(lightVector, spotLight.AttenuationRadius, spotLight.AttenuationFallOffExp);
                //const float distanceAttenuation = saturate(1.0f - distanceToLight / spotLight.AttenuationRadius);
                
                // spot attenuation
                const float cosInnerCutOff = cos(spotLight.SpotLightInnerConeRadian);
                float spotAttenuation = saturate((cosLS - cosOuterCutOff) / (cosInnerCutOff - cosOuterCutOff));

                // final spot light attenuation factor
                const float attenuationFactor = distanceAttenuation * RPG_SQR(spotAttenuation);
                
                lightContribColor += Rpg_PhongLightContributionColor(wsFragNormal, toViewDir, normalizedLightVector, spotLight.ColorIntensity, attenuationFactor, diffuseColor, specularColor, shininess) * shadowFactor;
            }
        }
    }
    
    
    // Directional lights
    for (int dl = RPG_SHADER_LIGHT_DIRECTIONAL_INDEX; dl < (RPG_SHADER_LIGHT_DIRECTIONAL_INDEX + WorldData.DirectionalLightCount); ++dl)
    {
        const RpgShaderLight directionalLight = WorldData.Lights[dl];
        lightContribColor += Rpg_PhongLightContributionColor(wsFragNormal, toViewDir, normalize(-directionalLight.WorldDirection.xyz), directionalLight.ColorIntensity, 1.0f, diffuseColor, specularColor, shininess);
    }
    
    
    return float4(ambientColor + lightContribColor, finalAlpha);
}
