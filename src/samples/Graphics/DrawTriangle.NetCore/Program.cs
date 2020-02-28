// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Windows.Forms;
using Alimer;

namespace DrawTriangle
{
    public static class Program
    {
        public class MyForm : Form
        {
            public MyForm()
            {
                Text = "Alimer - Hello Triangle";
                Width = 1200;
                Height = 800;
            }

            protected override void OnLoad(EventArgs e)
            {
                base.OnLoad(e);

                using (var game = new DrawTriangleGame(new WinFormsGameContext(this)))
                {
                    game.Run();
                }
            }
        }

        [STAThread]
        public static void Main()
        {
            Application.SetHighDpiMode(HighDpiMode.SystemAware);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MyForm());
        }
    }
}
