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
#include "AsyncCommand.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	class BumAsyncCommand : public CommandLib::AsyncCommand
	{
	public:
		class BumException : public std::exception
		{
		public:
			BumException(const char* what) : std::exception(what) {}
		};

		typedef std::shared_ptr<BumAsyncCommand> Ptr;
		static BumAsyncCommand::Ptr Create() { return Ptr(new BumAsyncCommand()); }

		virtual std::string ClassName() const override { return "BumAsyncCommand"; }
	private:
		BumAsyncCommand() {}

		virtual void AsyncExecuteImpl(CommandLib::CommandListener* listener) override final
		{
			throw BumException("boo hoo");
		}
	};

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

		TEST_METHOD(CommandDispatcher_TestBumAsyncCommand)
		{
			Monitor monitor;

			{
				CommandLib::CommandDispatcher dispatcher(1);
				dispatcher.AddMonitor(&monitor);
				dispatcher.Dispatch(CommandLib::PauseCommand::Create(10));
				dispatcher.Dispatch(BumAsyncCommand::Create());
				dispatcher.Dispatch(BumAsyncCommand::Create());
			}

			Assert::AreEqual(1U, monitor.m_completed.operator size_t());
			Assert::AreEqual(2U, monitor.m_failed.operator size_t());
			Assert::AreEqual(0U, monitor.m_aborted.operator size_t());
			monitor.Reset();

			{
				CommandLib::CommandDispatcher dispatcher(1);
				dispatcher.AddMonitor(&monitor);
                Assert::ExpectException<BumAsyncCommand::BumException>([&dispatcher]() { dispatcher.Dispatch(BumAsyncCommand::Create()); }, L"Caught unexpected type of exception");
			}

			Assert::AreEqual(0U, monitor.m_completed.operator size_t());
			Assert::AreEqual(0U, monitor.m_failed.operator size_t());
			Assert::AreEqual(0U, monitor.m_aborted.operator size_t());
			monitor.Reset();

			{
				CommandLib::CommandDispatcher dispatcher(10);
				dispatcher.AddMonitor(&monitor);
				dispatcher.Dispatch(CommandLib::PauseCommand::Create(10));
				Assert::ExpectException<BumAsyncCommand::BumException>([&dispatcher]() { dispatcher.Dispatch(BumAsyncCommand::Create()); }, L"Caught unexpected type of exception");
				Assert::ExpectException<BumAsyncCommand::BumException>([&dispatcher]() { dispatcher.Dispatch(BumAsyncCommand::Create()); }, L"Caught unexpected type of exception");
				dispatcher.Dispatch(CommandLib::PauseCommand::Create(0));
				Assert::ExpectException<BumAsyncCommand::BumException>([&dispatcher]() { dispatcher.Dispatch(BumAsyncCommand::Create()); }, L"Caught unexpected type of exception");
			}

			Assert::AreEqual(2U, monitor.m_completed.operator size_t());
			Assert::AreEqual(0U, monitor.m_failed.operator size_t());
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
