#pragma once
#include <exception>
#include <memory>

namespace CommandLib
{
	/// <summary>
	/// An object implementing this interface is required as a parameter to <see cref="Command::AsyncExecute(CommandListener*)"/>.
	/// Exactly one of its methods will eventually be called when a command is executed asynchronously, and it is guaranteed that the
	/// call will be on a thread different from the thread AsyncExecute was called from.
	/// </summary>
	/// <remarks>
	/// The <see cref="Command"/> is in the last stage of execution when making these callbacks, so do not re-execute the command from within
	/// your handler. Also, do not call the executing command's <see cref="Command::Wait"/> from within your handler, as that will cause deadlock.
	/// </remarks>
	class CommandListener
    {
	public:
		virtual ~CommandListener();

		/// <summary>
		/// Called when a <see cref="Command"/> launched via <see cref="Command::AsyncExecute(CommandListener*)"/> succeeds.
		/// </summary>
		/// <remarks>
		/// The <see cref="Command"/> is in the last stage of execution when making this callback, so do not re-execute the command from within
		/// your handler. Also, do not call the executing command's <see cref="Command::Wait"/> method from within your handler, as that will cause deadlock.
		/// </remarks>
		virtual void CommandSucceeded() = 0;

		/// <summary>
		/// Called when a <see cref="Command"/> launched via <see cref="Command::AsyncExecute(CommandListener*)"/> was aborted.
		/// </summary>
		/// <remarks>
		/// The <see cref="Command"/> is in the last stage of execution when making this callback, so do not re-execute the command from within
		/// your handler. Also, do not call the executing command's <see cref="Command::Wait"/> method from within your handler, as that will cause deadlock.
		/// </remarks>
		virtual void CommandAborted() = 0;

		/// Called when a <see cref="Command"/> launched via <see cref="Command::AsyncExecute(CommandListener*)"/> fails.
		/// </summary>
		/// <param name="exc">The reason for failure</param>
		/// <param name="excPtr">
		/// Allows you to clone the actual error object by calling std::rethrow_exception. This can be useful if you find yourself
		// needing to store the exception object for later.
		/// </param>
		/// <remarks>
		/// The <see cref="Command"/> is in the last stage of execution when making this callback, so do not re-execute the command from within
		/// your handler. Also, do not call the executing command's <see cref="Command::Wait"/> method from within your handler, as that will cause deadlock.
		/// </remarks>
		virtual void CommandFailed(const std::exception& exc, std::exception_ptr excPtr) = 0;
	};
}
