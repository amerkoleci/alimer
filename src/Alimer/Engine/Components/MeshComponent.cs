// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Rendering;

namespace Alimer.Engine;

public sealed partial class MeshComponent : RenderableComponent
{
    /// Local-space bounding box.
    private BoundingBox _localBoundingBox;
    /// World-space bounding box.
    private BoundingBox _worldBoundingBox;
    /// World-space bounding box dirty flag.
    private bool _worldBoundingBoxDirty;

    public MeshComponent()
    {
    }

    public MeshComponent(Mesh mesh)
    {
        Mesh = mesh;
        _localBoundingBox = mesh.Bounds;
        _worldBoundingBoxDirty = true;
    }

    public Mesh? Mesh { get; set; }

    public IList<Material> Materials { get; } = [];
    public BoundingBox LocalBoundingBox => _localBoundingBox;

    public BoundingBox WorldBoundingBox
    {
        get
        {
            if (Entity is null)
                return _localBoundingBox;

            if (!_worldBoundingBoxDirty)
                return _worldBoundingBox;

            //OnWorldBoundingBoxUpdate();
            _worldBoundingBox = BoundingBox.Transform(_localBoundingBox, Entity!.Transform.WorldMatrix);
            _worldBoundingBoxDirty = false;
            return _worldBoundingBox;
        }
    }
}
