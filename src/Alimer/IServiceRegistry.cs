// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer;

/// <summary>
/// A service registry is a <see cref="IServiceProvider"/> that provides methods to register and unregister services.
/// </summary>
public interface IServiceRegistry
{
    /// <summary>
    /// Occurs when a new service is added to the registry.
    /// </summary>
    event EventHandler<ServiceEventArgs>? ServiceAdded;

    /// <summary>
    /// Occurs when a service is removed from the registry.
    /// </summary>
    event EventHandler<ServiceEventArgs>? ServiceRemoved;

    /// <summary>
    /// Adds a service to this <see cref="ServiceRegistry"/>.
    /// </summary>
    /// <typeparam name="T">The type of service to add.</typeparam>
    /// <param name="service">The service to add.</param>
    /// <exception cref="ArgumentNullException">Thrown when the provided service is null.</exception>
    /// <exception cref="ArgumentException">Thrown when a service of the same type is already registered.</exception>
    /// <remarks>
    /// This implementation triggers the <see cref="ServiceAdded"/> event after a service is successfully added.
    /// </remarks>
    void AddService<T>([NotNull] T service) where T : class;

    /// <summary>
    /// Removes the object providing a specified service.
    /// </summary>
    /// <typeparam name="T">The type of the service to remove.</typeparam>
    void RemoveService<T>() where T : class;

    /// <summary>
    /// Try to get the service object of the specified type.
    /// </summary>
    /// <typeparam name="T">The type of the service to retrieve.</typeparam>
    /// <returns>A service of the requested type, or [null] if not found.</returns>
    //[CanBeNull]
    T? TryGetService<T>() where T : class;

    /// <summary>
    /// Gets the service object of the specified type.
    /// </summary>
    /// <typeparam name="T">The type of the service to retrieve.</typeparam>
    /// <returns>A service of the requested type, or throws exception if not found.</returns>
    /// <exception cref="ArgumentException">Thrown when the requested service is null.</exception>
    //[CanBeNull]
    T GetService<T>() where T : class;
}
