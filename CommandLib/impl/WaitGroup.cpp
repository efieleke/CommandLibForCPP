#include "WaitGroup.h"

using namespace CommandLib;

WaitGroup::WaitGroup() : m_impl(WaitGroupImpl::Create())
{
}

WaitGroup::~WaitGroup()
{
}

void WaitGroup::AddWaitable(Waitable::Ptr item)
{
	m_impl->AddWaitable(item);
}

int WaitGroup::WaitForAny() const
{
	return m_impl->WaitForAny();
}

int WaitGroup::WaitForAny(long long ms) const
{
	return WaitForAny(std::chrono::milliseconds(ms));
}

void WaitGroup::WaitForAll() const
{
	m_impl->WaitForAll();
}

bool WaitGroup::WaitForAll(long long ms) const
{
	return WaitForAll(std::chrono::milliseconds(ms));
}

std::shared_ptr<WaitGroup::WaitGroupImpl> WaitGroup::WaitGroupImpl::Create()
{
	return Ptr(new WaitGroupImpl());
}

WaitGroup::WaitGroupImpl::WaitGroupImpl() : m_firstSignaled(nullptr)
{
}

WaitGroup::WaitGroupImpl::~WaitGroupImpl()
{
	for (Waitable::Ptr item : m_list)
	{
		item->RemoveListener(shared_from_this());
	}
}

void WaitGroup::WaitGroupImpl::AddWaitable(Waitable::Ptr item)
{
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_list.push_back(item);
	}

	item->AddListener(shared_from_this());
}

int WaitGroup::WaitGroupImpl::WaitForAny() const
{
	InitializeSignaled();
	int result = AnySignaled();

	if (result == -1)
	{
		m_waitSignaledEvent.Wait();
		result = AnySignaled();
	}

	return result;
}

void WaitGroup::WaitGroupImpl::WaitForAll() const
{
	InitializeSignaled();

	while (!AllSignaled())
	{
		m_waitSignaledEvent.Wait();
	}
}

void WaitGroup::WaitGroupImpl::Signaled(const Waitable& waitable)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if (m_signaled.insert(&waitable).second)
	{
		if (m_firstSignaled == nullptr)
		{
			m_firstSignaled = &waitable;
		}

		m_waitSignaledEvent.Set();
	}
}

int WaitGroup::WaitGroupImpl::AnySignaled() const
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_waitSignaledEvent.Reset();

	if (m_firstSignaled != nullptr)
	{
		int result = 0;

		for (Waitable::Ptr item : m_list)
		{
			if (item.get() == m_firstSignaled)
			{
				return result;
			}

			++result;
		}
	}

	return -1;
}

bool WaitGroup::WaitGroupImpl::AllSignaled() const
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_waitSignaledEvent.Reset();
	return m_signaled.size() == m_list.size();
}

void WaitGroup::WaitGroupImpl::InitializeSignaled() const
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_firstSignaled = nullptr;
	m_signaled.clear();

	for (Waitable::Ptr item : m_list)
	{
		if (item->IsSignaled())
		{
			if (m_firstSignaled == nullptr)
			{
				m_firstSignaled = item.get();
			}

			m_signaled.insert(item.get());
		}
	}
}
