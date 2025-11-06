// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;

namespace Alimer.Editor.ViewModels;

public partial class MainViewModel : ViewModelBase, IMainView
{
    private static readonly string s_baseTitle = $"Alimer Editor ({RuntimeInformation.FrameworkDescription})";
    private string _title = s_baseTitle;

    public string Title
    {
        get => _title;
        set => SetProperty(ref _title, value);
    }

    public string Greeting => "Welcome to Avalonia!";
}
