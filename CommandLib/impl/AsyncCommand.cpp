#include "AsyncCommand.h"
#include "CommandAbortedException.h"

using namespace CommandLib;

void AsyncCommand::SyncExecuteImpl()
{
	m_doneEvent.Reset();
	std::unique_ptr<Listener> eventHandler(new Listener(this));
	AsyncExecuteImpl(eventHandler.get());
	m_doneEvent.Wait();
	eventHandler.release();
	
	if (m_lastException)
    {
		std::rethrow_exception(m_lastException);
	}
}

AsyncCommand::Listener::Listener(AsyncCommand* command) : m_command(command)
{
}

void AsyncCommand::Listener::CommandSucceeded()
{
	m_command->m_lastException = nullptr;
	m_command->m_doneEvent.Set();
	delete this;
}

void AsyncCommand::Listener::CommandAborted()
{
	m_command->m_lastException = std::make_exception_ptr(CommandAbortedException());
	m_command->m_doneEvent.Set();
	delete this;
}

void AsyncCommand::Listener::CommandFailed(const std::exception&, std::exception_ptr excPtr)
{
	m_command->m_lastException = excPtr;
	m_command->m_doneEvent.Set();
	delete this;
}
