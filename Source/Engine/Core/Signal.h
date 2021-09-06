// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// Based on: https://schneegans.github.io/tutorials/2015/09/20/signal-slot

#pragma once

#include <functional>
#include <map>

namespace Alimer
{
	template <typename... Args>
	class Signal final
	{
	public:
		/// Constructor
		Signal() = default;

		/// Constructor.
		Signal(const Signal&) = default;

		/// Move Constructor.
		Signal(Signal&& other) noexcept 
			: slots(std::move(other.slots))
			, currentId(other.currentId)
		{
		}

		/// Destructor.
		~Signal() = default;

		Signal& operator=(const Signal& other)
		{
			if (this != &other)
			{
				DisconnectAll();
			}

			return *this;
		}

		Signal& operator=(Signal&& other) noexcept
		{
			if (this != &other)
			{
				slots = std::move(other.slots);
				currentId = other.currentId;
			}

			return *this;
		}

		/**
		 * Connects a Function to the signal.
		 *
		 * @param slot The callback function slot.
		 * @return The slot connection id. It can be used to disconnect the function.
		 */
		uint32_t Connect(const std::function<void(Args...)>& slot) const
		{
			slots.insert(std::make_pair(++currentId, slot));
			return currentId;
		}

		/// Connects a member function of an object instance to this Signal.
		template <typename T>
		uint32_t ConnectMember(T* inst, void (T::* func)(Args...))
		{
			return Connect([=](Args... args) {
					(inst->*func)(args...);
			});
		}

		/// Connect a const member function of an object to this Signal.
		template <typename T>
		uint32_t ConnectMember(T* inst, void (T::* func)(Args...) const)
		{
			return Connect([=](Args... args) { 
				(inst->*func)(args...); 
			});
		}

		/// Disconnects a previously connected function slot.
		void Disconnect(uint32_t id) const
		{
			slots.erase(id);
		}

		/// Disconnects all previously connected function slots.
		void DisconnectAll() const
		{
			slots.clear();
		}

		/// Emits and calls all connected functions.
		void operator()(Args... args) const
		{
			for (const auto& it : slots)
			{
				it.second(args...);
			}
		}

		/// Emits and calls all connected functions.
		void Emit(Args... args)
		{
			for (auto const& it : slots)
			{
				it.second(args...);
			}
		}

		/**
		 * Calls only one connected function.
		 *
		 * @param id The id of the connected slot function to be called.
		 */
		constexpr void EmitFor(uint32_t id, Args... args)
		{
			auto const& it = slots.find(id);
			if (it != slots.end())
			{
				it->second(args...);
			}
		}

		/**
		 * Emits and calls all connected functions except for one.
		 *
		 * @param id The connnection id to ignore.
		 */
		void EmitForAllButOne(uint32_t id, Args... args)
		{
			for (auto const& it : slots)
			{
				if (it.first != id)
				{
					it.second(args...);
				}
			}
		}

	private:
		mutable std::map<uint32_t, std::function<void(Args...)>> slots;
		mutable uint32_t currentId{ 0 };
	};
}
