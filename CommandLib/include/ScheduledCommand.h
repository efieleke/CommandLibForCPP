#pragma once;
#include "SyncCommand.h"
#include "PauseCommand.h"

namespace CommandLib
{
	/// <summary>
	/// Represents a <see cref="Command"/> that executes at a given time. When a ScheduledCommand is executed, it will enter an
	/// efficient wait state until the time arrives at which to execute the underlying command.
	/// </summary>
	class ScheduledCommand : public SyncCommand
    {
	public:
		/// <summary>Shared pointer to a non-modifyable ScheduledCommand object</summary>
		typedef std::shared_ptr<const ScheduledCommand> ConstPtr;

		/// <summary>Shared pointer to a ScheduledCommand object</summary>
		typedef std::shared_ptr<ScheduledCommand> Ptr;

		/// <summary>
		/// Creates a ScheduledCommand
		/// </summary>
		/// <param name="command">
		/// The command to run. This object takes ownership of the command, so the passed command must not already have
		/// an owner. The passed command will be disposed when this ScheduledCommand object is disposed.
		/// </param>
		/// <param name="timeOfExecution">
		/// The time at which to execute the command to run. Note that unless this ScheduledCommand object is actually executed, the command to run will never execute.
		/// </param>
		/// <param name="runImmediatelyIfTimeIsPast">
		/// If, when this ScheduledCommand is executed, the time of execution is in the past, it will execute immediately if this parameter is set to true
		/// (otherwise it will throw an InvalidOperation exception).
		/// </param>
		static Ptr Create(
			Command::Ptr command,
			const std::chrono::time_point<std::chrono::system_clock>& timeOfExecution,
			bool runImmediatelyIfTimeIsPast);

		/// <summary>Gets the time at which to execute the command to run</summary>
		/// <returns>
		/// The time at which to execute the command to run. Note that unless this ScheduledCommand object is actually executed, the command to run will never execute.
		/// </returns>
		std::chrono::time_point<std::chrono::system_clock> GetTimeOfExecution() const;

		/// <summary>Sets the time at which to execute the command to run</summary>
		/// <param name="time">
		/// The time at which to execute the command to run. Note that unless this ScheduledCommand object is actually executed, the command to run will never execute.
		/// </param>
		/// </returns>
		/// <remarks>
		/// It is safe to change this property while this command is executing, although if the underlying command
		/// to run has already begun execution, it will have no effect.
		/// </remarks>
		void SetTimeOfExecution(const std::chrono::time_point<std::chrono::system_clock>& time);

		/// <summary>Skips the current wait time before the execution of the underlying command and executes it immediately</summary>
		/// <remarks>This is a no-op if this ScheduledCommand object is not currently executing</remarks>
		void SkipWait();

		/// <summary>
		/// Returns diagnostic information about this object's state
		/// </summary>
		/// <returns>
		/// The returned text includes the time to execute as well as whether to run immediately if the scheduled time is in the past.
		/// </returns>
		virtual std::string ExtendedDescription() const override;

		/// <inheritdoc/>
		virtual std::string ClassName() const override;
	protected:
		/// <summary>
		/// This constructor is not public so as to enforce creation using the Create() methods.
		/// </summary>
		ScheduledCommand(
			Command::Ptr command,
			const std::chrono::time_point<std::chrono::system_clock>& timeOfExecution,
			bool runImmediatelyIfTimeIsPast);
	private:
		virtual void SyncExeImpl() override final;

		PauseCommand::Ptr m_pauseCmd;
        Command::Ptr m_command;
        const bool m_runImmediatelyIfTimeIsPast;
		std::chrono::time_point<std::chrono::system_clock> m_timeOfExecution;
		mutable std::mutex m_mutex;
	};
}
