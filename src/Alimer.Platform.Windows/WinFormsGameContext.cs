// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Windows.Media;
using Microsoft.Extensions.DependencyInjection;

namespace Alimer;

public class WinFormsGameContext : GameContext
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

    public override void RunMainLoop(Action init, Action callback)
    {
        init();

        CompositionTarget.Rendering += (s, e) => callback();
    }
}
