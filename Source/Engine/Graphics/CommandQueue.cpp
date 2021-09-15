// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/CommandQueue.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
	CommandQueue::CommandQueue(CommandQueueType queueType_)
		: queueType(queueType_)
	{

	}


	void CommandQueue::Submit(CommandBuffer* commandBuffer, bool waitForCompletion)
	{
		Submit(&commandBuffer, 1u, waitForCompletion);
	}

	void CommandQueue::Submit(const std::vector<CommandBuffer*>& commandBuffers, bool waitForCompletion)
	{
		Submit(commandBuffers.data(), static_cast<uint32_t>(commandBuffers.size()), waitForCompletion);
	}
}
