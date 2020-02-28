// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Windows.Forms;
using Microsoft.Extensions.DependencyInjection;
using static Alimer.Win32Native;

namespace Alimer
{
    /// <summary>
    /// Defines a context for <see cref="Game"/> that handles logic using Windows Forms.
    /// </summary>
    public class WinFormsGameContext : GameContext
    {
        private bool _paused = false;
        //private readonly GraphicsDeviceFactory _graphicsDeviceFactory = new D3D11GraphicsDeviceFactory(false);

        public Control Control { get; protected set; }

        /// <inheritdoc/>
        public override GameWindow GameWindow { get; }

        public WinFormsGameContext(Control control)
        {
            //Guard.NotNull(control, nameof(control));

            Control = control;
            GameWindow = new WinFormsGameWindow(Control);
        }

        public override void ConfigureServices(IServiceCollection services)
        {
            base.ConfigureServices(services);

            services.AddSingleton<IRuntimePlatform, WindowsRuntimePlatform>();
            //services.AddSingleton(_graphicsDeviceFactory.CreateDevice());
        }

        public override bool Run(Action loadAction, Action tickAction)
        {
            loadAction();

            // Show main window.
            Control.Show();

            while (!GameWindow.IsExiting)
            {
                if (!_paused)
                {
                    const int PM_REMOVE = 1;
                    if (PeekMessage(out var msg, IntPtr.Zero, 0, 0, PM_REMOVE))
                    {
                        TranslateMessage(ref msg);
                        DispatchMessage(ref msg);

                        if (msg.Value == (uint)WindowMessage.Quit)
                        {
                            GameWindow.Exit();
                            break;
                        }
                    }

                    tickAction();
                }
                else
                {
                    var ret = GetMessage(out var msg, IntPtr.Zero, 0, 0);
                    if (ret == 0)
                    {
                        GameWindow.Exit();
                        break;
                    }
                    else if (ret == -1)
                    {
                        //Log.Error("[Win32] - Failed to get message");
                        GameWindow.Exit();
                        break;
                    }
                    else
                    {
                        TranslateMessage(ref msg);
                        DispatchMessage(ref msg);
                    }
                }
            }

            return true;
        }
    }
}
