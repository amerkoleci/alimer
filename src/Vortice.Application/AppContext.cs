// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;

namespace Vortice
{
    internal partial class AppContext
    {
        protected AppContext(Application application)
        {
            Application = application;
        }

        public Application Application { get; }
    }
}
