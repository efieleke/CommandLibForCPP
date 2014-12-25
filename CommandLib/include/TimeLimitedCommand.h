#pragma once;
#include "SyncCommand.h"

namespace CommandLib
{
	/// <summary>
	/// This <see cref="Command"/> wraps another <see cref="Command"/>, throwing a <see cref="TimeoutException"/> if a
	/// specified interval elapses before the underlying command finishes execution.
	/// </summary>
	/// <remarks>
	/// The underlying command to execute must be responsive to abort requests in order for the timeout interval to be honored.
	/// </remarks>
	class TimeLimitedCommand : public SyncCommand
    {
	public:
		typedef std::shared_ptr<const TimeLimitedCommand> ConstPtr;
		typedef std::shared_ptr<TimeLimitedCommand> Ptr;

		/// <summary>
		/// Creates a TimeLimitedCommand object
		/// </summary>
		/// <param name="commandToRun">
		/// The command to run. This object takes ownership of the command, so the passed command must not already have
		/// an owner.
		/// </param>
		/// <param name="timeoutMS">
		/// The timeout interval, in milliseconds. The countdown does not begin until this command is executed.
		/// </param>
		static Ptr Create(Command::Ptr commandToRun, long long timeoutMS);

		/// <summary>
		/// Creates a TimeLimitedCommand object
		/// </summary>
		/// <param name="commandToRun">
		/// The command to run. This object takes ownership of the command, so the passed command must not already have
		/// an owner.
		/// </param>
		/// <param name="timeout">
		/// The timeout interval. The countdown does not begin until this command is executed.
		/// </param>
		template<typename Rep, typename Period>
		static Ptr Create(Command::Ptr commandToRun, const std::chrono::duration<Rep, Period>& timeout)
		{
			return Create(commandToRun, std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
		}

		/// <summary>
		/// Returns diagnostic information about this object's state
		/// </summary>
		/// <returns>
		/// The returned text includes the timeout duration.
		/// </returns>
		virtual std::string ExtendedDescription() const override;

		/// <inheritdoc/>
		virtual std::string ClassName() const override;
	protected:
		explicit TimeLimitedCommand(long long timeoutMS);
	private:
		virtual void SyncExeImpl() override final;

        class Listener : public CommandListener
        {
		public:
			explicit Listener(TimeLimitedCommand* command);

			virtual void CommandSucceeded() override final;
			virtual void CommandAborted() override final;
			virtual void CommandFailed(const std::exception& exc, std::exception_ptr excPtr) override final;
		private:
			Listener(const Listener&) = delete;
			Listener& operator=(const Listener&) = delete;
            TimeLimitedCommand* const m_command;
		};

		Command::Ptr m_commandToRun;
		const long long m_timeoutMS;
        std::exception_ptr m_lastException;
	};
}
