// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;
using System.Diagnostics.CodeAnalysis;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public abstract unsafe class GraphicsManager : GraphicsObjectBase
{
    protected GraphicsManager(in GraphicsManagerOptions options)
        : base(options.Label)
    {
        ValidationMode = options.ValidationMode;
    }

    /// <summary>
    /// Get the type of the graphics backend.
    /// </summary>
    public abstract GraphicsBackendType BackendType { get; }

    /// <summary>
    /// Get the validation mode.
    /// </summary>
    public GraphicsValidationMode ValidationMode { get; }

    /// <summary>
    /// Gets the list of available <see cref="GraphicsAdapter"/>.
    /// </summary>
    public abstract ReadOnlySpan<GraphicsAdapter> Adapters { get; }

    public static bool IsBackendSupport(GraphicsBackendType backendType)
    {
        Guard.IsTrue(backendType != GraphicsBackendType.Default, nameof(backendType), "Invalid backend type");

        switch (backendType)
        {
#if !EXCLUDE_VULKAN_BACKEND
            case GraphicsBackendType.Vulkan:
                return Vulkan.VulkanGraphicsManager.IsSupported;
#endif
#if !EXCLUDE_D3D12_BACKEND
            case GraphicsBackendType.D3D12:
                return D3D12.D3D12GraphicsManager.IsSupported;
#endif
#if !EXCLUDE_METAL_BACKEND
            case GraphicsBackendType.Metal:
                return Metal.MetalGraphicsManager.IsSupported;
#endif

            default:
                return false;
        }
    }

    /// <summary>
    /// Creates a new <see cref="GraphicsManager"/> with the default options.
    /// </summary>
    public static GraphicsManager Create() => Create(new());

    /// <summary>
    /// Creates a new <see cref="GraphicsManager"/> with the specified options.
    /// </summary>
    /// <param name="options">Options for the graphics manager.</param>
    public static GraphicsManager Create(in GraphicsManagerOptions options)
    {
        GraphicsBackendType backend = options.PreferredBackend;
        if (backend == GraphicsBackendType.Default)
        {
            if (IsBackendSupport(GraphicsBackendType.D3D12))
            {
                backend = GraphicsBackendType.D3D12;
            }
            else if (IsBackendSupport(GraphicsBackendType.Metal))
            {
                backend = GraphicsBackendType.Metal;
            }
            else if (IsBackendSupport(GraphicsBackendType.Vulkan))
            {
                backend = GraphicsBackendType.Vulkan;
            }
        }

        GraphicsManager? manager = default;
        switch (backend)
        {
#if !EXCLUDE_VULKAN_BACKEND
            case GraphicsBackendType.Vulkan:
                if (Vulkan.VulkanGraphicsManager.IsSupported)
                {
                    manager = new Vulkan.VulkanGraphicsManager(in options);
                }
                break;
#endif

#if !EXCLUDE_D3D12_BACKEND 
            case GraphicsBackendType.D3D12:
                if (D3D12.D3D12GraphicsManager.IsSupported)
                {
                    manager = new D3D12.D3D12GraphicsManager(in options);
                }
                break;
#endif

#if !EXCLUDE_METAL_BACKEND
            case GraphicsBackendType.Metal:
                manager = new Metal.MetalGraphicsManager(in options);
                break;
#endif

            default:
                break;
        }

        if (manager == null)
        {
            throw new GraphicsException($"{backend} is not supported");
        }

        return manager!;
    }

    /// <summary>
    /// Tries to get the best adapter (first from discrete GPU, then integrated GPU, then virtual GPU, CPU, other).
    /// </summary>
    /// <param name="adapter"></param>
    /// <returns><c>true</c> if an adapter was found; otherwise <c>false</c></returns>
    public bool TryGetBestAdapter([NotNullWhen(true)] out GraphicsAdapter? adapter)
    {
        adapter = null;
        int kind = (int)GraphicsAdapterType.Other + 1;
        foreach (GraphicsAdapter adapter1 in Adapters)
        {
            if ((int)adapter1.Type < kind)
            {
                adapter = adapter1;
                kind = (int)adapter1.Type;
            }
        }

        return adapter != null;
    }



    /// <summary>
    /// Gets the best adapter (first from discrete GPU, then integrated GPU, then virtual GPU, CPU, other).
    /// </summary>
    /// <returns>The best adapter</returns>
    /// <exception cref="GraphicsException">If no adapter were found.</exception>
    public GraphicsAdapter GetBestAdapter()
    {
        if (TryGetBestAdapter(out GraphicsAdapter? adapter))
        {
            return adapter!;
        }

        throw new GraphicsException("No adapter found");
    }
}
