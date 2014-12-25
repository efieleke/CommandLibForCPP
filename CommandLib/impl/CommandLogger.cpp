#include "CommandLogger.h"
#include "CommandAbortedException.h"
#include "Command.h"
#include <chrono>
#include <iomanip>

using namespace CommandLib;

CommandLogger::CommandLogger(const std::string& filename)
{
	m_stream.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	m_stream.open(filename, std::ios_base::app);
}

void CommandLogger::CommandStarting(const Command& command)
{
    // Changing the format of this output will break the CommandLogViewer app.
    const std::string header = FormHeader(command, "Starting");
    WriteMessage(command, header);
}

void CommandLogger::CommandFinished(const Command& command, const std::exception* exc)
{
	// Changing the format of this output will break the CommandLogViewer app.
	std::string message;

    if (exc == nullptr)
    {
        message = FormHeader(command, "Completed");
    }
    else if (dynamic_cast<const CommandAbortedException*>(exc) == nullptr)
    {
		message = FormHeader(command, "Failed") + " Reason: " + exc->what();
	}
    else
    {
		message = FormHeader(command, "Aborted");
    }

    WriteMessage(command, message);
}

std::string CommandLogger::FormHeader(const Command& command, const std::string& action)
{
    long parentId = command.Parent() == nullptr ? 0 : command.Parent()->Id();
	const std::string spaces(command.Depth(), ' ');
	const std::chrono::system_clock::time_point now = std::chrono::high_resolution_clock::now();
	time_t nowAsTimeT = std::chrono::system_clock::to_time_t(now);
	char timeString[64]; // more than big enough
	timeString[0] = '\0';

	// TODO: posix does not have gmtime_s. In that envirornment, use gmtime_r
	tm asTm;
	gmtime_s(&asTm, &nowAsTimeT);
	std::strftime(timeString, sizeof(timeString), "%Y-%m-%dT%H:%M:%SZ", &asTm);

	return timeString + (" " + spaces) + std::to_string(command.Id()) + "(" + std::to_string(parentId) + ") " + action + " " + command.ClassName();
}

void CommandLogger::WriteMessage(const Command& command, std::string message)
{
	const std::string extendedInfo = command.ExtendedDescription();

    if (!extendedInfo.empty())
    {
        message += " [" + extendedInfo + "]";
    }

	std::unique_lock<std::mutex> lock(m_mutex);
	m_stream << message << std::endl;
}
