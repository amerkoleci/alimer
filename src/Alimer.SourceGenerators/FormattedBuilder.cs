// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text;

namespace Alimer.SourceGenerators;

internal class FormattedBuilder(StringBuilder builder)
{
    private int _indentation;

    public FormattedBuilder EndIndentation(int count = 1)
    {
        _indentation -= (count * 4);
        return this;
    }

    public FormattedBuilder BeginIndentation(int count = 1)
    {
        _indentation += (count * 4);
        return this;
    }

    public FormattedBuilder AppendLine(string? line = null)
    {
        if (_indentation > 0)
        {
            builder.Append(' ', _indentation);
        }
        builder.AppendLine(line);
        return this;
    }

    public override string ToString() => builder.ToString();
}
