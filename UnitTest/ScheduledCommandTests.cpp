#include "CppUnitTest.h"
#include "RecurringCommand.h"
#include "CommonTests.h"
#include "CmdListener.h"
#include "AddCommand.h"
#include "PauseCommand.h"
#include "ScheduledCommand.h"
#include "FailingCommand.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(ScheduledCommandTests)
	{
	public:
		TEST_METHOD(ScheduledCommand_TestAbort)
		{
			CommandLib::ScheduledCommand::Ptr scheduledCmd = CommandLib::ScheduledCommand::Create(
				CommandLib::PauseCommand::Create(0), Tomorrow(), false);

			CommonTests::TestAbort(scheduledCmd, 20);
			scheduledCmd = CommandLib::ScheduledCommand::Create(CommandLib::PauseCommand::Create(std::chrono::hours(24)), Yesterday(), true);
			CommonTests::TestAbort(scheduledCmd, 20);
		}

		TEST_METHOD(ScheduledCommand_TestHappyPath)
		{
			CommandLib::ScheduledCommand::Ptr scheduledCmd = CommandLib::ScheduledCommand::Create(
				CommandLib::PauseCommand::Create(0), Yesterday(), true);

			CommonTests::TestHappyPath(scheduledCmd);
			scheduledCmd = CommandLib::ScheduledCommand::Create(CommandLib::PauseCommand::Create(0), RealSoon(), true);
			CommonTests::TestHappyPath(scheduledCmd);
			scheduledCmd = CommandLib::ScheduledCommand::Create(CommandLib::PauseCommand::Create(0), RealSoon(), false);
			scheduledCmd->SyncExecute(); // only possible to run this once, because after that it will be in the past
		}

		TEST_METHOD(ScheduledCommand_TestFail)
		{
			CommandLib::ScheduledCommand::Ptr scheduledCmd = CommandLib::ScheduledCommand::Create(
				CommandLib::PauseCommand::Create(0), Yesterday(), false);

			CommonTests::TestFail<std::logic_error>(scheduledCmd);
			scheduledCmd = CommandLib::ScheduledCommand::Create(CommandLibTests::FailingCommand::Create(), RealSoon(), true);
			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(scheduledCmd);
		}

		TEST_METHOD(ScheduledCommand_TestSkipCurrentWait)
		{
			CommandLib::ScheduledCommand::Ptr scheduledCmd = CommandLib::ScheduledCommand::Create(
				CommandLib::PauseCommand::Create(0), Tomorrow(), false);

			CmdListener listener(CmdListener::CallbackType::Succeeded);
			scheduledCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give time for the thread to start executing
			scheduledCmd->SkipWait();
			scheduledCmd->Wait();
			listener.Check();

			listener.Reset(CmdListener::CallbackType::Succeeded);
			scheduledCmd->SkipWait(); // no-op
			scheduledCmd->AsyncExecute(&listener);
			Assert::IsFalse(scheduledCmd->Wait(100));
			scheduledCmd->SkipWait();
			scheduledCmd->Wait();
			listener.Check();
		}

		TEST_METHOD(ScheduledCommand_TestChange)
		{
			CommandLib::ScheduledCommand::Ptr scheduledCmd = CommandLib::ScheduledCommand::Create(
				CommandLib::PauseCommand::Create(0), Tomorrow(), false);

			CmdListener listener(CmdListener::CallbackType::Succeeded);
			scheduledCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give time for the thread to start executing
			scheduledCmd->SetTimeOfExecution(RealSoon());
			scheduledCmd->Wait();
			listener.Check();

			listener.Reset(CmdListener::CallbackType::Succeeded);
			scheduledCmd->SetTimeOfExecution(RealSoon());
			scheduledCmd->AsyncExecute(&listener);
			Assert::IsFalse(scheduledCmd->Wait(10));
			scheduledCmd->Wait();
			listener.Check();
			Assert::ExpectException<std::logic_error>([scheduledCmd](){ scheduledCmd->SetTimeOfExecution(Yesterday()); },
				L"Set time of operation to the past");
		}
	private:
		static std::chrono::time_point<std::chrono::system_clock> Tomorrow()
		{
			return std::chrono::high_resolution_clock::now() + std::chrono::hours(24);
		}

		static std::chrono::time_point<std::chrono::system_clock> Yesterday()
		{
			return std::chrono::high_resolution_clock::now() - std::chrono::hours(24);
		}

		static std::chrono::time_point<std::chrono::system_clock> RealSoon()
		{
			return std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(100);
		}
	};
}
