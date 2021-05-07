#pragma once
#include "SyncCommand.h"

namespace CommandLib
{
	/// <summary>
	/// SequentialCommands is a <see cref="Command"/> object which contains a collection of commands which are run in sequence
	/// </summary>
	class SequentialCommands : public Command
    {
	public:
		/// <summary>Shared pointer to a non-modifyable SequentialCommands object</summary>
		typedef std::shared_ptr<const SequentialCommands> ConstPtr;

		/// <summary>Shared pointer to a SequentialCommands object</summary>
		typedef std::shared_ptr<SequentialCommands> Ptr;

		/// <summary>
		/// Creates a SequentialCommands object
		/// </summary>
		static Ptr Create();

		/// <summary>Adds a <see cref="Command"/> to the collection to execute.</summary>
		/// <param name="command">The command to add</param>
		/// <remarks>
		/// This object takes ownership of any commands that are added, so the passed command must not already have an owner.
		/// <para>Behavior is undefined if you add a command while this SeqentialCommands object is executing</para>
		/// </remarks>
		void Add(Command::Ptr command);

		/// <summary>
		/// Empties all commands from the collection. Behavior is undefined if you call this while this command is executing.
		/// </summary>
		void Clear();

		/// <summary>
		/// Returns diagnostic information about this object's state
		/// </summary>
		/// <returns>
		/// The returned text includes the number of commands in the collection
		/// </returns>
		virtual std::string ExtendedDescription() const override;

		/// <inheritdoc/>
		virtual std::string ClassName() const override;

		/// <inheritdoc/>
		virtual bool IsNaturallySynchronous() const final;
	protected:
		/// <summary>
		/// This constructor is not public so as to enforce creation using the Create() methods.
		/// </summary>
		SequentialCommands();
	private:
		class Listener : public CommandListener
		{
		public:
			void CommandSucceeded() override;
			void CommandAborted() override;
			void CommandFailed(const std::exception& exc, std::exception_ptr excPtr);
			void SetExternalListener(CommandListener* listener);
			void SetCommands(std::list<Command::Ptr>::iterator iter, std::list<Command::Ptr>::iterator end);
		private:
			std::list<Command::Ptr>::iterator m_iter;
			std::list<Command::Ptr>::iterator m_end;
			CommandListener* volatile m_externalListener = nullptr;
		};

		virtual void SyncExecuteImpl() final;
		virtual void AsyncExecuteImpl(CommandListener* listener) final;
		void DoAsyncExecute(CommandListener* listener, std::list<Command::Ptr>::iterator iter, std::list<Command::Ptr>::iterator end);

        std::list<Command::Ptr> m_commands;
		Listener m_listener;
	};
}
