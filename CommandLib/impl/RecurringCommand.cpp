#include "RecurringCommand.h"

using namespace CommandLib;

RecurringCommand::Ptr RecurringCommand::Create(Command::Ptr command, ExecutionTimeCallback* callback)
{
	return Ptr(new RecurringCommand(command, callback));
}

std::string RecurringCommand::ClassName() const
{
	return "RecurringCommand";
}

RecurringCommand::RecurringCommand(Command::Ptr command, ExecutionTimeCallback* callback)
	: m_callback(callback), m_scheduledCmd(ScheduledCommand::Create(command, std::chrono::system_clock::now(), true))
{
	TakeOwnership(m_scheduledCmd);
}

void RecurringCommand::SkipCurrentWait()
{
    m_scheduledCmd->SkipWait();
}

void RecurringCommand::SetNextExecutionTime(const std::chrono::time_point<std::chrono::system_clock>& time)
{
    m_scheduledCmd->SetTimeOfExecution(time);
}

void RecurringCommand::SyncExeImpl()
{
    std::chrono::time_point<std::chrono::system_clock> executionTime;
    bool keepGoing = m_callback->GetFirstExecutionTime(&executionTime);

    while (keepGoing)
    {
        m_scheduledCmd->SetTimeOfExecution(executionTime);
        CheckAbortFlag();
        m_scheduledCmd->SyncExecute();
        executionTime = m_scheduledCmd->GetTimeOfExecution(); // in case it was changed
        keepGoing = m_callback->GetNextExecutionTime(&executionTime);
    }
}
