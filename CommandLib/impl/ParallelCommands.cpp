#include "ParallelCommands.h"
#include "AbortLinkedCommand.h"
#include "CommandAbortedException.h"
#include <thread>
#include <future>

using namespace CommandLib;

std::string ParallelCommands::ClassName() const
{
	return "ParallelCommands";
}

ParallelCommands::Ptr ParallelCommands::Create(bool abortUponFailure)
{
	return Ptr(new ParallelCommands(abortUponFailure));
}

ParallelCommands::ParallelCommands(bool abortUponFailure) : m_abortUponFailure(abortUponFailure)
{
}

ParallelCommands::~ParallelCommands()
{
	if (m_abortUponFailure)
	{
		for (Command::Ptr cmd : m_commands)
		{
			cmd->Wait(); // because these were top level, we must make sure they're really done
		}
	}
}

void ParallelCommands::Add(Command::Ptr command)
{
    if (m_abortUponFailure)
    {
        // Because we need to abort running commands in case one of them fails,
        // and we don't want the topmost command to abort as well, we keep these commands 
        // wrapped in topmost AbortEventedCommand objects. These top level objects will
        // still respond to abort requests to this ParallelCommands object via the
        // 'this' pointer we pass as an argument.
        m_commands.push_back(AbortLinkedCommand::Create(command, shared_from_this()));
    }
    else
    {
        TakeOwnership(command);
        m_commands.push_back(command);
    }
}

void ParallelCommands::Clear()
{
	for (Command::Ptr cmd : m_commands)
    {
        if (!m_abortUponFailure)
        {
            RelinquishOwnership(cmd);
        }
    }

    m_commands.clear();
}

std::string ParallelCommands::ExtendedDescription() const
{
	return "Number of commands: " + std::to_string(m_commands.size()) + "; Abort upon failure ? " + std::to_string(m_abortUponFailure);
}

void ParallelCommands::AsyncExecuteImpl(CommandListener* listener)
{
	if (m_commands.empty())
	{
        auto _ = std::async(std::launch::async, [listener]() { listener->CommandSucceeded(); });
    }
	else
	{
		std::unique_ptr<Listener> eventHandler(new Listener(this, listener));

		for (size_t i = 0; i < m_commands.size(); ++i)
		{
			m_commands[i]->AsyncExecute(eventHandler.get());
		}

		eventHandler.release();
	}
}

ParallelCommands::Listener::Listener(ParallelCommands* command, CommandListener* listener) : m_command(command), m_listener(listener)
{
	m_failCount = 0;
	m_abortCount = 0;
    m_remaining = m_command->m_commands.size();
}

void ParallelCommands::Listener::CommandSucceeded()
{
    OnCommandFinished();
}

void ParallelCommands::Listener::CommandAborted()
{
	++m_abortCount;
    OnCommandFinished();
}

void ParallelCommands::Listener::CommandFailed(const std::exception&, std::exception_ptr excPtr)
{
    if (++m_failCount == 1)
    {
		m_error = excPtr;

        if (m_command->m_abortUponFailure)
        {
			for (Command::Ptr cmd : m_command->m_commands)
            {
                cmd->Abort();
            }
        }
    }

    OnCommandFinished();
}

void ParallelCommands::Listener::OnCommandFinished()
{
    if (--m_remaining == 0)
    {
        if (m_error)
        {
			try
			{
				std::rethrow_exception(m_error);
			}
			catch (std::exception& exc)
			{
				m_listener->CommandFailed(exc, std::current_exception());
			}
        }
        else if (m_abortCount > 0)
        {
            m_listener->CommandAborted();
        }
        else
        {
            m_listener->CommandSucceeded();
        }
    }
}
