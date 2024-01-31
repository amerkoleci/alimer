// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Engine;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;
using Hexa.NET.ImGui;

namespace Alimer;

public sealed class ImGuiSystem : GameSystem
{
    private ImGuiContextPtr _context;

    public ImGuiSystem(IServiceRegistry services,
        ImGuiConfigFlags flags = ImGuiConfigFlags.NavEnableKeyboard | ImGuiConfigFlags.NavEnableGamepad | ImGuiConfigFlags.DockingEnable/* | ImGuiConfigFlags.ViewportsEnable*/)
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));

        GraphicsDevice = services.GetService<GraphicsDevice>();
        MainWindow = services.GetService<Window>();

        _context = ImGui.CreateContext();
        ImGui.SetCurrentContext(_context);

        ImGuiIOPtr io = ImGui.GetIO();
        //ImGui.GetIO().IniFilename = null;
        io.ConfigFlags |= flags;
        io.ConfigViewportsNoDecoration = false;
        io.ConfigDockingTransparentPayload = true;
        io.ConfigViewportsNoAutoMerge = false;
        io.ConfigViewportsNoTaskBarIcon = false;

        io.BackendFlags |= ImGuiBackendFlags.RendererHasVtxOffset;

        ImGui.StyleColorsDark();
        ImGuiStylePtr style = ImGui.GetStyle();
        style.GrabRounding = 4.0f;
        if ((io.ConfigFlags & ImGuiConfigFlags.ViewportsEnable) != 0)
        {
            style.WindowRounding = 0.0f;
            style.Colors[(int)ImGuiCol.WindowBg].W = 1.0f;
        }

        io.Fonts.AddFontDefault();
        io.Fonts.Flags |= ImFontAtlasFlags.NoBakedLines;
    }

    public GraphicsDevice GraphicsDevice { get; }
    public Window MainWindow { get; }
}
