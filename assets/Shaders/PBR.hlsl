// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer.hlsli"
#include "BRDF.hlsli"
#include "ShaderTypes.h"
#include "VertexInputOutput.hlsli"
#include "AlimerBindless.hlsli"

struct SurfaceInfo
{
    // Fill these yourself:
    float3 worldPos; // world space position
    float3 V; // normalized vector from the shading location to the eye
    float3 N; // surface normal in the world space

    float roughness;
    float metalness;
    float3 specularColor;
    float3 diffuseColor;

    float ao;
    float alpha;
    float3 emissive;
    float4 transmission;

    // Calculated properties
    float NdotV;
    float3 R;
    float3 F0;
};

float3 getDiffuseLightColor(TextureCube<float4> environmentTexture, SamplerState environmentSampler, in float3 N)
{
    float width, height, numberOfLevels;
    environmentTexture.GetDimensions(0, width, height, numberOfLevels);

    const float diffuseLevel = float(numberOfLevels - 1);
    return environmentTexture.SampleLevel(environmentSampler, N, diffuseLevel).rgb;
}

float3 getSpecularLightColor(TextureCube<float4> environmentTexture, SamplerState environmentSampler, in float3 R, in float roughness)
{
    float width, height, numberOfLevels;
    environmentTexture.GetDimensions(0, width, height, numberOfLevels);

    const float rough = numberOfLevels * roughness * (2.0f - roughness);
    return environmentTexture.SampleLevel(environmentSampler, R, rough).rgb;
}

float3 pbrSurfaceColorIbl(in SurfaceInfo surface, TextureCube<float4> environmentTexture, SamplerState environmentSampler)
{
    float3 kS = FresnelSchlickRoughness(surface.NdotV, surface.F0, surface.roughness);
    float3 kD = (float3(1.f, 1.f, 1.f) - kS) * (1.f - surface.metalness);
    float3 irradiance = getDiffuseLightColor(environmentTexture, environmentSampler, surface.N);
    float3 diffuse = irradiance * surface.diffuseColor;

    float3 prefilteredColor = getSpecularLightColor(environmentTexture, environmentSampler, surface.R, surface.roughness);
    float2 envBrdf = envBRDFApprox(surface.roughness, surface.NdotV);
    float3 specular = prefilteredColor * (surface.specularColor * envBrdf.x + envBrdf.y);

    float3 ambient = (kD * diffuse + specular) * surface.ao;
    return ambient;
}

float3 pbrSurfaceColor(in float3 L, in SurfaceInfo surface, in float3 lightColor, in float attenuation)
{
    const float3 H = normalize(surface.V + L);

    const float NdotL = saturate(dot(surface.N, L));
    const float VdotH = saturate(dot(surface.V, H));

    // cook-torrance brdf
    float NDF = DistributionGGX(surface.N, H, surface.roughness);
    float G = GeometrySmith(surface.N, surface.V, L, surface.roughness);
    float3 F = FresnelSchlick(VdotH, surface.F0);

    float3 kD = (1.f - F) * (1.f - surface.metalness);

    float3 numerator = NDF * G * F;
    float denominator = max(4 * max(dot(surface.N, surface.V), 0) * NdotL, 0.001f);
    float3 specular = numerator / denominator;

    float3 Fd = surface.diffuseColor / Pi;

    // add to outgoing radiance Lo
    float3 radiance = lightColor * attenuation;
    return (kD * Fd + specular) * radiance * NdotL * surface.ao;
}

float3 pbrDirectionalLight(in GPULight light, in SurfaceInfo surface)
{
    return pbrSurfaceColor(normalize(light.Direction), surface, light.Color, 1);
}

float pointLightAttenuation(in float3 worldToLight, in float lightRange)
{
    const float dist = length(worldToLight);
    return clamp(1.f - pow(dist / lightRange, 4.f), 0.f, 1.f) / pow(dist, 2.f);
}

float3 pbrPointLight(in GPULight light, in SurfaceInfo surface)
{
    const float3 worldToLight = light.Position - surface.worldPos;

    const float3 L = normalize(worldToLight);
    const float attenuation = pointLightAttenuation(worldToLight, light.Range);

    return pbrSurfaceColor(L, surface, light.Color, attenuation);
}

[shader("pixel")]
float4 fragmentMain(in VertexOutput input, in bool isFrontFace : SV_IsFrontFace) : SV_TARGET
{
    StructuredBuffer<GPUMaterialPBR> materials = bindlessGPUMaterial[push.MaterialBufferIndex];
    StructuredBuffer<GPULight> lights = bindlessGPULight[push.LightBufferIndex];

    GPUMaterialPBR material = materials[input.MaterialIndex];

    Texture2D<float4> baseColorTexture = bindlessTexture2D[material.baseIndex];
    
    Texture2D<float4> metallicRoughnessTexture = bindlessTexture2D[material.metallicRoughnessIndex];
    Texture2D<float4> emissiveTexture = bindlessTexture2D[material.emissiveIndex];
    Texture2D<float4> occlusionTexture = bindlessTexture2D[material.occlusionIndex];
    SamplerState pbrSampler = bindlessSamplers[material.samplerIndex];

    float2 baseColorUV = material.baseColorUVSet == 0 ? input.TexCoord0.xy : input.TexCoord1.xy;
    const float4 baseColorMap = baseColorTexture.Sample(pbrSampler, baseColorUV);

    // float4 baseColor = material.baseColorFactor * baseColorTexture.Sample(SamplerLinearClamp, input.TexCoord0);
    float4 baseColor = material.baseColorFactor * baseColorMap;
    baseColor.rgb *= input.Color;
    // clip(baseColor.a - material.GetAlphaCutoff());

    SurfaceInfo surface;
    surface.worldPos = input.WorldPosition;

    // Calculate view direction (fragment to camera)
    surface.V = normalize(frame.cameraPosition.xyz - surface.worldPos);

    surface.N = normalize(input.Normal);

    // Apply normal mapping if texture is available
    [branch]
    if (material.normalIndex >= 0)
    {
        Texture2D<float4> normalTexture = bindlessTexture2D[material.normalIndex];
        // Sample normal map and convert from [0,1] to [-1,1] range
        float2 normalColorUV = material.normalUVSet == 0 ? input.TexCoord0.xy : input.TexCoord1.xy;
        float3 tangentNormal = normalTexture.Sample(pbrSampler, normalColorUV).rgb * 2.0f - 1.0f;
        tangentNormal.rg *= material.normalScale;

        // Construct tangent-space to world-space transformation matrix (TBN)
        float3 T = normalize(input.Tangent.xyz); // Tangent
        float3 B = normalize(input.Bitangent.xyz); // Bitangent (w = handedness)
        float3x3 TBN = float3x3(T, B, surface.N);

        // Transform normal from tangent space to world space
        surface.N = normalize(mul(tangentNormal, TBN));
        //if (!isFrontFace) // MATERIAL_DOUBLE_SIDED?
        //{
        //    surface.N = -surface.N;
        //}
    }

    surface.NdotV = saturate(dot(surface.N, surface.V));
    surface.R = reflect(-surface.V, surface.N);

    const float3 color = input.Color * baseColor.rgb;
    surface.alpha = baseColor.a;
    surface.ao = material.occlusionStrength * occlusionTexture.Sample(pbrSampler, input.TexCoord0).r;
    surface.emissive = material.emissiveFactor * emissiveTexture.Sample(pbrSampler, input.TexCoord0).rgb;

    const float4 metallicRoughnessMap = metallicRoughnessTexture.Sample(pbrSampler, input.TexCoord0);
    const float4 occlusionMap = occlusionTexture.Sample(pbrSampler, input.TexCoord0);
    const float4 emissiveMap = emissiveTexture.Sample(pbrSampler, input.TexCoord0);

    surface.roughness = clamp(material.metallicRoughnessFactor.y * metallicRoughnessMap.g, 0.01f, 0.99f);
    surface.metalness = material.metallicRoughnessFactor.x * metallicRoughnessMap.b;

    surface.diffuseColor = color * (1.0f - surface.metalness);
    surface.specularColor = color * surface.metalness;

    // Constant normal incidence Fresnel factor for all dielectrics.
    static const float3 dielectricSpec = 0.04f;
    surface.F0 = lerp(dielectricSpec, color.rgb, surface.metalness);

    // reflectance equation
    TextureCube<float4> environmentTexture = bindlessTextureCube[frame.EnvironmentTextureIndex];
    SamplerState environmentSampler = SamplerLinearWrap; // bindlessSamplers[frame.EnvironmentSamplerIndex];
    float3 Lo = pbrSurfaceColorIbl(surface, environmentTexture, environmentSampler);

    // Calculate contribution from each light source
    for (uint i = 0; i < frame.activeLightCount; i++)
    {
        GPULight light = lights[i];
        [branch]
        if (light.Type != (int) LightType::Directional)
        {
            // calculate per-light radiance and add to outgoing radiance Lo
            Lo += pbrPointLight(light, surface);
        }
        else
        {
            Lo += pbrDirectionalLight(light, surface);
        }
    }
    Lo += (surface.diffuseColor * surface.ao * frame.ambientLight) + surface.emissive;

    return float4(Lo, surface.alpha);
}
