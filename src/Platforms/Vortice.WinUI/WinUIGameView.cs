// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Drawing;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using Vortice.Graphics;

namespace Vortice
{
    internal class WinUIGameView : GameView
    {
        private readonly SwapChainPanel _panel;

        public WinUIGameView(SwapChainPanel panel)
        {
            _panel = panel;
            _panel.SizeChanged += OnSwapChainPanelSizeChanged;
            _panel.CompositionScaleChanged += OnSwapChainPanelCompositionScaleChanged;

            //Source = SwapChainSource.CreateWin32(
            //    Marshal.GetHINSTANCE(Assembly.GetEntryAssembly()!.Modules.First()),
            //    _panel.Handle
            //    );
        }

        /// <inheritdoc />
        public override SizeF ClientSize
        {
            get
            {
                return new(
                    Math.Max(1.0f, (float)_panel.ActualWidth * _panel.CompositionScaleX + 0.5f),
                    Math.Max(1.0f, (float)_panel.ActualHeight * _panel.CompositionScaleY + 0.5f)
                    );
            }
        }

        /// <inheritdoc />
        public override SwapChainSource Source { get; }

        private void OnSwapChainPanelSizeChanged(object sender, SizeChangedEventArgs e)
        {
            OnSizeChanged();
        }

        private void OnSwapChainPanelCompositionScaleChanged(SwapChainPanel sender, object e)
        {
            OnSizeChanged();
        }
    }
} 
