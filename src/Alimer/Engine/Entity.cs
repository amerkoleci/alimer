// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.Numerics;
using System.Runtime.Serialization;
using System.Text.Json.Serialization;
using System.Linq;

namespace Alimer.Engine;

[DataContract]
[DebuggerTypeProxy(typeof(EntityDebugView))]
public partial class Entity
{
    private Entity? _parent;
    private string _name;

    public Entity()
        : this("Entity")
    {
    }

    public Entity(string name, in Vector3 position = default)
    {
        _name = name;
        Id = Guid.NewGuid();

        Children = [];
        Components = new EntityComponentCollection(this);

        Children.CollectionChanged += OnChildrenCollectionChanged;
        Components.CollectionChanged += OnComponentsCollectionChanged;

        Transform = new TransformComponent()
        {
            Position = position,
        };

        Components.Add(Transform);
    }

    [DataMember]
    [JsonPropertyOrder(0)]
    [Browsable(false)]
    public Guid Id { get; set; }

    [DataMember]
    [JsonPropertyOrder(10)]
    public string Name
    {
        get => _name;
        set => _name = value;
    }

    /// <summary>
    /// 
    /// </summary>
    [DataMember]
    [JsonPropertyOrder(20)]
    public EntityFlags Flags { get; set; } = EntityFlags.None;

    [DataMember]
    public EntityCollection Children { get; }

    [DataMember]
    public EntityComponentCollection Components { get; }

    [IgnoreDataMember]
    public Entity? Parent
    {
        get => _parent;
        set
        {
            Entity? oldParent = _parent;

            if (oldParent == value) return;

            oldParent?.Children.Remove(this);
            value?.Children.Add(this);
        }
    }

    public EntityManager? EntityManager { get; internal set; }

    public TransformComponent Transform { get; private set; }

    [IgnoreDataMember]
    [JsonIgnore]
    public ref Matrix4x4 WorldTransform => ref Transform.WorldMatrix;

    /// <summary>
    /// Adds the specified component to the entity and returns it.
    /// </summary>
    /// <typeparam name="T">The type of component to add. Must derive from EntityComponent.</typeparam>
    /// <param name="component">The component instance to add to the entity. Cannot be null.</param>
    /// <returns>The component that was added to the entity.</returns>
    public Component AddComponent<T>(T component)
        where T : Component
    {
        Components.Add(component);
        return component;
    }

    /// <summary>
    /// Adds a new component of the specified type to the entity and returns the instance.
    /// </summary>
    /// <typeparam name="T">The type of component to add. Must be a subclass of EntityComponent and have a parameterless constructor.</typeparam>
    /// <returns>The newly created component of type T that was added to the entity.</returns>
    public T AddComponent<T>()
        where T : Component, new()
    {
        T component = new();
        Components.Add(component);
        return component;
    }

    /// <summary>
    /// Retrieves the first component of the specified type from the entity.
    /// </summary>
    /// <remarks>Use this method to access a specific type of component associated with the entity. If
    /// multiple components of type T are present, only the first one found is returned.</remarks>
    /// <typeparam name="T">The type of component to retrieve. Must derive from EntityComponent.</typeparam>
    /// <returns>The first component of type T attached to the entity.</returns>
    /// <exception cref="InvalidOperationException">Thrown if the entity does not contain a component of type T.</exception>
    public T GetComponent<T>()
        where T : Component
    {
        foreach (Component component in Components)
        {
            if (component is T typedComponent)
            {
                return typedComponent;
            }
        }

        throw new InvalidOperationException($"Entity does not have a component of type {typeof(T)}.");
    }

    /// <summary>
    /// Attempts to retrieve the first component of the specified type from the collection of components.
    /// </summary>
    /// <remarks>If multiple components of type T exist, only the first one encountered is returned. The
    /// search includes derived types of T.</remarks>
    /// <typeparam name="T">The type of component to search for. Must derive from EntityComponent.</typeparam>
    /// <returns>The first component of type T if found; otherwise, null.</returns>
    public T? TryGetComponent<T>()
        where T : Component
    {
        foreach (Component component in Components)
        {
            if (component is T typedComponent)
            {
                return typedComponent;
            }
        }

        return default;
    }

    /// <summary>
    /// Retrieves an existing component of the specified type from the entity, or creates and adds a new one if it does
    /// not exist.
    /// </summary>
    /// <typeparam name="T">The type of component to retrieve or create. Must derive from EntityComponent and have a parameterless
    /// constructor.</typeparam>
    /// <returns>The existing component of type T if found; otherwise, a new instance of T that has been added to the entity.</returns>
    public T GetOrCreateComponent<T>()
        where T : Component, new()
    {
        T? component = GetComponent<T>();

        if (component is null)
        {
            component = new T();
            Components.Add(component);
        }

        return component;
    }

    /// <summary>
    /// Determines whether the entity contains a component of the specified type.
    /// </summary>
    /// <typeparam name="T">The type of component to search for. Must derive from EntityComponent.</typeparam>
    /// <returns>true if a component of type T is attached to the entity; otherwise, false.</returns>
	public bool HasComponent<T>()
        where T : Component
    {
        foreach (Component component in Components)
        {
            if (component is T typedComponent)
            {
                return true;
            }
        }

        return false;
    }

    /// <summary>
    /// Removes the first component of the specified type from the entity.
    /// </summary>
    /// <remarks>If multiple components of type T are attached to the entity, only the first occurrence is
    /// removed. No action is taken if no component of the specified type exists.</remarks>
    /// <typeparam name="T">The type of component to remove. Must derive from EntityComponent.</typeparam>
    /// <returns>true if a component of type T was found and removed; otherwise, false.</returns>
    public bool RemoveComponent<T>()
        where T : Component
    {
        for (int i = 0; i < Components.Count; i++)
        {
            Component component = Components[i];
            if (component is T typedComponent)
            {
                //component.OnDetach();
                Components.RemoveAt(i);
                return true;
            }
        }

        return false;
    }

    /// <summary>
    /// Removes the specified component from the entity.
    /// </summary>
    /// <param name="component">The component to remove from the entity. Cannot be null.</param>
    /// <returns>true if the component was successfully removed; otherwise, false.</returns>
    public bool RemoveComponent(Component component)
    {
        return Components.Remove(component);
    }

    public override string ToString() => $"Entity {Name}";

    private void AddInternal(Entity entity)
    {
        if (entity.Parent != null)
        {
            throw new InvalidOperationException("This entity already has parent.");
        }

        entity._parent = this;
    }

    private void RemoveInternal(Entity entity)
    {
        if (entity.Parent != this)
        {
            throw new InvalidOperationException("This entity is not a child of the expected parent.");
        }

        entity._parent = null;
    }

    private void AddInternal(Component component)
    {
        if (component.Entity != null)
        {
            throw new InvalidOperationException("An entity component cannot be set on more than one entity.");
        }

        if (component is TransformComponent transformComponent)
        {
            Transform = transformComponent;
        }

        component.Entity = this;
    }

    private void RemoveInternal(Component component)
    {
        if (component.Entity != this)
        {
            throw new InvalidOperationException("This entity component is not set on this entity.");
        }

        if (component is TransformComponent)
        {
            throw new InvalidOperationException("An entity always has to have a transform component.");
        }

        component.Entity = null;
    }

    private void OnChildrenCollectionChanged(object? sender, NotifyCollectionChangedEventArgs e)
    {
        switch (e.Action)
        {
            case NotifyCollectionChangedAction.Add:
                foreach (Entity entity in e.NewItems!)
                {
                    AddInternal(entity);
                }
                break;
            case NotifyCollectionChangedAction.Remove:
                foreach (Entity entity in e.OldItems!)
                {
                    RemoveInternal(entity);
                }
                break;
        }
    }

    private void OnComponentsCollectionChanged(object? sender, NotifyCollectionChangedEventArgs e)
    {
        switch (e.Action)
        {
            case NotifyCollectionChangedAction.Add:
                foreach (Component component in e.NewItems!)
                {
                    AddInternal(component);
                }
                break;
            case NotifyCollectionChangedAction.Remove:
                foreach (Component component in e.OldItems!)
                {
                    RemoveInternal(component);
                }
                break;
        }
    }

    /// <summary>
    /// Dedicated Debugger for an entity that displays children from Entity.Transform.Children
    /// </summary>
    internal class EntityDebugView(Entity entity)
    {
        private readonly Entity _entity = entity;

        public string Name => _entity.Name;

        public Guid Id => _entity.Id;

        public Entity? Parent => _entity.Transform.Parent?.Entity;

        public Entity[] Children => [.. _entity.Transform.Children.Select(x => x.Entity!)];

        public Component[] Components => [.. _entity.Components];
    }
}
