// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static SDL3;
using static SDL3.SDL_JoystickConnectionState;

namespace Alimer.Input;

internal unsafe class SDLGamepadDevice : GamepadDevice
{
    private bool[] _currentButtons = new bool[(int)GamepadButton.Count];
    private bool[] _previousButtons = new bool[(int)GamepadButton.Count];
    private float[] _axes = new float[(int)GamepadAxis.Count];

    public SDLGamepadDevice(SDL_JoystickID id)
    {
        Id = id;
        Handle = SDL_OpenGamepad(id);
        Name = SDL_GetGamepadName(Handle);
        Type = SDL_GetGamepadType(Handle);
        Vendor = SDL_GetGamepadVendor(Handle);
        Product = SDL_GetGamepadProduct(Handle);
        Version = SDL_GetGamepadProductVersion(Handle);
        PlayerIndex = SDL_GetGamepadPlayerIndex(Handle);
    }

    public SDL_JoystickID Id { get; }
    public SDL_Gamepad* Handle { get; private set; }
    public string Name { get; }
    public SDL_GamepadType Type { get; }
    public ushort Vendor { get; }
    public ushort Product { get; }
    public ushort Version { get; }
    public int PlayerIndex { get; }

    /// <inheritdoc />
    public override bool IsConnected
    {
        get
        {
            if (Handle == null)
                return false;

            return SDL_GamepadConnected(Handle);
        }
    }

    /// <inheritdoc />
    public override bool IsWireless
    {
        get
        {
            if (Handle == null)
                return false;

            SDL_JoystickConnectionState connectionState = SDL_GetGamepadConnectionState(Handle);
            return connectionState == SDL_JOYSTICK_CONNECTION_WIRELESS;
        }
    }

    public void Close()
    {
        if (Handle != null)
        {
            SDL_CloseGamepad(Handle);
            Handle = null;
        }
    }

    /// <summary>
    /// Called before processing new events.
    /// </summary>
    public void BeginFrame()
    {
        _previousButtons = _currentButtons;
    }

    /// <inheritdoc />
    public override bool IsButtonDown(GamepadButton button)
    {
        int index = (int)button;
        if (index < 0 || index >= (int)GamepadButton.Count)
            return false;

        return _currentButtons[index];
    }

    /// <inheritdoc />
    public override bool IsButtonPressed(GamepadButton button)
    {
        int index = (int)button;
        if (index < 0 || index >= (int)GamepadButton.Count)
            return false;

        return _currentButtons[index] && !_previousButtons[index];
    }

    /// <inheritdoc />
    public override bool IsButtonReleased(GamepadButton button)
    {
        int index = (int)button;
        if (index < 0 || index >= (int)GamepadButton.Count)
            return false;

        return !_currentButtons[index] && _previousButtons[index];
    }

    /// <inheritdoc />
    public override float GetAxis(GamepadAxis axis)
    {
        int index = (int)axis;
        if (index < 0 || index >= (int)GamepadAxis.Count)
            return 0;

        return _axes[index];
    }

    /// <inheritdoc />
    public override bool Rumble(float leftMotor, float rightMotor, uint durationMs)
    {
        if (Handle == null)
            return false;

        ushort lowFrequencyRumble = (ushort)(leftMotor * 0xFFFF);
        ushort highFrequencyRumble = (ushort)(rightMotor * 65535);

        // SDL_RumbleGamepadTriggers for leftTrigger and rightTrigger if supported by the device?
        return SDL_RumbleGamepad(Handle, lowFrequencyRumble, highFrequencyRumble, durationMs);
        //return SDL_RumbleGamepadTriggers(Handle, leftMotor, rightMotor) == 0;
    }

    public override BatteryStatus GetBatteryStatus(out int batteryLifePercent)
    {
        //int percent;
        //SDL_PowerState state = SDL_GetGamepadPowerInfo(Handle, &percent);
        //batteryLifePercent = percent;
        //return state.FromSDL();
        throw new NotImplementedException();
    }

    /// <inheritdoc />
    public override void Update()
    {

    }

    public void HandleButtonEvent(SDL_GamepadButtonEvent evt)
    {
        GamepadButton button = ((SDL_GamepadButton)evt.button).FromSDL();
        int index = (int)button;
        if (index >= 0 && index < (int)GamepadButton.Count)
        {
            _currentButtons[index] = evt.down;
        }
    }

    public void HandleAxisEvent(SDL_GamepadAxisEvent evt)
    {
        GamepadAxis axis = ((SDL_GamepadAxis)evt.axis).FromSDL();
        int index = (int)axis;
        if (index >= 0 && index < (int)GamepadAxis.Count)
        {
            float value = evt.value >= 0 ? evt.value / 32767.0f : evt.value / 32768.0f;
            _axes[index] = value;
        }
    }
}
