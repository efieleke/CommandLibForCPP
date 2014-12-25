#include "CommonTests.h"
#include "CppUnitTest.h"
#include "Command.h"
#include <cstdio>
#include "CommandAbortedException.h"
#include "CmdListener.h"
#include "CppUnitTestAssert.h"
#include <thread>
#include "TestMonitors.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void CommonTests::TestAbort(CommandLib::Command::Ptr cmd, long maxDelayTimeMS)
{
	TestMonitors testMonitors;
	CmdListener listener(CmdListener::CallbackType::Aborted);
	cmd->AsyncExecute(&listener);
	cmd->AbortAndWait();
	listener.Check();

	listener.Reset(CmdListener::CallbackType::Aborted);
	cmd->AsyncExecute(&listener);
	std::this_thread::sleep_for(std::chrono::milliseconds(maxDelayTimeMS));
	cmd->AbortAndWait();
	listener.Check();

	std::thread thread([cmd]()
	{
		Assert::ExpectException<CommandLib::CommandAbortedException>([cmd](){ cmd->SyncExecute(); }, L"Expected aborted exception");
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(20)); // give time for the command to start executing
	cmd->AbortAndWait();
	thread.join();
}

void CommonTests::TestHappyPath(CommandLib::Command::Ptr cmd)
{
	CmdListener listener(CmdListener::CallbackType::Succeeded);
	cmd->Abort(); // should be a no-op
	cmd->AsyncExecute(&listener);
	cmd->Wait();
	listener.Check();
	cmd->Abort(); // should be a no-op
	cmd->SyncExecute();
	cmd->Wait(); // should be a no-op
	cmd->AbortAndWait(); // should be a no-op
}
