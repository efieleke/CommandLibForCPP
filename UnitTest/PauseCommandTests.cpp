#include "CppUnitTest.h"
#include "PauseCommand.h"
#include "CommonTests.h"
#include "CmdListener.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{		
	TEST_CLASS(PauseCommandTests)
	{
	public:
		TEST_METHOD(PauseCommand_TestAbort)
		{
			CommandLib::PauseCommand::Ptr pauseCmd = CommandLib::PauseCommand::Create(std::chrono::hours(24));
			CommonTests::TestAbort(pauseCmd, 10);
		}

		TEST_METHOD(PauseCommand_TestHappyPath)
		{
			CommonTests::TestHappyPath(CommandLib::PauseCommand::Create(1));
			CommandLib::PauseCommand::Ptr shortPause = CommandLib::PauseCommand::Create(100);
			CmdListener listener(CmdListener::CallbackType::Succeeded);
			shortPause->AsyncExecute(&listener);
			CommandLib::PauseCommand::Ptr longPause = CommandLib::PauseCommand::Create(std::chrono::hours(24), shortPause->DoneEvent());
			CommonTests::TestHappyPath(longPause);
		}

		TEST_METHOD(PauseCommand_TestReset)
		{
			CommandLib::PauseCommand::Ptr pauseCmd = CommandLib::PauseCommand::Create(std::chrono::hours(24));
			CmdListener listener (CmdListener::CallbackType::Succeeded);
			pauseCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(10)); // give time for the command to start
			pauseCmd->SetDurationMS(1);
			Assert::IsFalse(pauseCmd->Wait(10));
			pauseCmd->Reset();
			Assert::IsTrue(pauseCmd->Wait(100));
			listener.Check();

			listener.Reset(CmdListener::CallbackType::Aborted);
			pauseCmd->SetDurationMS(100);
			pauseCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(20)); // give the async routine a moment to get going
			pauseCmd->SetDuration(std::chrono::hours(24));
			pauseCmd->Reset();
			Assert::IsFalse(pauseCmd->Wait(100));
			pauseCmd->AbortAndWait();
			listener.Check();

			listener.Reset(CmdListener::CallbackType::Succeeded);
			pauseCmd->SetDuration(std::chrono::hours(24));
			pauseCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(10)); // give time for the command to start
			pauseCmd->Reset();
			pauseCmd->CutShort();
			pauseCmd->Wait();
			listener.Check();
		}

		TEST_METHOD(PauseCommand_TestCutShort)
		{
			CommandLib::PauseCommand::Ptr pauseCmd = CommandLib::PauseCommand::Create(std::chrono::hours(24));
			CmdListener listener(CmdListener::CallbackType::Succeeded);
			pauseCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(20)); // give the async routine a moment to get going
			pauseCmd->CutShort();
			pauseCmd->Wait();
			listener.Check();

			listener.Reset(CmdListener::CallbackType::Aborted);
			pauseCmd->CutShort(); // no-op
			pauseCmd->AsyncExecute(&listener);
			Assert::IsFalse(pauseCmd->Wait(10));
			pauseCmd->AbortAndWait();
		}
	};
}