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

WaitGroup::WaitGroupImpl::WaitGroupImpl()
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
	m_list.push_back(item);
	item->AddListener(shared_from_this());
}

int WaitGroup::WaitGroupImpl::WaitForAny() const
{
	m_waitSignaledEvent.Reset();
	int result = AnySignaled();

	if (result == -1)
	{
		m_waitSignaledEvent.Wait();
		result = m_signaledIndex;
	}

	return result;
}

void WaitGroup::WaitGroupImpl::WaitForAll() const
{
	while (!AllSignaled())
	{
		m_waitSignaledEvent.Wait();
	}
}

void WaitGroup::WaitGroupImpl::Signaled(const Waitable& waitable)
{
	int index = 0;

	for (Waitable::Ptr item : m_list)
	{
		if (item.get() == &waitable)
		{
			m_signaledIndex = index;
			m_waitSignaledEvent.Set();
			break;
		}
		
		++index;
	}
}

int WaitGroup::WaitGroupImpl::AnySignaled() const
{
	int result = 0;

	for (Waitable::Ptr item : m_list)
	{
		if (item->IsSignaled())
		{
			return result;
		}

		++result;
	}

	return -1;
}

bool WaitGroup::WaitGroupImpl::AllSignaled() const
{
	for (Waitable::Ptr item : m_list)
	{
		if (!item->IsSignaled())
		{
			return false;
		}
	}

	return true;
}
