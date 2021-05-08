#include "SequentialCommands.h"
#include "CommandAbortedException.h"
#include "AbortLinkedCommand.h"
#include <algorithm>
#include <thread>
#include <future>

using namespace CommandLib;

std::string SequentialCommands::ClassName() const
{
	return "SequentialCommands";
}

SequentialCommands::Ptr SequentialCommands::Create()
{
	return Ptr (new SequentialCommands());
}

SequentialCommands::SequentialCommands()
{
}

SequentialCommands::~SequentialCommands()
{
	if (m_thread)
	{
		m_thread->join();
	}
}

void SequentialCommands::Add(Command::Ptr command)
{
    TakeOwnership(command);
    m_commands.push_back(command);
}

void SequentialCommands::Clear()
{
	for (Command::Ptr cmd : m_commands)
    {
        RelinquishOwnership(cmd);
    }

    m_commands.clear();
}

std::string SequentialCommands::ExtendedDescription() const
{
    return "Number of commands: " + std::to_string(m_commands.size());
}

bool SequentialCommands::IsNaturallySynchronous() const
{
	return std::all_of(m_commands.begin(), m_commands.end(), [](Command::Ptr cmd) { return cmd->IsNaturallySynchronous(); });
}

class DelegateListener : public CommandListener
{
public:
	explicit DelegateListener(Event* finishedEvent) : m_finishedEvent(finishedEvent)
	{
	}

	std::exception_ptr GetError() const
	{
		return m_error;
	}

	virtual void CommandSucceeded() final
	{
		m_finishedEvent->Set();
	}

	virtual void CommandAborted() final
	{
		m_error = std::make_exception_ptr(CommandAbortedException());
		m_finishedEvent->Set();
	}

	virtual void CommandFailed(const std::exception&, std::exception_ptr excPtr)
	{
		m_error = excPtr;
		m_finishedEvent;
	}
private:
	Event* const m_finishedEvent;
	std::exception_ptr m_error;
};

void SequentialCommands::SyncExecuteImpl()
{
	std::list<Command::Ptr>::iterator it = m_commands.begin();

	while (it != m_commands.end() && (*it)->IsNaturallySynchronous())
	{
		CheckAbortFlag();
		(*it)->SyncExecute();
		++it;
	}

	if (it != m_commands.end())
	{
		// We encountered a command that is asynchronous in nature.
		CheckAbortFlag();
		Event finishedEvent(false);
		DelegateListener delegateListener(&finishedEvent);
		DoAsyncExecute(&delegateListener, it, m_commands.end());
		finishedEvent.Wait();

		if (delegateListener.GetError())
		{
			std::rethrow_exception(delegateListener.GetError());
		}
	}
}

void SequentialCommands::AsyncExecuteImpl(CommandListener* listener)
{
	if (m_commands.empty())
	{
		// No commands in the collection. We must still notify the caller on a separate thread.
		if (m_thread)
		{
			m_thread->join();
		}

		m_thread.reset(new std::thread([listener]() {listener->CommandSucceeded(); }));
		return;
	}

	// We execute the first command asynchronously regardless of whether it is naturally asynchronous, because this call must not block.
	DoAsyncExecute(listener, m_commands.begin(), m_commands.end());
}

void SequentialCommands::DoAsyncExecute(
	CommandListener* listener, std::list<Command::Ptr>::iterator iter, std::list<Command::Ptr>::iterator end)
{
	m_listener.SetExternalListener(listener);
	m_listener.SetCommands(iter, end);
	(*iter)->AsyncExecute(&m_listener);
}

void SequentialCommands::Listener::SetExternalListener(CommandListener* listener)
{
	m_externalListener = listener;
}

void SequentialCommands::Listener::SetCommands(
	std::list<Command::Ptr>::iterator iter, std::list<Command::Ptr>::iterator end)
{
	m_iter = iter;
	m_end = end;
}

void SequentialCommands::Listener::CommandSucceeded()
{
	++m_iter;

	if (m_iter == m_end)
	{
		m_externalListener->CommandSucceeded();
	}
	else if ((*m_iter)->AbortEvent()->IsSignaled())
	{
		m_externalListener->CommandAborted();
	}
	else if ((*m_iter)->IsNaturallySynchronous())
	{
		try
		{
			(*m_iter)->SyncExecute();

			// Recursive call
			CommandSucceeded();
		}
		catch (CommandAbortedException&)
		{
			m_externalListener->CommandAborted();
		}
		catch (std::exception& exc)
		{
			m_externalListener->CommandFailed(exc, std::current_exception());
		}
	}
	else
	{
		(*m_iter)->AsyncExecute(this);
	}
}

void SequentialCommands::Listener::CommandAborted()
{
	m_externalListener->CommandAborted();
}

void SequentialCommands::Listener::CommandFailed(const std::exception& exc, std::exception_ptr excPtr)
{
	m_externalListener->CommandFailed(exc, excPtr);
}
