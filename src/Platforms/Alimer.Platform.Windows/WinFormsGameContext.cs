// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Windows.Media;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Toolkit.Diagnostics;

namespace Alimer;

public class WinFormsGameContext : GameContext
{
    public WinFormsGameContext(Control control)
    {
        Guard.IsNotNull(control, name: nameof(control));

        View = new WinFormsGameView(control);
    }

    // <inheritdoc />
    public override GameView View { get; }

    public override void ConfigureServices(IServiceCollection services)
    {
        base.ConfigureServices(services);

        //services.AddSingleton<GraphicsPresenter>(new HwndSwapChainGraphicsPresenter(GraphicsDevice, PresentationParameters, Control.Handle));
        //services.AddSingleton<IInputSourceConfiguration>(new WinFormsInputSourceConfiguration(Control));
    }

    public override void RunMainLoop(Action init, Action callback)
    {
        init();

        CompositionTarget.Rendering += (s, e) => callback();
    }
}
