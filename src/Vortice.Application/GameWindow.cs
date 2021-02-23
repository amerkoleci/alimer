// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Drawing;

namespace Vortice
{
    /// <summary>
    /// Defines a <see cref="Application"/> window.
    /// </summary>
    public abstract class GameWindow : IDisposable
    {
        public event EventHandler? SizeChanged;

        public bool IsExiting { get; private set; }

        public abstract IntPtr Handle { get; }

        /// <summary>
        /// Gets the screen dimensions of the game window's client rectangle.
        /// </summary>
        public abstract RectangleF ClientBounds { get; }

        /// <summary>
        /// Gets and sets the title of the window.
        /// </summary>
        public abstract string Title { get; set; }

        public virtual void Dispose()
        {
        }

        public void Exit()
        {
            IsExiting = true;
            Dispose();
        }

        /// <summary>
        /// Occurs when window size changed.
        /// </summary>
        protected virtual void OnSizeChanged()
        {
            SizeChanged?.Invoke(this, EventArgs.Empty);
        }
    }
}
