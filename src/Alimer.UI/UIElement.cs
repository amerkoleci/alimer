// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Numerics;
using static Alimer.Numerics.MathUtilities;

namespace Alimer.UI;

public abstract partial class UIElement
{
    private readonly List<UIElement> _children = [];

    /// <summary>
    /// Gets the parent 
    /// </summary>
	public UIElement? Parent
    {
        get;
        private set;
    }

    /// Children of this element.
	public IReadOnlyList<UIElement> Children => _children;

    /// <summary>
    /// Adds the specified UIElement as a child of this element.
    /// </summary>
    /// <remarks>If the specified child is already a child of another parent, it is automatically removed from
    /// that parent before being added to this element. After the child is added, the layout is invalidated to ensure
    /// the visual tree is updated.</remarks>
    /// <param name="child">The UIElement to add as a child. Cannot be null. If the element already has a parent, it is removed from its
    /// current parent before being added.</param>
	public void AddChild(UIElement child)
    {
        child.Parent?.RemoveChild(child);

        child.Parent = this;
        _children.Add(child);
        InvalidateMeasure();
    }

    /// <summary>
    /// Removes the specified child element from the collection of child elements.
    /// </summary>
    /// <remarks>If the child element is removed, its parent is set to null and the layout is invalidated. If
    /// the specified element is not a child, no action is taken.</remarks>
    /// <param name="child">The child <see cref="UIElement"/> to remove from the collection. Cannot be null.</param>
    /// <returns>true if the child element was successfully removed; otherwise, false.</returns>
	public bool RemoveChild(UIElement child)
    {
        if (_children.Remove(child))
        {
            child.Parent = null;
            InvalidateMeasure();
            return true;
        }

        return false;
    }
}
