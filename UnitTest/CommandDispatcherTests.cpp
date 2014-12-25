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

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	class Monitor : public CommandLib::CommandMonitor
	{
	public:
		Monitor()
		{
			Reset();
		}

		void Reset()
		{
			m_completed = 0;
			m_aborted = 0;
			m_failed = 0;
		}

		virtual void CommandStarting(const CommandLib::Command& command) override
		{
		}

		virtual void CommandFinished(const CommandLib::Command& command, const std::exception* exc) override
		{
			if (exc == nullptr)
			{
				++m_completed;
			}
			else if (dynamic_cast<const CommandLib::CommandAbortedException*>(exc) == nullptr)
			{
				++m_failed;
			}
			else
			{
				++m_aborted;
			}
		}

		std::atomic_uint m_completed;
		std::atomic_uint m_aborted;
		std::atomic_uint m_failed;
	};

	TEST_CLASS(CommandDispatcherTests)
	{
	public:
		TEST_METHOD(CommandDispatcher_TestAbort)
		{
			CommandLib::CommandDispatcher dispatcher(2);
			Monitor monitor;
			dispatcher.AddMonitor(&monitor);
			dispatcher.Dispatch(CommandLib::PauseCommand::Create(std::chrono::hours(24)));
			dispatcher.Dispatch(CommandLib::PauseCommand::Create(0));
			dispatcher.Dispatch(CommandLib::PauseCommand::Create(0));
			dispatcher.Dispatch(CommandLib::PauseCommand::Create(std::chrono::hours(24)));
			dispatcher.Dispatch(CommandLib::PauseCommand::Create(std::chrono::hours(24)));
			dispatcher.Dispatch(CommandLib::PauseCommand::Create(std::chrono::hours(24)));
			std::this_thread::sleep_for(std::chrono::milliseconds(20)); // give time for the thread to start executing
			dispatcher.AbortAndWait();
			Assert::AreEqual(2U, monitor.m_completed.operator size_t());
			Assert::AreEqual(0U, monitor.m_failed.operator size_t());
			Assert::AreEqual(2U, monitor.m_aborted.operator size_t());
		};

		TEST_METHOD(CommandDispatcher_TestHappyPath)
		{
			Monitor monitor;

			{
				CommandLib::CommandDispatcher dispatcher(2);
				dispatcher.AddMonitor(&monitor);
				dispatcher.Dispatch(CommandLib::PauseCommand::Create(10));
				dispatcher.Dispatch(CommandLib::PauseCommand::Create(0));
				dispatcher.Dispatch(CommandLib::PauseCommand::Create(0));
				dispatcher.Dispatch(CommandLib::PauseCommand::Create(100));
				dispatcher.Dispatch(CommandLibTests::FailingCommand::Create());
				dispatcher.Dispatch(CommandLib::PauseCommand::Create(0));
			}

			Assert::AreEqual(5U, monitor.m_completed.operator size_t());
			Assert::AreEqual(1U, monitor.m_failed.operator size_t());
			Assert::AreEqual(0U, monitor.m_aborted.operator size_t());
		}

		TEST_METHOD(CommandDispatcher_TestBadArgs)
		{
			Assert::ExpectException<std::invalid_argument>([](){ CommandLib::CommandDispatcher(0); }, L"Dispatcher with 0 pool size constructed");
			Monitor monitor;
			CommandLib::CommandDispatcher dispatcher(2);
			dispatcher.AddMonitor(&monitor);
			CommandLib::PauseCommand::Ptr pauseCmd = CommandLib::PauseCommand::Create(std::chrono::hours(24));
			CommandLib::SequentialCommands::Ptr seq = CommandLib::SequentialCommands::Create();
			seq->Add(pauseCmd);
			Assert::ExpectException<std::invalid_argument>([&dispatcher, seq, pauseCmd](){ dispatcher.Dispatch(pauseCmd); }, L"Dispatched a child command.");
		}
	};
}
