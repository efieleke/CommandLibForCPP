#include "CppUnitTest.h"
#include "RecurringCommand.h"
#include "CommonTests.h"
#include "CmdListener.h"
#include "AddCommand.h"
#include "PauseCommand.h"
#include "FailingCommand.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	class RecurCallback : public CommandLib::RecurringCommand::ExecutionTimeCallback
	{
	public:
		template <typename RepFirst, typename PeriodFirst, typename RepRest, typename PeriodRest>
		RecurCallback(
			const std::chrono::duration<RepFirst, PeriodFirst>& intervalBeforeFirst,
			const std::chrono::duration<RepRest, PeriodRest>& intervalBeforeRest,
			int repetitions)
		{
			Reset(intervalBeforeFirst, intervalBeforeRest, repetitions);
		}

		template <typename RepFirst, typename PeriodFirst, typename RepRest, typename PeriodRest>
		void Reset(
			const std::chrono::duration<RepFirst, PeriodFirst>& intervalBeforeFirst,
			const std::chrono::duration<RepRest, PeriodRest>& intervalBeforeRest,
			int repetitions)
		{
			m_intervalBeforeFirst = std::chrono::duration_cast<std::chrono::milliseconds>(intervalBeforeFirst);
			m_intervalBeforeRest = std::chrono::duration_cast<std::chrono::milliseconds>(intervalBeforeRest);
			m_repetitions = repetitions;
		}

		virtual bool GetFirstExecutionTime(std::chrono::time_point<std::chrono::system_clock>* time) override
		{
			*time = std::chrono::system_clock::now() + m_intervalBeforeFirst;
			return --m_repetitions >= 0;
		}

		virtual bool GetNextExecutionTime(std::chrono::time_point<std::chrono::system_clock>* time) override
		{
			if (--m_repetitions >= 0)
			{
				*time = std::chrono::system_clock::now() + m_intervalBeforeRest;
				return true;
			}

			return false;
		}
	private:
		std::chrono::milliseconds m_intervalBeforeFirst;
		std::chrono::milliseconds m_intervalBeforeRest;
		int m_repetitions;
	};

	TEST_CLASS(RecurringCommandTests)
	{
	public:
		TEST_METHOD(RecurringCommand_TestHappyPath)
		{
			RecurCallback callback(std::chrono::milliseconds(5), std::chrono::milliseconds(5), 7);
			CommandLib::RecurringCommand::Ptr recurringCmd = CommandLib::RecurringCommand::Create(CommandLib::PauseCommand::Create(0), &callback);
			CommonTests::TestHappyPath(recurringCmd);

			callback.Reset(std::chrono::hours(-24), std::chrono::hours(-24), 7);
			recurringCmd = CommandLib::RecurringCommand::Create(CommandLib::PauseCommand::Create(0), &callback);
			CommonTests::TestHappyPath(recurringCmd);
		}

		TEST_METHOD(RecurringCommand_TestAbort)
        {
			RecurCallback callback(std::chrono::hours(24), std::chrono::hours(24), 7);
			CommandLib::RecurringCommand::Ptr recurringCmd = CommandLib::RecurringCommand::Create(CommandLib::PauseCommand::Create(0), &callback);
			CommonTests::TestAbort(recurringCmd, 20);

			callback.Reset(std::chrono::hours(-24), std::chrono::hours(24), 7);
			recurringCmd = CommandLib::RecurringCommand::Create(CommandLib::PauseCommand::Create(0), &callback);
			CommonTests::TestAbort(recurringCmd, 20);

			callback.Reset(std::chrono::hours(-24), std::chrono::hours(-24), 7);
			recurringCmd = CommandLib::RecurringCommand::Create(CommandLib::PauseCommand::Create(std::chrono::hours(24)), &callback);
			CommonTests::TestAbort(recurringCmd, 20);
        }

		TEST_METHOD(RecurringCommand_TestFail)
		{
			RecurCallback callback(std::chrono::milliseconds(5), std::chrono::milliseconds(5), 7);
			CommandLib::RecurringCommand::Ptr recurringCmd = CommandLib::RecurringCommand::Create(CommandLibTests::FailingCommand::Create(), &callback);
			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(recurringCmd);
		}

		TEST_METHOD(RecurringCommand_TestSkipCurrentWait)
		{
			RecurCallback callback(std::chrono::hours(24), std::chrono::hours(24), 2);
			CmdListener listener(CmdListener::CallbackType::Succeeded);
			CommandLib::RecurringCommand::Ptr recurringCmd = CommandLib::RecurringCommand::Create(CommandLib::PauseCommand::Create(0), &callback);
			recurringCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give time for the thread to start executing
			recurringCmd->SkipCurrentWait();
			Assert::IsFalse(recurringCmd->Wait(100));
			recurringCmd->SkipCurrentWait();
			recurringCmd->Wait();
			listener.Check();
		}

		TEST_METHOD(RecurringCommand_TestSetNextExecutionTime)
		{
			RecurCallback callback(std::chrono::hours(24), std::chrono::hours(24), 1);
			CmdListener listener(CmdListener::CallbackType::Succeeded);
			CommandLib::RecurringCommand::Ptr recurringCmd = CommandLib::RecurringCommand::Create(CommandLib::PauseCommand::Create(0), &callback);
			recurringCmd->AsyncExecute(&listener);
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give time for the thread to start executing
			recurringCmd->SetNextExecutionTime(std::chrono::system_clock::now());
			recurringCmd->Wait();
			listener.Check();
		}
	};
}
