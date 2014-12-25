#include "RetryableCommand.h"
#include "CommandAbortedException.h"

using namespace CommandLib;

std::string RetryableCommand::ClassName() const
{
	return "RetryableCommand";
}

RetryableCommand::RetryCallback::~RetryCallback()
{
}

RetryableCommand::Ptr RetryableCommand::Create(Command::Ptr command, RetryCallback* callback)
{
	return Ptr (new RetryableCommand(command, callback));
}

RetryableCommand::RetryableCommand(Command::Ptr command, RetryCallback* callback)
	: m_command(command), m_callback(callback), m_pauseCmd(PauseCommand::Create(0))
{
	TakeOwnership(m_pauseCmd);
    TakeOwnership(m_command);
}

void RetryableCommand::SyncExeImpl()
{
    size_t i = 0;

	for (;;)
    {
        try
        {
            CheckAbortFlag();
            m_command->SyncExecute();
			return;
        }
        catch(CommandAbortedException&)
        {
            throw;
        }
        catch(std::exception& exc)
        {
            long long waitTime;

            if (!m_callback->OnCommandFailed(++i, exc, &waitTime))
            {
                throw;
            }

            m_pauseCmd->SetDurationMS(waitTime);
            m_pauseCmd->SyncExecute();
        }
    }
}
