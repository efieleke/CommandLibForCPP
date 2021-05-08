#pragma once
#include "SyncCommand.h"

namespace CommandLib
{
	/// <summary>
	/// This <see cref="Command"/> wraps another command, and runs a client-specified command upon either success
	/// of failure, and optionally upon abortion.
	/// </summary>
	class FinallyCommand : public SyncCommand
	{
	public:
		/// <summary>Shared pointer to a non-modifyable PauseCommand object</summary>
		typedef std::shared_ptr<const FinallyCommand> ConstPtr;

		/// <summary>Shared pointer to a PauseCommand object</summary>
		typedef std::shared_ptr<FinallyCommand> Ptr;

		/// <summary>
		/// Creates a FinallyCommand object as a top level <see cref="Command"/>
		/// </summary>
		/// <param name="commandToRun">
		/// The command to run. This object takes ownership of the command, so the passed command must not already have
		/// an owner.
		/// </param>
		/// <param name="uponCompletionCommand">
		/// The command to run upon success or failure (and optionally upon abortion). This command must not have an owner. It will
		/// be disposed when this FinallyCommand is disposed.
		/// </param>
		/// <param name="evenUponAbort">
		/// If this value is true, uponCompletionCommand will be run even upon abortion, and uponCompletionCommand will not be responsive to abort requests of this object.
		/// If false, uponCompletionCommand will not be run upon abortion, and if it is running while an abort request is made of this object, it will be aborted.
		/// </param>
		static Ptr Create(Command::Ptr commandToRun, Command::Ptr uponCompletionCommand, bool evenUponAbort);

		/// <inheritdoc/>
		virtual std::string ClassName() const override;
	protected:
		/// <summary>
		/// This constructor is not public so as to enforce creation using the Create() methods.
		/// </summary>
		FinallyCommand(Command::Ptr commandToRun, Command::Ptr uponCompletionCommand, bool evenUponAbort);
	private:
		virtual void SyncExeImpl() override final;

		class ErrorTrappingCommand : public SyncCommand
		{
		public:
			typedef std::shared_ptr<ErrorTrappingCommand> Ptr;
			static Ptr Create(Command::Ptr commandToRun, bool trapAbort);
			virtual std::string ClassName() const override;
			std::exception_ptr m_error;
		private:
			ErrorTrappingCommand(Command::Ptr commandToRun, bool trapAbort);
			virtual void SyncExeImpl() override final;

			Command::Ptr m_commandToRun;
			const bool m_trapAbort;
		};

		ErrorTrappingCommand::Ptr m_errorTrappingCommand;
		Command::Ptr m_uponCompletionCommand;
	};
}
