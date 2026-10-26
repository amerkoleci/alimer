// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Engine;

public abstract partial class SyncScript : StartupScript
{
    public abstract void Update(GameTime time);
}
