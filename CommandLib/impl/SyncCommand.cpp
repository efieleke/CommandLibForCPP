#include "SyncCommand.h"
#include "CommandAbortedException.h"
#include <cassert>

using namespace CommandLib;

SyncCommand::~SyncCommand()
{
	if (m_thread)
	{
		m_thread->join();
	}
}

void SyncCommand::PrepareExecute()
{
}

bool SyncCommand::IsNaturallySynchronous() const
{
	return true;
}

void SyncCommand::AsyncExecuteImpl(CommandListener* listener)
{
    PrepareExecute();

	if (m_thread)
	{
		m_thread->join();
	}

	m_thread.reset(new std::thread(ExecuteRoutine, this, listener));
}

void SyncCommand::SyncExecuteImpl()
{
    PrepareExecute();
    SyncExeImpl();
}

void SyncCommand::ExecuteRoutine(SyncCommand* syncCmd, CommandListener* listener)
{
    try
    {
		syncCmd->CheckAbortFlag(); // not strictly necessary, but can result in a more timely abort
		syncCmd->SyncExecuteImpl();
    }
    catch (CommandAbortedException&)
    {
        listener->CommandAborted();
        return;
    }
    catch (std::exception& exc)
    {
        listener->CommandFailed(exc, std::current_exception());
        return;
    }
	catch (...)
	{
		// All exceptions thrown by Command execution must be derived from std::exception
		assert(false);
		throw;
	}

    listener->CommandSucceeded();
}
