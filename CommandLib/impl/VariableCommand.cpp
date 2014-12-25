#include "VariableCommand.h"

using namespace CommandLib;

VariableCommand::Ptr VariableCommand::Create()
{
	return Ptr(new VariableCommand());
}

VariableCommand::VariableCommand()
{
}

std::string VariableCommand::ClassName() const
{
	return "VariableCommand";
}

Command::ConstPtr VariableCommand::GetCommandToRun() const
{
	return m_commandToRun;
}

void VariableCommand::SetCommandToRun(Command::Ptr command)
{
	if (command.get() == m_commandToRun.get())
	{
		return;
	}

	if (m_commandToRun)
	{
		RelinquishOwnership(m_commandToRun);
	}

	if (command)
	{
		TakeOwnership(command);
	}

	m_commandToRun = command;
}

void VariableCommand::SyncExecuteImpl()
{
    m_commandToRun->SyncExecute();
}

void VariableCommand::AsyncExecuteImpl(CommandListener* listener)
{
    m_commandToRun->AsyncExecute(listener);
}
