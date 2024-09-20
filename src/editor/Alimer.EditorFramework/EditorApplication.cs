// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.


using Alimer.Engine;
using Alimer.Graphics;
using Hexa.NET.ImGui;
using Hexa.NET.ImGuizmo;
using Hexa.NET.ImNodes;
using Hexa.NET.ImPlot;

namespace Alimer.Editor;

public class EditorApplication : GameApplication
{
    private readonly ImGuiSystem _imGui;
    private readonly ImNodesContextPtr _nodesContext;
    private readonly ImPlotContextPtr _plotContext;

    public unsafe EditorApplication(GraphicsBackendType preferredGraphicsBackend = GraphicsBackendType.Count)
        : base(preferredGraphicsBackend)
    {
        _imGui = new ImGuiSystem(Services);

        ImGui.SetCurrentContext(_imGui.Context);
        ImGuizmo.SetImGuiContext(_imGui.Context);
        ImGuizmo.AllowAxisFlip(false);
        ImPlot.SetImGuiContext(_imGui.Context);
        ImNodes.SetImGuiContext(_imGui.Context);

        _nodesContext = ImNodes.CreateContext();
        ImNodes.SetCurrentContext(_nodesContext);
        ImNodes.StyleColorsDark(ImNodes.GetStyle());

        _plotContext = ImPlot.CreateContext();
        ImPlot.SetCurrentContext(_plotContext);
        ImPlot.StyleColorsDark(ImPlot.GetStyle());

        GameSystems.Add(_imGui);
    }

    protected override void Initialize()
    {
        base.Initialize();

        MainWindow.Title = "Alimer Editor";
#if DEBUG
        MainWindow.Title += " - Debug";
#endif
    }

    protected override void Dispose(bool disposing)
    {
        base.Dispose(disposing);
    }

    protected override void Draw(RenderContext renderContext, Texture outputTexture, AppTime time)
    {
        base.Draw(renderContext, outputTexture, time);
    }
}
