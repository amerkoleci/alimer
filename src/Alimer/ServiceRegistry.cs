// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer;

/// <summary>
/// Provides a base implementation for managing services within an application. Implements the <see cref="IServiceRegistry"/> interface.
/// </summary>
/// <remarks>
/// This class uses a dictionary to store services by their types. It is thread-safe.
/// </remarks>
public class ServiceRegistry : IServiceRegistry
{
    private readonly Dictionary<Type, object> _registeredService = new();

    /// <inheritdoc />
    public event EventHandler<ServiceEventArgs>? ServiceAdded;

    /// <inheritdoc />
    public event EventHandler<ServiceEventArgs>? ServiceRemoved;

    /// <inheritdoc />
    public void AddService<T>([NotNull] T service) where T : class
    {
        ArgumentNullException.ThrowIfNull(service, nameof(service));

        Type type = typeof(T);
        lock (_registeredService)
        {
            if (_registeredService.ContainsKey(type))
                throw new ArgumentException("Service is already registered with this type", nameof(type));

            _registeredService.Add(type, service);
        }

        OnServiceAdded(new ServiceEventArgs(type, service));
    }

    public void AddService<T>() where T : class, new()
    {
        AddService(() => new T());
    }

    public void AddService<T>(Func<T> factory)
    {
        Type type = typeof(T);
        T service = factory();
        ArgumentNullException.ThrowIfNull(service, nameof(service));

        lock (_registeredService)
        {
            if (_registeredService.ContainsKey(type))
                throw new ArgumentException("Service is already registered with this type", nameof(type));

            _registeredService.Add(type, service);
        }

        OnServiceAdded(new ServiceEventArgs(type, service));
    }

    /// <inheritdoc />
    public void RemoveService<T>() where T : class
    {
        Type type = typeof(T);
        object? oldService = default;
        lock (_registeredService)
        {
            if (_registeredService.TryGetValue(type, out oldService))
                _registeredService.Remove(type);
        }

        if (oldService != null)
            OnServiceRemoved(new ServiceEventArgs(type, oldService));
    }

    /// <inheritdoc />
    public T? TryGetService<T>() where T : class
    {
        Type type = typeof(T);
        lock (_registeredService)
        {
            if (_registeredService.TryGetValue(type, out object? service))
            {
                return (T)service;
            }
        }

        return null;
    }

    /// <inheritdoc />
    public T GetService<T>() where T : class
    {
        Type type = typeof(T);
        lock (_registeredService)
        {
            if (!_registeredService.TryGetValue(type, out object? service))
            {
                throw new ArgumentException("No Service registered with this type", nameof(type));
            }

            return (T)service;
        }
    }

    protected virtual void OnServiceAdded(ServiceEventArgs e)
    {
        ServiceAdded?.Invoke(this, e);
    }

    protected virtual void OnServiceRemoved(ServiceEventArgs e)
    {
        ServiceRemoved?.Invoke(this, e);
    }
}
