// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections;
using System.ComponentModel;
using Alimer.Numerics;
using System.Runtime.Serialization;
using System.Text.Json.Serialization;

namespace Alimer.Engine;

[DefaultEntitySystem(typeof(TransformSystem))]
public partial class TransformComponent : Component, IEnumerable<TransformComponent>
{
    private Matrix4x4 _localMatrix = Matrix4x4.Identity;
    private Matrix4x4 _worldMatrix = Matrix4x4.Identity;

    private Vector3 _position;
    private Quaternion _rotation = Quaternion.Identity;
    private Vector3 _scale = Vector3.One;

    public IEnumerable<TransformComponent> Children
    {
        get
        {
            if (Entity is null) yield break;

            foreach (Entity entity in Entity.Children)
            {
                if (entity.Transform != null)
                {
                    yield return entity.Transform;
                }
            }
        }
    }

    [IgnoreDataMember]
    [JsonIgnore]
    public TransformComponent? Parent
    {
        get => Entity?.Parent?.Transform;
        set
        {
            if (Entity != null)
                Entity.Parent = value?.Entity;
        }
    }

    [IgnoreDataMember]
    [JsonIgnore]
    public bool IsTransformHierarchyRoot => Parent is null;

    [IgnoreDataMember]
    [JsonIgnore]
    public ref Matrix4x4 LocalMatrix => ref _localMatrix;

    [IgnoreDataMember]
    [JsonIgnore]
    public ref Matrix4x4 WorldMatrix => ref _worldMatrix;

    [JsonPropertyOrder(10)]
    public Vector3 Position
    {
        get => _position;
        set => _position = value;
    }

    [JsonPropertyOrder(20)]
    public Quaternion Rotation
    {
        get => _rotation;
        set => _rotation = value;
    }

    [JsonPropertyOrder(30)]
    [DefaultValue("1,1,1")]
    public Vector3 Scale
    {
        get => _scale;
        set => _scale = value;
    }

    [JsonIgnore]
    public Quaternion WorldRotation
    {
        get
        {
            // TODO
            //_worldRotation = _parent->GetRotation() * localRotation;
            return Rotation;
        }
    }

    [IgnoreDataMember]
    [JsonIgnore]
    public Vector3 RotationEuler { get => Rotation.ToEuler(); set => Rotation = value.FromEuler(); }

    public override string ToString() => $"Position: {Position}, Rotation: {RotationEuler}, Scale: {Scale}";

    public IEnumerator<TransformComponent> GetEnumerator() => Children.GetEnumerator();

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

    /// <summary>
    /// Rotate the scene node in the chosen transform space.
    /// </summary>
    /// <param name="delta"></param>
    /// <param name="space"></param>
    public void Rotate(in Quaternion delta, TransformSpace space = TransformSpace.World)
    {
        switch (space)
        {
            case TransformSpace.Local:
                _rotation = Quaternion.Normalize(_rotation * delta);
                break;

            case TransformSpace.Parent:
                _rotation = Quaternion.Normalize(delta * _rotation);
                break;

            case TransformSpace.World:
                if (IsTransformHierarchyRoot)
                {
                    _rotation = Quaternion.Normalize(delta * _rotation);
                }
                else
                {
                    Quaternion worldRotation = WorldRotation;
                    _rotation = _rotation * Quaternion.Inverse(worldRotation) * delta * worldRotation;
                }
                break;
        }

        //MarkDirty();
    }

    /// <summary>
    /// Rotate the entity using euler angles (in degrees) in the chosen transform space.
    /// </summary>
    /// <remarks>Eulers are specified in degrees in XYZ order.</remarks>
    /// <param name="angles"></param>
    /// <param name="space"></param>
    public void Rotate(in Vector3 angles, TransformSpace space = TransformSpace.World)
    {
        Quaternion rotation = angles.FromEuler();
        Rotate(rotation, space);
    }

    /// <summary>
    /// Rotate the entity using euler angles (in degrees) in the chosen transform space.
    /// </summary>
    /// <remarks>Eulers are specified in degrees in XYZ order.</remarks>
    /// <param name="angles"></param>
    /// <param name="space"></param>
    public void Rotate(float x, float y, float z, TransformSpace space = TransformSpace.World)
    {
        Quaternion rotation = QuaternionExtensions.FromEuler(x, y, z);
        Rotate(rotation, space);
    }

    public void UpdateLocalMatrix()
    {
        _localMatrix = Matrix4x4.CreateScale(Scale)
            * Matrix4x4.CreateFromQuaternion(Rotation)
            * Matrix4x4.CreateTranslation(Position);
    }

    public void UpdateLocalFromWorldMatrix()
    {
        if (Parent is null)
        {
            LocalMatrix = WorldMatrix;
        }
        else
        {
            if (Matrix4x4.Invert(Parent.WorldMatrix, out Matrix4x4 inverseParentMatrix))
            {
                LocalMatrix = WorldMatrix * inverseParentMatrix;
            }
        }

        (Scale, Rotation, Position) = LocalMatrix;
    }

    public void UpdateWorldMatrix()
    {
        UpdateLocalMatrix();
        UpdateWorldMatrixInternal(true);
    }

    internal void UpdateWorldMatrixInternal(bool recursive)
    {
        if (Parent is null)
        {
            WorldMatrix = LocalMatrix;
        }
        else
        {
            if (recursive)
            {
                Parent.UpdateWorldMatrix();
            }

            WorldMatrix = LocalMatrix * Parent.WorldMatrix;
        }
    }
}
