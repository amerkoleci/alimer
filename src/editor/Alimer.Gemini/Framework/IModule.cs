using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Windows;

namespace Gemini.Framework
{
	public interface IModule
	{
        IEnumerable<ResourceDictionary> GlobalResourceDictionaries { get; }
        IEnumerable<IDocument> DefaultDocuments { get; }
        IEnumerable<Type> DefaultTools { get; }

        void PreInitialize();
		void Initialize();
        Task PostInitializeAsync();
	}
}
