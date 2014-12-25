#pragma once;
#include "CommandMonitor.h"
#include <string>
#include <mutex>
#include <fstream>

namespace CommandLib
{
	/// <summary>
	/// Implements <see cref="CommandMonitor"/> by writing diagnostic information to a log file that can be parsed
	/// and dynamically displayed by the included CommandLogViewer application.
	/// </summary>
	class CommandLogger : public CommandMonitor
    {
	public:
		/// <summary>Constructor</summary>
		/// <param name="filename">The file name on which to append log entries</param>
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
