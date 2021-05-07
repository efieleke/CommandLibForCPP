#pragma once
#include "SyncCommand.h"
#include "PauseCommand.h"
#include <atomic>

namespace CommandLib
{
	/// <summary>Represents a <see cref="Command"/> that repeats periodically at a specified interval</summary>
	/// <remarks>
	/// If more dynamic control is needed around the period of time between executions, use <see cref="RecurringCommand"/> instead.
	/// </para>
	/// </remarks>
	class PeriodicCommand : public SyncCommand
    {
	public:
		/// <summary>
		/// Defines how the interval between command executions is performed within a <see cref="PeriodicCommand"/>
		/// </summary>
		enum class IntervalType
		{
			/// <summary>
			/// Pause first, then run the command, and repeat as many times as specified
			/// </summary>
			PauseBefore,

			/// <summary>
			/// Run the command first, then pause, and repeat as many times as specified. There is no pause after the last
			/// execution of the command.
			/// </summary>
			PauseAfter
		};

		/// <summary>Shared pointer to a non-modifyable PeriodicCommand object</summary>
		typedef std::shared_ptr<const PeriodicCommand> ConstPtr;

		/// <summary>Shared pointer to a PeriodicCommand object</summary>
		typedef std::shared_ptr<PeriodicCommand> Ptr;

		/// <summary>
		/// Creates a PeriodicCommand
		/// </summary>
		/// <param name="command">
		/// The command to run periodically. This object takes ownership of the command, so the passed command must not already have
		/// an owner. The passed command will be disposed when this PeriodicCommand object is disposed.
		/// </param>
		/// <param name="repeatCount">The number of times to repeat the command</param>
		/// <param name="interval">The interval of time between repetitions</param>
		/// <param name="intervalType">Specifies whether the pause interval occurs before or after the command executes</param>
		/// <param name="intervalIsInclusive">
		/// If false, the interval represents the time between when the command finishes and when it starts next.
		/// If true, the interval represents the time between the start of successive command executions (in this case, if the
		/// command execution takes longer than the interval, the next repetition will start immediately).
		/// </param>
		template<typename Rep, typename Period>
		static Ptr Create(
			Command::Ptr command,
			size_t repeatCount,
			std::chrono::duration<Rep, Period> interval,
			IntervalType intervalType,
			bool intervalIsInclusive)
		{
			return Create(
				command,
				repeatCount,
				std::chrono::duration_cast<std::chrono::milliseconds>(interval).count(),
				intervalType,
				intervalIsInclusive);
		}

		/// <summary>
		/// Creates a PeriodicCommand
		/// </summary>
		/// <param name="command">
		/// The command to run periodically. This object takes ownership of the command, so the passed command must not already have
		/// an owner. The passed command will be disposed when this PeriodicCommand object is disposed.
		/// </param>
		/// <param name="repeatCount">The number of times to repeat the command</param>
		/// <param name="intervalMS">The number of milliseconds between repetitions</param>
		/// <param name="intervalType">Specifies whether the pause interval occurs before or after the command executes</param>
		/// <param name="intervalIsInclusive">
		/// If false, the interval represents the time between when the command finishes and when it starts next.
		/// If true, the interval represents the time between the start of successive command executions (in this case, if the
		/// command execution takes longer than the interval, the next repetition will start immediately).
		/// </param>
		static Ptr Create(
			Command::Ptr command,
			size_t repeatCount,
			long long intervalMS,
			IntervalType intervalType,
			bool intervalIsInclusive);

		/// <summary>
		/// Creates a PeriodicCommand
		/// </summary>
		/// <param name="command">
		/// The command to run periodically. This object takes ownership of the command, so the passed command must not already have
		/// an owner. The passed command will be disposed when this PeriodicCommand object is disposed.
		/// </param>
		/// <param name="repeatCount">The number of times to repeat the command</param>
		/// <param name="interval">The interval of time between repetitions</param>
		/// <param name="intervalType">Specifies whether the pause interval occurs before or after the command executes</param>
		/// <param name="intervalIsInclusive">
		/// If false, the interval represents the time between when the command finishes and when it starts next.
		/// If true, the interval represents the time between the start of successive command executions (in this case, if the
		/// command execution takes longer than the interval, the next repetition will start immediately).
		/// </param>
		/// <param name="stopEvent">
		/// Event to indicate that the perdiodic command should stop. Raising this event is equivalent to calling <see cref="Stop"/>
		/// You can pass the <see cref="Command::DoneEvent"/> of a different <see cref="Command"/> as the stop event, which will cause this
		/// periodic command to stop when the other command finishes, but be sure that the other command begins execution before this command
		/// if you choose to do this.
		/// </param>
		template<typename Rep, typename Period>
		static Ptr Create(
			Command::Ptr command,
			size_t repeatCount,
			std::chrono::duration<Rep, Period> interval,
			IntervalType intervalType,
			bool intervalIsInclusive,
			Waitable::Ptr stopEvent)
		{
			return Create(
				command,
				repeatCount,
				std::chrono::duration_cast<std::chrono::milliseconds>(interval).count(),
				intervalType,
				intervalIsInclusive,
				stopEvent);
		}

		/// <summary>
		/// Creates a PeriodicCommand
		/// </summary>
		/// <param name="command">
		/// The command to run periodically. This object takes ownership of the command, so the passed command must not already have
		/// an owner. The passed command will be disposed when this PeriodicCommand object is disposed.
		/// </param>
		/// <param name="repeatCount">The number of times to repeat the command</param>
		/// <param name="intervalMS">The number of milliseconds between repetitions</param>
		/// <param name="intervalType">Specifies whether the pause interval occurs before or after the command executes</param>
		/// <param name="intervalIsInclusive">
		/// If false, the interval represents the time between when the command finishes and when it starts next.
		/// If true, the interval represents the time between the start of successive command executions (in this case, if the
		/// command execution takes longer than the interval, the next repetition will start immediately).
		/// </param>
		/// <param name="stopEvent">
		/// Event to indicate that the perdiodic command should stop. Raising this event is equivalent to calling <see cref="Stop"/>
		/// You can pass the <see cref="Command::DoneEvent"/> of a different <see cref="Command"/> as the stop event, which will cause this
		/// periodic command to stop when the other command finishes, but be sure that the other command begins execution before this command
		/// if you choose to do this.
		/// </param>
		static Ptr Create(
			Command::Ptr command,
			size_t repeatCount,
			long long intervalMS,
			IntervalType intervalType,
			bool intervalIsInclusive,
			Waitable::Ptr stopEvent);

		/// <summary>
		/// Gets the interval of time between command executions.
		/// </summary>
		/// <returns> The interval of time between command executions</returns>
		template<typename Unit>
		Unit GetInterval() const
		{
			return std::chrono::duration_cast<Unit>(std::chrono::milliseconds(GetIntervalMS()));
		}

		/// <summary>
		/// Sets the interval of time between command executions.
		/// </summary>
		/// <param name="interval">the interval of time between command executions</param>
		/// <remarks>It is safe to change this property while the command is executing</remarks>
		template<typename Rep, typename Period>
		void SetInterval(const std::chrono::duration<Rep, Period>& interval)
		{
			SetIntervalMS(std::chrono::duration_cast<std::chrono::milliseconds>(interval).count());
		}

		/// <summary>
		/// Gets the interval of time between command executions in milliseconds.
		/// </summary>
		/// <returns> The interval of time between command executions in milliseconds</returns>
		long long GetIntervalMS() const;

		/// <summary>
		/// Sets the number of milliseconds between command executions.
		/// </summary>
		/// <param name="interval">the number of milliseconds between command executions</param>
		/// <remarks>It is safe to change this property while the command is executing</remarks>
		void SetIntervalMS(long long interval);

		/// <summary>
		/// Causes the command to stop repeating. This will not cause the command to be aborted. If the command to run
		/// is currently executing when this is called, it will be allowed to finish.
		/// </summary>
		/// <remarks>This is a no-op if this PeriodicCommand instance is not currently executing</remarks>
		void Stop();

		/// <summary>
		/// If currently in the interval between command executions, skips the wait and executes the command right away.
		/// This only skips the current wait. It will not skip subsequent waits.
		/// </summary>
		void SkipCurrentWait();

		/// <summary>
		/// Rewinds the current pause to its full duration.
		/// </summary>
		void Reset();

		/// <summary>
		/// Returns diagnostic information about this object's state
		/// </summary>
		/// <returns>
		/// The returned text includes the repetition count, the duration between executions, whether to start with a pause,
		/// as well as whether an external stop event is defined
		/// </returns>
		virtual std::string ExtendedDescription() const override;

		/// <inheritdoc/>
		virtual std::string ClassName() const override;

		/// <summary>
		/// The total number of times the command to run will execute. If the command to run is currently executing when this is
		/// changed, it will be allowed to finish, even if the repeat count is set to a number lower than the number of times
		/// already executed.
		/// </summary>
		/// <remarks>It is safe to change this member while this command is executing</remarks>
		std::atomic_uint m_repeatCount;
	protected:
		/// <summary>
		/// This constructor is not public so as to enforce creation using the Create() methods.
		/// </summary>
		PeriodicCommand(
			Command::Ptr command,
			size_t repeatCount,
			long long intervalMS,
			IntervalType intervalType,
			bool intervalIsInclusive,
			Waitable::Ptr stopEvent);
	private:
		virtual void SyncExeImpl() override final;
        
		PauseCommand::Ptr m_pause;
		PauseCommand::Ptr m_initialPause;
        Command::Ptr m_collectionCmd;
        bool m_startWithPause;
		Waitable::Ptr m_stopEvent;
	};
}
