#include "Waitable.h"
#include "WaitMonitor.h"

using namespace CommandLib;

Waitable::~Waitable()
{
}

bool Waitable::AddListener(std::shared_ptr<WaitMonitor> listener)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	return m_listeners.insert(listener).second;
}

bool Waitable::RemoveListener(std::shared_ptr<WaitMonitor> listener)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	return m_listeners.erase(listener) > 0;
}

void Waitable::NotifyListeners()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	for (WaitMonitor::Ptr listener : m_listeners)
	{
		listener->Signaled(*this);
	}
}
