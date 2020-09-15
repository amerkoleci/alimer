// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.
// This is modified version of Gemini framework, see https://github.com/tgjones/gemini/blob/master/LICENCE.txt

using System;
using Gemini.Framework;
using Gemini.Framework.Commands;
using Gemini.Properties;

namespace Gemini.Modules.Toolbox.Commands
{
    [CommandDefinition]
    public class ViewToolboxCommandDefinition : CommandDefinition
    {
        public const string CommandName = "View.Toolbox";

        public override string Name => CommandName;
        public override string Text => Resources.ViewToolboxCommandText;
        public override string ToolTip => Resources.ViewToolboxCommandToolTip;
        public override Uri IconSource => Utilities.PackUri("Resources/Icons/Toolbox.png");
    }
}
