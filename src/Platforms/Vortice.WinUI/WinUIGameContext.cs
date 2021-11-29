// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;

namespace Vortice
{
    public class WinUIGameContext : GameContext
    {
        public WinUIGameContext(SwapChainPanel control)
        {
            Control = control;
        }

        public SwapChainPanel Control { get; }
            
        public override void ConfigureServices(IServiceCollection services)
        {
            base.ConfigureServices(services);

            services.AddSingleton<GameView>(new WinUIGameView(Control));
            //services.AddSingleton<GraphicsPresenter>(new HwndSwapChainGraphicsPresenter(GraphicsDevice, PresentationParameters, Control.Handle));
            //services.AddSingleton<IInputSourceConfiguration>(new WinFormsInputSourceConfiguration(Control));
        }

        public override void RunMainLoop(Action init, Action callback)
        {
            init();

            CompositionTarget.Rendering += (s, e) => callback();
        }
    }
}
