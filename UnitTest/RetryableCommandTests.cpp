#include "CppUnitTest.h"
#include "RetryableCommand.h"
#include "CommonTests.h"
#include "CmdListener.h"
#include "AddCommand.h"
#include "PauseCommand.h"
#include "FailingCommand.h"
#include <limits>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	class RetryHandler : public CommandLib::RetryableCommand::RetryCallback
	{
	public:
		template <typename Rep, typename Period>
		RetryHandler(size_t maxRetries, const std::chrono::duration<Rep, Period>& waitTime)
		{
			Reset(maxRetries, waitTime);
		}

		template <typename Rep, typename Period>
		void Reset(size_t maxRetries, const std::chrono::duration<Rep, Period>& waitTime)
		{
			m_maxRetries = maxRetries;
			m_waitTime = std::chrono::duration_cast<std::chrono::milliseconds>(waitTime);
		}

		virtual bool OnCommandFailed(size_t failNumber, const std::exception& reason, long long* waitMS) override
		{
			*waitMS = m_waitTime.count();
			return failNumber <= m_maxRetries;
		}
	private:
		size_t m_maxRetries;
		std::chrono::milliseconds m_waitTime;
	};

	TEST_CLASS(RetryableCommandTests)
	{
	public:
		TEST_METHOD(RetryableCommand_TestHappyPath)
		{
			RetryHandler handler(std::numeric_limits<size_t>::max(), std::chrono::milliseconds(1));

			CommandLib::RetryableCommand::Ptr retryableCmd = CommandLib::RetryableCommand::Create(
				CommandLib::PauseCommand::Create(0),
				&handler);

			CommonTests::TestHappyPath(retryableCmd);
		}

		TEST_METHOD(RetryableCommand_TestAbort)
		{
			RetryHandler handler(std::numeric_limits<size_t>::max(), std::chrono::milliseconds(1));

			CommandLib::RetryableCommand::Ptr retryableCmd = CommandLib::RetryableCommand::Create(
				CommandLibTests::FailingCommand::Create(),
				&handler);

			CommonTests::TestAbort(retryableCmd, 10);

			retryableCmd = CommandLib::RetryableCommand::Create(
				CommandLib::PauseCommand::Create(std::chrono::hours(24)),
				&handler);

			CommonTests::TestAbort(retryableCmd, 10);
		}

		TEST_METHOD(RetryableCommand_TestFail)
		{
			RetryHandler handler(5, std::chrono::milliseconds(1));

			CommandLib::RetryableCommand::Ptr retryableCmd = CommandLib::RetryableCommand::Create(
				CommandLibTests::FailingCommand::Create(),
				&handler);

			CommonTests::TestFail<CommandLibTests::FailingCommand::FailException>(retryableCmd);
		}
	};
}
