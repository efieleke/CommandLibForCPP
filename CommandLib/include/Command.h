#pragma once
#include "CommandMonitor.h"
#include "CommandListener.h"
#include <string>
#include <memory>
#include <set>
#include <chrono>
#include <mutex>
#include <atomic>
#include <list>
#include <map>
#include <vector>
#include "Event.h"
#include <thread>

/*! \mainpage CommandLib for C++
*
* \section intro_sec Introduction
*
* This project contains classes that simplify coordination of asynchronous and synchronous activities. To get a better feel for
* features and usage, read the CommandLib namespace detailed documentation. The CommandLibSample project provides
* example usage.
*
* Note that this was compiled using Visual Studio 2013. The unit tests make use of Microsoft-specific classes.
*/

/// <summary>
/// CommandLib contains a set of classes that can be used to easily coordinate synchronous and asynchronous activities in
/// complex ways. Most classes in this library inherit from <see cref="Command"/>, which represents an action. Any
/// <see cref="Command"/> can be run synchronously or asynchronously, and may be aborted.
/// <para>
/// Using <see cref="ParallelCommands"/>, you can run a collection of <see cref="Command"/> objects concurrently, and using
/// <see cref="SequentialCommands"/>, you can run a collection of <see cref="Command"/> objects in sequence. Any command
/// can be added to these two types (including <see cref="ParallelCommands"/> and <see cref="SequentialCommands"/> themselves,
/// because they are <see cref="Command"/> objects), so it's possible to create a deep nesting of coordinated activities.
/// </para>
/// <para>
/// <see cref="PeriodicCommand"/> repeats its action at a given interval, <see cref="ScheduledCommand"/> runs once at a specific
/// time, and <see cref="RecurringCommand"/> runs at times that are provided via a callback.
/// </para>
/// <para>
/// <see cref="RetryableCommand"/> provides the option to keep retrying a failed command until the caller decides enough is enough,
/// and <see cref="TimeLimitedCommand"/> fails with a timeout exception if a given duration elapses before the command finishes
/// execution.
/// </para>
/// <para>
/// All of the above <see cref="Command"/> classes are simply containers for <see cref="Command"/> objects that presumably do
/// something of interest. It is expected that users of this library will create their own <see cref="Command"/>-derived classes.
/// </para>
/// <para>
/// <see cref="CommandDispatcher"/> provides the capability to set up a pool for command execution.
/// </para>
/// <para>
/// Documentation for <see cref="Command"/>, <see cref="AsyncCommand"/> and <see cref="SyncCommand"/> should be read before
/// developing a <see cref="Command"/>-derived class. <see cref="AbortLinkedCommand"/> might also serve as an aid in the development
/// of a <see cref="Command"/>.
/// </para>
/// <para>
/// Windows implementations must be careful when specifying durations. The implementations of various routines in CommandLib
/// make use of std::condition_variable_any. In my experience, Windows systems will not behave properly if you pass a value
/// that equates to more milliseconds than can fit into the max value of a Windows DWORD (which is under a month long duration).
/// This could be considered a bug in std::condition_variable_any. I have done nothing to work around it, so for now, it's up
/// to users of this library to be aware of this.
/// </para>
/// </summary>
namespace CommandLib
{
	/// <summary>Represents an action that can be run synchronously or asynchronously.</summary>
	/// <remarks>
	/// Commands are abortable. Even a synchronously running command can be aborted from a separate thread.
	/// <para>
	/// Command types are only instantiable via static Create() methods. This is because commands often own other commands.
	/// It was necessary to either enforce clone interface support, or enforce working with smart pointers. Smart pointers felt like the
	/// easier approach for clients (writing clone methods is a nuisance).
	/// </para>
	/// <para>
	/// When developing a Command subclass, be sure to inherit from either <see cref="SyncCommand"/> (if your command is naturally
	/// synchronous in its implementation) or <see cref="AsyncCommand"/>. Those classes take care of the unnatural implementations
	/// (<see cref="SyncCommand"/> implements <see cref="AsyncExecuteImpl"/>, and <see cref="AsyncCommand"/> implements
	/// <see cref="SyncExecuteImpl"/>).
	/// </para>
	/// <para>
	/// Also, when developing a Command subclass, make sure that any member variables that are Commands are properly
	/// owned by calling <see cref="TakeOwnership"/> within your constructor body. The advantage of doing this
	/// is that owned commands will automatically respond to abort requests issued to the owner.
	/// </para>
	/// <para>
	/// If you write a method that accepts a Command as an argument, you may wish to assume ownership of that Command.
	/// <see cref="TakeOwnership"/> allows you to do this. The <see cref="SequentialCommands::Add"/> member of
	/// <see cref="SequentialCommands"/> is an example of this behavior.
	/// </para>
	/// <para>
	/// If you find that you need to create a Command object within the execution method of its owning command
	/// (perhaps because which type of Command to create depends upon runtime conditions), there are some things to
	/// consider. Owned commands are not destroyed until the owner is destroyed. If the owner is executed many times
	/// before it is destroyed, and you create a new child command upon every execution, resource usage will grow unbounded.
	/// The better approach is not assign an owner to the locally created command, but instead
	/// have it run within the context of the launching command using <see cref="SyncExecute(Command*)"/>.
	/// Alternatively, you could opt to make use of <see cref="CreateAbortLinkedCommand"/>. This will return a top-level
	/// command that responds to abort requests to the command that created it. The former is more efficient for
	/// <see cref="SyncCommand"/>-derived objects, and the latter is more efficient for <see cref="AsyncCommand"/>-derived objects.
	/// <para>
	/// Generally speaking, when authoring Commands, it's best to make them as granular as possible. That makes it much easier
	/// to reuse them while composing command structures. Also, ensure that your commands are responsive to abort requests if
	/// they take a noticeable amount of time to complete.
	/// </para>
	/// </remarks>
	class Command : public std::enable_shared_from_this<Command>
    {
	public:
		/// <summary>Shared pointer to a non-modifyable Command object</summary>
		typedef std::shared_ptr<const Command> ConstPtr;

		/// <summary>Shared pointer to a Command object</summary>
		typedef std::shared_ptr<Command> Ptr;

		/// <summary>
		/// The objects that define command monitoring behavior. Monitoring is meant for logging and diagnostic purposes.
		/// </summary>
		/// <remarks>
		/// This static member is not thread-safe. Be sure not to change it while any commands are executing.
		/// <para>
		/// There are no default monitors. <see cref="CommandTracer"/> and <see cref="CommandLogger"/> are implementations of
		/// <see cref="CommandMonitor"/> that can be used.
		/// </para>
		/// </remarks>
		static std::list<CommandMonitor*> sm_monitors;

		virtual ~Command();

		/// <summary>The unique identifier for this command</summary>
		/// <returns>
		/// The unique identifier for this command.
		/// </returns>
		long Id() const;

		/// <summary>The command under which this command is nested, if any</summary>
		/// <returns>
		/// The owner, or the command that an <see cref="AbortLinkedCommand"/> is linked to (if any).
		/// </summary>
		const Command* Parent() const;

		/// <summary>How deeply nested this command is</summary>
		/// <returns>
		/// The number of parents until the top level command is reached
		/// </summary>
		/// <remarks>A parent is considered an owner, or the command that an <see cref="AbortLinkedCommand"/> is linked to (if any).</remarks>
		int Depth() const;

		/// <summary>A description of the Command</summary>
		/// <returns>
		/// The name of the concrete class of this command, preceded by the names of the classes of each parent,
		/// up to the top-level parent. This is followed with this command's unique id (for example, 'SequentialCommands=>PauseCommand(23)').
		/// The description ends with details of about the current state of the command, if available.
		/// </returns>
		/// <remarks>
		/// A parent is considered the owner, or the command that an <see cref="AbortLinkedCommand"/> is linked to (if any).
		/// </remarks>
		std::string Description() const;

		/// <summary>
		/// Information about the command (beyond its type and id), if available, for diagnostic purposes.
		/// </summary>
		/// <returns>
		/// Implementations should return information about the current state of the Command, if available. Return an empty string
		/// or null if there is no useful state information to report.
		/// </returns>
		/// <remarks>
		/// This information is included as part of <see cref="GetDescription"/>. It is meant for diagnostic purposes.
		/// <para>
		/// Implementations must be thread safe, and they must not not throw.
		/// </para>
		/// </remarks>
		virtual std::string ExtendedDescription() const;

		/// <summary>Executes the command and does not return until it finishes.</summary>
		/// <exception cref="CommandAbortedException">Thrown when execution is aborted</exception>
		/// <exception cref="std::exception">
		/// Thrown if execution does not complete successfully.
		/// </exception>
		/// <remarks>
		/// It is safe to call this any number of times, but it will cause undefined behavior to re-execute a
		/// command that is already executing.
		/// </remarks>
		void SyncExecute();

		/// <summary>Executes the command and does not return until it finishes.</summary>
		/// <exception cref="CommandAbortedException">Thrown when execution is aborted</exception>
		/// <exception cref="std::exception">
		/// Thrown if execution does not complete successfully.
		/// </exception>
		/// <remarks>
		/// It is safe to call this any number of times, but it will cause undefined behavior to re-execute a
		/// command that is already executing.
		/// </remarks>
		/// <param name="owner">
		/// If you want this command to pay attention to abort requests of a different command, set this value to that command.
		/// Note that if this Command is already assigned an owner, passing a non-null value will raise an exception. Also note
		/// that the owner assignment is only in effect during the scope of this call. Upon return, this command will not
		/// have an owner, and it is the caller's responsibility to properly dispose it.
		/// </param>
		void SyncExecute(Command* owner);

		/// <summary>
		/// Starts executing the command and returns immediately.
		/// </summary>
		/// <remarks>
		/// Call <see cref="Wait"/> if you need to block until the command finishes.
		/// <para>
		/// It is safe to call this any number of times, but it will cause undefined behavior to re-execute a
		/// command that is already executing.
		/// </para>
		/// </remarks>
		/// <param name="listener">
		/// One of the methods of this interface will be called upon completion, on a separate thread. See the
		/// <see cref="CommandListener"/> documentation for details.
		/// </param>
		void AsyncExecute(CommandListener* listener);

		/// <summary>Aborts a running command</summary>
		/// <remarks>
		/// This method will have no effect on a command that is not running (nor will it cause a future execution of this command to abort).
		/// Synchronous execution will throw a <see cref="CommandAbortedException"/> if aborted, and asynchronous execution will invoke
		/// <see cref="CommandListener::CommandAborted"/> on the listener if aborted. Note that if a command is near completion, it may finish
		/// successfully (or fail) before an abort request is processed.
		/// <para>
		/// It is an error to call Abort() on anything other than a top level command.
		/// </para>
		/// </remarks>
		void Abort();

		/// <summary>
		/// Waits for a running command to complete. Will return immediately if the command is not currently executing.
		/// </summary>
		void Wait() const;

		/// <summary>
		/// Waits a specified duration for a running command to complete. Will return immediately if the command is not currently executing.
		/// </summary>
		/// <param name="interval">The maximum amount of time to wait</param>
		/// <returns>true if the the command completed within 'duration', false otherwise</returns>
		template<typename Rep, typename Period>
		bool Wait(const std::chrono::duration<Rep, Period>& interval) const
		{
			return Wait(std::chrono::duration_cast<std::chrono::milliseconds>(interval).count());
		}

		/// <summary>
		/// Waits the specified milliseconds for a running command to complete. Will return immediately if the command is not currently executing.
		/// </summary>
		/// <param name="milliseconds">The maximum number of milliseconds to wait</param>
		/// <returns>true if the the command completed within 'duration', false otherwise</returns>
		bool Wait(long long milliseconds) const;

		/// <summary>
		/// The exact same effect as a call to <see cref="Abort"/> immediately followed by a call to <see cref="Wait"/>
		/// </summary>
		void AbortAndWait();

		/// <summary>
		/// The exact same effect as a call to <see cref="Abort"/> immediately followed by a call to <see cref="Wait(const std::chrono::duration<Rep, Period>&)"/>
		/// </summary>
		/// <param name="interval">The maximum amount of time to wait</param>
		/// <returns>true if the the command completed within 'duration', false otherwise</returns>
		template<typename Rep, typename Period>
		bool AbortAndWait(const std::chrono::duration<Rep, Period>& interval) const
		{
			return AbortAndWait(std::chrono::duration_cast<std::chrono::milliseconds>(interval).count());
		}

		/// <summary>
		/// The exact same effect as a call to <see cref="Abort"/> immediately followed by a call to <see cref="Wait(const std::chrono::duration<Rep, Period>&)"/>
		/// </summary>
		/// <param name="milliseconds">The maximum number of milliseconds to wait</param>
		/// <returns>true if the the command completed within 'duration', false otherwise</returns>
		bool AbortAndWait(long long milliseconds);

		/// <summary>
		/// Gets the name of the runtime instance of this class. Used for logging and diagnostic purposes.
		/// </summary>
		/// <remarks>
		/// The returned string should be the name of the derived class, without any namespace qualification.
		/// I could have used typeid instead, but that requires compiling with RTTI, which is not something
		/// that everyone wants.
		/// </remarks>
		virtual std::string ClassName() const = 0;

		/// <summary>
		/// Signaled when this command has finished execution, regardless of whether it succeeded, failed or was aborted.
		/// </summary>
		/// <returns>The object that can waited up for this command to be not executing.</returns>
		/// <remarks>
		/// Calling <see cref="Wait"/> has the same effect as waiting upon the object returned from this method.
		/// </remarks>
		Waitable::Ptr DoneEvent() const;

		/// <summary>
		/// Signaled when this command is to be aborted. Note that this event is only reset when the command next begins execution.
		/// </summary>
		/// <returns>The object that can waited up for this command to be signaled to abort.</returns>
		/// <remarks>Note that this is signaled when the command should abort, which will be before the command finishes aborting itself</remarks>
		Waitable::Ptr AbortEvent() const;
	protected:
		/// <summary>
		/// Constructor
		/// </summary>
		Command();

		/// <summary>Make this command the owner of the command passed as an argument.</summary>
		/// <param name="orphan">
		/// The command to be owned. Only un-owned commands can take a new owner. Allowing
		/// other types of owner transfer would invite misuse and the bad behavior that results
		/// (e.g. adding the same Command instance to <see cref="SequentialCommands"/> and <see cref="ParallelCommands"/>).
		/// </param>
		void TakeOwnership(Ptr orphan);

		/// <summary>Makes what used to be an owned command a top-level command.</summary>
		/// <remarks>The caller of this method must be responsible for ensuring that the relinquished command is properly disposed.</remarks>
		/// <param name="command">The command to relinquish ownership. Note that it must currently be a direct child command of this object (not a grandchild, for example)</param>
		void RelinquishOwnership(Ptr command);

		/// <summary>
		/// Throws a <see cref="CommandAbortedException"/> if an abort is pending. Synchronous implementations may find this useful in
		/// order to respond to an abort request in a timely manner.
		/// </summary>
		void CheckAbortFlag() const;
	private:
		class ListenerProxy : public CommandLib::CommandListener
		{
		public:
			ListenerProxy(Command* command, CommandListener* listener);
			virtual void CommandSucceeded() final;
			virtual void CommandAborted() final;
			virtual void CommandFailed(const std::exception& exc, std::exception_ptr excPtr) final;
		private:
			ListenerProxy(const ListenerProxy&) = delete;
			ListenerProxy& operator=(const ListenerProxy&) = delete;
			Command* const m_command;
			CommandListener* m_listener;
			const std::thread::id m_asyncExeThreadId;
		};

		friend class AsyncCommand;

		class CommandLess
		{
		public:
			bool operator()(const Ptr& a, const Ptr& b) const;
		};

		static const Command* Parent(const Command* command);
		static void AbortImplAllDescendents(Command* command);

		Command(const Command&) = delete;
		Command& operator= (const Command&) = delete;

		/// <summary>Executes the command and does not return until it finishes.</summary>
		/// <remarks>
		/// If a command is aborted, implementations should throw a <see cref="CommandAbortedException"/>. Implementations may do so by either periodically
		/// calling <see cref="CheckAbortFlag"/>, or by implementating this method via calls to owned commands. In rare cases, <see cref="AbortImpl"/>
		/// may need to be overridden.
		/// <para>
		/// If a concrete Command class is most naturally implemented in asynchronous fashion, it should inherit from <see cref="AsyncCommand"/>.
		/// That class takes care of implementing SyncExecuteImpl().
		/// </para>
		/// </remarks>
		virtual void SyncExecuteImpl() = 0;

		/// <summary>Starts executing the command and returns immediately.</summary>
		/// <param name="listener">One of the methods of the listener will be called upon command completion, on a separate thread.</param>
		/// <remarks>
		/// Implementions must invoke one of the listener methods on a thread other than the one this method was called from when execution finishes.
		/// Also, implementations will likely need to override <see cref="AbortImpl"/> in order to respond to abort requests in a timely manner.
		/// <para>
		/// If a concrete Command class is most naturally implemented in synchronous fashion, it should inherit from <see cref="SyncCommand"/>.
		/// That class takes care of implementing AsyncExecuteImpl().
		/// </para>
		/// </remarks>
		virtual void AsyncExecuteImpl(CommandListener* listener) = 0;

		/// <summary>
		/// Implementations should override to return false if their Command class must never be owned by another Command.
		/// This is expected to be a rare restriction. Within CommandLib, only <see cref="AbortLinkedCommand"/> has this restriction.
		/// </summary>
		/// <returns>true if the Command subclass must be top level. Default is false.</returns>
		virtual bool MustBeTopLevel() const;

		/// <summary>Implementations should override if there's something in particular they can do to more effectively respond to an abort request.</summary>
		/// <remarks>
		/// Note that <see cref="AsyncCommand"/>-derived classes are likely to need to override this method. <see cref="SyncCommand"/>-derived classes will
		/// typically not need to override this method, instead calling <see cref="CheckAbortFlag"/> periodically, and/or passing work off to owned commands,
		/// which themselves will respond to abort requests.
		/// <para>
		/// Implementations of this method must be asynchronous. Do not wait for the command to fully abort, or a deadlock possibility will arise.
		/// </para>
		/// </remarks>
		virtual void AbortImpl();

		void SetAbortEvent(Ptr target) const;
		void PreExecute();
		void DecrementExecuting(CommandListener* listener, const std::exception* exc, std::exception_ptr excPtr);
		void InformCommandStarting() const;
		void InformCommandFinished(const std::exception* exc) const;

		static std::atomic_uint sm_nextId;
		
		const unsigned int m_id = ++sm_nextId;
        const Command* m_owner = nullptr;
        std::set<Ptr, CommandLess> m_children;
        std::atomic_int m_executing;

		std::shared_ptr<Event> m_abortEvent;
		std::shared_ptr<Event> m_doneEvent = std::shared_ptr<Event>(new Event(true));

        mutable std::mutex m_mutex;
	};
}
