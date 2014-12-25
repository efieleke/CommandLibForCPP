#include "CommandTracer.h"
#include "Command.h"
#include "CommandAbortedException.h"

using namespace CommandLib;

CommandTracer::CommandTracer(std::ostream& os) : m_stream(os)
{
}

void CommandTracer::CommandStarting(const Command& command)
{
	const std::string parentId = command.Parent() == nullptr ? "none" : std::to_string(command.Parent()->Id());
	const std::string spaces = std::string(command.Depth(), ' ');
	const std::string message = spaces + command.ClassName() + "(" + std::to_string(command.Id()) + ") started. Parent Id: " + parentId;
    PrintMessage(command, message);
}

void CommandTracer::CommandFinished(const Command& command, const std::exception* exc)
{
	const std::string parentId = command.Parent() == nullptr ? "none" : std::to_string(command.Parent()->Id());
	const std::string spaces = std::string(command.Depth(), ' ');
	std::string message;

    if (exc == nullptr)
    {
		message = spaces + command.ClassName() + "(" + std::to_string(command.Id()) + ") succeeded. Parent Id: " + parentId;
	}
    else if (dynamic_cast<const CommandAbortedException*>(exc) == nullptr)
    {
		message = spaces + command.ClassName() + "(" + std::to_string(command.Id()) + ") failed. Parent Id: " + parentId + ". Reason: " + exc->what();
	}
    else
    {
		message = spaces + command.ClassName() + "(" + std::to_string(command.Id()) + ") aborted. Parent Id: " + parentId;
	}

    PrintMessage(command, message);
}

void CommandTracer::PrintMessage(const Command& command, std::string message)
{
    const std::string extendedInfo = command.ExtendedDescription();

	if (!extendedInfo.empty())
    {
        message += " [" + extendedInfo + "]";
    }

	m_stream << message << std::endl;
}
