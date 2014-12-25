#include "PeriodicCommand.h"
#include "ParallelCommands.h"
#include "SequentialCommands.h"

using namespace CommandLib;

PeriodicCommand::Ptr PeriodicCommand::Create(
	Command::Ptr command,
	size_t repeatCount,
	long long intervalMS,
	IntervalType intervalType,
	bool intervalIsInclusive)
{
	return Create(command, repeatCount, intervalMS, intervalType, intervalIsInclusive, Waitable::Ptr());
}

PeriodicCommand::Ptr PeriodicCommand::Create(
	Command::Ptr command,
	size_t repeatCount,
	long long intervalMS,
	IntervalType intervalType,
	bool intervalIsInclusive,
	Waitable::Ptr stopEvent)
{
	return Ptr(new PeriodicCommand(command, repeatCount, intervalMS, intervalType, intervalIsInclusive, stopEvent));
}

std::string PeriodicCommand::ClassName() const
{
	return "PeriodicCommand";
}

PeriodicCommand::PeriodicCommand(
	Command::Ptr command,
	size_t repeatCount,
	long long intervalMS,
	IntervalType intervalType,
    bool intervalIsInclusive,
	Waitable::Ptr stopEvent)
	: m_stopEvent(stopEvent),
	  m_initialPause(PauseCommand::Create(intervalMS, stopEvent)),
	  m_pause(PauseCommand::Create(intervalMS, stopEvent))
{
	TakeOwnership(m_initialPause);

	if (intervalIsInclusive)
	{
		switch (intervalType)
		{
		case IntervalType::PauseBefore:
			m_startWithPause = true;
			break;
		case IntervalType::PauseAfter:
			m_startWithPause = false;
			break;
		default:
			throw std::invalid_argument("Unknown interval type " + std::to_string((int)intervalType));
		}

		ParallelCommands::Ptr parallelCmds = ParallelCommands::Create(true);
		parallelCmds->Add(command);
		parallelCmds->Add(m_pause);
		m_collectionCmd = parallelCmds;
	}
	else
	{
		SequentialCommands::Ptr sequence(SequentialCommands::Create());
		m_collectionCmd = sequence;

		switch (intervalType)
		{
		case IntervalType::PauseAfter:
			sequence->Add(command);
			sequence->Add(m_pause);
			break;
		case IntervalType::PauseBefore:
			sequence->Add(m_pause);
			sequence->Add(command);
			break;
		default:
			throw std::invalid_argument("Unknown interval type " + std::to_string((int)intervalType));
		}
	}

	TakeOwnership(m_collectionCmd);
	m_repeatCount = repeatCount;
}

long long PeriodicCommand::GetIntervalMS() const
{
    return m_pause->GetDurationMS();
}

void PeriodicCommand::SetIntervalMS(long long milliseconds)
{
	m_initialPause->SetDurationMS(milliseconds);
	m_pause->SetDurationMS(milliseconds);
}

void PeriodicCommand::Stop()
{
    m_repeatCount = 0;
    SkipCurrentWait();
}

void PeriodicCommand::SkipCurrentWait()
{
    m_initialPause->CutShort();
    m_pause->CutShort();
}

void PeriodicCommand::Reset()
{
    m_initialPause->Reset();
    m_pause->Reset();
}

std::string PeriodicCommand::ExtendedDescription() const
{
	return "Repetitions: " + std::to_string(m_repeatCount) + "; Interval: " + std::to_string(GetIntervalMS()) + "ms";
}

void PeriodicCommand::SyncExeImpl()
{
    if (m_startWithPause && m_repeatCount > 0)
    {
        m_initialPause->SyncExecute();
    }

    for (size_t i = 0; i < m_repeatCount; ++i)
    {
		if (m_stopEvent.get() != nullptr && m_stopEvent->IsSignaled())
		{
			break;
		}

        CheckAbortFlag();

        if (i == m_repeatCount - 1)
        {
            // Don't pause for the last execution
            long long prevInterval = m_pause->GetDurationMS();
            m_pause->SetDurationMS(0);

            try
            {
                m_collectionCmd->SyncExecute();
            }
			catch (...)
            {
                m_pause->SetDurationMS(prevInterval);
				throw;
            }
		
			m_pause->SetDurationMS(prevInterval);
		}
        else
        {
            m_collectionCmd->SyncExecute();
        }
    }
}
