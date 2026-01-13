// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Specialized;
using System.Linq;
using System.Reflection;
using Alimer.Graphics;

namespace Alimer.Engine;

public abstract class EntityManager : DisposableObject, IGameSystem, IEnumerable<Entity>
{
    private static readonly ConcurrentDictionary<Type, Func<IServiceRegistry, EntitySystem>> s_registeredFactories = [];
    private readonly HashSet<Entity> _entities = [];
    private readonly Dictionary<Type, List<EntitySystem>> _systemsPerComponentType = [];

    public static bool RegisterSystemFactory<T>() where T : EntitySystem, new()
    {
        return s_registeredFactories.TryAdd(typeof(T), (services) => new T());
    }

    public static bool RegisterSystemFactory<T>(Func<IServiceRegistry, T> factory) where T : EntitySystem
    {
        return s_registeredFactories.TryAdd(typeof(T), factory);
    }

    protected EntityManager(IServiceRegistry services)
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));

        Services = services;
    }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            foreach (EntitySystem system in Systems)
            {
                system.Dispose();
            }
        }
    }

    public IServiceRegistry Services { get; }

    public EntitySystemCollection Systems { get; } = [];

    public IEnumerator<Entity> GetEnumerator() => _entities.GetEnumerator();

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

    public virtual void Update(GameTime time)
    {
        foreach (EntitySystem system in Systems)
        {
            system.Update(time);
        }
    }

    public void BeginDraw()
    {
        foreach (EntitySystem system in Systems)
        {
            system.BeginDraw();
        }
    }

    public virtual void Draw(CommandBuffer renderContext, Texture outputTexture, GameTime time)
    {
        foreach (EntitySystem system in Systems)
        {
            system.Draw(renderContext, outputTexture, time);
        }
    }

    public void EndDraw()
    {
        foreach (EntitySystem system in Systems)
        {
            system.EndDraw();
        }
    }

    internal void AddRoot(Entity entity)
    {
        if (entity.Parent != null)
        {
            throw new ArgumentException("This entity should not have a parent.", nameof(entity));
        }

        Add(entity);
    }

    internal void Add(Entity entity)
    {
        if (_entities.Contains(entity)) return;

        if (entity.EntityManager != null)
        {
            throw new ArgumentException("This entity is already used by another entity manager.", nameof(entity));
        }

        entity.EntityManager = this;

        _entities.Add(entity);

        foreach (EntityComponent component in entity.Components)
        {
            AddComponent(component, entity);
        }

        foreach (Entity child in entity.Children)
        {
            Add(child);
        }

        entity.Children.CollectionChanged += OnChildrenCollectionChanged;
        entity.Components.CollectionChanged += OnComponentsCollectionChanged;
    }

    internal void RemoveRoot(Entity entity)
    {
        Remove(entity);
    }

    internal void Remove(Entity entity)
    {
        if (!_entities.Remove(entity)) return;

        entity.Components.CollectionChanged -= OnComponentsCollectionChanged;
        entity.Children.CollectionChanged -= OnChildrenCollectionChanged;

        foreach (EntityComponent component in entity.Components)
        {
            RemoveComponent(component, entity);
        }

        foreach (Entity child in entity.Children)
        {
            Remove(child);
        }

        entity.EntityManager = null;
    }

    protected virtual void AddComponent(EntityComponent component, Entity entity)
    {
        CheckEntityComponentWithSystems(component, entity, false);
    }

    protected virtual void RemoveComponent(EntityComponent component, Entity entity)
    {
        CheckEntityComponentWithSystems(component, entity, true);
    }

    private void CheckEntityComponentWithSystems(EntityComponent component, Entity entity, bool forceRemove)
    {
        Type componentType = component.GetType();

        if (_systemsPerComponentType.TryGetValue(componentType, out List<EntitySystem>? systemsForComponent))
        {
            foreach (EntitySystem system in systemsForComponent)
            {
                system.ProcessEntityComponent(component, entity, forceRemove);
            }
        }
        else
        {
            if (!forceRemove)
            {
                CollectNewEntitySystems(componentType);
            }

            systemsForComponent = new List<EntitySystem>();

            foreach (EntitySystem system in Systems)
            {
                if (system.Accepts(componentType))
                {
                    systemsForComponent.Add(system);
                }
            }

            _systemsPerComponentType.Add(componentType, systemsForComponent);

            foreach (EntitySystem system in systemsForComponent)
            {
                system.ProcessEntityComponent(component, entity, forceRemove);
            }
        }
    }

    private void CollectNewEntitySystems(Type componentType)
    {
        IEnumerable<DefaultEntitySystemAttribute> entitySystemAttributes = componentType.GetCustomAttributes<DefaultEntitySystemAttribute>();

        foreach (DefaultEntitySystemAttribute entitySystemAttribute in entitySystemAttributes)
        {
            bool addNewSystem = !Systems.Any(s => s.GetType() == entitySystemAttribute.Type);

            if (addNewSystem)
            {
                if (s_registeredFactories.TryGetValue(entitySystemAttribute.Type, out Func<IServiceRegistry, EntitySystem>? factory))
                {
                    EntitySystem system = factory(Services);
                    system.EntityManager = this;

                    Systems.Add(system);
                    return;
                }

                throw new InvalidOperationException("No EntitySystem registered");
            }
        }
    }

    private void UpdateDependentSystems(Entity entity, EntityComponent skipComponent)
    {
        foreach (EntityComponent component in entity.Components)
        {
            if (component == skipComponent) continue;

            Type componentType = component.GetType();

            if (_systemsPerComponentType.TryGetValue(componentType, out var systemsForComponent))
            {
                foreach (EntitySystem system in systemsForComponent)
                {
                    system.ProcessEntityComponent(component, entity, false);
                }
            }
        }
    }

    private void OnChildrenCollectionChanged(object? sender, NotifyCollectionChangedEventArgs e)
    {
        switch (e.Action)
        {
            case NotifyCollectionChangedAction.Add:
                foreach (Entity entity in e.NewItems!.Cast<Entity>())
                {
                    Add(entity);
                }
                break;
            case NotifyCollectionChangedAction.Remove:
                foreach (Entity entity in e.OldItems!.Cast<Entity>())
                {
                    Remove(entity);
                }
                break;
        }
    }

    private void OnComponentsCollectionChanged(object? sender, NotifyCollectionChangedEventArgs e)
    {
        EntityComponentCollection entityComponentCollection = (EntityComponentCollection)sender!;
        Entity entity = entityComponentCollection.Entity;

        switch (e.Action)
        {
            case NotifyCollectionChangedAction.Add:
                foreach (EntityComponent component in e.NewItems!.Cast<EntityComponent>())
                {
                    AddComponent(component, entity);
                    UpdateDependentSystems(entity, component);
                }
                break;
            case NotifyCollectionChangedAction.Remove:
                foreach (EntityComponent component in e.OldItems!.Cast<EntityComponent>())
                {
                    RemoveComponent(component, entity);
                    UpdateDependentSystems(entity, component);
                }
                break;
        }
    }
}
