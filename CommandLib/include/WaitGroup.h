#pragma once
#include "WaitMonitor.h"
#include "Event.h"
#include <chrono>
#include <atomic>
#include <list>

namespace CommandLib
{
	/// <summary>
	/// This class attempts to provide functionality like Windows' WaitForMultipleObjects
	/// </summary>
	class WaitGroup
    {
	public:
		/// <summary>Contructs a WaitGroup</summary>
		WaitGroup();

		virtual ~WaitGroup();

		/// <summary>Adds a Waitable to this WaitGroup</summary>
		/// <param name="waitable">The object to include in the group wait.</param>
		/// <remarks>
		/// This object must remain in scope for as long as the item passed as a parameter,
		/// otherwise behavior is undefined.
		/// </remarks>
		void AddWaitable(Waitable::Ptr item);

		/// <summary>Waits until any one of the waitable objects becomes signaled.</summary>
		/// <returns>
		/// The index of the first item to become signaled (<see cref="AddWaitable"/> adds
		/// items in sequential order).
		/// </returns>
		/// <remarks>
		/// If multiple items are simultaneously signaled, this will return the one with the lowest index
		/// </remarks>
		int WaitForAny() const;

		/// <summary>Waits until any one of the waitable objects becomes signaled.</summary>
		/// <param name="dur">The maximum amount of time to wait</param>
		/// <returns>
		/// The index of the first item to become signaled (<see cref="AddWaitable"/> adds
		/// items in sequential order). If none of the items are signaled before the duration
		/// elapses, this will return -1.
		/// <remarks>
		/// If multiple items are simultaneously signaled, this will return the one with the lowest index
		/// </remarks>
		template<typename Rep, typename Period>
		int WaitForAny(const std::chrono::duration<Rep, Period>& dur) const
		{
			return m_impl->WaitForAny(dur);
		}

		/// <summary>Waits until any one of the waitable objects becomes signaled.</summary>
		/// <param name="ms">The maximum amount of time to wait</param>
		/// <returns>
		/// The index of the first item to become signaled (<see cref="AddWaitable"/> adds
		/// items in sequential order). If none of the items are signaled before the duration
		/// elapses, this will return -1.
		/// </returns>
		/// <remarks>
		/// If multiple items are simultaneously signaled, this will return the one with the lowest index
		/// </remarks>
		int WaitForAny(long long ms) const;

		/// <summary>Waits until all of the waitable objects are simulataneously signaled.</summary>
		void WaitForAll() const;

		/// <summary>Waits until all of the waitable objects are simulataneously signaled.</summary>
		/// <param name="dur">The maximum amount of time to wait</param>
		/// <returns>true if all objects are simultaneously signaled before the duration expires, false otherwise</returns>
		template<typename Rep, typename Period>
		bool WaitForAll(const std::chrono::duration<Rep, Period>& dur) const
		{
			return m_impl->WaitForAll(dur);
		}

		/// <summary>Waits until all of the waitable objects are simulataneously signaled.</summary>
		/// <param name="ms">The maximum amount of milliseconds to wait</param>
		/// <returns>true if all objects are simultaneously signaled before the duration expires, false otherwise</returns>
		bool WaitForAll(long long ms) const;
	private:
		class WaitGroupImpl : public std::enable_shared_from_this<WaitGroupImpl>, public WaitMonitor
		{
		public:
			typedef std::shared_ptr<WaitGroupImpl> Ptr;
			static Ptr Create();

			virtual ~WaitGroupImpl();
			void AddWaitable(Waitable::Ptr item);
			int WaitForAny() const;

			template<typename Rep, typename Period>
			int WaitForAny(const std::chrono::duration<Rep, Period>& dur) const
			{
				m_waitSignaledEvent.Reset();
				int result = AnySignaled();

				if (result == -1 && m_waitSignaledEvent.WaitFor(dur))
				{
					result = m_signaledIndex;
				}

				return result;
			}

			void WaitForAll() const;

			template<typename Rep, typename Period>
			bool WaitForAll(const std::chrono::duration<Rep, Period>& dur) const
			{
				while (!AllSignaled())
				{
					if (!m_waitSignaledEvent.WaitFor(dur))
					{
						return false;
					}
				}

				return true;
			}
		private:
			WaitGroupImpl();
			WaitGroupImpl(const WaitGroup&);
			WaitGroupImpl& operator=(const WaitGroupImpl&);

			virtual void Signaled(const Waitable& item) override final;
			int AnySignaled() const;
			bool AllSignaled() const;

			std::list<Waitable::Ptr> m_list;
			mutable Event m_waitSignaledEvent;
			mutable std::atomic_int m_signaledIndex;
		};

		WaitGroup(const WaitGroup&);
		WaitGroup& operator=(const WaitGroup&);

		WaitGroupImpl::Ptr m_impl;
	};
}
