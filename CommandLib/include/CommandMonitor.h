#pragma once
#include <exception>
#include <memory>

namespace CommandLib
{
	class Command;

	/// <summary>
	/// This is a callback interface for <see cref="Command"/> starting and finishing events. Its intended use is for logging and diagnostics.
	/// </summary>
	/// <remarks>
	/// <see cref="CommandTracer"/> and <see cref="CommandLogger"/> are available implementations.
	/// You may add a monitor via the static <see cref="Command::sm_monitors"/> member of <see cref="Command"/>. Monitors added to that
	/// member will be called for every Command object that executes. To receive callbacks only for commands dispatched via
	/// a <see cref="CommandDispatcher"/>, use <see cref="CommandDispatcher::AddMonitor(CommandMonitor*)"/>.
	/// </remarks>
	class CommandMonitor
    {
	public:
		virtual ~CommandMonitor();

		/// <summary>
		/// Invoked when a <see cref="Command"/> (including owned commands) starts execution
		/// </summary>
		/// <param name="command">
		/// The command that is starting execution.
		/// </param>
		/// <remarks>
		/// Implementations of this method must not throw.
		/// </remarks>
		virtual void CommandStarting(const Command& command) = 0;

		/// <summary>
		/// Invoked by the framework whenever a <see cref="Command"/> (including owned commands) is finishing execution, for whatever reason (success, fail, or abort).
		/// </summary>
		/// <param name="command">
		/// The command that is finishing execution.
		/// </param>
		/// <param name="exc">
		/// Will be null if the command succeeded. Otherwise will be a <see cref="CommandAbortedException"/> if the command was aborted, or some other
		/// Exception type if the command failed.
		/// </param>
		/// <remarks>
		/// Implementations of this method must not throw.
		/// </remarks>
		virtual void CommandFinished(const Command& command, const std::exception* exc) = 0;
	};
}
