#pragma once
#include "WaitMonitor.h"
#include <chrono>
#include <mutex>
#include <set>
#include <condition_variable>
#include <memory>
#include "Waitable.h"

namespace CommandLib
{
	/// <summary>This object mimics a Windows manual reset event</summary>
	class Event : public Waitable
    {
	public:
		typedef std::shared_ptr<Event> Ptr;

		/// <summary>Construct an unsignaled Event</summary>
		Event();

		/// <summary>Construct an Event></summary>
		/// <param name="initiallySignaled">Whether the event should be initially signaled</param>
		explicit Event(bool initiallySignaled);

		virtual ~Event();

		/// <summary>Signal this event</summary>
		/// <remarks>
		/// This object will stay signaled until <see cref="Reset"/> is called. It is safe to call this multiple times in a row.
		/// </remarks>
		void Set();

		/// <summary>Make this event not signaled</summary>
		/// <remarks>
		/// This object will stay not signaled until <see cref="Set"/> is called. It is safe to call this multiple times in a row.
		/// </remarks>
		void Reset();

		/// <inheritdoc/>
		virtual bool IsSignaled() const override final;

		/// <inheritdoc/>
		virtual void Wait() const override final;

		/// <inheritdoc/>
		virtual bool Wait(long long ms) const override final;
	private:
		Event(const Event&);
		Event& operator=(const Event&);
		bool m_signaled;
		mutable std::condition_variable_any m_condition;
		mutable std::recursive_mutex m_mutex;
	};
}
