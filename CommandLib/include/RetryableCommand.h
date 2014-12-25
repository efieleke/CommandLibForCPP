#pragma once;
#include "SyncCommand.h"
#include "PauseCommand.h"

namespace CommandLib
{
	/// <summary>
	/// This <see cref="Command"/> wraps another command, allowing the command to be retried upon failure, up to any number of times.
	/// </summary>
	class RetryableCommand : public SyncCommand
    {
	public:
		/// <summary>
		/// Interface that defines aspects of retry behavior
		/// </summary>
		class RetryCallback
        {
		public:
			virtual ~RetryCallback();

			/// <summary>
			/// Callback for when a command to be retried fails
			/// </summary>
			/// <param name="failNumber">The number of times the command has failed (including this time)</param>
			/// <param name="reason">The reason for failure.</param>
			/// <param name="waitMS">
			/// The number of milliseconds to wait before retrying. This value is ignored if the method returns false.
			/// </param>
			/// <returns>false if the command should not be retried (which will propogate the exception). Otherwise true to perform a retry after the specified wait time</returns>
			virtual bool OnCommandFailed(size_t failNumber, const std::exception& reason, long long* waitMS) = 0;
		};

		/// <summary>Shared pointer to a non-modifyable RetryableCommand object</summary>
		typedef std::shared_ptr<const RetryableCommand> ConstPtr;

		/// <summary>Shared pointer to a RetryableCommand object</summary>
		typedef std::shared_ptr<RetryableCommand> Ptr;

		/// <summary>
		/// Creates a RetryableCommand
		/// </summary>
		/// <param name="command">
		/// The command to run. This object takes ownership of the command, so the passed command must not already have
		/// an owner. The passed command will be disposed when this RetryableCommand object is disposed.
		/// </param>
		/// <param name="callback">This object defines aspects of retry behavior</param>
		static Ptr Create(Command::Ptr command, RetryCallback* callback);

		/// <inheritdoc/>
		virtual std::string ClassName() const override;
	protected:
		/// <summary>
		/// This constructor is not public so as to enforce creation using the Create() methods.
		/// </summary>
		RetryableCommand(Command::Ptr command, RetryCallback* callback);
	private:
		virtual void SyncExeImpl() override final;

        Command::Ptr m_command;
        std::shared_ptr<PauseCommand> m_pauseCmd;
        RetryCallback* const m_callback;
	};
}
