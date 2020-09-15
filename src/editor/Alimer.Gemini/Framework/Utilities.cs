// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Caliburn.Micro;

namespace Gemini.Framework
{
    public static class Utilities
    {
        public static Uri PackUri(string path)
        {
            return new Uri($"pack://application:,,,/{typeof(Utilities).Assembly.GetAssemblyName()};component/{path}"); 
        }
    }
}
