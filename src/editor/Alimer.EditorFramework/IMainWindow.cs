// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Editor;

public interface IMainWindow : IMainView
{
    double Width { get; set; }
    double Height { get; set; }
}
