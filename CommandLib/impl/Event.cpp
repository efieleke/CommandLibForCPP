#include "Event.h"

using namespace CommandLib;

Event::Event() : m_signaled(false)
{
}

Event::Event(bool initiallySignaled) : m_signaled(initiallySignaled)
{
}

Event::~Event()
{
}

void Event::Set()
{
	{
		std::unique_lock<std::recursive_mutex> lock(m_mutex);
		m_signaled = true;
		m_condition.notify_all();
	}

	NotifyListeners();
}

void Event::Reset()
{
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	m_signaled = false;
}

bool Event::IsSignaled() const
{
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	return m_signaled;
}

void Event::Wait() const
{
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	
	while (!m_signaled)
	{
		m_condition.wait(lock);
	}
}

bool Event::Wait(long long ms) const
{
	std::unique_lock<std::recursive_mutex> lock(m_mutex);

	while (!m_signaled)
	{
		if (m_condition.wait_for(lock, std::chrono::milliseconds(ms)) == std::cv_status::timeout)
		{
			return false;
		}
	}

	return true;
}
