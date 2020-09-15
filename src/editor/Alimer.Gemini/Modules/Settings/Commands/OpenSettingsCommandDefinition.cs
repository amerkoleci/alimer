// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.
// This is modified version of Gemini framework, see https://github.com/tgjones/gemini/blob/master/LICENCE.txt

using System;
using Gemini.Framework;
using Gemini.Framework.Commands;
using Gemini.Properties;

namespace Gemini.Modules.Settings.Commands
{
    [CommandDefinition]
    public class OpenSettingsCommandDefinition : CommandDefinition
    {
        public const string CommandName = "Tools.Options";

        public override string Name
        {
            get { return CommandName; }
        }

        public override string Text
        {
            get { return Resources.ToolsOptionsCommandText; }
        }

        public override string ToolTip
        {
            get { return Resources.ToolsOptionsCommandToolTip; }
        }

        public override Uri IconSource => Utilities.PackUri("Resources/Icons/Settings.png");
    }
}
