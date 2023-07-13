// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.ObjectModel;
using System.Linq;

namespace Alimer.Engine;

public class EntitySystemCollection : Collection<EntitySystem>
{
    public T? Get<T>() where T : EntitySystem
    {
        return this.OfType<T>().FirstOrDefault();
    }
}
