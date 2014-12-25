#include "CppUnitTest.h"
#include "SequentialCommands.h"
#include "CommonTests.h"
#include "CmdListener.h"
#include "AddCommand.h"
#include "PauseCommand.h"
#include "FailingCommand.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(SequentialCommandsTests)
	{
	public:
		TEST_METHOD(SequentialCommands_TestHappyPath)
		{
			CommandLib::SequentialCommands::Ptr seqCmds = CommandLib::SequentialCommands::Create();
            CommonTests::TestHappyPath(seqCmds);
			std::atomic_int total;
			total = 0;
            const int count = 10;

            for (int i = 0; i < count; ++i)
            {
                seqCmds->Add(CommandLibTests::AddCommand::Create(&total, 1));
            }

			CommonTests::TestHappyPath(seqCmds);
			Assert::AreEqual((int)total, 20); // 20, because TestHappyPath executes the command twice
			seqCmds->Clear();
			CommonTests::TestHappyPath(seqCmds);
			Assert::AreEqual((int)total, 20);
			seqCmds->Clear();
        }

        TEST_METHOD(SequentialCommands_TestAbort)
        {
			CommandLib::SequentialCommands::Ptr seqCmds = CommandLib::SequentialCommands::Create();
            seqCmds->Add(CommandLib::PauseCommand::Create(10));
			seqCmds->Add(CommandLib::PauseCommand::Create(std::chrono::hours(24)));
			CommonTests::TestAbort(seqCmds, 20);

			seqCmds = CommandLib::SequentialCommands::Create();
			seqCmds->Add(CommandLib::PauseCommand::Create(std::chrono::hours(24)));
			seqCmds->Add(CommandLib::PauseCommand::Create(10));
			CommonTests::TestAbort(seqCmds, 20);
		}

        TEST_METHOD(SequentialCommand_TestFail)
        {
			CommandLib::SequentialCommands::Ptr seqCmds = CommandLib::SequentialCommands::Create();
			seqCmds->Add(CommandLib::PauseCommand::Create(0));
			seqCmds->Add(CommandLibTests::FailingCommand::Create());
			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(seqCmds);

			seqCmds = CommandLib::SequentialCommands::Create();
			seqCmds->Add(CommandLibTests::FailingCommand::Create());
			seqCmds->Add(CommandLib::PauseCommand::Create(0));
			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(seqCmds);
		}
	};
}
