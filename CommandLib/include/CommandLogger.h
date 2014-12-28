#pragma once;
#include "CommandMonitor.h"
#include <string>
#include <mutex>
#include <fstream>

namespace CommandLib
{
	/// <summary>
	/// Implements <see cref="CommandMonitor"/> by writing diagnostic information to a log file that can be parsed
	/// and dynamically displayed by the CommandLogViewer application included with the C# version of this project
	/// (found at https://github.com/efieleke/CommandLib.git).
	/// </summary>
	class CommandLogger : public CommandMonitor
    {
	public:
		/// <summary>Constructor</summary>
		/// <param name="filename">Name of the log file. Will be overwritten if it exists.</param>
		explicit CommandLogger(const std::string& filename);

		/// <inheritdoc/>
		virtual void CommandStarting(const Command& command) override;

		/// <inheritdoc/>
		virtual void CommandFinished(const Command& command, const std::exception* exc) override;
	private:
		static std::string FormHeader(const Command& command, const std::string& action);

		void WriteMessage(const Command& command, std::string message);

        std::ofstream m_stream;
		std::mutex m_mutex;
	};
}
