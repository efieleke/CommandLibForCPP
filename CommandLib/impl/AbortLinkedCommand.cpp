#include "AbortLinkedCommand.h"
#include "CommandAbortedException.h"
#include "WaitGroup.h"

using namespace CommandLib;

AbortLinkedCommand::Ptr AbortLinkedCommand::Create(Command::Ptr commandToRun, Waitable::Ptr abortEvent)
{
	return Ptr(new AbortLinkedCommand(commandToRun, abortEvent, Command::ConstPtr()));
}

AbortLinkedCommand::Ptr AbortLinkedCommand::Create(Command::Ptr commandToRun, Command::ConstPtr commandToWatch)
{
	return Ptr(new AbortLinkedCommand(commandToRun, Waitable::Ptr(), commandToWatch));
}

AbortLinkedCommand::AbortLinkedCommand(Command::Ptr commandToRun, Waitable::Ptr abortEvent, Command::ConstPtr commandToWatch) :
	m_commandToRun(commandToRun), m_abortEvent(abortEvent), m_commandToWatch(commandToWatch)
{
	TakeOwnership(commandToRun);
}

std::string AbortLinkedCommand::ClassName() const
{
	return "AbortLinkedCommand";
}

const Command::ConstPtr AbortLinkedCommand::CommandToWatch() const
{
     return m_commandToWatch;
}

void AbortLinkedCommand::SyncExeImpl()
{
	{
		WaitGroup waitGroup;
		waitGroup.AddWaitable(m_commandToRun->DoneEvent());
		waitGroup.AddWaitable(ExternalAbortEvent());
		std::unique_ptr<Listener> listener(new Listener(this));
		m_commandToRun->AsyncExecute(listener.get());
		const int waitResult = waitGroup.WaitForAny();
		listener.release();

		if (waitResult == 1)
		{
			AbortChildCommand(m_commandToRun);
		}
	}

	m_commandToRun->Wait();

	if (m_lastException)
	{
		std::rethrow_exception(m_lastException);
	}
}

Waitable::Ptr AbortLinkedCommand::ExternalAbortEvent() const
{
	return m_abortEvent.get() == nullptr ? m_commandToWatch->AbortEvent() : m_abortEvent;
}

AbortLinkedCommand::Listener::Listener(AbortLinkedCommand* cmd) : m_cmd(cmd)
{
}

void AbortLinkedCommand::Listener::CommandSucceeded()
{
	m_cmd->m_lastException = nullptr;
	delete this;
}

void AbortLinkedCommand::Listener::CommandAborted()
{
	m_cmd->m_lastException = std::make_exception_ptr(CommandAbortedException());
	delete this;
}

void AbortLinkedCommand::Listener::CommandFailed(const std::exception&, std::exception_ptr excPtr)
{
	m_cmd->m_lastException = excPtr;
	delete this;
}
