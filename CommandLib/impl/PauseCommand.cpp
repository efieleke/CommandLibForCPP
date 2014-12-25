#include "PauseCommand.h"
#include "CommandAbortedException.h"
#include <functional>
#include "WaitGroup.h"

using namespace CommandLib;

PauseCommand::Ptr PauseCommand::Create(long long ms)
{
	return Create(ms, Waitable::Ptr());
}

PauseCommand::Ptr PauseCommand::Create(long long ms, Waitable::Ptr stopEvent)
{
	return Ptr(new PauseCommand(ms, stopEvent));
}

std::string PauseCommand::ClassName() const
{
	return "PauseCommand";
}

PauseCommand::PauseCommand(long long ms, Waitable::Ptr stopEvent)
	: m_externalCutShortEvent(stopEvent),
	m_milliseconds(ms)
{
}

void PauseCommand::CutShort()
{
	m_cutShortEvent->Set();
}

void PauseCommand::Reset()
{
	m_resetEvent->Set();
}

long long PauseCommand::GetDurationMS() const
{
	std::unique_lock<std::mutex> lock(m_durationMutex);
	return m_milliseconds;
}

void PauseCommand::SetDurationMS(long long ms)
{
	std::unique_lock<std::mutex> lock(m_durationMutex);
	m_milliseconds = ms;
}

std::string PauseCommand::ExtendedDescription() const
{
	return "Duration: " + std::to_string(m_milliseconds) + "ms";
}

void PauseCommand::PrepareExecute()
{
	m_cutShortEvent->Reset();
	m_resetEvent->Reset();
}

void PauseCommand::SyncExeImpl()
{
	int result = WaitForDuration();

	while (result == 1)
	{
		m_resetEvent->Reset();
		result = WaitForDuration();
	}

	if (result == 0)
	{
		throw CommandAbortedException();
	}
}

int PauseCommand::WaitForDuration() const
{
	WaitGroup eventGroup;
	eventGroup.AddWaitable(AbortEvent());
	eventGroup.AddWaitable(m_resetEvent);
	eventGroup.AddWaitable(m_cutShortEvent);

	if (m_externalCutShortEvent.get() != nullptr)
	{
		eventGroup.AddWaitable(m_externalCutShortEvent);
	}

	// TODO: deal with overflow?
	return eventGroup.WaitForAny(m_milliseconds);
}