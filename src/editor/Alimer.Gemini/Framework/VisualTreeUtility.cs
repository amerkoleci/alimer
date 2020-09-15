// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.
// This is modified version of Gemini framework, see https://github.com/tgjones/gemini/blob/master/LICENCE.txt

using System.Windows;
using System.Windows.Media;

namespace Gemini.Framework
{
    public static class VisualTreeUtility
    {
        public static T? FindParent<T>(DependencyObject child) where T : DependencyObject
        {
            DependencyObject? parentObject = VisualTreeHelper.GetParent(child);

            if (parentObject == null)
                return null;

            if (parentObject is T parent)
                return parent;

            return FindParent<T>(parentObject);
        }
    }
}
