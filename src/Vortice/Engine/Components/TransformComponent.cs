// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.Serialization;

namespace Vortice.Engine;

//[DefaultEntitySystem(typeof(TransformSystem))]
public sealed class TransformComponent : EntityComponent//, IEnumerable<TransformComponent>, INotifyPropertyChanged
{
    private Vector3 _position;
    private Quaternion _rotation = Quaternion.Identity;
    private Vector3 _scale = Vector3.One;

    private Matrix4x4 _localMatrix = Matrix4x4.Identity;
    private Matrix4x4 _worldMatrix = Matrix4x4.Identity;

    [IgnoreDataMember]
    public ref Matrix4x4 LocalMatrix => ref _localMatrix;

    [IgnoreDataMember]
    public ref Matrix4x4 WorldMatrix => ref _worldMatrix;
}
