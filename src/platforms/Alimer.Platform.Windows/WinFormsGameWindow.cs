// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Drawing;
using System.Windows.Forms;
using System.Windows.Media;

namespace Alimer
{
    /// <summary>
    /// Defines a for <see cref="Game"/> window created using Windows Forms.
    /// </summary>
    internal class WinFormsGameWindow : GameWindow
    {
        private readonly Control _control;

        /// <inheritdoc/>
        public override string Title 
        {
            get => _control.Text;
            set => _control.Text = value;
        }

        /// <inheritdoc/>
        public override RectangleF ClientBounds => _control.ClientRectangle;

        public WinFormsGameWindow(Control control)
        {
            //Guard.NotNull(control, nameof(control));

            _control = control;
            _control.ClientSizeChanged += Control_ClientSizeChanged;

            //var hInstance = Win32Native.GetModuleHandle(null);
            //ConfigureSwapChain(SwapChainHandle.CreateWin32(hInstance, _control.Handle));
        }

        private void Control_ClientSizeChanged(object sender, EventArgs e)
        {
            OnSizeChanged();
        }
    }
}
