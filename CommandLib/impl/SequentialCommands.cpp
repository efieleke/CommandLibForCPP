#include "SequentialCommands.h"

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

void SequentialCommands::SyncExeImpl()
{
	for (Command::Ptr cmd : m_commands)
	{
        CheckAbortFlag();
        cmd->SyncExecute();
    }
}
