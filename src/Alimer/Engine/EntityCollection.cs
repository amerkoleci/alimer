// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.ObjectModel;

namespace Alimer.Engine;

public sealed class EntityCollection : ObservableCollection<Entity>
{
    protected override void InsertItem(int index, Entity item)
    {
        if (Contains(item))
            return;

        base.InsertItem(index, item);
    }
}
