// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#if TODO
using System;
using System.Runtime.InteropServices;
using System.Text;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;

namespace Vortice
{
    internal class CoreWindowAppContext : AppContext, IFrameworkViewSource
    {
        private readonly CoreWindowGameWindow _view;

        public CoreWindowAppContext(Application application)
            : base(application)
        {
            _view = new CoreWindowGameWindow();
        }

        public override View View => _view;

        public override void Run() => CoreApplication.Run(this);

        IFrameworkView IFrameworkViewSource.CreateView() => _view;
    }
}

#endif
