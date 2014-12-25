#pragma once;
#include <stdexcept>

namespace CommandLib
{
	/// <summary>The type of exception thrown by a <see cref="TimeLimitedCommand"/> when times runs out</summary>
	class CommandTimeoutException : public std::runtime_error
	{
	public:
		/// <summary>Constructor</summary>
		CommandTimeoutException();

		/// <summary>Constructor</summary>
		/// <param name="message">The specific error message, if desired</param>
		explicit CommandTimeoutException(const char* message);

		/// <summary>Constructor</summary>
		/// <param name="message">The specific error message, if desired</param>
		explicit CommandTimeoutException(const std::string& message);

		virtual ~CommandTimeoutException();
	};
}
