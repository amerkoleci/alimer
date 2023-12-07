// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.Serialization;
using System.Text.Json.Serialization;

namespace Alimer.Engine;

[DataContract]
[DebuggerTypeProxy(typeof(EntityDebugView))]
public sealed partial class Entity : IEnumerable<EntityComponent>, INotifyPropertyChanged
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

    public event PropertyChangedEventHandler? PropertyChanged;

    [DataMember]
    [JsonPropertyOrder(0)]
    [Browsable(false)]
    public Guid Id { get; set; }

    [DataMember]
    [JsonPropertyOrder(10)]
    public string Name
    {
        get => _name;
        set => Set(ref _name, value);
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

    public IEnumerator<EntityComponent> GetEnumerator() => Components.GetEnumerator();

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

    public void Add(EntityComponent component)
    {
        Components.Add(component);
    }

    public T Add<T>() where T : EntityComponent, new()
    {
        T component = new();
        Components.Add(component);
        return component;
    }

    public T? Get<T>() where T : EntityComponent?
    {
        return this.OfType<T>().FirstOrDefault();
    }

    public T GetOrCreate<T>() where T : EntityComponent, new()
    {
        T? component = Get<T>();

        if (component is null)
        {
            component = new T();
            Components.Add(component);
        }

        return component;
    }

    public bool Remove<T>() where T : EntityComponent
    {
        T? component = Get<T>();

        if (component != null)
        {
            return Components.Remove(component);
        }

        return false;
    }

    public bool Remove(EntityComponent component)
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

    private void AddInternal(EntityComponent component)
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

    private void RemoveInternal(EntityComponent component)
    {
        if (component.Entity != this)
        {
            throw new InvalidOperationException("This entity component is not set on this entity.");
        }

        if (component is TransformComponent)
        {
            if (Get<TransformComponent?>() is null)
            {
                throw new InvalidOperationException("An entity always has to have a transform component.");
            }
        }

        component.Entity = null;
    }

    private void OnChildrenCollectionChanged(object? sender, NotifyCollectionChangedEventArgs e)
    {
        switch (e.Action)
        {
            case NotifyCollectionChangedAction.Add:
                foreach (Entity entity in e.NewItems!.Cast<Entity>())
                {
                    AddInternal(entity);
                }
                break;
            case NotifyCollectionChangedAction.Remove:
                foreach (Entity entity in e.OldItems!.Cast<Entity>())
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
                foreach (EntityComponent component in e.NewItems!.Cast<EntityComponent>())
                {
                    AddInternal(component);
                }
                break;
            case NotifyCollectionChangedAction.Remove:
                foreach (EntityComponent component in e.OldItems!.Cast<EntityComponent>())
                {
                    RemoveInternal(component);
                }
                break;
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

    /// <summary>
    /// Dedicated Debugger for an entity that displays children from Entity.Transform.Children
    /// </summary>
    internal class EntityDebugView(Entity entity)
    {
        private readonly Entity _entity = entity;

        public string Name => _entity.Name;

        public Guid Id => _entity.Id;

        public Entity? Parent => _entity.Transform.Parent?.Entity;

        public Entity[] Children => _entity.Transform.Children.Select(x => x.Entity!).ToArray();

        public EntityComponent[] Components => _entity.Components.ToArray();
    }
}
