#if TODO
// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.Serialization;
using System.Text.Json.Serialization;
using static Box3DApi;

namespace Alimer.Physics;

//[Meta]
public partial class RigidBodyComponent : PhysicsComponent
{
    private ColliderShape? _shape;
    private Vector3 _linearVelocity = Vector3.Zero;


    /// <summary>
    /// Getrs or sets the body type
    /// </summary>
    public RigidBodyType BodyType
    {
        get;
        set
        {
            if (field == value)
                return;

            field = value;
            if (IsValid)
            {
                b3Body_SetType(ID, value.ToBox3D());
            }
        }
    } = RigidBodyType.Dynamic;

    public virtual ColliderShape? ColliderShape
    {
        get => _shape;
        set
        {
            if (_shape == value)
                return;

            _shape = value;
        }
    }

    public float Mass
    {
        get;
        set
        {
            field = MathF.Max(value, 0.001f);

            if (IsValid)
            {
                //b3Body_SetMassData()
            }
        }
    } = 1.0f;

    /// <summary>
    /// Gets or sets the linear velocity.
    /// </summary>
    /// <value>
    /// The linear velocity.
    /// </value>
    [IgnoreDataMember]
    [JsonIgnore]
    public Vector3 LinearVelocity
    {
        get
        {
            if (!IsValid)
                return _linearVelocity;

            _linearVelocity = b3Body_GetLinearVelocity(ID);
            return _linearVelocity;
        }
        set
        {
            if (IsValid)
            {
                b3Body_SetLinearVelocity(ID, value);
            }

            _linearVelocity = value;
        }
    }


    //[IgnoreDataMember]
    //[JsonIgnore]
    //public override Matrix4x4 PhysicsWorldTransform
    //{
    //    get
    //    {
    //        if (Handle.IsNull)
    //            return Matrix4x4.Identity;

    //        alimerPhysicsBodyGetWorldTransform(Handle, out Matrix4x4 transform);
    //        return transform;
    //    }
    //    set
    //    {
    //    }
    //}

    protected override void OnAttach()
    {
        base.OnAttach();

        if (IsValid)
        {
            b3DestroyBody(ID);
            ID = default;
        }

        if (_shape is null)
            return;

        //Matrix4x4.Decompose(Entity!.Transform.WorldMatrix, out _, out Quaternion rotation, out Vector3 translation);

        b3BodyDef bodyDef = b3DefaultBodyDef();
        //bodyDef.position =
        //bodyDesc.initialTransform = new PhysicsBodyTransform
        //{
        //    position = translation,
        //    rotation = rotation
        //};

        bodyDef.type = BodyType.ToBox3D();
        bodyDef.linearVelocity = _linearVelocity;


        ID = b3CreateBody(World!.ID, in bodyDef);

        // Add it to the world
        World!.RigidBodies.Add(ID, this);
    }

    protected override void OnDetach()
    {
        base.OnDetach();

        World!.RigidBodies.Remove(ID);
        b3DestroyBody(ID);
        ID = default;
    }
}

#endif
