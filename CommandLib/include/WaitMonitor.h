#pragma once
#include <memory>
#include "Waitable.h"

namespace CommandLib
{
	/// <summary>Objects of this time can wait upon <see cref="Waitable"/> objects</summary>
	/// <remarks>WaitMonitor objects can be registered by calling <see cref="Waitable::AddListener"/></remarks>
	class WaitMonitor
    {
	public:
		/// <summary>Shared pointer to a WaitMonitor object</summary>
		typedef std::shared_ptr<WaitMonitor> Ptr;

		virtual ~WaitMonitor();

		/// <summary>Invoked when a <see cref="Waitable"/> object becomes signaled</summary>
		/// <param name="waitable">The object that became signaled</param>
		/// <remarks>
		/// This object must have been registered with the waitable via <see cref="Waitable::AddListener"/>
		/// in order for the callback to occur
		/// </remarks>
		virtual void Signaled(const Waitable& waitable) = 0;
	};
}
