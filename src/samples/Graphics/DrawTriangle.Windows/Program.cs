// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Windows.Forms;
using Vortice;
using Vortice.Graphics;

namespace DrawTriangle
{
    public class MainForm : Form
    {
        public MainForm()
        {
            Width = 1200;
            Height = 800;

            Load += OnMainFormLoad;
        }

        private void OnMainFormLoad(object? sender, EventArgs e)
        {
#if DEBUG
            GraphicsDevice.EnableValidationLayers = true;
#endif

            //StorageFolder rootFolder = await StorageFolder.GetFolderFromPathAsync(Directory.GetCurrentDirectory());

            var game = new DrawTriangleGame(new WinFormsGameContext(this));
            game.Run();
        }
    }


    public static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        private static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }
    }
}
