#include "CppUnitTest.h"
#include "VariableCommand.h"
#include "CommonTests.h"
#include "CmdListener.h"
#include "SequentialCommands.h"
#include "FailingCommand.h"
#include "PauseCommand.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	class TestCommand : public CommandLib::Command
	{
	protected:
		TestCommand()
		{
			m_variableCommand = CommandLib::VariableCommand::Create();
			TakeOwnership(m_variableCommand);
			Assert::IsTrue(!m_variableCommand->GetCommandToRun());
		}
	private:
		virtual CommandLib::Command::Ptr GenerateCommand() = 0;

		virtual void AsyncExecuteImpl(CommandLib::CommandListener* listener) override final
		{
			SetupCommand();
			m_variableCommand->AsyncExecute(listener);
		}

		virtual void SyncExecuteImpl() override final
		{
			SetupCommand();
			return m_variableCommand->SyncExecute();
		}

		void SetupCommand()
		{
			CommandLib::Command::Ptr cmd = GenerateCommand();
			m_variableCommand->SetCommandToRun(cmd);
			Assert::IsFalse(!m_variableCommand->GetCommandToRun());
			m_variableCommand->SetCommandToRun(cmd);
			m_variableCommand->SetCommandToRun(GenerateCommand());
		}

		CommandLib::VariableCommand::Ptr m_variableCommand;
	};
	
	class TestNeverEndCommand : public TestCommand
	{
	public:
		typedef std::shared_ptr<TestNeverEndCommand> Ptr;
		static TestNeverEndCommand::Ptr Create() { return Ptr(new TestNeverEndCommand()); }
		virtual std::string ClassName() const { return "TestFailCommand"; }
	private:
		TestNeverEndCommand() {}

		virtual CommandLib::Command::Ptr GenerateCommand() override final
		{
			return CommandLib::PauseCommand::Create(std::chrono::hours(24));
		}
	};

	class TestEmptySequenceCommand : public TestCommand
	{
	public:
		typedef std::shared_ptr<TestEmptySequenceCommand> Ptr;
		static TestEmptySequenceCommand::Ptr Create() { return Ptr(new TestEmptySequenceCommand()); }
		virtual std::string ClassName() const { return "TestFailCommand"; }
	private:
		TestEmptySequenceCommand() {}

		virtual CommandLib::Command::Ptr GenerateCommand() override final
		{
			return CommandLib::SequentialCommands::Create();
		}
	};

	class TestFailCommand : public TestCommand
	{
	public:
		typedef std::shared_ptr<TestFailCommand> Ptr;
		static TestFailCommand::Ptr Create() { return Ptr(new TestFailCommand()); }
		virtual std::string ClassName() const { return "TestFailCommand"; }
	private:
		TestFailCommand() {}

		virtual CommandLib::Command::Ptr GenerateCommand() override final
		{
			return CommandLibTests::FailingCommand::Create();
		}
	};

	TEST_CLASS(VariableCommandTests)
    {
	public:
		TEST_METHOD(VariableCommand_TestHappyPath)
		{
			TestEmptySequenceCommand::Ptr test = TestEmptySequenceCommand::Create();
			CommonTests::TestHappyPath(test);
		}

		TEST_METHOD(VariableCommand_TestFailPath)
        {
			TestFailCommand::Ptr test = TestFailCommand::Create();
			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(test);
        }

		TEST_METHOD(VariableCommand_TestAbortPath)
        {
			TestNeverEndCommand::Ptr test = TestNeverEndCommand::Create();
			CommonTests::TestAbort(test, 10);
        }
	};
}
