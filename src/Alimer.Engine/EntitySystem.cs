// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Engine;

public abstract class EntitySystem : DisposableObject, IGameSystem
{
    protected EntitySystem()
    {
    }

    protected EntitySystem(Type? mainComponentType)
    {
        MainComponentType = mainComponentType;
    }

    protected EntitySystem(Type? mainComponentType, params Type[] requiredComponentTypes)
    {
        MainComponentType = mainComponentType;

        foreach (Type type in requiredComponentTypes)
        {
            RequiredComponentTypes.Add(type);
        }
    }

    public Type? MainComponentType { get; }

    public IList<Type> RequiredComponentTypes { get; } = new List<Type>();

    public EntityManager? EntityManager { get; internal set; }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
    }

    public virtual void Update(AppTime time)
    {
    }

    public virtual void BeginDraw()
    {
    }

    public virtual void Draw(RenderContext renderContext, Texture outputTexture, AppTime time)
    {
    }

    public virtual void EndDraw()
    {
    }

    public abstract void ProcessEntityComponent(EntityComponent component, Entity entity, bool forceRemove);

    public virtual bool Accepts(Type type) => MainComponentType?.IsAssignableFrom(type) ?? false;
}

public abstract class EntitySystem<TComponent> : EntitySystem where TComponent : EntityComponent
{
    protected EntitySystem() : base(typeof(TComponent))
    {
    }

    protected EntitySystem(params Type[] requiredComponentTypes) : base(typeof(TComponent), requiredComponentTypes)
    {
    }

    protected HashSet<TComponent> Components { get; } = new HashSet<TComponent>();

    public override void ProcessEntityComponent(EntityComponent component, Entity entity, bool forceRemove)
    {
        if (component is not TComponent entityComponent)
            throw new ArgumentException("The entity component must be assignable to TComponent", nameof(component));

        bool entityMatch = !forceRemove && EntityMatch(entity);
        bool entityAdded = Components.Contains(entityComponent);

        if (entityMatch && !entityAdded)
        {
            Components.Add(entityComponent);
            OnEntityComponentAdded(entityComponent);
        }
        else if (!entityMatch && entityAdded)
        {
            Components.Remove(entityComponent);
            OnEntityComponentRemoved(entityComponent);
        }
    }

    protected virtual void OnEntityComponentAdded(TComponent component)
    {
    }

    protected virtual void OnEntityComponentRemoved(TComponent component)
    {
    }

    private bool EntityMatch(Entity entity)
    {
        if (RequiredComponentTypes.Count == 0) return true;

        List<Type> remainingRequiredTypes = new(RequiredComponentTypes);

        foreach (EntityComponent component in entity.Components)
        {
            remainingRequiredTypes.RemoveAll(t => t.IsAssignableFrom(component.GetType()));

            if (remainingRequiredTypes.Count == 0) return true;
        }

        return false;
    }
}
