// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Windows.Forms;
using System.Windows.Media;
using Microsoft.Extensions.DependencyInjection;

namespace Vortice
{
    public class WinFormsGameContext : GameContextWithGraphics
    {
        public WinFormsGameContext(Control control)
        {
            Control = control;
        }

        public Control Control { get; }

        public override void ConfigureServices(IServiceCollection services)
        {
            base.ConfigureServices(services);

            services.AddSingleton<GameView>(new WinFormsGameView(Control));
            //services.AddSingleton<GraphicsPresenter>(new HwndSwapChainGraphicsPresenter(GraphicsDevice, PresentationParameters, Control.Handle));
            //services.AddSingleton<IInputSourceConfiguration>(new WinFormsInputSourceConfiguration(Control));
        }

        public override void RunMainLoop(Action callback)
        {
            CompositionTarget.Rendering += (s, e) => callback();
        }
    }
}
