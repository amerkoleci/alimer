// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/CommandBuffer.h"
#include <memory>

namespace Alimer
{
	class ALIMER_API CommandQueue 
	{
		friend class Graphics;

	public:
		CommandQueue(CommandQueue&&) = delete;
		CommandQueue(const CommandQueue&) = delete;
		CommandQueue& operator=(RefCounted&&) = delete;
		CommandQueue& operator=(const CommandQueue&) = delete;

		virtual ~CommandQueue() = default;

		virtual CommandBuffer* GetCommandBuffer() = 0;

		virtual void WaitIdle() = 0;

		virtual void Submit(CommandBuffer* const* commandBuffers, uint32_t count, bool waitForCompletion = false) = 0;
		void Submit(CommandBuffer* commandBuffer, bool waitForCompletion = false);
		void Submit(const std::vector<CommandBuffer*>& commandBuffers, bool waitForCompletion = false);

		CommandQueueType GetQueueType() const noexcept { return queueType; }

	protected:
		CommandQueue(CommandQueueType queueType);

		CommandQueueType queueType;
	};
}
