#include "TimeLimitedCommand.h"
#include "AbortLinkedCommand.h"
#include "CommandTimeoutException.h"
#include "CommandAbortedException.h"

using namespace CommandLib;

std::string TimeLimitedCommand::ClassName() const
{
	return "TimeLimitedCommand";
}

TimeLimitedCommand::Ptr TimeLimitedCommand::Create(Command::Ptr commandToRun, long long timeoutMS)
{
	Ptr result(new TimeLimitedCommand(timeoutMS));
	result->m_commandToRun = AbortLinkedCommand::Create(commandToRun, result);
	return result;
}

TimeLimitedCommand::TimeLimitedCommand(long long timeoutMS) : m_timeoutMS(timeoutMS)
{
}

std::string TimeLimitedCommand::ExtendedDescription() const
{
	return "Timeout MS: " + std::to_string(m_timeoutMS);
}

void TimeLimitedCommand::SyncExeImpl()
{
	std::unique_ptr<Listener> eventHandler(new Listener(this));
	m_commandToRun->AsyncExecute(eventHandler.get());
	eventHandler.release();
	const bool finished = m_commandToRun->DoneEvent()->Wait(m_timeoutMS);

    if (!finished)
    {
		m_commandToRun->AbortAndWait();
		throw CommandTimeoutException("Timed out after waiting " + std::to_string(m_timeoutMS) + "ms for command '" + m_commandToRun->Description() + "' to finish");
    }

    if (m_lastException)
    {
		std::rethrow_exception(m_lastException);
    }
}

TimeLimitedCommand::Listener::Listener(TimeLimitedCommand* command) : m_command(command)
{
}

void TimeLimitedCommand::Listener::CommandSucceeded()
{
	m_command->m_lastException = nullptr;
	delete this;
}

void TimeLimitedCommand::Listener::CommandAborted()
{
	m_command->m_lastException = std::make_exception_ptr(CommandAbortedException());
	delete this;
}

void TimeLimitedCommand::Listener::CommandFailed(const std::exception&, std::exception_ptr excPtr)
{
	m_command->m_lastException = excPtr;
	delete this;
}
