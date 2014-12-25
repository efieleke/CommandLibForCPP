#pragma once
#include "CppUnitTest.h"
#include "Command.h"
#include "CommandLogger.h"
#include "CommandTracer.h"
#include <iostream>
#include "TestMonitors.h"

namespace CommonTests
{
	void TestAbort(CommandLib::Command::Ptr cmd, long maxDelayTimeMS);
	void TestHappyPath(CommandLib::Command::Ptr cmd);

	template<typename T>
	void TestFail(CommandLib::Command::Ptr cmd)
	{
		TestMonitors testMonitors;
		CmdListener listener(CmdListener::CallbackType::Failed);
		cmd->AsyncExecute(&listener);
		cmd->Wait();
		listener.Check<T>();
		
		// Exception must be of the expected type, and it must inherit from std::exception
		Assert::ExpectException<std::exception>([cmd](){ cmd->SyncExecute(); }, L"Caught unexpected type of exception");
		Assert::ExpectException<T>([cmd](){ cmd->SyncExecute(); }, L"Caught unexpected type of exception");
	}
}
