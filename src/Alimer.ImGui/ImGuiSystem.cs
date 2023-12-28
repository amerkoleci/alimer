// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Engine;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;
using ImGuiNET;

namespace Alimer;

public sealed class ImGuiSystem : GameSystem
{
    public ImGuiSystem(GraphicsDevice device)
    {
        Guard.IsNotNull(device, nameof(device));

        Device = device;

        IntPtr context = ImGui.CreateContext();
        ImGui.SetCurrentContext(context);

        ImGuiIOPtr io = ImGui.GetIO();
        //ImGui.GetIO().IniFilename = null;
        io.ConfigViewportsNoDecoration = false;
        io.ConfigDockingTransparentPayload = true;
        io.ConfigFlags |= ImGuiConfigFlags.NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags.NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags.DockingEnable;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;
        //io.BackendRendererName = "Alimer";
        io.BackendFlags |= ImGuiBackendFlags.RendererHasVtxOffset;

        ImGui.StyleColorsDark();
        ImGuiStylePtr style = ImGui.GetStyle();
        style.GrabRounding = 4.0f;
        if ((io.ConfigFlags & ImGuiConfigFlags.ViewportsEnable) != 0)
        {
            style.WindowRounding = 0.0f;
            style.Colors[(int)ImGuiCol.WindowBg].W = 1.0f;
        }


        ImGui.GetIO().Fonts.AddFontDefault();
        ImGui.GetIO().Fonts.Flags |= ImFontAtlasFlags.NoBakedLines;
    }

    public GraphicsDevice Device { get; }
}
