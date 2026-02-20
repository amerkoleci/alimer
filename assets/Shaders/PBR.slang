#include "Alimer.hlsli"
#include "BRDF.hlsli"
#include "ShaderTypes.h"
#include "VertexInputOutput.hlsli"

struct SurfaceInfo
{
    // Fill these yourself:
    float3 worldPos; // world space position
    float3 V;        // normalized vector from the shading location to the eye
    float3 N;        // surface normal in the world space

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

float3 getDiffuseLightColor(in float3 N)
{
    // TODO
    // float width, height, numberOfLevels;
    // environmentTexture.GetDimensions(0, width, height, numberOfLevels);

    // const float diffuseLevel = float(numberOfLevels - 1);
    // return environmentTexture.SampleLevel(environmentSampler, N, diffuseLevel).rgb;
    return float3(1.0f, 1.0f, 1.0f);
}

float3 getSpecularLightColor(in float3 R, in float roughness)
{
    // TODO
    // float width, height, numberOfLevels;
    // environmentTexture.GetDimensions(0, width, height, numberOfLevels);
    //
    // const float rough = numberOfLevels * roughness * (2.0f - roughness);
    // return environmentTexture.SampleLevel(environmentSampler, R, rough).rgb;
    return float3(1.0f, 1.0f, 1.0f);
}

float3 pbrSurfaceColorIbl(in SurfaceInfo surface)
{
    float3 kS = FresnelSchlickRoughness(surface.NdotV, surface.F0, surface.roughness);
    float3 kD = (float3(1.f, 1.f, 1.f) - kS) * (1.f - surface.metalness);
    float3 irradiance = getDiffuseLightColor(surface.N);
    float3 diffuse = irradiance * surface.diffuseColor;

    float3 prefilteredColor = getSpecularLightColor(surface.R, surface.roughness);
    float2 envBrdf = envBRDFApprox(surface.roughness, surface.NdotV);
    float3 specular = prefilteredColor * (surface.specularColor * envBrdf.x + envBrdf.y);

    float3 ambient = (kD * diffuse + specular) * surface.ao;
    return ambient;
}

// Material (space 0)
ConstantBuffer<PBRMaterialUniforms> material : register(b0);
Texture2D<float4> baseColorTexture : register(t0);
Texture2D<float4> normalTexture : register(t1);
Texture2D<float4> metallicRoughnessTexture : register(t2);
Texture2D<float4> emissiveTexture : register(t3);
Texture2D<float4> occlusionTexture : register(t4);
// Texture2D<float4> transmissionTexture : register(t5, space1);
SamplerState pbrSampler : register(s0);

[shader("pixel")]
float4 fragmentMain(in VertexOutput input, in bool isFrontFace: SV_IsFrontFace) : SV_TARGET
{
    const float4 baseColorMap = baseColorTexture.Sample(pbrSampler, input.TexCoord0);

    // float4 baseColor = material.baseColorFactor * baseColorTexture.Sample(SamplerLinearClamp, input.TexCoord0);
    float4 baseColor = material.baseColorFactor * baseColorMap;
    baseColor.rgb *= input.Color;
    // clip(baseColor.a - material.GetAlphaCutoff());

    SurfaceInfo surface;
    surface.worldPos = input.WorldPosition;
    surface.V = normalize(view.cameraPosition.xyz - surface.worldPos);

    // Should we normalize here?
    float3x3 TBN = float3x3(input.Tangent, input.Bitangent, input.Normal);
    //[branch]
    // if (material.textures[NORMALMAP].IsValid())
    {
        float3 N = float3(normalTexture.Sample(pbrSampler, input.TexCoord0).rg, 1.f);
        N = N * 2.f - 1.f;
        // N.rg *= material.GetNormalMapStrength();
        surface.N = normalize(mul(N, TBN));
        if (!isFrontFace) // MATERIAL_DOUBLE_SIDED?
        {
            surface.N = -surface.N;
        }
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
    float3 Lo = pbrSurfaceColorIbl(surface);

#if defined(USE_SHADOWS)
    // Directional light
    [branch]
    if (lights.directionalLight.intensity > 0)
    {
        Lo += pbrDirectionalLight(lights.directionalLight, surface);
    }

    // Point lights
    for (uint i = 0; i < lights.pointLightCount; i++)
    {
        PointLight light = pointLights[i];
        // calculate per-light radiance and add to outgoing radiance Lo
        Lo += pbrPointLight(light, surface);
    }

    Lo += (surface.diffuseColor * surface.ao * lights.ambient) + surface.emissive;
#else
    Lo += (surface.diffuseColor * surface.ao) + surface.emissive;
#endif

    return float4(Lo, surface.alpha);
}
