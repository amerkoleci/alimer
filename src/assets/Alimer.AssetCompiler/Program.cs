// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Assets.Compiler;
using Alimer.Graphics;
using XenoAtom.CommandLine;

namespace Alimer.Shaders;

public partial class AssetCompilerApp
{
    private readonly Stack<AssetCompilerContext> _contexts = new();
    private readonly Dictionary<string, DateTime> _includeFilesLastWriteTime = new(OperatingSystem.IsWindows() ? StringComparer.OrdinalIgnoreCase : StringComparer.Ordinal);

    /// <summary>
    /// Gets or sets the exception factory for the command line.
    /// </summary>
    public Func<string, Exception> GetCommandException { get; set; } = message => new InvalidOperationException(message);

    /// <summary>
    /// Gets or sets the exception factory for the options of the command line.
    /// </summary>
    public Func<string, string, Exception> GetOptionException { get; set; } = (message, paramName) => new ArgumentException(message, paramName);

    /// <summary>
    /// Gets or sets the maximum thread count used for compilation.
    /// </summary>
    public int? MaxThreadCount { get; set; }

    public PlatformID TargetPlatform { get; set; } = PlatformID.Windows;

    /// <summary>
    /// Gets the list of input directories.
    /// </summary>
    public List<string> InputDirectories { get; } = [];

    public string? OutputDirectory { get; set; }

    /// <summary>
    /// Runs the shader compiler.
    /// </summary>
    /// <param name="output">The standard output to generate log/errors to.</param>
    /// <returns>Zero for success; otherwise a non 0 value.</returns>
    public async Task<int> Run(TextWriter output)
    {
        if (InputDirectories.Count == 0)
        {
            throw GetCommandException("Error: Expecting an input directory");
        }

        if (string.IsNullOrEmpty(OutputDirectory))
        {
            throw GetCommandException($"Error: Expecting an valid output directory");
        }

        int runResult = 0;

        try
        {
            if (MaxThreadCount == 1 || InputDirectories.Count == 1)
            {
                AssetCompilerContext context = GetOrCreateCompilerContext();
                try
                {
                    foreach (string? inputDirectory in InputDirectories)
                    {
                        try
                        {
                            // logic
                            if (!await context.Run(inputDirectory, OutputDirectory, output/*, mergedOptionsBase*/))
                            {
                                runResult = 1;
                            }
                        }
                        finally
                        {
                            context.Reset();
                        }
                    }
                }
                finally
                {
                    ReleaseCompilerContext(context);
                }
            }
            else
            {
                Parallel.ForEach(InputDirectories, new ParallelOptions
                {
                    MaxDegreeOfParallelism = MaxThreadCount is not > 0 ? Environment.ProcessorCount : MaxThreadCount.Value
                },
                    inputDirectory =>
                    {
                        var context = GetOrCreateCompilerContext();
                        try
                        {
                            // logic
                            var result = context.Run(inputDirectory, OutputDirectory, output/*, mergedOptionsBase*/).Result;
                            if (!result)
                            {
                                runResult = 1;
                            }
                        }
                        finally
                        {
                            ReleaseCompilerContext(context);
                        }
                    });
            }

            // Process remaining tar files
            //ProcessTar();
        }
        finally
        {
            DisposeAllContexts();

            lock (_includeFilesLastWriteTime)
            {
                _includeFilesLastWriteTime.Clear();
            }

            //lock (_processedFiles)
            //{
            //    _processedFiles.Clear();
            //}
        }
        return runResult;
    }

    internal AssetCompilerContext GetOrCreateCompilerContext()
    {
        lock (_contexts)
        {
            if (_contexts.Count == 0)
            {
                _contexts.Push(new AssetCompilerContext());
            }

            return _contexts.Pop();
        }
    }

    internal void ReleaseCompilerContext(AssetCompilerContext context)
    {
        context.Reset();
        lock (_contexts)
        {
            _contexts.Push(context);
        }
    }

    private void DisposeAllContexts()
    {
        lock (_contexts)
        {
            while (_contexts.Count > 0)
            {
                _contexts.Pop().Dispose();
            }
        }
    }

    /// <summary>
    /// Entry point for the ShaderCompilerProgram.
    /// </summary>
    public static async Task<int> Main(string[] args)
    {
        CommandApp app = CreateCommandApp();
        return await app.RunAsync(args);
    }

    /// <summary>
    /// Creates a new instance of <see cref="CommandApp"/> for the ShaderCompilerApp.
    /// </summary>
    /// <param name="config">The configuration for the command line app.</param>
    public static CommandApp CreateCommandApp(CommandConfig? config = null)
    {
        const string _ = "";
        AssetCompilerApp app = new()
        {
            GetCommandException = message => new CommandException(message),
            GetOptionException = (message, paramName) => new OptionException(message, paramName)
        };
        int age = 0;
        var inputFileNames = new List<string>();
        var commandApp = new CommandApp("alimer-assetc", config: config)
        {
            _,
            "A command-line Asset compiler for alimer engine.",
            _,
            "Overall Options:",
            _,
            new HelpOption(),
            new VersionOption(),
            { "i=|input=", "Adds the specified {<directory>} to the search path for processing folders.", app.InputDirectories },
            { "o=", "The output directory.", v => app.OutputDirectory = v},
            { "p|platform=", $"The target platform {{ENUM}} accepting the following values: {EnumWrapper<PlatformID>.Names}", (EnumWrapper<PlatformID> v)  => app.TargetPlatform = v},
            new ResponseFileSource(),
            // Run the command
            async (CommandRunContext context, string[] _) =>
            {
                if(string.IsNullOrEmpty(app.OutputDirectory))
                {
                    app.OutputDirectory = Path.Combine(AppContext.BaseDirectory, "Assets", app.TargetPlatform.ToString());
                }

                if (!Directory.Exists(app.OutputDirectory))
                {
                    Directory.CreateDirectory(app.OutputDirectory);
                }

                return await app.Run(context.Out);
            }
        };

        return commandApp;
    }
}
