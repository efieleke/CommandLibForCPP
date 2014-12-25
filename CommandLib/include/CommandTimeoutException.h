#pragma once;
#include <stdexcept>

namespace CommandLib
{
	/// <summary>The type of exception thrown by a <see cref="TimeLimitedCommand"/> when times runs out</summary>
	class CommandTimeoutException : public std::runtime_error
	{
	public:
		CommandTimeoutException();
		explicit CommandTimeoutException(const char* message);
		explicit CommandTimeoutException(const std::string& message);
		virtual ~CommandTimeoutException();
	};
}
