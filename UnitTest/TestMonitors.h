#pragma once
#include "CommandLogger.h"
#include "CommandTracer.h"

namespace CommonTests
{
	class TestMonitors
	{
	public:
		TestMonitors();
		~TestMonitors();
	private:
		static std::string GetUniqueFileName();
		const std::string m_logFileName;
		CommandLib::CommandLogger m_logger;
		CommandLib::CommandTracer m_tracer;
	};
}
