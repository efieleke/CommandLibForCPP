#pragma once
#include "SyncCommand.h"

namespace CommandLib
{
	/// <summary>
	/// A <see cref="Command"/> wrapper that, in addition to responding to normal <see cref="Command::Abort"/> requests, also aborts in response to either
	/// 1) a request to abort a different, specified <see cref="Command"/> instance, or 2) the signaling  of a specified <see cref="Waitable"/>
	/// (typically an <see cref="Event"/>). 
	/// </summary>
	/// <remarks>
	/// AbortLinkedCommand objects must be top level. Any attempt by another <see cref="Command"/> to take ownership of an AbortLinkedCommand
	/// will raise an exception. For example, adding this type to a <see cref="SequentialCommands"/> will raise an exception because
	/// <see cref="SequentialCommands"/> would attempt to assume ownership.
	/// </remarks>
	class AbortLinkedCommand : public SyncCommand
    {
	public:
		/// <summary>Shared pointer to a non-modifyable AbortLinkedCommand object</summary>
		typedef std::shared_ptr<const AbortLinkedCommand> ConstPtr;

		/// <summary>Shared pointer to an AbortLinkedCommand object</summary>
		typedef std::shared_ptr<AbortLinkedCommand> Ptr;

		/// <summary>
		/// Creates an AbortLinkedCommand object as a top-level <see cref="Command"/>
		/// </summary>
		/// <param name="commandToRun">
		/// The command to run. This object takes ownership of the command, so the passed command must not already have an owner.
		/// </param>
		/// <param name="abortSignal">
		/// When signaled, the command to run will be aborted. This object does not take ownership of this parameter.
		/// </param>
		static Ptr Create(Command::Ptr commandToRun, Waitable::Ptr abortSignal);

		/// <summary>
		/// Constructs an AbortLinkedCommand object as a top-level <see cref="Command"/>
		/// </summary>
		/// <param name="commandToRun">
		/// The command to run. This object takes ownership of the command, so the passed command must not already have an owner.
		/// </param>
		/// <param name="commandToWatch">
		/// When this 'commandToWatch' is aborted, the command to run will also be aborted.
		/// </param>
		static Ptr Create(Command::Ptr commandToRun, Command::ConstPtr commandToWatch);

		/// <inheritdoc/>
		virtual std::string ClassName() const override;

		/// <summary>The command that will be monitored for abort events</summary>
		/// <returns>The same value that was passed to the overloaded form of Create. This may be null, if that Create method was not used</returns>
		const Command::ConstPtr CommandToWatch() const;
	protected:
		/// <summary>
		/// This constructor is not public so as to enforce creation using the Create() methods.
		/// </summary>
		AbortLinkedCommand(Command::Ptr commandToRun, Waitable::Ptr abortEvent, Command::ConstPtr commandToWatch);
	private:
		class Listener : public CommandListener
		{
		public:
			explicit Listener(AbortLinkedCommand* cmd);
			virtual void CommandSucceeded() override final;
			virtual void CommandAborted() override final;
			virtual void CommandFailed(const std::exception& exc, std::exception_ptr excPtr) override final;
		private:
			Listener(const Listener&);
			Listener& operator= (const Listener&);
			AbortLinkedCommand* const m_cmd;
		};

		virtual void SyncExeImpl() final;
		virtual bool MustBeTopLevel() const final;
		Waitable::Ptr ExternalAbortEvent() const;

		Command::Ptr m_commandToRun;
		Waitable::Ptr m_abortEvent;
		Command::ConstPtr m_commandToWatch;
		std::exception_ptr m_lastException;
	};
}
