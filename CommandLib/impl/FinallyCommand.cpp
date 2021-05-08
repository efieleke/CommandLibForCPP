#include "FinallyCommand.h"
#include "CommandAbortedException.h"

using namespace CommandLib;

FinallyCommand::Ptr FinallyCommand::Create(Command::Ptr commandToRun, Command::Ptr uponCompletionCommand, bool evenUponAbort)
{
	return Ptr(new FinallyCommand(commandToRun, uponCompletionCommand, evenUponAbort));
}

std::string FinallyCommand::ClassName() const
{
	return "FinallyCommand";
}

FinallyCommand::FinallyCommand(Command::Ptr commandToRun, Command::Ptr uponCompletionCommand, bool evenUponAbort)
	: m_errorTrappingCommand(ErrorTrappingCommand::Create(commandToRun, evenUponAbort)), m_uponCompletionCommand(uponCompletionCommand)
{
	TakeOwnership(m_errorTrappingCommand);
	TakeOwnership(m_uponCompletionCommand);
}

void FinallyCommand::SyncExeImpl()
{
	m_errorTrappingCommand->SyncExecute();

	if (m_errorTrappingCommand->m_error)
	{
		try
		{
			std::rethrow_exception(m_errorTrappingCommand->m_error);
		}
		catch (CommandAbortedException&)
		{
			ResetChildAbortEvent(m_uponCompletionCommand);
		}
		catch (...)
		{
			// deal with other error types below
		}
	}

	try
	{
		m_uponCompletionCommand->SyncExecute();
	}
	catch (...)
	{
		if (m_errorTrappingCommand->m_error)
		{
			// throw the original
			std::rethrow_exception(m_errorTrappingCommand->m_error);
		}
		
		throw;
	}

	if (m_errorTrappingCommand->m_error)
	{
		std::rethrow_exception(m_errorTrappingCommand->m_error);
	}
}

FinallyCommand::ErrorTrappingCommand::Ptr FinallyCommand::ErrorTrappingCommand::Create(Command::Ptr commandToRun, bool trapAbort)
{
	return FinallyCommand::ErrorTrappingCommand::Ptr(new FinallyCommand::ErrorTrappingCommand(commandToRun, trapAbort));
}

FinallyCommand::ErrorTrappingCommand::ErrorTrappingCommand(Command::Ptr commandToRun, bool trapAbort) :
	m_commandToRun(commandToRun), m_trapAbort(trapAbort)
{
	TakeOwnership(m_commandToRun);
}

std::string FinallyCommand::ErrorTrappingCommand::ClassName() const
{
	return "FinallyCommand::ErrorTrappingCommand";
}

void FinallyCommand::ErrorTrappingCommand::SyncExeImpl()
{
	try
	{
		m_error = nullptr;
		m_commandToRun->SyncExecute();
	}
	catch (CommandAbortedException e)
	{
		if (m_trapAbort)
		{
			m_error = std::make_exception_ptr(e);
		}
		else
		{
			throw;
		}
	}
	catch (...)
	{
		m_error = std::current_exception();
	}
}
