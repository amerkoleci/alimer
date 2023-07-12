#ifndef _ALIMER_SHADER__
#define _ALIMER_SHADER__

//#define ALIMER_BINDLESS
//#include <Alimer/Shaders/ShaderDefinitions.h>

#define CONCAT_X(a, b) a##b
#define CONCAT(a, b) CONCAT_X(a, b)

#if defined(VULKAN)
#   define VERTEX_ATTRIBUTE(type, name, loc) [[vk::location(loc)]] type name : CONCAT(ATTRIBUTE, loc)
#   define PUSH_CONSTANT(type, name, slot) [[vk::push_constant]] type name
#elif defined(HLSL5)
#   define PUSH_CONSTANT(type, name, slot) cbuffer name : register(CONCAT(b, slot)) { type name; }
#else
#   define VERTEX_ATTRIBUTE(type, name, loc) type name : CONCAT(ATTRIBUTE, loc)
#   define PUSH_CONSTANT(type, name, slot) ConstantBuffer<type> name : register(CONCAT(b, slot))
#endif

#define CBUFFER(type, name, slot) ConstantBuffer<type> name : register(CONCAT(b, slot))

#if defined(ALIMER_BINDLESS)
// Bindless tables
ByteAddressBuffer bindlessBuffers[] : register(space1);
Texture2D bindlessTexture2D[] : register(space2);
#endif /* ALIMER_BINDLESS */

// Static samplers
SamplerState SamplerPointWrap           : register(s0);
SamplerState SamplerPointClamp          : register(s1);
SamplerState SamplerLinearWrap          : register(s2);
SamplerState SamplerLinearClamp         : register(s3);
SamplerState SamplerAnisotropicWrap     : register(s4);
SamplerState SamplerAnisotropicClamp    : register(s5);

#endif // _ALIMER_SHADER__
