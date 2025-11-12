// https://github.com/NoelFB/glTF2/blob/main/LICENSE

using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Text.Json.Serialization;

namespace GLTF2;

/// <summary>
/// Parses glTF 2.0 json data
/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html
/// </summary>
public partial class Gltf2
{
	/// <summary>
	/// Specifies if the accessor’s elements are scalars, vectors, or matrices.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_accessor_type
	/// </summary>
	public enum AccessorType
	{
		Scalar,
		Vec2,
		Vec3,
		Vec4,
		Mat2,
		Mat3,
		Mat4
	}

	/// <summary>
	/// The datatype of the accessor’s components.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_accessor_componenttype
	/// </summary>
	public enum ComponentType
	{
		Byte = 5120,
		UnsignedByte = 5121,
		Short = 5122,
		UnsignedShort = 5123,
		Int = 5125,
		Float = 5126
	}

	/// <summary>
	/// The name of the node’s TRS property to animate, or the "weights" of the Morph Targets it instantiates. For the "translation" property, the values that are provided by the sampler are the translation along the X, Y, and Z axes. For the "rotation" property, the values are a quaternion in the order (x, y, z, w), where w is the scalar. For the "scale" property, the values are the scaling factors along the X, Y, and Z axes.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_animation_channel_target_path
	/// </summary>
	public enum Path
	{
		Translation,
		Rotation,
		Scale,
		Weights
	}

	/// <summary>
	/// Interpolation algorithm.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_animation_sampler_interpolation
	/// </summary>
	public enum Interpolation
	{
		Linear,
		Step,
		CubicSpline
	}

	/// <summary>
	/// The hint representing the intended GPU buffer type to use with this buffer view.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_bufferview_target
	/// </summary>
	public enum BufferViewTarget
	{
		ArrayBuffer = 34962,
		ElementArrayBuffer = 34963
	}

	/// <summary>
	/// The image’s media type.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_image_mimetype
	/// </summary>
	public enum MimeType
	{
		ImageJpeg,
		ImagePng
	}

	/// <summary>
	/// The material’s alpha rendering mode enumeration specifying the interpretation of the alpha value of the base color.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_alphamode
	/// </summary>
	public enum AlphaMode
	{
		Opaque,
		Mask,
		Blend
	}

	/// <summary>
	/// The topology type of primitives to render.<br />
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_mesh_primitive_mode
	/// </summary>
	public enum PrimitiveMode
	{
		Points = 0,
		Lines = 1,
		LineLoop = 2,
		LineStrip = 3,
		Triangles = 4,
		TriangleStrip = 5,
		TriangleFan = 6
	}

	/// <summary>
	/// Magnification filter.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_sampler_magfilter
	/// </summary>
	public enum MagFilter
	{
		Nearest = 9728,
		Linear = 9729
	}

	/// <summary>
	/// Minification filter.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_sampler_minfilter
	/// </summary>
	public enum MinFilter
	{
		Nearest = 9728,
		Linear = 9729,
		NearestMipMapNearest = 9984,
		LinearMipMapNearest = 9985,
		NearestMipMapLinear = 9986,
		LinearMipMapLinear = 9987,
	}

	/// <summary>
	/// Wrapping mode.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_sampler_wraps
	/// </summary>
	public enum Wrap
	{
		ClampToEdge = 33071,
		MirroredRepeat = 33648,
		Repeat = 10497,
	}

	/// <summary>
	/// A typed view into a buffer view that contains raw binary data.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-accessor
	/// </summary>
	public record struct Accessor(
		int? BufferView,
		int ByteOffset,
		ComponentType ComponentType,
		bool Normalized,
		int Count,
		AccessorType Type,
		float[] Max, // TODO: stackalloc
		float[] Min, // TODO: stackalloc
		AccessorSparse? Sparse,
		string? Name,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public Accessor()
			: this(null, 0, ComponentType.Byte, false, 0, AccessorType.Scalar, [], [], null, null) { }
	}

	/// <summary>
	/// Sparse storage of accessor values that deviate from their initialization value.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-accessor-sparse
	/// </summary>
	public record struct AccessorSparse(
		int Count,
		AccessorSparseIndices Indices,
		AccessorSparseValues Values,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	);

	/// <summary>
	/// An object pointing to a buffer view containing the indices of deviating accessor values.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-accessor-sparse-indices
	/// </summary>
	public record struct AccessorSparseIndices(
		int BufferView,
		int ByteOffset,
		int ComponentType,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	);

	/// <summary>
	/// An object pointing to a buffer view containing the deviating accessor values.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-accessor-sparse-values
	/// </summary>
	public record struct AccessorSparseValues(
		int BufferView,
		int ByteOffset,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	);

	/// <summary>
	/// A keyframe animation.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-animation
	/// </summary>
	public record struct Animation(
		AnimationChannel[] Channels,
		AnimationSampler[] Samplers,
		string? Name,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public Animation()
			: this([], [], null) { }
	}

	/// <summary>
	/// An animation channel combines an animation sampler with a target property being animated.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-animation-channel
	/// </summary>
	public record struct AnimationChannel(
		int Sampler,
		AnimationChannelTarget Target,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	);

	/// <summary>
	/// The descriptor of the animated property.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-animation-channel-target
	/// </summary>
	public record struct AnimationChannelTarget(
		int? Node,
		Path Path,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	);

	/// <summary>
	/// An animation sampler combines timestamps with a sequence of output values and defines an interpolation algorithm.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-animation-sampler
	/// </summary>
	public record struct AnimationSampler(
		int Input,
		Interpolation Interpolation,
		int Output,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	);

	/// <summary>
	/// Metadata about the glTF asset.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-asset
	/// </summary>
	public record struct AssetInfo(
		string? Copyright,
		string? Generator,
		string Version,
		string? MinVersion,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public AssetInfo()
			: this(null, null, string.Empty, null) { }
	}

	/// <summary>
	/// A buffer points to binary geometry, animation, or skins.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-asset
	/// </summary>
	public record struct Buffer(
		string Uri,
		int ByteLength,
		string? Name,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public Buffer()
			: this(string.Empty, 0, null) { }
	}

	/// <summary>
	/// A view into a buffer generally representing a subset of the buffer.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-bufferview
	/// </summary>
	public record struct BufferView(
		int Buffer,
		int ByteOffset,
		int ByteLength,
		int? ByteStride,
		BufferViewTarget? Target,
		string? Name,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public BufferView()
			: this(0, 0, 0, null, new(), null) { }
	}

	/// <summary>
	/// Image data used to create a texture. Image MAY be referenced by an URI (or IRI) or a buffer view index.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-image
	/// </summary>
	public record struct Image(
		string? Uri,
		MimeType? MimeType,
		int? BufferView,
		string? Name,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	);

	/// <summary>
	/// The material appearance of a primitive.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-material
	/// </summary>
	public record struct Material(
		string? Name,
		MaterialPbrMetallicRoughness? PbrMetallicRoughness,
		MaterialNormalTextureInfo? NormalTexture,
		MaterialOcclusionTextureInfo? OcclusionTexture,
		Vector3 EmissiveFactor,
		AlphaMode AlphaMode,
		float AlphaCutoff,
		bool DoubleSided,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public Material()
			: this(null, null, null, null, Vector3.Zero, AlphaMode.Opaque, 0.50f, false, null, null) { }
	}

	/// <summary>
	/// Reference to a texture.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-material-normaltextureinfo
	/// </summary>
	public record struct MaterialNormalTextureInfo(
		int Index,
		int TexCoord,
		float Scale,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public MaterialNormalTextureInfo()
			: this(0, 0, 1, null, null) { }
	}

	/// <summary>
	/// Reference to a texture.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-material-occlusiontextureinfo
	/// </summary>
	public record struct MaterialOcclusionTextureInfo(
		int Index,
		int TexCoord,
		float Strength,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public MaterialOcclusionTextureInfo()
			: this(0, 0, 1, null, null) { }
	}

	/// <summary>
	/// A set of parameter values that are used to define the metallic-roughness material model from Physically-Based Rendering (PBR) methodology.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-material-pbrmetallicroughness
	/// </summary>
	public record struct MaterialPbrMetallicRoughness(
		Vector4 BaseColorFactor,
		TextureInfo? BaseColorTexture,
		float MetallicFactor,
		float RoughnessFactor,
		TextureInfo? MetallicRoughnessTexture,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public MaterialPbrMetallicRoughness()
			: this(Vector4.One, null, 1, 1, null, null, null) { }
	}

	/// <summary>
	/// A set of primitives to be rendered. Its global transform is defined by a node that references it.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-mesh
	/// </summary>
	public record struct Mesh(
		MeshPrimitive[] Primitives,
		float[] Weights,
		string? Name,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public Mesh()
			: this([], [], null) { }
	}

	/// <summary>
	/// Geometry to be rendered with the given material.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-mesh-primitive
	/// </summary>
	public record struct MeshPrimitive(
		Dictionary<string, int> Attributes,
		int? Indices,
		int? Material,
		PrimitiveMode Mode,
		Dictionary<string, int>? Targets,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public MeshPrimitive()
			: this([], null, null, PrimitiveMode.Triangles, null, null, null) { }
	}

	/// <summary>
	/// A node in the node hierarchy.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-node
	/// </summary>
	public record struct Node(
		int? Camera,
		int[] Children,
		int? Skin,
		Matrix4x4 Matrix,
		int? Mesh,
		Quaternion Rotation,
		Vector3 Scale,
		Vector3 Translation,
		float[] Weights,
		string? Name,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public Node()
			: this(null, [], null, Matrix4x4.Identity, null, Quaternion.Identity, Vector3.One, Vector3.Zero, [], null, null, null) { }
	}

	/// <summary>
	/// Texture sampler properties for filtering and wrapping modes.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-sampler
	/// </summary>
	public record struct Sampler(
		MagFilter? MagFilter,
		MinFilter? MinFilter,
		Wrap WrapS,
		Wrap WrapT,
		string? Name,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public Sampler()
			: this(null, null, Wrap.Repeat, Wrap.Repeat, null, null, null) { }
	}

	/// <summary>
	/// The root nodes of a scene.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-scene
	/// </summary>
	public record struct SceneInfo(
		int[] Nodes,
		string? Name,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public SceneInfo()
			: this([], null) { }
	}

	/// <summary>
	/// Joints and matrices defining a skin.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-skin
	/// </summary>
	public record struct Skin(
		int? InverseBindMatrices,
		int? Skeleton,
		int[] Joints,
		string? Name,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public Skin()
			: this(null, null, [], null) { }
	}

	/// <summary>
	/// A texture and its sampler.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-texture
	/// </summary>
	public record struct Texture(
		int? Sampler,
		int? Source,
		string? Name,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	)
	{
		public Texture()
			: this(null, null, null) { }
	}

	/// <summary>
	/// Reference to a texture.<br/>
	/// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-textureinfo
	/// </summary>
	public record struct TextureInfo(
		int Index,
		int TexCoord,
		JsonObject? Extensions = null,
		JsonObject? Extras = null
	);

	/// <summary>
	/// Names of glTF extensions used in this asset.
	/// </summary>
	public string[] ExtensionsUsed { get; set; } = [];

	/// <summary>
	/// Names of glTF extensions required to properly load this asset.
	/// </summary>
	public string[] ExtensionsRequired { get; set; } = [];

	public Accessor[] Accessors { get; set; } = [];
	public Animation[] Animations { get; set; } = [];
	public AssetInfo Asset { get; set; } = new();
	public Buffer[] Buffers { get; set; } = [];
	public BufferView[] BufferViews { get; set; } = [];
	public Image[] Images { get; set; } = [];
	public Material[] Materials { get; set; } = [];
	public Mesh[] Meshes { get; set; } = [];
	public Node[] Nodes { get; set; } = [];
	public Sampler[] Samplers { get; set; } = [];
	public int Scene { get; set; } = 0;
	public SceneInfo[] Scenes { get; set; } = [];
	public Skin[] Skins { get; set; } = [];
	public Texture[] Textures { get; set; } = [];

	/// <summary>
	/// JSON object with extension-specific objects.
	/// </summary>
	public JsonObject Extensions { get; set; } = [];

	/// <summary>
	/// Application-specific data.
	/// </summary>
	public JsonObject Extras { get; set; } = [];

	public Gltf2() { }

	public static Gltf2? Deserialize(Stream stream)
		=> JsonSerializer.Deserialize(stream, Gltf2JsonContext.Default.Gltf2);

    public static Gltf2? Deserialize(ReadOnlySpan<byte> data)
		=> JsonSerializer.Deserialize(data, Gltf2JsonContext.Default.Gltf2);

	public static Gltf2? Deserialize(ref Utf8JsonReader reader)
		=> JsonSerializer.Deserialize(ref reader, Gltf2JsonContext.Default.Gltf2);

	public string Serialize()
		=> JsonSerializer.Serialize(this, Gltf2JsonContext.Default.Gltf2);

	public void Serialize(Stream stream)
		=> JsonSerializer.Serialize(stream, this, Gltf2JsonContext.Default.Gltf2);

	public void Serialize(Utf8JsonWriter writer)
		=> JsonSerializer.Serialize(writer, this, Gltf2JsonContext.Default.Gltf2);

	/// <summary>
	/// All scalar float types are stored in float arrays.
	/// This utility parses them quickly without allocating.
	/// </summary>
	private class FloatStructJsonConverter<T> : JsonConverter<T> where T : unmanaged
	{
		private static readonly int Count = Marshal.SizeOf<T>() / 4;

		public override T Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
		{
			Span<float> values = stackalloc float[Count];
			int length = 0;

			if (reader.TokenType == JsonTokenType.StartArray)
			{
				// read values until the end
				while (reader.Read() && reader.TokenType == JsonTokenType.Number && length < Count)
				{
					values[length] = reader.GetSingle();
					length++;
				}

				// skip any remaining values
				if (reader.TokenType != JsonTokenType.EndArray)
					reader.Skip();
			}

			if (length == Count)
				return MemoryMarshal.Cast<float, T>(values)[0];

			return default;
		}

		public override void Write(Utf8JsonWriter writer, T value, JsonSerializerOptions options)
		{
			var values = MemoryMarshal.Cast<T, float>([value]);
			var result = new StringBuilder();
			result.Append("[ ");
			result.Append($"{values[0]}");
			for (int i = 1; i < values.Length; i++)
				result.Append($", {values[i]}");
			result.Append(" ]");
			writer.WriteRawValue(result.ToString());
		}
	}

	/// <summary>
	/// Some enums are stored as strings, others as numbers.
	/// This handles both cases without throwing.
	/// </summary>
	private class EnumJsonConverter<T>(bool serializeAsNumber, Dictionary<string, T> jsonValues) : JsonConverter<T> where T : struct, Enum
	{
		private readonly bool serializeAsNumber = serializeAsNumber;
		private readonly Dictionary<string, T> fromJsonString = jsonValues;
		private readonly Dictionary<T, string> toJsonString = jsonValues.ToDictionary(it => it.Value, it => it.Key);
		private static readonly Dictionary<int, T> fromJsonNumber = Enum.GetValues<T>().ToDictionary(it => (int)(object)it, it => it);
		private static readonly Dictionary<T, int> toJsonNumber = Enum.GetValues<T>().ToDictionary(it => it, it => (int)(object)it);

		public override T Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
		{
			if (reader.TokenType == JsonTokenType.String &&
				reader.GetString() is { } str &&
				fromJsonString.TryGetValue(str, out T result))
				return result;

			if (reader.TokenType == JsonTokenType.Number &&
				reader.TryGetInt32(out int number) &&
				fromJsonNumber.TryGetValue(number, out result))
				return result;

			return default;
		}

		public override void Write(Utf8JsonWriter writer, T value, JsonSerializerOptions options)
		{
			if (serializeAsNumber)
			{
				if (toJsonNumber.TryGetValue(value, out var num))
					writer.WriteNumberValue(num);
				else
					writer.WriteNumberValue(0);
			}
			else
			{
				if (toJsonString.TryGetValue(value, out var str))
					writer.WriteStringValue(str);
				else
					writer.WriteStringValue(string.Empty);
			}
		}
	}

	private class AccesorTypeJsonConverter() : EnumJsonConverter<AccessorType>(false, new(StringComparer.OrdinalIgnoreCase)
	{
		["SCALAR"] = AccessorType.Scalar,
		["VEC2"] = AccessorType.Vec2,
		["VEC3"] = AccessorType.Vec3,
		["VEC4"] = AccessorType.Vec4,
		["MAT2"] = AccessorType.Mat2,
		["MAT3"] = AccessorType.Mat3,
		["MAT4"] = AccessorType.Mat4
	});

	private class ComponentTypeJsonConverter() : EnumJsonConverter<ComponentType>(true, new(StringComparer.OrdinalIgnoreCase)
	{
		["BYTE"] = ComponentType.Byte,
		["UNSIGNED_BYTE"] = ComponentType.UnsignedByte,
		["SHORT"] = ComponentType.Short,
		["UNSIGNED_SHORT"] = ComponentType.UnsignedShort,
		["INT"] = ComponentType.Int,
		["FLOAT"] = ComponentType.Float
	});

	private class PathJsonConverter() : EnumJsonConverter<Path>(false, new(StringComparer.OrdinalIgnoreCase)
	{
		["translation"] = Path.Translation,
		["rotation"] = Path.Rotation,
		["scale"] = Path.Scale,
		["weights"] = Path.Weights
	});

	private class InterpolationJsonConverter() : EnumJsonConverter<Interpolation>(false, new(StringComparer.OrdinalIgnoreCase)
	{
		["LINEAR"] = Interpolation.Linear,
		["STEP"] = Interpolation.Step,
		["CUBICSPLINE"] = Interpolation.CubicSpline
	});

	private class BufferViewTargetJsonConverter() : EnumJsonConverter<BufferViewTarget>(true, new(StringComparer.OrdinalIgnoreCase)
	{
		["ARRAY_BUFFER"] = BufferViewTarget.ArrayBuffer,
		["ELEMENT_ARRAY_BUFFER"] = BufferViewTarget.ElementArrayBuffer
	});

	private class MimeTypeJsonConverter() : EnumJsonConverter<MimeType>(false, new(StringComparer.OrdinalIgnoreCase)
	{
		["image/jpeg"] = MimeType.ImageJpeg,
		["image/png"] = MimeType.ImagePng
	});

	private class AlphaModeJsonConverter() : EnumJsonConverter<AlphaMode>(false, new(StringComparer.OrdinalIgnoreCase)
	{
		["OPAQUE"] = AlphaMode.Opaque,
		["MASK"] = AlphaMode.Mask,
		["BLEND"] = AlphaMode.Blend
	});

	private class PrimitiveModeJsonConverter() : EnumJsonConverter<PrimitiveMode>(true, new(StringComparer.OrdinalIgnoreCase)
	{
		["POINTS"] = PrimitiveMode.Points,
		["LINES"] = PrimitiveMode.Lines,
		["LINE_LOOP"] = PrimitiveMode.LineLoop,
		["LINE_STRIP"] = PrimitiveMode.LineStrip,
		["TRIANGLES"] = PrimitiveMode.Triangles,
		["TRIANGLE_STRIP"] = PrimitiveMode.TriangleStrip,
		["TRIANGLE_FAN"] = PrimitiveMode.TriangleFan
	});

	private class MagFilterJsonConverter() : EnumJsonConverter<MagFilter>(true, new(StringComparer.OrdinalIgnoreCase)
	{
		["NEAREST"] = MagFilter.Nearest,
		["LINEAR"] = MagFilter.Linear
	});

	private class MinFilterJsonConverter() : EnumJsonConverter<MinFilter>(true, new(StringComparer.OrdinalIgnoreCase)
	{
		["NEAREST"] = MinFilter.Nearest,
		["LINEAR"] = MinFilter.Linear,
		["NEAREST_MIPMAP_NEAREST"] = MinFilter.NearestMipMapNearest,
		["LINEAR_MIPMAP_NEAREST"] = MinFilter.LinearMipMapNearest,
		["NEAREST_MIPMAP_LINEAR"] = MinFilter.NearestMipMapLinear,
		["LINEAR_MIPMAP_LINEAR"] = MinFilter.LinearMipMapLinear,
	});

	private class WrapJsonConverter() : EnumJsonConverter<Wrap>(true, new(StringComparer.OrdinalIgnoreCase)
	{
		["CLAMP_TO_EDGE"] = Wrap.ClampToEdge,
		["MIRRORED_REPEAT"] = Wrap.MirroredRepeat,
		["REPEAT"] = Wrap.Repeat,
	});

	[JsonSerializable(typeof(Gltf2))]
	[JsonSourceGenerationOptions(
		AllowTrailingCommas = true,
		PropertyNameCaseInsensitive = true,
		PropertyNamingPolicy = JsonKnownNamingPolicy.CamelCase,
		WriteIndented = true,
		IndentSize = 2,
		DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
		Converters = [
			typeof(FloatStructJsonConverter<Quaternion>),
			typeof(FloatStructJsonConverter<Vector2>),
			typeof(FloatStructJsonConverter<Vector3>),
			typeof(FloatStructJsonConverter<Vector4>),
			typeof(FloatStructJsonConverter<Matrix3x2>),
			typeof(FloatStructJsonConverter<Matrix4x4>),
			typeof(AccesorTypeJsonConverter),
			typeof(ComponentTypeJsonConverter),
			typeof(PathJsonConverter),
			typeof(InterpolationJsonConverter),
			typeof(BufferViewTargetJsonConverter),
			typeof(MimeTypeJsonConverter),
			typeof(AlphaModeJsonConverter),
			typeof(PrimitiveModeJsonConverter),
			typeof(MagFilterJsonConverter),
			typeof(MinFilterJsonConverter),
			typeof(WrapJsonConverter)
		]
	)]
	private partial class Gltf2JsonContext : JsonSerializerContext { }
}
