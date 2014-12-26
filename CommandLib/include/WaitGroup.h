#pragma once
#include "WaitMonitor.h"
#include "Event.h"
#include <chrono>
#include <atomic>
#include <list>
#include <unordered_set>
#include <mutex>

namespace CommandLib
{
	/// <summary>
	/// This class attempts to provide functionality like Windows' WaitForMultipleObjects. It provides a way to
	/// efficiently wait upon any number of <see cref="Waitable"/> objects, until either one or all of the objects
	/// enter a signaled state.
	/// </summary>
	/// <remarks>Behavior is undefined if you access the same WaitGroup object across multiple threads.</remarks>
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
		void AddWaitable(Waitable::Ptr waitable);

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

		/// <summary>Waits until all of the waitable objects have entered the signaled state.</summary>
		/// <remarks>
		/// Note that this will return after all of the items have been in the signaled at any time
		/// since the wait began. This does *not* wait until the items are simultaneously signaled.
		/// For example, if waiting upon two events, and the first becomes signaled, and is then reset,
		/// and then the second event becomes signaled, this method will return (even though the two
		/// events were never both in the signaled state at the same time.
		/// </remarks>
		void WaitForAll() const;

		/// <summary>Waits until all of the waitable objects have entered the signaled state.</summary>
		/// <param name="dur">The maximum amount of time to wait</param>
		/// <returns>
		/// true if all objects have entered the signaled state at some point before the duration expires,
		/// false otherwise
		/// </returns>
		/// <remarks>
		/// Note that this will return after all of the items have been in the signaled at any time
		/// since the wait began (or the duration elapses). This does *not* wait until the items
		/// are simultaneously signaled. For example, if waiting upon two events, and the first becomes
		/// signaled, and is then reset, and then the second event becomes signaled, this method will
		/// return (even though the two events were never both in the signaled state at the same time).
		/// </remarks>
		template<typename Rep, typename Period>
		bool WaitForAll(const std::chrono::duration<Rep, Period>& dur) const
		{
			return m_impl->WaitForAll(dur);
		}

		/// <summary>Waits until all of the waitable objects are simulataneously signaled.</summary>
		/// <param name="ms">The maximum amount of milliseconds to wait</param>
		/// <returns>true if all objects are simultaneously signaled before the duration expires, false otherwise</returns>
		/// Note that this will return after all of the items have been in the signaled at any time
		/// since the wait began, or the duration elapses. This does *not* wait until the items
		/// are simultaneously signaled.For example, if waiting upon two events, and the first becomes
		/// signaled, and is then reset, and then the second event becomes signaled, this method will
		/// return (even though the two events were never both in the signaled state at the same time).
		/// </remarks>
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
				InitializeSignaled();
				int result = AnySignaled();

				if (result == -1 && m_waitSignaledEvent.WaitFor(dur))
				{
					result = AnySignaled();
				}

				return result;
			}

			void WaitForAll() const;

			template<typename Rep, typename Period>
			bool WaitForAll(const std::chrono::duration<Rep, Period>& dur) const
			{
				InitializeSignaled();

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
			void InitializeSignaled() const;

			std::list<Waitable::Ptr> m_list;
			mutable std::unordered_set<const Waitable*> m_signaled;
			mutable const Waitable* m_firstSignaled;
			mutable Event m_waitSignaledEvent;
			mutable std::mutex m_mutex;
		};

		WaitGroup(const WaitGroup&);
		WaitGroup& operator=(const WaitGroup&);

		WaitGroupImpl::Ptr m_impl;
	};
}
