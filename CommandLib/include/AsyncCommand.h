#pragma once;
#include "Command.h"

namespace CommandLib
{
	/// <summary>
	/// Represents a <see cref="Command"/> which is most naturally asynchronous in its implementation. If you inherit from this class, you
	/// are responsible for implementing <see cref="Command::AsyncExecuteImpl"/>. This class  implements <see cref="Command::SyncExecuteImpl"/>.
	/// </summary>
	class AsyncCommand : public Command
    {
	public:
		/// <inheritdoc/>
		virtual bool IsNaturallySynchronous() const final;
	private:
		virtual void SyncExecuteImpl() final;

        class Listener : public CommandListener
        {
		public:
			explicit Listener(AsyncCommand* command);
			virtual void CommandSucceeded() final;
			virtual void CommandAborted() final;
			virtual void CommandFailed(const std::exception& exc, std::exception_ptr excPtr) final;
		private:
			Listener(const Listener&) = delete;
			Listener& operator=(const Listener&) = delete;
            AsyncCommand* const m_command;
		};

		Event m_doneEvent;
		std::exception_ptr m_lastException;
	};
}
