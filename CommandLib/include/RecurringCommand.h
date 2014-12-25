#pragma once;
#include "SyncCommand.h"
#include "ScheduledCommand.h"

namespace CommandLib
{
	/// <summary>Represents a <see cref="Command"/> that repeatedly executes at times specified by the caller</summary>
	/// <remarks>
	/// If the interval between execution times is fixed, it would be simpler to use <see cref="PeriodicCommand"/> instead.
	/// </remarks>
	class RecurringCommand : public SyncCommand
    {
	public:
		/// <summary>
		/// Defines at what times the underlying command executes
		/// </summary>
		class ExecutionTimeCallback
        {
		public:
			virtual ~ExecutionTimeCallback() {}

			/// <summary>
			/// Called when a RecurringCommand needs to know the first time to execute its underlying command to run.
			/// </summary>
			/// <param name="time">
			/// Implementations should set this to the next time to execute. If a time in the past is specified, the command to run
			/// will execute immediately. However, if this method returns false, the value set here will be ignored.
			/// </param>
			/// <returns>
			/// Implementations should return true to indicate that the command to run should be executed at the provided time. Returning false causes the
			/// RecurringCommand to finish execution.
			/// </returns>
			virtual bool GetFirstExecutionTime(std::chrono::time_point<std::chrono::system_clock>* time) = 0;

			/// <summary>
			/// Called when a RecurringCommand needs to know the next time to execute its underlying command to run.
			/// </summary>
			/// <param name="time">
			/// This will be initialized to the last time the command to run was set to begin execution. Implementations
			/// should set this to the next time to execute. If a time in the past is specified, the command to run will execute
			/// immediately. However, if this method returns false, the value set here will be ignored.
			/// </param>
			/// <returns>
			/// Implementations should return true to indicate that the command to run should be executed at the provided time.
			/// Returning false causes the RecurringCommand to finish execution.
			/// </returns>
			virtual bool GetNextExecutionTime(std::chrono::time_point<std::chrono::system_clock>* time) = 0;
		};

		typedef std::shared_ptr<const RecurringCommand> ConstPtr;
		typedef std::shared_ptr<RecurringCommand> Ptr;

		/// <summary>
		/// Creates a RecurringCommand object
		/// </summary>
		/// <param name="command">
		/// The command to run. This object takes ownership of the command, so the passed command must not already have
		/// an owner.
		/// </param>
		/// <param name="callback">Defines at what times the underlying command executes</param>
		static Ptr Create(Command::Ptr command, ExecutionTimeCallback* callback);

		/// <summary>If currently waiting until the time to next execute the command to run, skip the wait and execute the command right away.</summary>
		/// <remarks>This is a no-op if this ScheduledCommand object is not currently executing</remarks>
		void SkipCurrentWait();

		/// <summary>
		/// If currently waiting until the time to next execute the command to run, resets that time to the time specified.
		/// </summary>
		/// <param name="time">
		/// The time to execute the command to run. If a time in the past is specified, the command to run will execute immediately.
		/// </param>
		/// <remarks>
		/// This is a no-op if this ScheduledCommand object is not currently executing.
		/// command is executing.
		/// </remarks>
		void SetNextExecutionTime(const std::chrono::time_point<std::chrono::system_clock>& time);

		/// <inheritdoc/>
		virtual std::string ClassName() const override;
	protected:
		RecurringCommand(Command::Ptr command, ExecutionTimeCallback* callback);
	private:
		virtual void SyncExeImpl() override final;

        std::shared_ptr<ScheduledCommand> m_scheduledCmd;
        ExecutionTimeCallback* const m_callback;
	};
}
