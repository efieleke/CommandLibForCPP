#pragma once;
#include "SyncCommand.h"
#include <chrono>

namespace CommandLib
{
	/// <summary>A <see cref="Command"/> that efficiently does nothing for a specified duration.</summary>
	class PauseCommand : public SyncCommand
    {
	public:
		typedef std::shared_ptr<const PauseCommand> ConstPtr;
		typedef std::shared_ptr<PauseCommand> Ptr;

		/// <summary>Creates a PauseCommand object as a top-level <see cref="Command"/></summary>
		/// <param name="dur">The amount of time to pause</param>
		/// <remarks>
		/// The <see cref="Event"/> used in the implemention of this class uses std::condition_variable_any. In my experience,
		/// Windows systems will not behave properly if you pass a value that equates to more milliseconds than can
		/// fit into the max value of a Windows DWORD (which is under a month long duration).
		/// </remarks>
		template<typename Rep, typename Period>
		static Ptr Create(const std::chrono::duration<Rep, Period>& dur)
		{
			return Create(std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
		}

		/// <summary>Creates a PauseCommand object as a top-level <see cref="Command"/></summary>
		/// <param name="ms">The number of milliseconds to pause.</param>
		/// <remarks>
		/// The <see cref="Event"/> used in the implemention of this class uses std::condition_variable_any. In my experience,
		/// Windows systems will not behave properly if you pass a value that equates to more milliseconds than can
		/// fit into the max value of a Windows DWORD (which is under a month long duration).
		/// </remarks>
		static Ptr Create(long long ms);

		/// <summary>Constructs a PauseCommand object as a top-level <see cref="Command"/></summary>
		/// <param name="dur">The amount of time to pause</param>
		/// <param name="stopEvent">
		/// Optional event to indicate that the PauseCommand should stop. Raising this event is equivalent to calling <see cref="CutShort"/>
		/// </param>
		/// <remarks>
		/// The <see cref="Event"/> used in the implemention of this class uses std::condition_variable_any. In my experience,
		/// Windows systems will not behave properly if you pass a value that equates to more milliseconds than can
		/// fit into the max value of a Windows DWORD (which is under a month long duration).
		/// </remarks>
		template<typename Rep, typename Period>
		static Ptr Create(const std::chrono::duration<Rep, Period>& dur, Waitable::Ptr stopEvent)
		{
			return Create(std::chrono::duration_cast<std::chrono::milliseconds>(dur).count(), stopEvent);
		}

		/// <summary>Constructs a PauseCommand object as a top-level <see cref="Command"/></summary>
		/// <param name="ms">The number of milliseconds to pause</param>
		/// <param name="stopEvent">
		/// Optional event to indicate that the PauseCommand should stop. Raising this event is equivalent to calling <see cref="CutShort"/>
		/// </param>
		/// <remarks>
		/// The <see cref="Event"/> used in the implemention of this class uses std::condition_variable_any. In my experience,
		/// Windows systems will not behave properly if you pass a value that equates to more milliseconds than can
		/// fit into the max value of a Windows DWORD (which is under a month long duration).
		/// </remarks>
		static Ptr Create(long long ms, Waitable::Ptr stopEvent);

		/// <summary>
		/// If currently executing, finishes the pause now. Does *not* cause this command to be aborted.
		/// </summary>
		void CutShort();

		/// <summary>
		/// If currently executing, starts the pause all over again, with the currently set duration value
		/// </summary>
		void Reset();
        
		/// <summary>
		/// Gets the amount of time to pause
		/// </summary>
		/// <returns>The amount of time to pause</returns>
		template<typename Rep, typename Period>
		std::chrono::duration<Rep, Period> GetDuration() const
		{
			std::chrono::duration_cast<std::chrono::duration<Rep, Period>(std::chrono::milliseconds(m_milliseconds));
		}

		/// <summary>
		/// The amount of time to pause in milliseconds
		/// </summary>
		/// <returns>The amount of time to pause</returns>
		long long GetDurationMS() const;

		/// <summary>
		/// Sets the amount of time to pause
		/// </summary>
		/// <param name="dur"/>The amount of time to pause</param>
		/// <remarks>It is safe to change this while the command is executing, but doing so will have no effect until the next time it is executed.</remarks>
		template<typename Rep, typename Period>
		void SetDuration(const std::chrono::duration<Rep, Period>& dur)
		{
			SetDurationMS(std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
		}

		/// <summary>
		/// Sets the amount of time to pause in milliseconds
		/// </summary>
		/// <param name="dur"/>The amount of milliseconds to pause</param>
		/// <remarks>It is safe to change this property while the command is executing, but doing so will have no effect until the next time it is executed.</remarks>
		void SetDurationMS(long long ms);

		/// <inheritdoc/>
		virtual std::string ExtendedDescription() const override;

		/// <inheritdoc/>
		virtual std::string ClassName() const override;
	protected:
		PauseCommand(long long ms, Waitable::Ptr stopEvent);
	private:
		virtual void PrepareExecute() override final;
		virtual void SyncExeImpl() override final;

		int WaitForDuration() const;

		Waitable::Ptr m_externalCutShortEvent;
		std::shared_ptr<Event> m_resetEvent = std::shared_ptr<Event>(new Event());
		std::shared_ptr<Event> m_cutShortEvent = std::shared_ptr<Event>(new Event());
		long long m_milliseconds;
		mutable std::mutex m_durationMutex;
	};
}
