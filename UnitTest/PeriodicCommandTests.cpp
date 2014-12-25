#include "CppUnitTest.h"
#include "PeriodicCommand.h"
#include "CommonTests.h"
#include "CmdListener.h"
#include "AddCommand.h"
#include "PauseCommand.h"
#include "FailingCommand.h"
#include "CommandTimeoutException.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(PeriodicCommandTests)
	{
	public:
		TEST_METHOD(PeriodicCommand_TestAbort)
		{
			CommandLib::PeriodicCommand::Ptr periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLib::PauseCommand::Create(10),
				std::numeric_limits<size_t>::max(),
				std::chrono::hours(24),
				CommandLib::PeriodicCommand::IntervalType::PauseAfter,
				false);

			CommonTests::TestAbort(periodicCmd, 20);

			periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLib::PauseCommand::Create(10),
				std::numeric_limits<size_t>::max(),
				std::chrono::hours(24),
				CommandLib::PeriodicCommand::IntervalType::PauseBefore,
				false);

			CommonTests::TestAbort(periodicCmd, 20);

			periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLib::PauseCommand::Create(10),
				std::numeric_limits<size_t>::max(),
				std::chrono::hours(24),
				CommandLib::PeriodicCommand::IntervalType::PauseAfter,
				true);

			CommonTests::TestAbort(periodicCmd, 20);
		}

		TEST_METHOD(PeriodicCommand_TestHappyPath)
        {
			CommandLib::PeriodicCommand::Ptr periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLib::PauseCommand::Create(0),
				5,
				1,
				CommandLib::PeriodicCommand::IntervalType::PauseAfter,
				false);

			CommonTests::TestHappyPath(periodicCmd);

			periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLib::PauseCommand::Create(0),
				5,
				1,
				CommandLib::PeriodicCommand::IntervalType::PauseAfter,
				false);

			CommonTests::TestHappyPath(periodicCmd);

			periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLib::PauseCommand::Create(0),
				5,
				10,
				CommandLib::PeriodicCommand::IntervalType::PauseAfter,
				true);

			CommonTests::TestHappyPath(periodicCmd);

			periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLib::PauseCommand::Create(0),
				0,
				std::chrono::hours(24),
				CommandLib::PeriodicCommand::IntervalType::PauseBefore,
				true);

			CommonTests::TestHappyPath(periodicCmd);
        }

		TEST_METHOD(PeriodicCommand_TestFail)
        {
			CommandLib::PeriodicCommand::Ptr periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLibTests::FailingCommand::Create(),
				5,
				1,
				CommandLib::PeriodicCommand::IntervalType::PauseAfter,
				false);
			
			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(periodicCmd);

			periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLibTests::FailingCommand::Create(),
				5,
				1,
				CommandLib::PeriodicCommand::IntervalType::PauseBefore,
				false);

			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(periodicCmd);

			periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLibTests::FailingCommand::Create(),
				5,
				1,
				CommandLib::PeriodicCommand::IntervalType::PauseAfter,
				true);
		
			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(periodicCmd);
        }

		TEST_METHOD(PeriodicCommand_TestSkipCurrentWait)
        {
			CommandLib::PeriodicCommand::Ptr periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLib::PauseCommand::Create(0),
				3,
				std::chrono::hours(24),
				CommandLib::PeriodicCommand::IntervalType::PauseAfter,
				false);

			CmdListener listener(CmdListener::CallbackType::Succeeded);
            periodicCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give the async routine a moment to get going

            for(int i = 0; i < 2; ++i)
            {
                periodicCmd->SkipCurrentWait();
				std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give a little time for the next repetition to start
            }

            periodicCmd->Wait();
            listener.Check();

            listener.Reset(CmdListener::CallbackType::Succeeded);
            periodicCmd->SkipCurrentWait(); // no-op
            periodicCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(20)); // give the async routine a moment to get going

            for (int i = 0; i < 1; ++i)
            {
                periodicCmd->SkipCurrentWait();
				std::this_thread::sleep_for(std::chrono::milliseconds(20)); // give a little time for the next repetition to start
			}

            Assert::IsFalse(periodicCmd->Wait(10));
            periodicCmd->SkipCurrentWait();
            periodicCmd->Wait();
            listener.Check();
        }

		TEST_METHOD(PeriodicCommand_TestReset)
        {
			CommandLib::PeriodicCommand::Ptr periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLib::PauseCommand::Create(0),
				2,
				std::chrono::hours(24),
				CommandLib::PeriodicCommand::IntervalType::PauseBefore,
				true);
			
			Assert::AreEqual(std::chrono::hours(24).count(), periodicCmd->GetInterval<std::chrono::hours>().count());
            CmdListener listener(CmdListener::CallbackType::Succeeded);
			periodicCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give the async routine a moment to get going
			periodicCmd->SetIntervalMS(1);
            Assert::IsFalse(periodicCmd->Wait(10));
            periodicCmd->Reset();
            Assert::IsTrue(periodicCmd->Wait(std::chrono::seconds(1)));
            listener.Check();
            Assert::AreEqual(1L, static_cast<long>(periodicCmd->GetIntervalMS()));

            listener.Reset(CmdListener::CallbackType::Aborted);
            periodicCmd->SetIntervalMS(100);
            periodicCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(20)); // give the async routine a moment to get going
			periodicCmd->SetInterval(std::chrono::hours(24));
            periodicCmd->Reset();
            Assert::IsFalse(periodicCmd->Wait(100));
            periodicCmd->AbortAndWait();
            listener.Check();

			periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLib::PauseCommand::Create(0),
				1,
				std::chrono::hours(24),
				CommandLib::PeriodicCommand::IntervalType::PauseBefore,
				true);

			listener.Reset(CmdListener::CallbackType::Succeeded);
            periodicCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give the async routine a moment to get going
			periodicCmd->Reset();
            periodicCmd->SkipCurrentWait();
            periodicCmd->Wait();
            listener.Check();
        }

		TEST_METHOD(PeriodicCommand_TestStop)
		{
			CommandLib::PeriodicCommand::Ptr periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLib::PauseCommand::Create(0),
				2,
				std::chrono::hours(24),
				CommandLib::PeriodicCommand::IntervalType::PauseBefore,
				true);

            periodicCmd->Stop();
            CmdListener listener(CmdListener::CallbackType::Succeeded);
            periodicCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(10)); // give the async routine a moment to get going
			periodicCmd->Stop();
            periodicCmd->Wait();
            listener.Check();
            periodicCmd->Stop();

            listener.Reset(CmdListener::CallbackType::Succeeded);
            periodicCmd->AsyncExecute(&listener);
            periodicCmd->Stop();
            periodicCmd->Wait();
            listener.Check();

            listener.Reset(CmdListener::CallbackType::Succeeded);
            periodicCmd->AsyncExecute(&listener);
			periodicCmd->m_repeatCount = std::numeric_limits<unsigned int>::max();
			periodicCmd->SetIntervalMS(0);
            periodicCmd->SkipCurrentWait();
			std::this_thread::sleep_for(std::chrono::milliseconds(10)); // give time for the command to execute many times
            periodicCmd->Stop();
            periodicCmd->Wait();
            listener.Check();

			CommandLib::Event::Ptr stopEvent(new CommandLib::Event());

			periodicCmd = CommandLib::PeriodicCommand::Create(
				CommandLib::PauseCommand::Create(0),
				2,
				std::chrono::hours(24),
				CommandLib::PeriodicCommand::IntervalType::PauseBefore,
				true,
				stopEvent);
				
            listener.Reset(CmdListener::CallbackType::Succeeded);
            periodicCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(10)); // give the async routine a moment to get going
			stopEvent->Set();
            periodicCmd->Wait();
            listener.Check();

            listener.Reset(CmdListener::CallbackType::Succeeded);
			periodicCmd->AsyncExecute(&listener);
			periodicCmd->Wait();
            listener.Check();

            stopEvent->Reset();
            listener.Reset(CmdListener::CallbackType::Succeeded);
            periodicCmd->AsyncExecute(&listener);
			periodicCmd->m_repeatCount = std::numeric_limits<unsigned int>::max();
			periodicCmd->SetIntervalMS(0);
            periodicCmd->SkipCurrentWait();
			std::this_thread::sleep_for(std::chrono::milliseconds(10)); // give time for the command to execute many times
			stopEvent->Set();
            periodicCmd->Wait();
            listener.Check();
        }
	};
}
