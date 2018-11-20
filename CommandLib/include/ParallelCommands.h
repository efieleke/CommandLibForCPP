#pragma once;
#include "AsyncCommand.h"
#include "CommandListener.h"
#include "SyncCommand.h"
#include <atomic>
#include <vector>

namespace CommandLib
{
	/// <summary>Represents a collection of <see cref="Command"/> objects that execute in parallel, wrapped in a <see cref="Command"/> object</summary>
	class ParallelCommands : public AsyncCommand
    {
	public:
		/// <summary>Shared pointer to a non-modifyable ParallelCommands object</summary>
		typedef std::shared_ptr<const ParallelCommands> ConstPtr;

		/// <summary>Shared pointer to a ParallelCommands object</summary>
		typedef std::shared_ptr<ParallelCommands> Ptr;

		/// <summary>
		/// Creates a ParallelCommands object as a top-level <see cref="Command"/>
		/// </summary>
		/// <param name="abortUponFailure">
		/// If true, and any <see cref="Command"/> within the collection fails, the rest of the executing commands will immediately be aborted
		/// </param>
		static Ptr Create(bool abortUponFailure);

		virtual ~ParallelCommands();

		/// <summary>Adds a <see cref="Command"/> to the collection to execute.</summary>
		/// <param name="command">The command to add</param>
		/// <remarks>
		/// This object takes ownership of any commands that are added, so the passed command must not already have an owner.
		/// <para>Behavior is undefined if you add a command while this ParallelCommands object is executing</para>
		/// <para>If multiple commands fail, only the first failure reason is reported via <see cref="CommandListener"/>.</para>
		/// </remarks>
		void Add(Command::Ptr command);

		/// <summary>Empties all commands from the collection.</summary>
		/// <remarks>Behavior is undefined if you call this while this command is executing.</remarks>
		void Clear();

		/// <inheritdoc/>
		virtual std::string ExtendedDescription() const override;

		/// <inheritdoc/>
		virtual std::string ClassName() const override;
	protected:
		/// <summary>
		/// This constructor is not public so as to enforce creation using the Create() methods.
		/// </summary>
		explicit ParallelCommands(bool abortUponFailure);
	private:
		virtual void AsyncExecuteImpl(CommandListener* listener) override;

        class Listener : public CommandListener
        {
		public:
			Listener(ParallelCommands* command, CommandListener* listener);

			virtual void CommandSucceeded() final;
			virtual void CommandAborted() final;
			virtual void CommandFailed(const std::exception& exc, std::exception_ptr excPtr) final;
		private:
			Listener(const Listener&) = delete;
			Listener& operator=(const Listener&) = delete;
			void OnCommandFinished();

            CommandListener* const m_listener;
            ParallelCommands* const m_command;
            std::atomic_uint m_failCount;
			std::atomic_uint m_abortCount;
            std::atomic_uint m_remaining;
            std::exception_ptr m_error;
		};

        std::vector<Command::Ptr> m_commands;
        const bool m_abortUponFailure;
	};
}
