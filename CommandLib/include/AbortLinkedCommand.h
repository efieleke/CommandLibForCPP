#pragma once
#include "SyncCommand.h"

namespace CommandLib
{
	/// <summary>
	/// A <see cref="Command"/> wrapper that, in addition to responding to normal <see cref="Command::Abort"/> requests, also aborts in response to
	/// the signaling  of a specified <see cref="Waitable"/> (typically an <see cref="Event"/>). 
	/// </summary>
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

		/// <inheritdoc/>
		virtual std::string ClassName() const override;
	protected:
		/// <summary>
		/// This constructor is not public so as to enforce creation using the Create() methods.
		/// </summary>
		AbortLinkedCommand(Command::Ptr commandToRun, Waitable::Ptr abortEvent);
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
			Listener& operator= (const Listener&) = delete;
			AbortLinkedCommand* const m_cmd;
		};

		virtual void SyncExeImpl() final;

		Command::Ptr m_commandToRun;
		Waitable::Ptr m_abortEvent;
		std::exception_ptr m_lastException;
	};
}
