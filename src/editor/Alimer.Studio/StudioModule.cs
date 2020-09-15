// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.ComponentModel.Composition;
using Gemini.Framework;

namespace Alimer.Studio
{
    // https://github.com/tgjones/gemini/blob/master/src/Gemini.Demo/Modules/Startup/Module.cs

    [Export(typeof(IModule))]
    public class StudioModule : ModuleBase
    {
        public override void Initialize()
        {
            MainWindow.Title = "Alimer Studio";
        }
    }
}
