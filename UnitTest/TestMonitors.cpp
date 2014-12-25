#include "CppUnitTest.h"
#include "TestMonitors.h"
#include "Command.h"
#include <cstdio>
#include <iostream>
#include "CppUnitTestAssert.h"

using namespace CommonTests;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TestMonitors::TestMonitors() : m_logFileName(GetUniqueFileName()), m_logger(m_logFileName), m_tracer(std::cout)
{
	Assert::IsTrue(CommandLib::Command::sm_monitors.empty());
	CommandLib::Command::sm_monitors.push_back(&m_logger);
	CommandLib::Command::sm_monitors.push_back(&m_tracer);
}

TestMonitors::~TestMonitors()
{
	CommandLib::Command::sm_monitors.clear();
	remove(m_logFileName.c_str());
}

#include <Windows.h>

std::string TestMonitors::GetUniqueFileName()
{
	// TODO: the following code will only work on Windows.
	char tempPath[MAX_PATH + 1];
	::GetTempPathA(sizeof(tempPath), tempPath);
	char tempFileName[MAX_PATH];
	::GetTempFileNameA(tempPath, "~", 0, tempFileName);
	return tempFileName;
}
