﻿#pragma once
#include "CommandMonitor.h"
#include <string>
#include <ostream>

namespace CommandLib
{
	/// <summary>
	/// Implements <see cref="CommandMonitor"/> by writing diagnostic information to an output stream.
	/// </summary>
	class CommandTracer : public CommandMonitor
    {
	public:
		/// <summary>Constructor</summary>
		/// <param name="os">The stream of which to write output</param>
		/// <remarks>
		/// Easiest usage is to just pass std::cout as the argument. Windows implementations may wish to
		/// define an output stream that wraps OutputDebugStream
		/// </remarks>
		explicit CommandTracer(std::ostream& os);

		/// <inheritdoc/>
		virtual void CommandStarting(const Command& command) override;

		/// <inheritdoc/>
		virtual void CommandFinished(const Command& command, const std::exception* exc) override;
	private:
		CommandTracer(const CommandTracer&) = delete;
		CommandTracer& operator=(const CommandTracer&) = delete;
		void PrintMessage(const Command& command, std::string message);
		std::ostream& m_stream;
	};
}
