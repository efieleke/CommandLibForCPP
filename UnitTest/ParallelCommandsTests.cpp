#include "CppUnitTest.h"
#include "ParallelCommands.h"
#include "CommonTests.h"
#include "CmdListener.h"
#include "AddCommand.h"
#include "PauseCommand.h"
#include "FailingCommand.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(ParallelCommandsTests)
	{
	public:
		TEST_METHOD(ParallelCommands_TestHappyPath)
		{
			TestHappyPath(false);
			TestHappyPath(true);
		}

		TEST_METHOD(ParallelCommands_TestAbort)
		{
			TestAbort(false);
			TestAbort(true);
		}

		TEST_METHOD(ParallelCommands_TestFail)
		{
			TestFail(false);
			TestFail(true);
		}
	private:
		void TestHappyPath(bool abortUponFailure)
		{
			CommandLib::ParallelCommands::Ptr parallelCmds = CommandLib::ParallelCommands::Create(abortUponFailure);
			CommonTests::TestHappyPath(parallelCmds);
			parallelCmds = CommandLib::ParallelCommands::Create(abortUponFailure);
			std::atomic_int total;
			total = 0;
			const int count = 10;

			for (int i = 0; i < count; ++i)
			{
				parallelCmds->Add(CommandLibTests::AddCommand::Create(&total, 1));
			}

			CommonTests::TestHappyPath(parallelCmds);
			Assert::AreEqual((int)total, 20); // 20, because TestHappyPath executes the command twice
			parallelCmds->Clear();
			CommonTests::TestHappyPath(parallelCmds);
		}

		void TestAbort(bool abortUponFailure)
		{
			CommandLib::ParallelCommands::Ptr parallelCmds = CommandLib::ParallelCommands::Create(abortUponFailure);
			parallelCmds->Add(CommandLib::PauseCommand::Create(10));
			parallelCmds->Add(CommandLib::PauseCommand::Create(std::chrono::hours(24)));
			CommonTests::TestAbort(parallelCmds, 10);

			parallelCmds = CommandLib::ParallelCommands::Create(abortUponFailure);
			parallelCmds->Add(CommandLib::PauseCommand::Create(std::chrono::hours(24)));
			parallelCmds->Add(CommandLib::PauseCommand::Create(10));
			CommonTests::TestAbort(parallelCmds, 20);

			parallelCmds = CommandLib::ParallelCommands::Create(abortUponFailure);
			parallelCmds->Add(CommandLib::PauseCommand::Create(std::chrono::hours(24)));
			parallelCmds->Add(CommandLib::PauseCommand::Create(std::chrono::hours(24)));
			CommonTests::TestAbort(parallelCmds, 20);
		}

		void TestFail(bool abortUponFailure)
		{
			CommandLib::ParallelCommands::Ptr parallelCmds = CommandLib::ParallelCommands::Create(abortUponFailure);
			parallelCmds->Add(CommandLib::PauseCommand::Create(0));
			parallelCmds->Add(CommandLibTests::FailingCommand::Create());
			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(parallelCmds);

			parallelCmds = CommandLib::ParallelCommands::Create(abortUponFailure);
			parallelCmds->Add(CommandLibTests::FailingCommand::Create());
			parallelCmds->Add(CommandLib::PauseCommand::Create(0));
			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(parallelCmds);

			parallelCmds = CommandLib::ParallelCommands::Create(abortUponFailure);
			parallelCmds->Add(CommandLib::PauseCommand::Create(0));
			parallelCmds->Add(CommandLibTests::FailingCommand::Create());
			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(parallelCmds);
		}
	};
}

