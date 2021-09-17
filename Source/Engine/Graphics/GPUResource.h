// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsDefs.h"

namespace Alimer
{
	class Graphics;

    /// Base class for objects that allocate GPU object.
    class ALIMER_API GPUObjectOld
    {
    public:
        /// Destructor. 
        virtual ~GPUObjectOld() = default;

        /// Unconditionally destroy the GPU resource.
        virtual void Destroy() = 0;

    protected:
        /// Constructor. 
        GPUObjectOld() = default;
    };

	/// Base class for objects that allocate GPU resources.
	class ALIMER_API GPUResource 
	{
	public:
		enum class Type
		{
			Buffer,
			Texture,
		};

		/// Unconditionally destroy the GPU resource.
		virtual void Destroy() = 0;

        [[nodiscard]] virtual uint64_t GetAllocatedSize() const = 0;

	protected:
		/// Constructor. 
        GPUResource(Type type)
            : type{ type }
        {

        }

        void OnCreated();
        void OnDestroyed();

		Type type;
	};
}
