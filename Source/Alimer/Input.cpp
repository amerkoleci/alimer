// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Assert.h"
#include "Alimer/Core/Log.h"
#include "Alimer/Input.h"

#include <atomic>

using namespace Alimer;

namespace
{
    InputState gEmptyState;
    std::atomic_bool initialized{ false };
}

Signal<KeyboardKeys, KeyModifiers> Input::KeyDown;
Signal<KeyboardKeys, KeyModifiers> Input::KeyUp;
Signal<const Vector2&> Input::MouseMove;
InputState Input::currentState;
InputState Input::lastState;

void Input::Initialize()
{
    if (initialized.load())
    {
        return;
    }

    Input::lastState = gEmptyState;
    Input::currentState = gEmptyState;

    LOGI("Input Initialized");
    initialized.store(true);
}

void Input::Shutdown()
{
    if (!initialized.load())
    {
        return;
    }

    initialized.store(false);
}

void Input::Update()
{
    // cycle states
    lastState = currentState;

    // update other state for next frame
    for (uint32_t i = 0; i < kMaxKeyboardKeys; i++)
    {
        currentState.keyboard.pressed[i] = false;
        currentState.keyboard.released[i] = false;
    }

    for (uint32_t i = 0; i < kMaxMouseButtons; i++)
    {
        currentState.mouse.pressed[i] = false;
        currentState.mouse.released[i] = false;
    }

    currentState.mouse.wheel = Vector2::Zero;
}

Vector2 Input::GetMousePosition()
{
    return currentState.mouse.position;
}

Vector2 Input::GetMousePositionDelta()
{
    return currentState.mouse.position - lastState.mouse.position;
}

bool Input::IsKeyboardKeyDown(KeyboardKeys key)
{
    return currentState.keyboard.down[ecast(key)];
}

bool Input::IsKeyboardKeyPressed(KeyboardKeys key)
{
    return currentState.keyboard.pressed[ecast(key)];
}

bool Input::IsKeyboardKeyReleased(KeyboardKeys key)
{
    return currentState.keyboard.pressed[ecast(key)];
}

bool Input::IsMouseButtonDown(MouseButton button)
{
    return currentState.mouse.down[ecast(button)];
}

bool Input::IsMouseButtonPressed(MouseButton button)
{
    return currentState.mouse.pressed[ecast(button)];
}

bool Input::IsMouseButtonReleased(MouseButton button)
{
    return currentState.mouse.released[ecast(button)];
}

void Input::OnKeyboardKey(KeyboardKeys key, KeyModifiers modifiers, bool pressed)
{
    const uint32_t  index = ecast(key);
    ALIMER_ASSERT(index >= 0 && index < kMaxKeyboardKeys);
    if (pressed)
    {
        currentState.keyboard.down[index] = true;
        currentState.keyboard.pressed[index] = true;
        KeyDown.Emit(key, modifiers);
    }
    else
    {
        currentState.keyboard.down[index] = false;
        currentState.keyboard.released[index] = true;
        KeyUp.Emit(key, modifiers);
    }
}

void Input::OnMouseButton(MouseButton button, float x, float y, bool pressed)
{
    const uint32_t index = ecast(button);
    if (pressed)
    {
        currentState.mouse.down[index] = true;
        currentState.mouse.pressed[index] = true;
    }
    else
    {
        currentState.mouse.down[index] = false;
        currentState.mouse.released[index] = true;
    }

    currentState.mouse.position.x = x;
    currentState.mouse.position.y = y;

    // TODO: Fire event
    //MouseButtonEvent evt{};
    //evt.button = button;
    //evt.pressed = pressed;
    //evt.position.x = x;
    //evt.position.y = y;
    //mouseButtonEvent.Emit(evt);
}

void Input::OnMouseMove(float x, float y)
{
    currentState.mouse.position.x = x;
    currentState.mouse.position.y = y;
    MouseMove(currentState.mouse.position);
}

void Input::OnMouseWheel(float xOffset, float yOffset)
{
    currentState.mouse.wheel.x += xOffset;
    currentState.mouse.wheel.y += yOffset;
}
