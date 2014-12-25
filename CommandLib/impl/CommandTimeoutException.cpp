#include "CommandTimeoutException.h"

using namespace CommandLib;

CommandTimeoutException::CommandTimeoutException() : std::runtime_error("Command timed out")
{
}

CommandTimeoutException::CommandTimeoutException(const char* message) : std::runtime_error(message)
{
}

CommandTimeoutException::CommandTimeoutException(const std::string& message) : std::runtime_error(message)
{
}

CommandTimeoutException::~CommandTimeoutException()
{
}
