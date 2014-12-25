#include "CppUnitTest.h"
#include "AbortLinkedCommand.h"
#include "CommonTests.h"
#include "CmdListener.h"
#include "PauseCommand.h"
#include "CppUnitTestAssert.h"
#include "CommandAbortedException.h"
#include "CmdListener.h"
#include "TestMonitors.h"
#include "FailingCommand.h"
#include "SequentialCommands.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(AbortLinkedCommandTests)
	{
	public:
		TEST_METHOD(AbortLinkedCommand_TestAbortedPause)
		{
			CommonTests::TestMonitors testMonitors;

			CmdListener linkedCmdListener(CmdListener::CallbackType::Aborted);
			CommandLib::PauseCommand::Ptr linkedCommand = CommandLib::PauseCommand::Create(std::chrono::hours(24));
			linkedCommand->AsyncExecute(&linkedCmdListener);

			CommandLib::AbortLinkedCommand::Ptr abortLinkedPauseCmd =
				CommandLib::AbortLinkedCommand::Create(CommandLib::PauseCommand::Create(std::chrono::hours(24)), linkedCommand);

			std::thread thread([abortLinkedPauseCmd]()
			{
				Assert::ExpectException<CommandLib::CommandAbortedException>([abortLinkedPauseCmd]()
					{ abortLinkedPauseCmd->SyncExecute(); }, L"Expected aborted exception");
			});

			std::this_thread::sleep_for(std::chrono::milliseconds(20)); // give time for the thread to start executing
			linkedCommand->Abort();
			abortLinkedPauseCmd->Wait();
			linkedCommand->Wait();
			thread.join();
			linkedCommand->AsyncExecute(&linkedCmdListener);
			CmdListener listener(CmdListener::CallbackType::Aborted);
			abortLinkedPauseCmd->AsyncExecute(&listener);
			Assert::AreEqual(abortLinkedPauseCmd->Wait(10), false);
			abortLinkedPauseCmd->AbortAndWait();
			listener.Check();

			listener.Reset(CmdListener::CallbackType::Aborted);
			abortLinkedPauseCmd->AsyncExecute(&listener);
			linkedCommand->AbortAndWait();
			abortLinkedPauseCmd->Wait();
			listener.Check();

			listener.Reset(CmdListener::CallbackType::Aborted);
			abortLinkedPauseCmd->AsyncExecute(&listener);
			linkedCommand->AsyncExecute(&linkedCmdListener);
			Assert::AreEqual(abortLinkedPauseCmd->Wait(10), false);
			abortLinkedPauseCmd->AbortAndWait();
			listener.Check();
			Assert::AreEqual(linkedCommand->Wait(10), false);

			listener.Reset(CmdListener::CallbackType::Aborted);
			abortLinkedPauseCmd->AsyncExecute(&listener);
			Assert::AreEqual(abortLinkedPauseCmd->Wait(10), false);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			linkedCommand->AbortAndWait();
			abortLinkedPauseCmd->Wait();
			listener.Check();
		}

		TEST_METHOD(AbortEventedCommand_TestAbort)
		{
			CommandLib::Event::Ptr abortEvent(new CommandLib::Event());
			
			CommandLib::AbortLinkedCommand::Ptr abortLinkedPauseCmd = CommandLib::AbortLinkedCommand::Create(
				CommandLib::PauseCommand::Create(std::chrono::hours(24)), abortEvent);

			CommonTests::TestAbort(abortLinkedPauseCmd, 10);
		}

		TEST_METHOD(AbortEventedCommand_TestHappyPath)
		{
			CommandLib::Event::Ptr abortEvent(new CommandLib::Event());
			CommandLib::PauseCommand::Ptr pauseCmd = CommandLib::PauseCommand::Create(10);
			CommandLib::AbortLinkedCommand::Ptr abortLinkedPauseCmd = CommandLib::AbortLinkedCommand::Create(pauseCmd, abortEvent);
			CommonTests::TestHappyPath(abortLinkedPauseCmd);
		}

		TEST_METHOD(AbortEventedCommand_TestFail)
		{
			CommandLib::Event::Ptr abortEvent(new CommandLib::Event());
			CommandLib::AbortLinkedCommand::Ptr abortLinkedFailCmd = CommandLib::AbortLinkedCommand::Create(CommandLibTests::FailingCommand::Create(), abortEvent);
			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(abortLinkedFailCmd);
		}

		TEST_METHOD(AbortEventedCommand_TestMustBeTopLevel)
		{
			CommandLib::Event::Ptr abortEvent(new CommandLib::Event());
			CommandLib::PauseCommand::Ptr pauseCmd = CommandLib::PauseCommand::Create(10);
			CommandLib::AbortLinkedCommand::Ptr abortLinkedPauseCmd = CommandLib::AbortLinkedCommand::Create(pauseCmd, abortEvent);
			CommandLib::SequentialCommands::Ptr seqCmd = CommandLib::SequentialCommands::Create();
			Assert::ExpectException<std::logic_error>([seqCmd, abortLinkedPauseCmd](){ seqCmd->Add(abortLinkedPauseCmd); }, L"AbortLinkedCommand was given an owner");
			Assert::ExpectException<std::logic_error>([seqCmd, pauseCmd](){ seqCmd->Add(pauseCmd); }, L"PauseCommand already had an owner");
		}
	};
}
