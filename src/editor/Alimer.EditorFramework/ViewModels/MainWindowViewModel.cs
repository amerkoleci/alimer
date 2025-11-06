// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Editor.ViewModels;

public partial class MainWindowViewModel : MainViewModel, IMainWindow
{
    private double _width = 1280;
    public double Width
    {
        get => _width;
        set => SetProperty(ref _width, value);
    }

    private double _height = 900;
    public double Height
    {
        get => _height;
        set => SetProperty(ref _height, value);
    }
}
