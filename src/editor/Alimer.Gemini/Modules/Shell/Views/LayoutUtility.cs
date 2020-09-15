// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.
// This is modified version of Gemini framework, see https://github.com/tgjones/gemini/blob/master/LICENCE.txt

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using AvalonDock;
using AvalonDock.Layout;
using AvalonDock.Layout.Serialization;
using Gemini.Framework;

namespace Gemini.Modules.Shell.Views
{
    internal static class LayoutUtility
    {
        public static void SaveLayout(DockingManager manager, Stream stream)
        {
            var layoutSerializer = new XmlLayoutSerializer(manager);
            layoutSerializer.Serialize(stream);
        }

        public static void LoadLayout(DockingManager manager, Stream stream, Action<IDocument> addDocumentCallback,
                                      Action<ITool> addToolCallback, Dictionary<string, ILayoutItem> items)
        {
            var layoutSerializer = new XmlLayoutSerializer(manager);

            layoutSerializer.LayoutSerializationCallback += (s, e) =>
                {
                    ILayoutItem item;
                    if (items.TryGetValue(e.Model.ContentId, out item))
                    {
                        e.Content = item;

                        var tool = item as ITool;
                        var anchorable = e.Model as LayoutAnchorable;

                        var document = item as IDocument;
                        var layoutDocument = e.Model as LayoutDocument;

                        if (tool != null && anchorable != null)
                        {
                            addToolCallback(tool);
                            tool.IsVisible = anchorable.IsVisible;

                            if (anchorable.IsActive)
                                tool.ActivateAsync(CancellationToken.None).Wait();

                            tool.IsSelected = e.Model.IsSelected;

                            return;
                        }

                        if (document != null && layoutDocument != null)
                        {
                            addDocumentCallback(document);

                            // Nasty hack to get around issue that occurs if documents are loaded from state,
                            // and more documents are opened programmatically.
                            layoutDocument.GetType().GetProperty("IsLastFocusedDocument").SetValue(layoutDocument, false, null);

                            document.IsSelected = layoutDocument.IsSelected;
                            return;
                        }
                    }

                    // Don't create any panels if something went wrong.
                    e.Cancel = true;
                };

            try
            {
                layoutSerializer.Deserialize(stream);
            }
            catch
            {
            }
        }
    }
}
