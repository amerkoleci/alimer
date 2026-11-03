// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text;

namespace Alimer.SourceGenerators;

internal class FormattedBuilder
{
    private bool _shouldIndent = true;
    private readonly string[] _indentStrings;
    private string _indentString = string.Empty;
    private readonly StringBuilder _builder;

    public FormattedBuilder()
        : this(new StringBuilder())
    {

    }

    public FormattedBuilder(StringBuilder builder)
    {
        _builder = builder;

        _indentStrings = new string[10];
        for (int i = 0; i < _indentStrings.Length; i++)
        {
            _indentStrings[i] = new string('\t', i);
        }

    }

    public int IndentLevel { get; private set; }

    public FormattedBuilder Append(char chr)
    {
        AppendIndented(chr);
        return this;
    }

    public FormattedBuilder Append(string @string)
    {
        AppendIndented(@string);
        return this;
    }

    public FormattedBuilder AppendUndindented(string @string)
    {
        _builder.Append(@string);
        return this;
    }

    public FormattedBuilder AppendLine()
    {
        _builder.AppendLine();
        _shouldIndent = true;
        return this;
    }

    public FormattedBuilder AppendLine(string @string)
    {
        AppendIndented(@string);
        _builder.AppendLine();
        _shouldIndent = true;
        return this;
    }

    public FormattedBuilder AppendLineUndindented(string @string)
    {
        _builder.AppendLine(@string);
        _shouldIndent = true;
        return this;
    }

    public FormattedBuilder BeginBlock(string content)
    {
        AppendLine(content);
        AppendLine("{");
        Indent(1);
        return this;
    }

    public FormattedBuilder EndBlock()
    {
        Dedent(1);
        AppendLine("}");
        return this;
    }

    public IDisposable PushBlock(string marker = "{") => new CodeBlock(this, marker);

    public FormattedBuilder Indent(int count = 1)
    {
        IndentLevel += count;

        if (IndentLevel < _indentStrings.Length)
        {
            _indentString = _indentStrings[IndentLevel];
        }
        else
        {
            _indentString = new string('\t', IndentLevel);
        }
        return this;
    }

    public FormattedBuilder Dedent(int count = 1)
    {
        if (count > IndentLevel)
            throw new ArgumentException("count out of range.", nameof(count));

        IndentLevel -= count;
        if (IndentLevel < _indentStrings.Length)
        {
            _indentString = _indentStrings[IndentLevel];
        }
        else
        {
            _indentString = new string('\t', IndentLevel);
        }
        return this;
    }

    private void AppendIndented(char chr)
    {
        if (_shouldIndent)
        {
            _builder.Append(_indentString);
            _shouldIndent = false;
        }

        _builder.Append(chr);
    }

    private void AppendIndented(string @string)
    {
        if (_shouldIndent)
        {
            _builder.Append(_indentString);
            _shouldIndent = false;
        }

        _builder.Append(@string);
    }

    struct CodeBlock : IDisposable
    {
        private readonly FormattedBuilder _builder;

        public CodeBlock(FormattedBuilder builder, string content)
        {
            _builder = builder;
            _builder.BeginBlock(content);
        }

        public void Dispose()
        {
            _builder.EndBlock();
        }
    }

    public override string ToString() => _builder.ToString();
}
