#include "CppUnitTest.h"
#include "TimeLimitedCommand.h"
#include "CommonTests.h"
#include "CmdListener.h"
#include "AddCommand.h"
#include "PauseCommand.h"
#include "FailingCommand.h"
#include "CommandTimeoutException.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(SequentialCommandsTests)
	{
	public:
		TEST_METHOD(TimeLimitedCommand_TestAbort)
		{
			CommandLib::TimeLimitedCommand::Ptr timeLimitedCmd = CommandLib::TimeLimitedCommand::Create(
				CommandLib::PauseCommand::Create(std::numeric_limits<long>::max()), std::numeric_limits<long>::max());

			CommonTests::TestAbort(timeLimitedCmd, 10);
        }

		TEST_METHOD(TimeLimitedCommand_TestHappyPath)
        {
			std::atomic_int total;
			total = 10;

			CommandLib::TimeLimitedCommand::Ptr timeLimitedCmd = CommandLib::TimeLimitedCommand::Create(
				CommandLibTests::AddCommand::Create(&total, 4), std::numeric_limits<long>::max());

			CommonTests::TestHappyPath(timeLimitedCmd);
			Assert::AreEqual((int)total, 18); // 18, because TestHappyPath executes the command twice

			timeLimitedCmd = CommandLib::TimeLimitedCommand::Create(CommandLib::PauseCommand::Create(10), 100);
			CommonTests::TestHappyPath(timeLimitedCmd);
		}

		TEST_METHOD(TimeLimitedCommand_TestFail)
        {
			CommandLib::TimeLimitedCommand::Ptr timeLimitedCmd = CommandLib::TimeLimitedCommand::Create(
				CommandLibTests::FailingCommand::Create(), std::numeric_limits<long>::max());

			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(timeLimitedCmd);
			timeLimitedCmd = CommandLib::TimeLimitedCommand::Create(CommandLib::PauseCommand::Create(100), 10);
			CommonTests::TestFail<CommandLib::CommandTimeoutException>(timeLimitedCmd);
		}

		TEST_METHOD(TimeLimitedCommand_TestTimeout)
        {
			CommandLib::TimeLimitedCommand::Ptr innerCmd = CommandLib::TimeLimitedCommand::Create(
				CommandLib::PauseCommand::Create(std::numeric_limits<long>::max()), 10);

			CommandLib::TimeLimitedCommand::Ptr timeLimitedCmd = CommandLib::TimeLimitedCommand::Create(
				innerCmd, std::numeric_limits<long>::max());

			CommonTests::TestFail<CommandLib::CommandTimeoutException>(timeLimitedCmd);

			innerCmd = CommandLib::TimeLimitedCommand::Create(
				CommandLib::PauseCommand::Create(std::numeric_limits<long>::max()),
				std::numeric_limits<long>::max());

			timeLimitedCmd = CommandLib::TimeLimitedCommand::Create(innerCmd, std::chrono::milliseconds(10));
			CommonTests::TestFail<CommandLib::CommandTimeoutException>(timeLimitedCmd);
        }
	};
}
