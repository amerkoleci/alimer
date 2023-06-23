// ------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
// ------------------------------------------------------------------------------

using System;

namespace Alimer.Graphics.VGPU;

internal enum VGPULogLevel
{
	/// <unmanaged>VGPULogLevel_Error</unmanaged>
	Error = 0,
	/// <unmanaged>VGPULogLevel_Warning</unmanaged>
	Warning = 1,
	/// <unmanaged>VGPULogLevel_Info</unmanaged>
	Info = 2,
	/// <unmanaged>_VGPULogLevel_Count</unmanaged>
	Count = 3,
}

internal enum VGPUBackend
{
	/// <unmanaged>_VGPUBackend_Default</unmanaged>
	Default = 0,
	/// <unmanaged>VGPUBackend_Vulkan</unmanaged>
	Vulkan = 1,
	/// <unmanaged>VGPUBackend_D3D12</unmanaged>
	D3D12 = 2,
	/// <unmanaged>VGPUBackend_D3D11</unmanaged>
	D3D11 = 3,
	/// <unmanaged>_VGPUBackend_Count</unmanaged>
	Count = 4,
}

internal enum VGPUValidationMode
{
	/// <unmanaged>VGPUValidationMode_Disabled</unmanaged>
	Disabled = 0,
	/// <unmanaged>VGPUValidationMode_Enabled</unmanaged>
	Enabled = 1,
	/// <unmanaged>VGPUValidationMode_Verbose</unmanaged>
	Verbose = 2,
	/// <unmanaged>VGPUValidationMode_GPU</unmanaged>
	GPU = 3,
	/// <unmanaged>_VGPUValidationMode_Count</unmanaged>
	Count = 4,
}

internal enum VGPUPowerPreference
{
	/// <unmanaged>VGPUPowerPreference_Undefined</unmanaged>
	Undefined = 0,
	/// <unmanaged>VGPUPowerPreference_LowPower</unmanaged>
	LowPower = 1,
	/// <unmanaged>VGPUPowerPreference_HighPerformance</unmanaged>
	HighPerformance = 2,
}

internal enum VGPUCommandQueue
{
	/// <unmanaged>VGPUCommandQueue_Graphics</unmanaged>
	Graphics = 0,
	/// <unmanaged>VGPUCommandQueue_Compute</unmanaged>
	Compute = 1,
	/// <unmanaged>VGPUCommandQueue_Copy</unmanaged>
	Copy = 2,
	/// <unmanaged>_VGPUCommandQueue_Count</unmanaged>
	Count = 3,
}

internal enum VGPUAdapterType
{
	/// <unmanaged>VGPUAdapterType_Other</unmanaged>
	Other = 0,
	/// <unmanaged>VGPUAdapterType_IntegratedGPU</unmanaged>
	IntegratedGPU = 1,
	/// <unmanaged>VGPUAdapterType_DiscreteGPU</unmanaged>
	DiscreteGPU = 2,
	/// <unmanaged>VGPUAdapterType_VirtualGPU</unmanaged>
	VirtualGPU = 3,
	/// <unmanaged>VGPUAdapterType_CPU</unmanaged>
	CPU = 4,
	/// <unmanaged>_VGPUAdapterType_Count</unmanaged>
	Count = 5,
}

internal enum VGPUCpuAccessMode
{
	/// <unmanaged>VGPUCpuAccessMode_None</unmanaged>
	None = 0,
	/// <unmanaged>VGPUCpuAccessMode_Write</unmanaged>
	Write = 1,
	/// <unmanaged>VGPUCpuAccessMode_Read</unmanaged>
	Read = 2,
	/// <unmanaged>_VGPUCpuAccessMode_Count</unmanaged>
	Count = 3,
}

internal enum VGPUBufferUsage
{
	/// <unmanaged>VGPUBufferUsage_None</unmanaged>
	None = 0,
	/// <unmanaged>VGPUBufferUsage_Vertex</unmanaged>
	Vertex = 1,
	/// <unmanaged>VGPUBufferUsage_Index</unmanaged>
	Index = 2,
	/// <unmanaged>VGPUBufferUsage_Constant</unmanaged>
	Constant = 4,
	/// <unmanaged>VGPUBufferUsage_ShaderRead</unmanaged>
	ShaderRead = 8,
	/// <unmanaged>VGPUBufferUsage_ShaderWrite</unmanaged>
	ShaderWrite = 16,
	/// <unmanaged>VGPUBufferUsage_Indirect</unmanaged>
	Indirect = 32,
	/// <unmanaged>VGPUBufferUsage_Predication</unmanaged>
	Predication = 64,
	/// <unmanaged>VGPUBufferUsage_RayTracing</unmanaged>
	RayTracing = 128,
}

internal enum VGPUTextureDimension
{
	/// <unmanaged>_VGPUTextureDimension_Default</unmanaged>
	Default = 0,
	/// <unmanaged>VGPUTextureDimension_1D</unmanaged>
	_1D = 1,
	/// <unmanaged>VGPUTextureDimension_2D</unmanaged>
	_2D = 2,
	/// <unmanaged>VGPUTextureDimension_3D</unmanaged>
	_3D = 3,
	/// <unmanaged>_VGPUTextureDimension_Count</unmanaged>
	Count = 4,
}

internal enum VGPUTextureUsage
{
	/// <unmanaged>VGPUTextureUsage_None</unmanaged>
	None = 0,
	/// <unmanaged>VGPUTextureUsage_ShaderRead</unmanaged>
	ShaderRead = 1,
	/// <unmanaged>VGPUTextureUsage_ShaderWrite</unmanaged>
	ShaderWrite = 2,
	/// <unmanaged>VGPUTextureUsage_RenderTarget</unmanaged>
	RenderTarget = 4,
	/// <unmanaged>VGPUTextureUsage_Transient</unmanaged>
	Transient = 8,
	/// <unmanaged>VGPUTextureUsage_ShadingRate</unmanaged>
	ShadingRate = 16,
}

internal enum VGPUTextureFormat
{
	/// <unmanaged>VGPUTextureFormat_Undefined</unmanaged>
	Undefined = 0,
	/// <unmanaged>VGPUTextureFormat_R8Unorm</unmanaged>
	R8Unorm = 1,
	/// <unmanaged>VGPUTextureFormat_R8Snorm</unmanaged>
	R8Snorm = 2,
	/// <unmanaged>VGPUTextureFormat_R8Uint</unmanaged>
	R8Uint = 3,
	/// <unmanaged>VGPUTextureFormat_R8Sint</unmanaged>
	R8Sint = 4,
	/// <unmanaged>VGPUTextureFormat_R16Unorm</unmanaged>
	R16Unorm = 5,
	/// <unmanaged>VGPUTextureFormat_R16Snorm</unmanaged>
	R16Snorm = 6,
	/// <unmanaged>VGPUTextureFormat_R16Uint</unmanaged>
	R16Uint = 7,
	/// <unmanaged>VGPUTextureFormat_R16Sint</unmanaged>
	R16Sint = 8,
	/// <unmanaged>VGPUTextureFormat_R16Float</unmanaged>
	R16Float = 9,
	/// <unmanaged>VGPUTextureFormat_RG8Unorm</unmanaged>
	RG8Unorm = 10,
	/// <unmanaged>VGPUTextureFormat_RG8Snorm</unmanaged>
	RG8Snorm = 11,
	/// <unmanaged>VGPUTextureFormat_RG8Uint</unmanaged>
	RG8Uint = 12,
	/// <unmanaged>VGPUTextureFormat_RG8Sint</unmanaged>
	RG8Sint = 13,
	/// <unmanaged>VGPUTextureFormat_BGRA4Unorm</unmanaged>
	BGRA4Unorm = 14,
	/// <unmanaged>VGPUTextureFormat_B5G6R5Unorm</unmanaged>
	B5G6R5Unorm = 15,
	/// <unmanaged>VGPUTextureFormat_B5G5R5A1Unorm</unmanaged>
	B5G5R5A1Unorm = 16,
	/// <unmanaged>VGPUTextureFormat_R32Uint</unmanaged>
	R32Uint = 17,
	/// <unmanaged>VGPUTextureFormat_R32Sint</unmanaged>
	R32Sint = 18,
	/// <unmanaged>VGPUTextureFormat_R32Float</unmanaged>
	R32Float = 19,
	/// <unmanaged>VGPUTextureFormat_RG16Unorm</unmanaged>
	RG16Unorm = 20,
	/// <unmanaged>VGPUTextureFormat_RG16Snorm</unmanaged>
	RG16Snorm = 21,
	/// <unmanaged>VGPUTextureFormat_RG16Uint</unmanaged>
	RG16Uint = 22,
	/// <unmanaged>VGPUTextureFormat_RG16Sint</unmanaged>
	RG16Sint = 23,
	/// <unmanaged>VGPUTextureFormat_RG16Float</unmanaged>
	RG16Float = 24,
	/// <unmanaged>VGPUTextureFormat_RGBA8Uint</unmanaged>
	RGBA8Uint = 25,
	/// <unmanaged>VGPUTextureFormat_RGBA8Sint</unmanaged>
	RGBA8Sint = 26,
	/// <unmanaged>VGPUTextureFormat_RGBA8Unorm</unmanaged>
	RGBA8Unorm = 27,
	/// <unmanaged>VGPUTextureFormat_RGBA8UnormSrgb</unmanaged>
	RGBA8UnormSrgb = 28,
	/// <unmanaged>VGPUTextureFormat_RGBA8Snorm</unmanaged>
	RGBA8Snorm = 29,
	/// <unmanaged>VGPUTextureFormat_BGRA8Unorm</unmanaged>
	BGRA8Unorm = 30,
	/// <unmanaged>VGPUTextureFormat_BGRA8UnormSrgb</unmanaged>
	BGRA8UnormSrgb = 31,
	/// <unmanaged>VGPUTextureFormat_RGB9E5Ufloat</unmanaged>
	RGB9E5Ufloat = 32,
	/// <unmanaged>VGPUTextureFormat_RGB10A2Unorm</unmanaged>
	RGB10A2Unorm = 33,
	/// <unmanaged>VGPUTextureFormat_RGB10A2Uint</unmanaged>
	RGB10A2Uint = 34,
	/// <unmanaged>VGPUTextureFormat_RG11B10Float</unmanaged>
	RG11B10Float = 35,
	/// <unmanaged>VGPUTextureFormat_RG32Uint</unmanaged>
	RG32Uint = 36,
	/// <unmanaged>VGPUTextureFormat_RG32Sint</unmanaged>
	RG32Sint = 37,
	/// <unmanaged>VGPUTextureFormat_RG32Float</unmanaged>
	RG32Float = 38,
	/// <unmanaged>VGPUTextureFormat_RGBA16Unorm</unmanaged>
	RGBA16Unorm = 39,
	/// <unmanaged>VGPUTextureFormat_RGBA16Snorm</unmanaged>
	RGBA16Snorm = 40,
	/// <unmanaged>VGPUTextureFormat_RGBA16Uint</unmanaged>
	RGBA16Uint = 41,
	/// <unmanaged>VGPUTextureFormat_RGBA16Sint</unmanaged>
	RGBA16Sint = 42,
	/// <unmanaged>VGPUTextureFormat_RGBA16Float</unmanaged>
	RGBA16Float = 43,
	/// <unmanaged>VGPUTextureFormat_RGBA32Uint</unmanaged>
	RGBA32Uint = 44,
	/// <unmanaged>VGPUTextureFormat_RGBA32Sint</unmanaged>
	RGBA32Sint = 45,
	/// <unmanaged>VGPUTextureFormat_RGBA32Float</unmanaged>
	RGBA32Float = 46,
	/// <unmanaged>VGPUTextureFormat_Stencil8</unmanaged>
	Stencil8 = 47,
	/// <unmanaged>VGPUTextureFormat_Depth16Unorm</unmanaged>
	Depth16Unorm = 48,
	/// <unmanaged>VGPUTextureFormat_Depth32Float</unmanaged>
	Depth32Float = 49,
	/// <unmanaged>VGPUTextureFormat_Depth24UnormStencil8</unmanaged>
	Depth24UnormStencil8 = 50,
	/// <unmanaged>VGPUTextureFormat_Depth32FloatStencil8</unmanaged>
	Depth32FloatStencil8 = 51,
	/// <unmanaged>VGPUTextureFormat_Bc1RgbaUnorm</unmanaged>
	Bc1RgbaUnorm = 52,
	/// <unmanaged>VGPUTextureFormat_Bc1RgbaUnormSrgb</unmanaged>
	Bc1RgbaUnormSrgb = 53,
	/// <unmanaged>VGPUTextureFormat_Bc2RgbaUnorm</unmanaged>
	Bc2RgbaUnorm = 54,
	/// <unmanaged>VGPUTextureFormat_Bc2RgbaUnormSrgb</unmanaged>
	Bc2RgbaUnormSrgb = 55,
	/// <unmanaged>VGPUTextureFormat_Bc3RgbaUnorm</unmanaged>
	Bc3RgbaUnorm = 56,
	/// <unmanaged>VGPUTextureFormat_Bc3RgbaUnormSrgb</unmanaged>
	Bc3RgbaUnormSrgb = 57,
	/// <unmanaged>VGPUTextureFormat_Bc4RUnorm</unmanaged>
	Bc4RUnorm = 58,
	/// <unmanaged>VGPUTextureFormat_Bc4RSnorm</unmanaged>
	Bc4RSnorm = 59,
	/// <unmanaged>VGPUTextureFormat_Bc5RgUnorm</unmanaged>
	Bc5RgUnorm = 60,
	/// <unmanaged>VGPUTextureFormat_Bc5RgSnorm</unmanaged>
	Bc5RgSnorm = 61,
	/// <unmanaged>VGPUTextureFormat_Bc6hRgbUfloat</unmanaged>
	Bc6hRgbUfloat = 62,
	/// <unmanaged>VGPUTextureFormat_Bc6hRgbSfloat</unmanaged>
	Bc6hRgbSfloat = 63,
	/// <unmanaged>VGPUTextureFormat_Bc7RgbaUnorm</unmanaged>
	Bc7RgbaUnorm = 64,
	/// <unmanaged>VGPUTextureFormat_Bc7RgbaUnormSrgb</unmanaged>
	Bc7RgbaUnormSrgb = 65,
	/// <unmanaged>VGPUTextureFormat_Etc2Rgb8Unorm</unmanaged>
	Etc2Rgb8Unorm = 66,
	/// <unmanaged>VGPUTextureFormat_Etc2Rgb8UnormSrgb</unmanaged>
	Etc2Rgb8UnormSrgb = 67,
	/// <unmanaged>VGPUTextureFormat_Etc2Rgb8A1Unorm</unmanaged>
	Etc2Rgb8A1Unorm = 68,
	/// <unmanaged>VGPUTextureFormat_Etc2Rgb8A1UnormSrgb</unmanaged>
	Etc2Rgb8A1UnormSrgb = 69,
	/// <unmanaged>VGPUTextureFormat_Etc2Rgba8Unorm</unmanaged>
	Etc2Rgba8Unorm = 70,
	/// <unmanaged>VGPUTextureFormat_Etc2Rgba8UnormSrgb</unmanaged>
	Etc2Rgba8UnormSrgb = 71,
	/// <unmanaged>VGPUTextureFormat_EacR11Unorm</unmanaged>
	EacR11Unorm = 72,
	/// <unmanaged>VGPUTextureFormat_EacR11Snorm</unmanaged>
	EacR11Snorm = 73,
	/// <unmanaged>VGPUTextureFormat_EacRg11Unorm</unmanaged>
	EacRg11Unorm = 74,
	/// <unmanaged>VGPUTextureFormat_EacRg11Snorm</unmanaged>
	EacRg11Snorm = 75,
	/// <unmanaged>VGPUTextureFormat_Astc4x4Unorm</unmanaged>
	Astc4x4Unorm = 76,
	/// <unmanaged>VGPUTextureFormat_Astc4x4UnormSrgb</unmanaged>
	Astc4x4UnormSrgb = 77,
	/// <unmanaged>VGPUTextureFormat_Astc5x4Unorm</unmanaged>
	Astc5x4Unorm = 78,
	/// <unmanaged>VGPUTextureFormat_Astc5x4UnormSrgb</unmanaged>
	Astc5x4UnormSrgb = 79,
	/// <unmanaged>VGPUTextureFormat_Astc5x5Unorm</unmanaged>
	Astc5x5Unorm = 80,
	/// <unmanaged>VGPUTextureFormat_Astc5x5UnormSrgb</unmanaged>
	Astc5x5UnormSrgb = 81,
	/// <unmanaged>VGPUTextureFormat_Astc6x5Unorm</unmanaged>
	Astc6x5Unorm = 82,
	/// <unmanaged>VGPUTextureFormat_Astc6x5UnormSrgb</unmanaged>
	Astc6x5UnormSrgb = 83,
	/// <unmanaged>VGPUTextureFormat_Astc6x6Unorm</unmanaged>
	Astc6x6Unorm = 84,
	/// <unmanaged>VGPUTextureFormat_Astc6x6UnormSrgb</unmanaged>
	Astc6x6UnormSrgb = 85,
	/// <unmanaged>VGPUTextureFormat_Astc8x5Unorm</unmanaged>
	Astc8x5Unorm = 86,
	/// <unmanaged>VGPUTextureFormat_Astc8x5UnormSrgb</unmanaged>
	Astc8x5UnormSrgb = 87,
	/// <unmanaged>VGPUTextureFormat_Astc8x6Unorm</unmanaged>
	Astc8x6Unorm = 88,
	/// <unmanaged>VGPUTextureFormat_Astc8x6UnormSrgb</unmanaged>
	Astc8x6UnormSrgb = 89,
	/// <unmanaged>VGPUTextureFormat_Astc8x8Unorm</unmanaged>
	Astc8x8Unorm = 90,
	/// <unmanaged>VGPUTextureFormat_Astc8x8UnormSrgb</unmanaged>
	Astc8x8UnormSrgb = 91,
	/// <unmanaged>VGPUTextureFormat_Astc10x5Unorm</unmanaged>
	Astc10x5Unorm = 92,
	/// <unmanaged>VGPUTextureFormat_Astc10x5UnormSrgb</unmanaged>
	Astc10x5UnormSrgb = 93,
	/// <unmanaged>VGPUTextureFormat_Astc10x6Unorm</unmanaged>
	Astc10x6Unorm = 94,
	/// <unmanaged>VGPUTextureFormat_Astc10x6UnormSrgb</unmanaged>
	Astc10x6UnormSrgb = 95,
	/// <unmanaged>VGPUTextureFormat_Astc10x8Unorm</unmanaged>
	Astc10x8Unorm = 96,
	/// <unmanaged>VGPUTextureFormat_Astc10x8UnormSrgb</unmanaged>
	Astc10x8UnormSrgb = 97,
	/// <unmanaged>VGPUTextureFormat_Astc10x10Unorm</unmanaged>
	Astc10x10Unorm = 98,
	/// <unmanaged>VGPUTextureFormat_Astc10x10UnormSrgb</unmanaged>
	Astc10x10UnormSrgb = 99,
	/// <unmanaged>VGPUTextureFormat_Astc12x10Unorm</unmanaged>
	Astc12x10Unorm = 100,
	/// <unmanaged>VGPUTextureFormat_Astc12x10UnormSrgb</unmanaged>
	Astc12x10UnormSrgb = 101,
	/// <unmanaged>VGPUTextureFormat_Astc12x12Unorm</unmanaged>
	Astc12x12Unorm = 102,
	/// <unmanaged>VGPUTextureFormat_Astc12x12UnormSrgb</unmanaged>
	Astc12x12UnormSrgb = 103,
	/// <unmanaged>_VGPUTextureFormat_Count</unmanaged>
	Count = 104,
}

internal enum VGPUFormatKind
{
	/// <unmanaged>VGPUFormatKind_Unorm</unmanaged>
	Unorm = 0,
	/// <unmanaged>VGPUFormatKind_UnormSrgb</unmanaged>
	UnormSrgb = 1,
	/// <unmanaged>VGPUFormatKind_Snorm</unmanaged>
	Snorm = 2,
	/// <unmanaged>VGPUFormatKind_Uint</unmanaged>
	Uint = 3,
	/// <unmanaged>VGPUFormatKind_Sint</unmanaged>
	Sint = 4,
	/// <unmanaged>VGPUFormatKind_Float</unmanaged>
	Float = 5,
	/// <unmanaged>_VGPUFormatKind_Count</unmanaged>
	Count = 6,
}

internal enum VGPUPresentMode
{
	/// <unmanaged>VGPUPresentMode_Immediate</unmanaged>
	Immediate = 0,
	/// <unmanaged>VGPUPresentMode_Mailbox</unmanaged>
	Mailbox = 1,
	/// <unmanaged>VGPUPresentMode_Fifo</unmanaged>
	Fifo = 2,
	/// <unmanaged>_VGPUPresentMode_Count</unmanaged>
	Count = 3,
}

internal enum VGPUShaderStage
{
	/// <unmanaged>VGPUShaderStage_None</unmanaged>
	None = 0,
	/// <unmanaged>VGPUShaderStage_Vertex</unmanaged>
	Vertex = 1,
	/// <unmanaged>VGPUShaderStage_Hull</unmanaged>
	Hull = 2,
	/// <unmanaged>VGPUShaderStage_Domain</unmanaged>
	Domain = 4,
	/// <unmanaged>VGPUShaderStage_Geometry</unmanaged>
	Geometry = 8,
	/// <unmanaged>VGPUShaderStage_Fragment</unmanaged>
	Fragment = 16,
	/// <unmanaged>VGPUShaderStage_Compute</unmanaged>
	Compute = 32,
	/// <unmanaged>VGPUShaderStage_Amplification</unmanaged>
	Amplification = 64,
	/// <unmanaged>VGPUShaderStage_Mesh</unmanaged>
	Mesh = 128,
}

internal enum VGPUFeature
{
	/// <unmanaged>VGPUFeature_DepthClipControl</unmanaged>
	DepthClipControl = 0,
	/// <unmanaged>VGPUFeature_Depth32FloatStencil8</unmanaged>
	Depth32FloatStencil8 = 1,
	/// <unmanaged>VGPUFeature_TimestampQuery</unmanaged>
	TimestampQuery = 2,
	/// <unmanaged>VGPUFeature_PipelineStatisticsQuery</unmanaged>
	PipelineStatisticsQuery = 3,
	/// <unmanaged>VGPUFeature_TextureCompressionBC</unmanaged>
	TextureCompressionBC = 4,
	/// <unmanaged>VGPUFeature_TextureCompressionETC2</unmanaged>
	TextureCompressionETC2 = 5,
	/// <unmanaged>VGPUFeature_TextureCompressionASTC</unmanaged>
	TextureCompressionASTC = 6,
	/// <unmanaged>VGPUFeature_IndirectFirstInstance</unmanaged>
	IndirectFirstInstance = 7,
	/// <unmanaged>VGPUFeature_ShaderFloat16</unmanaged>
	ShaderFloat16 = 8,
	/// <unmanaged>VGPUFeature_GeometryShader</unmanaged>
	GeometryShader = 9,
	/// <unmanaged>VGPUFeature_TessellationShader</unmanaged>
	TessellationShader = 10,
	/// <unmanaged>VGPUFeature_DepthBoundsTest</unmanaged>
	DepthBoundsTest = 11,
	/// <unmanaged>VGPUFeature_SamplerAnisotropy</unmanaged>
	SamplerAnisotropy = 12,
	/// <unmanaged>VGPUFeature_SamplerMinMax</unmanaged>
	SamplerMinMax = 13,
	/// <unmanaged>VGPUFeature_DescriptorIndexing</unmanaged>
	DescriptorIndexing = 14,
	/// <unmanaged>VGPUFeature_Predication</unmanaged>
	Predication = 15,
	/// <unmanaged>VGPUFeature_ShaderOutputViewportIndex</unmanaged>
	ShaderOutputViewportIndex = 16,
	/// <unmanaged>VGPUFeature_VariableRateShading</unmanaged>
	VariableRateShading = 17,
	/// <unmanaged>VGPUFeature_VariableRateShadingTier2</unmanaged>
	VariableRateShadingTier2 = 18,
	/// <unmanaged>VGPUFeature_RayTracing</unmanaged>
	RayTracing = 19,
	/// <unmanaged>VGPUFeature_RayTracingTier2</unmanaged>
	RayTracingTier2 = 20,
	/// <unmanaged>VGPUFeature_MeshShader</unmanaged>
	MeshShader = 21,
}

internal enum VGPULoadAction
{
	/// <unmanaged>VGPULoadAction_Load</unmanaged>
	Load = 0,
	/// <unmanaged>VGPULoadAction_Clear</unmanaged>
	Clear = 1,
	/// <unmanaged>VGPULoadAction_DontCare</unmanaged>
	DontCare = 2,
}

internal enum VGPUStoreAction
{
	/// <unmanaged>VGPUStoreAction_Store</unmanaged>
	Store = 0,
	/// <unmanaged>VGPUStoreAction_DontCare</unmanaged>
	DontCare = 1,
}

internal enum VGPUIndexType
{
	/// <unmanaged>VGPUIndexType_Uint16</unmanaged>
	Uint16 = 0,
	/// <unmanaged>VGPUIndexType_Uint32</unmanaged>
	Uint32 = 1,
	/// <unmanaged>_VGPUIndexType_Count</unmanaged>
	Count = 2,
}

internal enum VGPUCompareFunction
{
	/// <unmanaged>_VGPUCompareFunction_Default</unmanaged>
	Default = 0,
	/// <unmanaged>VGPUCompareFunction_Never</unmanaged>
	Never = 1,
	/// <unmanaged>VGPUCompareFunction_Less</unmanaged>
	Less = 2,
	/// <unmanaged>VGPUCompareFunction_Equal</unmanaged>
	Equal = 3,
	/// <unmanaged>VGPUCompareFunction_LessEqual</unmanaged>
	LessEqual = 4,
	/// <unmanaged>VGPUCompareFunction_Greater</unmanaged>
	Greater = 5,
	/// <unmanaged>VGPUCompareFunction_NotEqual</unmanaged>
	NotEqual = 6,
	/// <unmanaged>VGPUCompareFunction_GreaterEqual</unmanaged>
	GreaterEqual = 7,
	/// <unmanaged>VGPUCompareFunction_Always</unmanaged>
	Always = 8,
}

internal enum VGPUStencilOperation
{
	/// <unmanaged>VGPUStencilOperation_Keep</unmanaged>
	Keep = 0,
	/// <unmanaged>VGPUStencilOperation_Zero</unmanaged>
	Zero = 1,
	/// <unmanaged>VGPUStencilOperation_Replace</unmanaged>
	Replace = 2,
	/// <unmanaged>VGPUStencilOperation_IncrementClamp</unmanaged>
	IncrementClamp = 3,
	/// <unmanaged>VGPUStencilOperation_DecrementClamp</unmanaged>
	DecrementClamp = 4,
	/// <unmanaged>VGPUStencilOperation_Invert</unmanaged>
	Invert = 5,
	/// <unmanaged>VGPUStencilOperation_IncrementWrap</unmanaged>
	IncrementWrap = 6,
	/// <unmanaged>VGPUStencilOperation_DecrementWrap</unmanaged>
	DecrementWrap = 7,
}

internal enum VGPUSamplerFilter
{
	/// <unmanaged>VGPUSamplerFilter_Nearest</unmanaged>
	Nearest = 0,
	/// <unmanaged>VGPUSamplerFilter_Linear</unmanaged>
	Linear = 1,
}

internal enum VGPUSamplerMipFilter
{
	/// <unmanaged>VGPUSamplerMipFilter_Nearest</unmanaged>
	Nearest = 0,
	/// <unmanaged>VGPUSamplerMipFilter_Linear</unmanaged>
	Linear = 1,
}

internal enum VGPUSamplerAddressMode
{
	/// <unmanaged>VGPUSamplerAddressMode_Wrap</unmanaged>
	Wrap = 0,
	/// <unmanaged>VGPUSamplerAddressMode_Mirror</unmanaged>
	Mirror = 1,
	/// <unmanaged>VGPUSamplerAddressMode_Clamp</unmanaged>
	Clamp = 2,
	/// <unmanaged>VGPUSamplerAddressMode_Border</unmanaged>
	Border = 3,
}

internal enum VGPUSamplerBorderColor
{
	/// <unmanaged>VGPUSamplerBorderColor_TransparentBlack</unmanaged>
	TransparentBlack = 0,
	/// <unmanaged>VGPUSamplerBorderColor_OpaqueBlack</unmanaged>
	OpaqueBlack = 1,
	/// <unmanaged>VGPUSamplerBorderColor_OpaqueWhite</unmanaged>
	OpaqueWhite = 2,
}

internal enum VGPUPrimitiveTopology
{
	/// <unmanaged>_VGPUPrimitiveTopology_Default</unmanaged>
	Default = 0,
	/// <unmanaged>VGPUPrimitiveTopology_PointList</unmanaged>
	PointList = 1,
	/// <unmanaged>VGPUPrimitiveTopology_LineList</unmanaged>
	LineList = 2,
	/// <unmanaged>VGPUPrimitiveTopology_LineStrip</unmanaged>
	LineStrip = 3,
	/// <unmanaged>VGPUPrimitiveTopology_TriangleList</unmanaged>
	TriangleList = 4,
	/// <unmanaged>VGPUPrimitiveTopology_TriangleStrip</unmanaged>
	TriangleStrip = 5,
	/// <unmanaged>VGPUPrimitiveTopology_PatchList</unmanaged>
	PatchList = 6,
}

internal enum VGPUBlendFactor
{
	/// <unmanaged>VGPUBlendFactor_Zero</unmanaged>
	Zero = 0,
	/// <unmanaged>VGPUBlendFactor_One</unmanaged>
	One = 1,
	/// <unmanaged>VGPUBlendFactor_SourceColor</unmanaged>
	SourceColor = 2,
	/// <unmanaged>VGPUBlendFactor_OneMinusSourceColor</unmanaged>
	OneMinusSourceColor = 3,
	/// <unmanaged>VGPUBlendFactor_SourceAlpha</unmanaged>
	SourceAlpha = 4,
	/// <unmanaged>VGPUBlendFactor_OneMinusSourceAlpha</unmanaged>
	OneMinusSourceAlpha = 5,
	/// <unmanaged>VGPUBlendFactor_DestinationColor</unmanaged>
	DestinationColor = 6,
	/// <unmanaged>VGPUBlendFactor_OneMinusDestinationColor</unmanaged>
	OneMinusDestinationColor = 7,
	/// <unmanaged>VGPUBlendFactor_DestinationAlpha</unmanaged>
	DestinationAlpha = 8,
	/// <unmanaged>VGPUBlendFactor_OneMinusDestinationAlpha</unmanaged>
	OneMinusDestinationAlpha = 9,
	/// <unmanaged>VGPUBlendFactor_SourceAlphaSaturated</unmanaged>
	SourceAlphaSaturated = 10,
	/// <unmanaged>VGPUBlendFactor_BlendColor</unmanaged>
	BlendColor = 11,
	/// <unmanaged>VGPUBlendFactor_OneMinusBlendColor</unmanaged>
	OneMinusBlendColor = 12,
}

internal enum VGPUBlendOperation
{
	/// <unmanaged>VGPUBlendOperation_Add</unmanaged>
	Add = 0,
	/// <unmanaged>VGPUBlendOperation_Subtract</unmanaged>
	Subtract = 1,
	/// <unmanaged>VGPUBlendOperation_ReverseSubtract</unmanaged>
	ReverseSubtract = 2,
	/// <unmanaged>VGPUBlendOperation_Min</unmanaged>
	Min = 3,
	/// <unmanaged>VGPUBlendOperation_Max</unmanaged>
	Max = 4,
}

internal enum VGPUColorWriteMask
{
	/// <unmanaged>VGPUColorWriteMask_None</unmanaged>
	None = 0,
	/// <unmanaged>VGPUColorWriteMask_Red</unmanaged>
	Red = 0x01,
	/// <unmanaged>VGPUColorWriteMask_Green</unmanaged>
	Green = 0x02,
	/// <unmanaged>VGPUColorWriteMask_Blue</unmanaged>
	Blue = 0x04,
	/// <unmanaged>VGPUColorWriteMask_Alpha</unmanaged>
	Alpha = 0x08,
	/// <unmanaged>VGPUColorWriteMask_All</unmanaged>
	All = 0x0F,
}

internal enum VGPUVertexFormat
{
	/// <unmanaged>VGPUVertexFormat_Undefined</unmanaged>
	Undefined = 0x00000000,
	/// <unmanaged>VGPUVertexFormat_UByte2</unmanaged>
	UByte2 = 1,
	/// <unmanaged>VGPUVertexFormat_UByte4</unmanaged>
	UByte4 = 2,
	/// <unmanaged>VGPUVertexFormat_Byte2</unmanaged>
	Byte2 = 3,
	/// <unmanaged>VGPUVertexFormat_Byte4</unmanaged>
	Byte4 = 4,
	/// <unmanaged>VGPUVertexFormat_UByte2Normalized</unmanaged>
	UByte2Normalized = 5,
	/// <unmanaged>VGPUVertexFormat_UByte4Normalized</unmanaged>
	UByte4Normalized = 6,
	/// <unmanaged>VGPUVertexFormat_Byte2Normalized</unmanaged>
	Byte2Normalized = 7,
	/// <unmanaged>VGPUVertexFormat_Byte4Normalized</unmanaged>
	Byte4Normalized = 8,
	/// <unmanaged>VGPUVertexFormat_UShort2</unmanaged>
	UShort2 = 9,
	/// <unmanaged>VGPUVertexFormat_UShort4</unmanaged>
	UShort4 = 10,
	/// <unmanaged>VGPUVertexFormat_Short2</unmanaged>
	Short2 = 11,
	/// <unmanaged>VGPUVertexFormat_Short4</unmanaged>
	Short4 = 12,
	/// <unmanaged>VGPUVertexFormat_UShort2Normalized</unmanaged>
	UShort2Normalized = 13,
	/// <unmanaged>VGPUVertexFormat_UShort4Normalized</unmanaged>
	UShort4Normalized = 14,
	/// <unmanaged>VGPUVertexFormat_Short2Normalized</unmanaged>
	Short2Normalized = 15,
	/// <unmanaged>VGPUVertexFormat_Short4Normalized</unmanaged>
	Short4Normalized = 16,
	/// <unmanaged>VGPUVertexFormat_Half2</unmanaged>
	Half2 = 17,
	/// <unmanaged>VGPUVertexFormat_Half4</unmanaged>
	Half4 = 18,
	/// <unmanaged>VGPUVertexFormat_Float</unmanaged>
	Float = 19,
	/// <unmanaged>VGPUVertexFormat_Float2</unmanaged>
	Float2 = 20,
	/// <unmanaged>VGPUVertexFormat_Float3</unmanaged>
	Float3 = 21,
	/// <unmanaged>VGPUVertexFormat_Float4</unmanaged>
	Float4 = 22,
	/// <unmanaged>VGPUVertexFormat_UInt</unmanaged>
	UInt = 23,
	/// <unmanaged>VGPUVertexFormat_UInt2</unmanaged>
	UInt2 = 24,
	/// <unmanaged>VGPUVertexFormat_UInt3</unmanaged>
	UInt3 = 25,
	/// <unmanaged>VGPUVertexFormat_UInt4</unmanaged>
	UInt4 = 26,
	/// <unmanaged>VGPUVertexFormat_Int</unmanaged>
	Int = 27,
	/// <unmanaged>VGPUVertexFormat_Int2</unmanaged>
	Int2 = 28,
	/// <unmanaged>VGPUVertexFormat_Int3</unmanaged>
	Int3 = 29,
	/// <unmanaged>VGPUVertexFormat_Int4</unmanaged>
	Int4 = 30,
	/// <unmanaged>VGPUVertexFormat_Int1010102Normalized</unmanaged>
	Int1010102Normalized = 31,
	/// <unmanaged>VGPUVertexFormat_UInt1010102Normalized</unmanaged>
	UInt1010102Normalized = 32,
}

internal enum VGPUVertexStepMode
{
	/// <unmanaged>VGPUVertexStepMode_Vertex</unmanaged>
	Vertex = 0,
	/// <unmanaged>VGPUVertexStepMode_Instance</unmanaged>
	Instance = 1,
}

internal enum VGPUPipelineType
{
	/// <unmanaged>VGPUPipelineType_Render</unmanaged>
	Render = 0,
	/// <unmanaged>VGPUPipelineType_Compute</unmanaged>
	Compute = 1,
	/// <unmanaged>VGPUPipelineType_RayTracing</unmanaged>
	RayTracing = 2,
}

internal enum VGPUQueryType
{
	/// <unmanaged>VGPUQueryType_Occlusion</unmanaged>
	Occlusion = 0,
	/// <unmanaged>VGPUQueryType_BinaryOcclusion</unmanaged>
	BinaryOcclusion = 1,
	/// <unmanaged>VGPUQueryType_Timestamp</unmanaged>
	Timestamp = 2,
	/// <unmanaged>VGPUQueryType_PipelineStatistics</unmanaged>
	PipelineStatistics = 3,
}

internal enum VGPUQueryPipelineStatistic
{
	/// <unmanaged>VGPUQueryPipelineStatistic_None</unmanaged>
	None = 0,
	/// <unmanaged>VGPUQueryPipelineStatistic_InputAssemblyVertices</unmanaged>
	InputAssemblyVertices = 1,
	/// <unmanaged>VGPUQueryPipelineStatistic_InputAssemblyPrimitives</unmanaged>
	InputAssemblyPrimitives = 2,
	/// <unmanaged>VGPUQueryPipelineStatistic_VertexShaderInvocations</unmanaged>
	VertexShaderInvocations = 4,
	/// <unmanaged>VGPUQueryPipelineStatistic_ClipperInvocations</unmanaged>
	ClipperInvocations = 8,
	/// <unmanaged>VGPUQueryPipelineStatistic_ClipperPrimitives</unmanaged>
	ClipperPrimitives = 16,
	/// <unmanaged>VGPUQueryPipelineStatistic_FragmentShaderInvocations</unmanaged>
	FragmentShaderInvocations = 32,
	/// <unmanaged>VGPUQueryPipelineStatistic_HullShaderInvocations</unmanaged>
	HullShaderInvocations = 64,
	/// <unmanaged>VGPUQueryPipelineStatistic_DomainShaderInvocations</unmanaged>
	DomainShaderInvocations = 128,
	/// <unmanaged>VGPUQueryPipelineStatistic_ComputeShaderInvocations</unmanaged>
	ComputeShaderInvocations = 256,
}

internal enum VGPUDescriptorType
{
	/// <unmanaged>VGPUDescriptorType_ShaderResource</unmanaged>
	ShaderResource = 0,
	/// <unmanaged>VGPUDescriptorType_ConstantBuffer</unmanaged>
	ConstantBuffer = 1,
	/// <unmanaged>VGPUDescriptorType_UnorderedAccess</unmanaged>
	UnorderedAccess = 2,
	/// <unmanaged>VGPUDescriptorType_Sampler</unmanaged>
	Sampler = 3,
}

