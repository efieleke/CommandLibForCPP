#pragma once
#include <chrono>
#include <mutex>
#include <set>
#include <memory>

namespace CommandLib
{
	class WaitMonitor;

	/// <summary>
	/// This class represents an object that can be waited upon until it enters a signaled state. The reason for its
	/// existence is so that it can be added to a <see cref="WaitGroup"/> object, thus allowing a wait for multiple
	/// objects (a feature available in Windows but not in other operating systems).
	/// </summary>
	class Waitable
    {
	public:
		/// <summary>Shared pointer to a non-modifyable Waitable object</summary>
		typedef std::shared_ptr<const Waitable> ConstPtr;

		/// <summary>Shared pointer to a Waitable object</summary>
		typedef std::shared_ptr<Waitable> Ptr;

		virtual ~Waitable();

		/// <summary>Add a listener for when this object becomes signaled</summary>
		/// <param name="listener">The listener to add</param>
		/// <remarks>This method is thread safe</remarks>
		bool AddListener(std::shared_ptr<WaitMonitor> listener);

		/// <summary>Removes the listener for when this object becomes signaled</summary>
		/// <param name="listener">The listener to remove</param>
		/// <remarks>This method is thread safe</remarks>
		bool RemoveListener(std::shared_ptr<WaitMonitor> listener);

		/// <summary>Indicates whether or not this object is in a signaled state</summary>
		/// <returns>true if this object is signaled</returns>
		virtual bool IsSignaled() const = 0;

		/// <summary>Waits until this object is in a signaled state</summary>
		/// <remarks>
		/// This will return immediately if this object is currently signaled
		/// </remarks>
		virtual void Wait() const = 0;

		/// <summary>Waits the given milliseconds until this object is in a signaled state</summary>
		/// <param name="ms">The number of milliseconds to wait</param>
		/// <returns>true if this object became signaled before the wait time elapsed, false otherwise</returns>
		/// <remarks>
		/// This will return immediately if this object is currently signaled.
		/// </remarks>
		virtual bool Wait(long long ms) const = 0;
		
		/// <summary>Waits the given duration until this object is in a signaled state</summary>
		/// <param name="dur">The duration to wait</param>
		/// <returns>true if this object became signaled before the wait time elapsed, false otherwise</returns>
		/// <remarks>
		/// This will return immediately if this object is currently signaled.
		/// </remarks>
		template<typename Rep, typename Period>
		bool WaitFor(const std::chrono::duration<Rep, Period>& dur) const
		{
			return Wait(std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
		}
	protected:
		/// <summary>Implementations should call this when this object is signaled</summary>
		void NotifyListeners();
	private:
		std::set<std::shared_ptr<WaitMonitor>> m_listeners;
		std::mutex m_mutex;
 	};
}
