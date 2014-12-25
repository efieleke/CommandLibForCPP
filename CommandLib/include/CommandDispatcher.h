#pragma once
#include "Command.h"
#include <queue>
#include <list>

namespace CommandLib
{
	/// <summary>
	/// Dispatches <see cref="Command"/> objects to a pool for execution.
	/// </summary>
	/// <remarks>
	/// This class can be useful when commands are dynamically generated at runtime, and must be dynamically executed upon generation.
	/// (for example, asynchronous handling of requests sent over a data stream).
	/// </remarks>
	class CommandDispatcher
    {
	public:
		/// <summary>
		/// Constructs a CommandDispatcher object
		/// </summary>
		/// <param name="poolSize">The maximum number of commands that can be executed concurrently by this dispatcher.</param>
		explicit CommandDispatcher(size_t poolSize);

		virtual ~CommandDispatcher();

		/// <summary>Adds a listener that will receive callbacks about the status of commands executed by this dispatcher</summary>
		void AddMonitor(CommandMonitor* monitor);

		/// <summary>
		/// If there is room in the pool, asynchronously executes the command immediately. Otherwise, places the command in a queue for processing when room in the pool becomes available.
		/// </summary>
		/// <param name="command">
		/// The command to execute as soon as there is room in the pool. The command must be top-level (that is, it must have no parent).
		/// <para>
		/// Note that it will cause undefined behavior to dispatch a <see cref="Command"/> object that is currently executing, or that has already been dispatched but has not yet executed.
		/// </para>
		/// </param>
		/// <remarks>
		/// When the command evenutally finishes execution, the <see cref="CommandMonitor"/> subscribers will be notified on a different thread.
		/// </remarks>
		void Dispatch(Command::Ptr command);

		/// <summary>
		/// Aborts all dispatched commands, and empties the queue of not yet executed commands.
		/// </summary>
		void Abort();

		/// <summary>
		/// Waits for all dispatched commands to finish execution
		/// </summary>
		void Wait();

		/// <summary>
		/// Exact same effect as calling <see cref="Abort"/> followed immediately by a call to <see cref="Wait"/>.
		/// </summary>
		void AbortAndWait();
	private:
		CommandDispatcher(const CommandDispatcher&) = delete;
		CommandDispatcher& operator= (const CommandDispatcher&) = delete;
		void OnCommandFinished(Command::Ptr command, const std::exception* exc);

        class Listener : public CommandListener
        {
		public:
			Listener(CommandDispatcher* dispatcher, Command::Ptr command);
			virtual void CommandSucceeded() override final;
			virtual void CommandAborted() override final;
			virtual void CommandFailed(const std::exception& exc, std::exception_ptr excPtr) override final;

		private:
			Listener(const Listener&) = delete;
			Listener& operator=(const Listener&) = delete;
			Command::Ptr m_command;
            CommandDispatcher* const m_dispatcher;
		};

        const size_t m_poolSize;
		std::list<CommandMonitor*> m_monitors;
        std::vector<Command::Ptr> m_runningCommands;
		std::queue<Command::Ptr> m_commandBacklog;
		std::list<Command::Ptr> m_finishedCommands;
        Event m_nothingToDoEvent;
		std::mutex m_mutex;
	};
}
