// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.
// This is modified version of Gemini framework, see https://github.com/tgjones/gemini/blob/master/LICENCE.txt

using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Windows;
using Gemini.Framework;

namespace Gemini.Modules.ToolBars
{
    [Export(typeof(IModule))]
    public class Module : ModuleBase
    {
        public override IEnumerable<ResourceDictionary> GlobalResourceDictionaries
        {
            get
            {
                yield return new ResourceDictionary
                {
                    Source = new Uri("/Alimer.Gemini;component/Modules/ToolBars/Resources/Styles.xaml", UriKind.Relative)
                };
            }
        }
    }
}
