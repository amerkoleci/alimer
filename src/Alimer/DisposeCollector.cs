// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;

namespace Alimer;

/// <summary>
/// A class to dispose <see cref="IDisposable"/> instances.
/// </summary>
public class DisposeCollector : DisposableObject
{
    private List<IDisposable>? _disposables;

    /// <summary>
    /// Gets the number of elements to dispose.
    /// </summary>
    /// <value>The number of elements to dispose.</value>
    public int Count => _disposables?.Count ?? 0;

    protected override void Dispose(bool disposing)
    {
        DisposeAndClear(disposing);
        _disposables = null;
    }

    /// <summary>
    /// Disposes all object collected by this class and clear the list. The collector can still be used for collecting.
    /// </summary>
    /// <param name="disposeManagedResources">If true, managed resources should be
    /// disposed of in addition to unmanaged resources.</param>
    /// <remarks>
    /// To completely dispose this instance and avoid further dispose, use <see cref="Dispose"/> method instead.
    /// </remarks>
    public void DisposeAndClear(bool disposeManagedResources = true)
    {
        if (_disposables == null)
            return;

        for (int i = _disposables.Count - 1; i >= 0; i--)
        {
            IDisposable disposable = _disposables[i];
            if (disposeManagedResources)
            {
                disposable.Dispose();
            }

            _disposables.RemoveAt(i);
        }
        _disposables.Clear();
    }

    /// <summary>
    /// Adds a <see cref="IDisposable"/> object or a <see cref="IntPtr"/> allocated using <see cref="Utilities.AllocateMemory"/> to the list of the objects to dispose.
    /// </summary>
    /// <param name="toDispose">To dispose.</param>
    /// <exception cref="ArgumentException">If toDispose argument is not IDisposable or a valid memory pointer allocated by <see cref="Utilities.AllocateMemory"/></exception>
    public T Collect<T>([NotNull] T toDispose) where T : IDisposable
    {
        ArgumentNullException.ThrowIfNull(toDispose, nameof(toDispose));

        _disposables ??= new List<IDisposable>();

        if (!_disposables.Contains(toDispose))
        {
            _disposables.Add(toDispose);
        }

        return toDispose;
    }

    /// <summary>
    /// Dispose a disposable object and set the reference to null. Removes this object from this instance..
    /// </summary>
    /// <param name="disposable">Object to dispose.</param>
    public void RemoveAndDispose<T>(ref T? disposable) where T : IDisposable
    {
        if (_disposables == null || disposable == null)
            return;

        Remove(disposable);

        // Dispose the component
        disposable.Dispose();
        disposable = default;
    }

    /// <summary>
    /// Removes a disposable object to the list of the objects to dispose.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="disposable">To dispose.</param>
    public void Remove<T>(T disposable) where T : IDisposable
    {
        if (_disposables != null &&
            _disposables.Contains(disposable))
        {
            _disposables.Remove(disposable);
        }
    }
}
