// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Engine;

public sealed class ScriptSystem : EntitySystem<ScriptComponent>
{
    private readonly HashSet<ScriptComponent> _registeredScripts = [];
    private readonly HashSet<ScriptComponent> _scriptsToStart = [];
    private readonly HashSet<SyncScript> _syncScripts = [];
    private readonly List<ScriptComponent> _scriptsToStartCopy = [];
    private readonly List<SyncScript> _syncScriptsCopy = [];

    public ScriptSystem(IServiceRegistry services)
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));

        Services = services;
    }

    public IServiceRegistry Services { get; }

    public override void Update(GameTime time)
    {
        _scriptsToStartCopy.AddRange(_scriptsToStart);
        _scriptsToStart.Clear();

        _syncScriptsCopy.AddRange(_syncScripts);

        foreach (ScriptComponent script in _scriptsToStartCopy)
        {
            if (script is StartupScript startupScript)
            {
                startupScript.Start();
            }
            else if (script is AsyncScript asyncScript)
            {
                asyncScript.CancellationTokenSource = new CancellationTokenSource();
                asyncScript.ExecuteAsync();
            }
        }

        foreach (SyncScript syncScript in _syncScriptsCopy)
        {
            syncScript.Update(time);
        }

        _scriptsToStartCopy.Clear();
        _syncScriptsCopy.Clear();
    }

    protected override void OnEntityComponentAdded(ScriptComponent component)
    {
        component.Initialize(Services);

        _registeredScripts.Add(component);
        _scriptsToStart.Add(component);

        if (component is SyncScript syncScript)
        {
            _syncScripts.Add(syncScript);
        }
    }

    protected override void OnEntityComponentRemoved(ScriptComponent component)
    {
        bool startWasPending = _scriptsToStart.Remove(component);
        bool wasRegistered = _registeredScripts.Remove(component);

        if (!startWasPending && wasRegistered)
        {
            component.Cancel();

            if (component is AsyncScript asyncScript)
            {
                asyncScript.CancellationTokenSource?.Cancel();
            }
        }

        if (component is SyncScript syncScript)
        {
            _syncScripts.Remove(syncScript);
        }
    }
}
