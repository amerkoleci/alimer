// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using Vortice.Graphics;

namespace Vortice
{
    internal class WinFormsGameView : GameView
    {
        private readonly Control _control;

        public WinFormsGameView(Control control)
        {
            _control = control;

            Source = SwapChainSource.CreateWin32(
                Marshal.GetHINSTANCE(Assembly.GetEntryAssembly()!.Modules.First()),
                _control.Handle
                );

            _control.ClientSizeChanged += OnControlClientSizeChanged;
        }

        /// <inheritdoc />
        public override SizeF ClientSize => _control.ClientSize;

        /// <inheritdoc />
        public override SwapChainSource Source { get; }

        private void OnControlClientSizeChanged(object? sender, EventArgs e)
        {
            OnSizeChanged();
        }
    }
} 
