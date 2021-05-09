#include "CppUnitTest.h"
#include "PauseCommand.h"
#include "CommonTests.h"
#include "CmdListener.h"
#include "SyncCommand.h"
#include "SequentialCommands.h"
#include "CommandAbortedException.h"
#include "FinallyCommand.h"
#include "AddCommand.h"
#include "FailingCommand.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace CommandLib;
using namespace CommandLibTests;

namespace UnitTest
{
	TEST_CLASS(FinallyCommandTests)
	{
	public:
		TEST_METHOD(FinallyCommand_TestAbort)
		{
            CommonTests::TestAbort(FinallyCommand::Create(PauseCommand::Create(std::chrono::hours(24)), CleanupCommand::Create(CleanupCommand::Behavior::Succeed), false), 10);
            CommonTests::TestAbort(FinallyCommand::Create(PauseCommand::Create(std::chrono::hours(24)), CleanupCommand::Create(CleanupCommand::Behavior::Abort), false), 10);
            CommonTests::TestAbort(FinallyCommand::Create(PauseCommand::Create(std::chrono::hours(24)), CleanupCommand::Create(CleanupCommand::Behavior::Fail), false), 10);

            CleanupCommand::Ptr cleanupCmd = CleanupCommand::Create(CleanupCommand::Behavior::Succeed);

            {
                FinallyCommand::Ptr cmd = FinallyCommand::Create(PauseCommand::Create(std::chrono::hours(24)), cleanupCmd, false);
                CmdListener listener(CmdListener::CallbackType::None);
                cmd->AsyncExecute(&listener);

                // If we abort super quick, the framework will raise the abort exception before the FinallyCommand even startsSystem.Threading.Thread.Sleep(100);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                cmd->AbortAndWait();
                Assert::IsFalse(cleanupCmd->Executed());
            }

            cleanupCmd = CleanupCommand::Create(CleanupCommand::Behavior::Succeed);

            {
                FinallyCommand::Ptr cmd = FinallyCommand::Create(PauseCommand::Create(std::chrono::hours(24)), cleanupCmd, true);
                CmdListener listener(CmdListener::CallbackType::None);
                cmd->AsyncExecute(&listener);

                // If we abort super quick, the framework will raise the abort exception before the FinallyCommand even startsSystem.Threading.Thread.Sleep(100);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                cmd->AbortAndWait();
                Assert::IsTrue(cleanupCmd->Executed());
            }
        }

		TEST_METHOD(FinallyCommand_TestHappyPath)
		{
            std::atomic_int value;
            CommonTests::TestHappyPath(FinallyCommand::Create(AddCommand::Create(&value, 6), CleanupCommand::Create(CleanupCommand::Behavior::Succeed), false));
            Assert::AreEqual(12, value.load()); // TestHappyPath executes the command twice
            CommonTests::TestHappyPath(FinallyCommand::Create(PauseCommand::Create(0), CleanupCommand::Create(CleanupCommand::Behavior::Succeed), false));
		}

        TEST_METHOD(FinallyCommand_TestFail)
        {
            CommonTests::TestFail<FailingCommand::FailException>(FinallyCommand::Create(FailingCommand::Create(), CleanupCommand::Create(CleanupCommand::Behavior::Succeed), false));
            CommonTests::TestFail<FailingCommand::FailException>(FinallyCommand::Create(FailingCommand::Create(), CleanupCommand::Create(CleanupCommand::Behavior::Fail), false));
            CommonTests::TestFail<FailingCommand::FailException>(FinallyCommand::Create(FailingCommand::Create(), CleanupCommand::Create(CleanupCommand::Behavior::Abort), false));
            CommonTests::TestFail<MockException>(FinallyCommand::Create(PauseCommand::Create(0), CleanupCommand::Create(CleanupCommand::Behavior::Fail), false));
        }

        class MockException : public std::exception {};
    private:
        class CleanupCommand : public SyncCommand
        {
        public:
            typedef std::shared_ptr<CleanupCommand> Ptr;
            enum class Behavior { Succeed, Fail, Abort };
            
            static Ptr Create(Behavior behavior)
            {
                return Ptr(new CleanupCommand(behavior));
            }

            virtual std::string ClassName() const override
            {
                return "CleanupCommand";
            }

            bool Executed() const
            {
                return m_executed;
            }
        private:
            CleanupCommand(Behavior behavior) :
                m_behavior(behavior),
                m_executed(false),
                m_nestedCommands(CommandLib::SequentialCommands::Create())
            {
                m_nestedCommands->Add(CommandLib::PauseCommand::Create(0));
            }

            virtual void SyncExeImpl() override
            {
                CheckAbortFlag();
                m_executed = true;

                switch (m_behavior)
                {
                case Behavior::Succeed:
                    m_nestedCommands->SyncExecute();
                    break;
                case Behavior::Abort:
                    throw CommandLib::CommandAbortedException();
                case Behavior::Fail:
                    throw MockException();
                default:
                    Assert::Fail(L"Should not get here");
                    break;
                }
            }

            const Behavior m_behavior;
            SequentialCommands::Ptr m_nestedCommands;
            bool m_executed;
        };
	};
}