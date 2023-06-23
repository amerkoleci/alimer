// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Input;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;

namespace Alimer;

internal unsafe class WindowsPlatform : GamePlatform
{
    private readonly WindowsInput _input;
    private readonly SwapChainPanelView _window;
    private bool _exitRequested;

    public WindowsPlatform(SwapChainPanel? swapChainPanel = default)
    {
        _input = new WindowsInput(this);
        MainView = (_window = new SwapChainPanelView(this, swapChainPanel));
    }

    // <inheritdoc />
    public override InputManager Input => _input;

    // <inheritdoc />
    public override bool SupportsMultipleViews => false;

    // <inheritdoc />
    public override GameView MainView { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="WindowsPlatform" /> class.
    /// </summary>
    ~WindowsPlatform() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
        }
    }

    /// <inheritdoc />
    public override void RunMainLoop()
    {
        OnReady();

        _window.Show();

        CompositionTarget.Rendering += OnCompositionTargetRendering;
    }

    private void OnCompositionTargetRendering(object? sender, object e)
    {
        OnTick();
    }
}
