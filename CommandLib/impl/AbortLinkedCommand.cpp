#include "AbortLinkedCommand.h"
#include "CommandAbortedException.h"
#include "WaitGroup.h"

using namespace CommandLib;

AbortLinkedCommand::Ptr AbortLinkedCommand::Create(Command::Ptr commandToRun, Waitable::Ptr abortEvent)
{
	return Ptr(new AbortLinkedCommand(commandToRun, abortEvent));
}

AbortLinkedCommand::AbortLinkedCommand(Command::Ptr commandToRun, Waitable::Ptr abortEvent) :
	m_commandToRun(commandToRun), m_abortEvent(abortEvent)
{
	TakeOwnership(commandToRun);
}

std::string AbortLinkedCommand::ClassName() const
{
	return "AbortLinkedCommand";
}

void AbortLinkedCommand::SyncExeImpl()
{
	{
		WaitGroup waitGroup;
		waitGroup.AddWaitable(m_commandToRun->DoneEvent());
		waitGroup.AddWaitable(m_abortEvent);
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
