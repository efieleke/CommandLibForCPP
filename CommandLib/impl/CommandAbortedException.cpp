#include "CommandAbortedException.h"

using namespace CommandLib;

CommandAbortedException::CommandAbortedException() : std::runtime_error("Command aborted")
{
}

CommandAbortedException::CommandAbortedException(const char* message) : std::runtime_error(message)
{
}

CommandAbortedException::CommandAbortedException(const std::string& message) : std::runtime_error(message)
{
}

CommandAbortedException::~CommandAbortedException()
{
}
