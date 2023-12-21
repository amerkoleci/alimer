// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections;
using System.ComponentModel;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.Serialization;
using System.Text.Json.Serialization;
using Alimer.Numerics;
using Vortice.Mathematics;

namespace Alimer.Engine;

[DefaultEntitySystem(typeof(TransformSystem))]
public sealed class TransformComponent : EntityComponent, IEnumerable<TransformComponent>, INotifyPropertyChanged
{
    private Matrix4x4 localMatrix = Matrix4x4.Identity;
    private Matrix4x4 worldMatrix = Matrix4x4.Identity;

    private Vector3 _position;
    private Quaternion _rotation = Quaternion.Identity;
    private Vector3 _scale = Vector3.One;

    public event PropertyChangedEventHandler? PropertyChanged;

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
    public ref Matrix4x4 LocalMatrix => ref localMatrix;

    [IgnoreDataMember]
    [JsonIgnore]
    public ref Matrix4x4 WorldMatrix => ref worldMatrix;

    [JsonPropertyOrder(10)]
    public Vector3 Position { get => _position; set => Set(ref _position, value); }

    [JsonPropertyOrder(20)]
    public Quaternion Rotation { get => _rotation; set => Set(ref _rotation, value); }

    [JsonPropertyOrder(30)]
    [DefaultValue("1,1,1")]
    public Vector3 Scale { get => _scale; set => Set(ref _scale, value); }

    [IgnoreDataMember]
    [JsonIgnore]
    public Vector3 RotationEuler { get => Rotation.ToEuler(); set => Rotation = value.FromEuler(); }

    public override string ToString() => $"Position: {Position}, Rotation: {RotationEuler}, Scale: {Scale}";

    public IEnumerator<TransformComponent> GetEnumerator() => Children.GetEnumerator();

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

    public void UpdateLocalMatrix()
    {
        LocalMatrix = Matrix4x4.CreateScale(Scale)
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

    private void OnPropertyChanged([CallerMemberName] string propertyName = "")
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    private bool Set<T>(ref T field, T value, [CallerMemberName] string propertyName = "")
    {
        if (!EqualityComparer<T>.Default.Equals(field, value))
        {
            field = value;
            OnPropertyChanged(propertyName);
            return true;
        }

        return false;
    }
}
