// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;

namespace Vortice
{
    public abstract class Application : IDisposable
    {
        private readonly AppContext _context;

        protected Application()
        {
            _context = AppContext.Create(this);
        }

        public void Dispose()
        {

        }
    }
}
