#include "ScheduledCommand.h"

using namespace CommandLib;

namespace
{
	std::string TimeAsText(const std::chrono::time_point<std::chrono::system_clock>& time)
	{
		std::chrono::system_clock::now();
		const time_t asTimeT = std::chrono::system_clock::to_time_t(time);
		char timeString[64]; // more than big enough
		timeString[0] = '\0';

		// TODO: posix does not have gmtime_s. In that envirornment, use gmtime_r
		tm asTm;
		gmtime_s(&asTm, &asTimeT);
		strftime(timeString, sizeof(timeString), "%Y-%m-%dT%H:%M:%SZ", &asTm);
		return timeString;
	}
}

ScheduledCommand::Ptr ScheduledCommand::Create(
	Command::Ptr command,
	const std::chrono::time_point<std::chrono::system_clock>& timeOfExecution,
	bool runImmediatelyIfTimeIsPast)
{
	return Ptr(new ScheduledCommand(command, timeOfExecution, runImmediatelyIfTimeIsPast));
}

ScheduledCommand::ScheduledCommand(
	Command::Ptr command,
	const std::chrono::time_point<std::chrono::system_clock>& timeOfExecution,
	bool runImmediatelyIfTimeIsPast)
	: m_command(command),
	  m_runImmediatelyIfTimeIsPast(runImmediatelyIfTimeIsPast),
	  m_timeOfExecution(timeOfExecution),
	  m_pauseCmd(PauseCommand::Create(0))
{
    TakeOwnership(m_command);
	TakeOwnership(m_pauseCmd);
}

std::string ScheduledCommand::ClassName() const
{
	return "ScheduledCommand";
}

std::chrono::time_point<std::chrono::system_clock> ScheduledCommand::GetTimeOfExecution() const
{
	std::unique_lock<std::mutex> lock(m_mutex);
	return m_timeOfExecution;
}

void ScheduledCommand::SetTimeOfExecution(const std::chrono::time_point<std::chrono::system_clock>& time)
{
	const auto newInterval = time - std::chrono::system_clock::now();

	if (newInterval.count() < 0 && !m_runImmediatelyIfTimeIsPast)
	{
		throw std::invalid_argument("'" + Description() + "' was scheduled to run at " + TimeAsText(time) + ", which is in the past");
	}

	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_timeOfExecution = time;
	}

	if (newInterval.count() >= 0)
	{
		m_pauseCmd->SetDuration(newInterval);
		m_pauseCmd->Reset();
	}
	else
	{
		m_pauseCmd->CutShort();
	}
}

void ScheduledCommand::SkipWait()
{
    m_pauseCmd->CutShort();
}

std::string ScheduledCommand::ExtendedDescription() const
{
    return "Time to execute: " + TimeAsText(GetTimeOfExecution()) + "; Run immediately if time is in the past? " + (m_runImmediatelyIfTimeIsPast ? "yes" : "no");
}

void ScheduledCommand::SyncExeImpl()
{
	const auto waitTime = GetTimeOfExecution() - std::chrono::system_clock::now();

    if (waitTime.count() >= 0)
    {
		m_pauseCmd->SetDuration(waitTime);
        m_pauseCmd->SyncExecute();
    }
    else if (!m_runImmediatelyIfTimeIsPast)
    {
        throw std::invalid_argument("'" + Description() + "' was scheduled to run at " + TimeAsText(GetTimeOfExecution()) + ", which is in the past");
    }

    m_command->SyncExecute();
}
