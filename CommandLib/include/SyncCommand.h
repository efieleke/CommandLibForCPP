#pragma once;
#include "Command.h"
#include <thread>

namespace CommandLib
{
	/// <summary>
	/// Represents a <see cref="Command"/> which is most naturally synchronous in its implementation. If you inherit from this class,
	/// you are responsible for implementing <see cref="Command::SyncExeImpl"/>. This class implements <see cref="Command::AsyncExecuteImpl"/>.
	/// </summary>
	class SyncCommand : public Command
    {
	public:
		/// <inheritdoc/>
		virtual bool IsNaturallySynchronous() const final;

		virtual ~SyncCommand();
	private:
		static void ExecuteRoutine(SyncCommand* syncCmd, CommandListener* listener);

		/// <summary>
		/// This will be called just before command execution, on the same thread from which SyncExecute or AsyncExecute
		/// was called.
		/// </summary>
		/// <remarks>
		/// Implementations may initialize values that need to be set just before the command runs (e.g. resetting events
		/// and such). Doing so here will prevent timing issues when a command is asychronously executed followed by
		/// an immediate operation upon the command that is dependent upon it being in the executed state.
		/// </remarks>
		virtual void PrepareExecute();

		/// <summary>Executes the command and does not return until it finishes.</summary>
		/// <remarks>
		/// Implementations that take noticable time should be responsive to abort requests, if possible, by either periodically
		/// calling <see cref="Command::CheckAbortFlag"/>, or by implementating this method via calls to owned commands. In rare cases,
		/// <see cref="Command::AbortImpl"/> may need to be overridden.
		/// </remarks>
		virtual void SyncExeImpl() = 0;

		virtual void AsyncExecuteImpl(CommandListener* listener) final;
		virtual void SyncExecuteImpl() final;

		std::unique_ptr<std::thread> m_thread;
	};
}
