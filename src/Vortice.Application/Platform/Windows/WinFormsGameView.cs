// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Windows.Forms;

namespace Vortice
{
    internal class WinFormsGameView : GameView
    {
        private readonly Control _control;

        public WinFormsGameView(Control control)
        {
            _control = control;

            control.ClientSizeChanged += OnControlClientSizeChanged;
        }

        private void OnControlClientSizeChanged(object? sender, EventArgs e)
        {
            OnSizeChanged();
        }
    }
} 
