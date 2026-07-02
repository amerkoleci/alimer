// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include <Alimer/AlimerConfig.h>

// Core
#include <Alimer/Core/Signal.h>
//#include <Alimer/Core/Span.h>
#include <Alimer/Core/Log.h>
#include <Alimer/Core/Memory.h>
#include <Alimer/Core/StringId.h>
#include <Alimer/Core/Object.h>
#include <Alimer/Core/JobSystem.h>
#include "Alimer/Core/Stopwatch.h"

// Math
#include <Alimer/Math/Color.h>
#include <Alimer/Math/Vector2.h>
#include <Alimer/Math/Vector3.h>
#include <Alimer/Math/Vector4.h>
//#include <Alimer/Math/Rect.h>
#include <Alimer/Math/Quaternion.h>
#include <Alimer/Math/Matrix4x4.h>
#include <Alimer/Math/Plane.h>
#include <Alimer/Math/BoundingBox.h>
#include <Alimer/Math/BoundingSphere.h>
#include <Alimer/Math/BoundingFrustum.h>
#include <Alimer/Math/Ray.h>

// IO
#include <Alimer/IO/MemoryStream.h>
#include <Alimer/IO/FileSystem.h>

// Assets
#include <Alimer/Assets/AssetManager.h>

// Audio
#include <Alimer/Audio/AudioClip.h>
#include <Alimer/Audio/AudioSource.h>
#include <Alimer/Audio/Audio.h>

// Scene
#include <Alimer/Scene/Entity.h>
#include <Alimer/Scene/Scene.h>
//#include <Alimer/Scene/Camera.h>
//#include <Alimer/Scene/MeshComponent.h>
//#include <Alimer/Scene/Light.h>

// Renderer
#include <Alimer/Shaders/ShaderDefinitions.h>
#include <Alimer/Renderer/Components/CameraComponent.h>
#include <Alimer/Renderer/Components/LightComponent.h>
#include <Alimer/Renderer/Components/MeshComponent.h>

// Physics
#include <Alimer/Physics/Components/CollisionShapeComponent.h>
#include <Alimer/Physics/Components/RigidBodyComponent.h>

#if TODO
#include <Alimer/Renderer/Texture.h>
#include <Alimer/Renderer/Font.h>
#include <Alimer/Renderer/SpriteBatch.h>
#include <Alimer/Renderer/SceneRenderer.h>

// Animation
#include "Alimer/Animations/AnimationSystem.h"
#include "Alimer/Animations/AnimationTarget.h"
#include "Alimer/Animations/AnimationClip.h"
#endif

// Platform
#include <Alimer/Platform/Platform.h>
#include <Alimer/Platform/DynamicLibrary.h>

// Application
#include <Alimer/Input.h>
#include <Alimer/Application.h>

#if TODO
#if defined(ALIMER_2D)
#include <Alimer/2D/SpriteComponent.h>
#include <Alimer/2D/CollisionShape2DComponent.h>
#include <Alimer/2D/RigidBody2DComponent.h>
#endif

// ImGui
#include <Alimer/ImGui/imgui.h>
#include <Alimer/ImGui/imgui_internal.h>  
#endif // TODO

