#include "CommandDispatcher.h"
#include "CommandAbortedException.h"

using namespace CommandLib;

CommandDispatcher::CommandDispatcher(size_t maxConcurrent) : m_maxConcurrent(maxConcurrent), m_nothingToDoEvent(true)
{
    if (m_maxConcurrent == 0)
    {
        throw std::invalid_argument("maxConcurrent must be greater than 0");
    }
}

void CommandDispatcher::AddMonitor(CommandMonitor* monitor)
{
	m_monitors.push_back(monitor);
}

void CommandDispatcher::Dispatch(Command::Ptr command)
{
    if (command->Parent() != nullptr)
    {
        throw std::invalid_argument("Only top-level commands can be dispatched");
    }

	std::unique_lock<std::mutex> lock(m_mutex);
	m_nothingToDoEvent.Reset();
	m_finishedCommands.clear();

    if (m_runningCommands.size() == m_maxConcurrent)
    {
        m_commandBacklog.push(command);
    }
    else
    {
        m_runningCommands.push_back(command);
		std::unique_ptr<Listener> listener(new Listener(this, command));
		lock.unlock();

		try
		{
			command->AsyncExecute(listener.get());
			listener.release();
		}
		catch(...)
		{
			lock.lock();
			m_runningCommands.pop_back();

			if (m_runningCommands.empty())
			{
				m_nothingToDoEvent.Set();
			}

			throw;
		}
    }
}

void CommandDispatcher::Abort()
{
	std::unique_lock<std::mutex> lock(m_mutex);
    
	while (!m_commandBacklog.empty())
	{
		m_commandBacklog.pop();
	}

	for (Command::Ptr cmd : m_runningCommands)
    {
        cmd->Abort();
    }
}

void CommandDispatcher::Wait()
{
	m_nothingToDoEvent.Wait();
}

void CommandDispatcher::AbortAndWait()
{
    Abort();
    Wait();
}

CommandDispatcher::~CommandDispatcher()
{
	Wait();
}

void CommandDispatcher::OnCommandFinished(Command::Ptr command, const std::exception* exc)
{
	for (CommandLib::CommandMonitor* monitor : m_monitors)
    {
        monitor->CommandFinished(*command, exc);
    }

	std::unique_lock<std::mutex> lock(m_mutex);
    m_runningCommands.erase(std::remove(m_runningCommands.begin(), m_runningCommands.end(), command), m_runningCommands.end());

    // We cannot dispose of this command here, because it's not quite done executing yet.
    m_finishedCommands.push_back(command);

    if (m_commandBacklog.empty())
    {
        if (m_runningCommands.empty())
        {
			lock.unlock();
			m_nothingToDoEvent.Set();
        }
    }
    else
    {
        Command::Ptr nextInLine = m_commandBacklog.front();
		m_commandBacklog.pop();
        m_runningCommands.push_back(nextInLine);
		std::unique_ptr<Listener> listener(new Listener(this, nextInLine));
		lock.unlock();

		try
		{
			nextInLine->AsyncExecute(listener.get());
		}
		catch (std::exception exc)
		{
			OnCommandFinished(nextInLine, &exc);
		}

		listener.release();
    }
}

CommandDispatcher::Listener::Listener(CommandDispatcher* dispatcher, Command::Ptr command) : m_dispatcher(dispatcher), m_command(command)
{
}

void CommandDispatcher::Listener::CommandSucceeded()
{
    m_dispatcher->OnCommandFinished(m_command, nullptr);
	delete this;
}

void CommandDispatcher::Listener::CommandAborted()
{
	CommandAbortedException exc;
	m_dispatcher->OnCommandFinished(m_command, &exc);
	delete this;
}

void CommandDispatcher::Listener::CommandFailed(const std::exception& exc, std::exception_ptr)
{
	m_dispatcher->OnCommandFinished(m_command, &exc);
	delete this;
}
