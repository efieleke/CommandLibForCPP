#include "CppUnitTest.h"
#include "CommandDispatcher.h"
#include "CommonTests.h"
#include "CmdListener.h"
#include "PauseCommand.h"
#include "CppUnitTestAssert.h"
#include "CommandAbortedException.h"
#include "CmdListener.h"
#include "TestMonitors.h"
#include "FailingCommand.h"
#include "SequentialCommands.h"
#include "ParallelCommands.h"
#include "PeriodicCommand.h"
#include "TimeLimitedCommand.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(ComplexCommandTest)
	{
	public:
		TEST_METHOD(ComplexCommand_TestHappyPath)
		{
			CommandLib::Command::Ptr test = ComplexCommand::Create(1, false);
			CommonTests::TestHappyPath(test);
		}

		TEST_METHOD(ComplexCommand_TestFail)
		{
			CommandLib::Command::Ptr test = ComplexCommand::Create(1, true);
			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(test);
		}

		TEST_METHOD(ComplexCommand_TestNestedCommandAbort)
		{
			CommandLib::SequentialCommands::Ptr outerSeq = CommandLib::SequentialCommands::Create();
			CommandLib::SequentialCommands::Ptr innerSeq = CommandLib::SequentialCommands::Create();
			innerSeq->Add(CommandLib::PauseCommand::Create(std::chrono::hours(24)));
			outerSeq->Add(innerSeq);
			CmdListener listener(CmdListener::CallbackType::Aborted);
			outerSeq->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			outerSeq->AbortAndWait();
		}

		TEST_METHOD(ComplexCommand_TestAbort)
		{
			CommandLib::Command::Ptr test = ComplexCommand::Create(1, false);
			CommonTests::TestAbort(test, 10);
		}
	private:
		class ComplexCommand : public CommandLib::SyncCommand
		{
		public:
			typedef std::shared_ptr<ComplexCommand> Ptr;

			static CommandLib::Command::Ptr Create(int maxPauseMS, bool insertFailure)
			{
				return Ptr(new ComplexCommand(maxPauseMS, insertFailure));
			}

			virtual std::string ClassName() const override
			{
				return "ComplexCommand";
			}
		protected:
			virtual void SyncExeImpl() override
			{
				Assert::ExpectException<std::logic_error>([this]() { RelinquishOwnership(m_cmd); }, L"Relinquished ownership of an un-owned command");
				m_cmd->SyncExecute(this);
			}
		private:
			static CommandLib::ParallelCommands::Ptr GenerateParallelCommands(int maxPauseMS, bool insertFailure)
			{
				CommandLib::ParallelCommands::Ptr cmds = CommandLib::ParallelCommands::Create(false);

				for (CommandLib::Command::Ptr cmd : GenerateCommands(maxPauseMS, insertFailure))
				{
					cmds->Add(cmd);
				}

				return cmds;
			}

			static CommandLib::SequentialCommands::Ptr GenerateSequentialCommands(int maxPauseMS, bool insertFailure)
			{
				CommandLib::SequentialCommands::Ptr cmds = CommandLib::SequentialCommands::Create();

				for (CommandLib::Command::Ptr cmd : GenerateCommands(maxPauseMS, insertFailure))
				{
					cmds->Add(cmd);
				}

				return cmds;
			}

			static std::vector<CommandLib::Command::Ptr> GenerateCommands(int maxPauseMS, bool insertFailure)
			{
				std::vector<CommandLib::Command::Ptr> result;
				result.push_back(CommandLib::PauseCommand::Create(maxPauseMS));
				result.push_back(NoOpCommand::Create());
				Command::Ptr cmd = insertFailure ? (Command::Ptr)CommandLibTests::FailingCommand::Create() : NoOpCommand::Create();

				result.push_back(CommandLib::PeriodicCommand::Create(
					cmd,
					5,
					maxPauseMS,
					CommandLib::PeriodicCommand::IntervalType::PauseBefore,
					true));

				return result;
			}

			ComplexCommand(int maxPauseMS, bool insertFailure)
			{
				CommandLib::ParallelCommands::Ptr parallel = GenerateParallelCommands(maxPauseMS, insertFailure);
				CommandLib::SequentialCommands::Ptr seq = GenerateSequentialCommands(maxPauseMS, insertFailure);
				parallel->Add(GenerateSequentialCommands(maxPauseMS, insertFailure));
				parallel->Add(GenerateParallelCommands(maxPauseMS, insertFailure));
				seq->Add(GenerateParallelCommands(maxPauseMS, insertFailure));
				seq->Add(GenerateSequentialCommands(maxPauseMS, insertFailure));
				CommandLib::ParallelCommands::Ptr combined = CommandLib::ParallelCommands::Create(false);
				combined->Add(seq);
				combined->Add(parallel);

				CommandLib::PeriodicCommand::Ptr periodic = CommandLib::PeriodicCommand::Create(
					combined, 3, maxPauseMS, CommandLib::PeriodicCommand::IntervalType::PauseAfter, true);

				m_cmd = CommandLib::TimeLimitedCommand::Create(periodic, std::chrono::hours(24));
				TakeOwnership(m_cmd);

				// For code coverage. Also, gives us an opportunity to try the third overload of SyncExecute.
				RelinquishOwnership(m_cmd);
			}

			CommandLib::Command::Ptr m_cmd;
		};

		class NoOpCommand : public CommandLib::SyncCommand
		{
		public:
			typedef std::shared_ptr<NoOpCommand> Ptr;

			static Ptr Create()
			{
				return Ptr(new NoOpCommand());
			}

			virtual std::string ClassName() const override
			{
				return "NoOpCommand";
			}
		private:
			NoOpCommand()
			{
			}

			virtual void SyncExeImpl() override final
			{
				return;
			}
		};
	};
}
