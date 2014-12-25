#pragma once;
#include <stdexcept>

namespace CommandLib
{
	/// <summary>
	/// This is thrown from Command.SyncExecute() when a command is aborted.
	/// </summary>
	class CommandAbortedException : public std::runtime_error
	{
	public:
		/// <summary>
		/// Constructs a CommandAbortedException object
		/// </summary>
		CommandAbortedException();

		/// <summary>
		/// Constructs a CommandAbortedException object
		/// </summary>
		/// <param name="message">A description of the reason for abort</param>
		explicit CommandAbortedException(const char* message);

		/// <summary>
		/// Constructs a CommandAbortedException object
		/// </summary>
		/// <param name="message">A description of the reason for abort</param>
		explicit CommandAbortedException(const std::string& message);

		virtual ~CommandAbortedException();
	};
}
