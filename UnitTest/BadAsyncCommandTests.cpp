#include "CppUnitTest.h"
#include "RetryableCommand.h"
#include "CommonTests.h"
#include "CmdListener.h"
#include "AddCommand.h"
#include "PauseCommand.h"
#include "FailingCommand.h"
#include "AsyncCommand.h"
#include <limits>
#include <thread>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
    class BadAsyncCommand : public CommandLib::AsyncCommand
    {
	public:
		typedef std::shared_ptr<BadAsyncCommand> Ptr;
		enum FinishType { Succeed, Fail, Abort };

		static Ptr Create(FinishType finishType)
		{
			return Ptr(new BadAsyncCommand(finishType));
		}

		virtual std::string ClassName() const override
		{
			return "BadAsyncCommand";
		}

		virtual void AsyncExecuteImpl(CommandLib::CommandListener* listener) override
		{
            switch(m_finishType)
            {
                case Abort:
                    listener->CommandAborted();
                    break;
                case Fail:
                    listener->CommandFailed(m_error, std::make_exception_ptr(m_error));
                    break;
                case Succeed:
                    listener->CommandSucceeded();
                    break;
            }
        }
	private:
		explicit BadAsyncCommand(FinishType finishType) : m_finishType(finishType), m_error("boo hoo")
		{
		}

		const FinishType m_finishType;
		const std::exception m_error;
	};

	TEST_CLASS(BadAsyncCommandTests)
	{
	public:
		TEST_METHOD(BadAsyncCommand_TestBadSuccess)
		{
			BadAsyncCommand::Ptr test = BadAsyncCommand::Create(BadAsyncCommand::FinishType::Succeed);
			CmdListener listener(CmdListener::CallbackType::Succeeded);
			Assert::ExpectException<std::logic_error>([test, &listener]() { test->AsyncExecute(&listener); }, L"Called back on same thread");
			Assert::ExpectException<std::invalid_argument>([test]() { test->AsyncExecute(nullptr); }, L"Used a null listener");
		}

		TEST_METHOD(BadAsyncCommand_TestBadFail)
		{
			BadAsyncCommand::Ptr test = BadAsyncCommand::Create(BadAsyncCommand::FinishType::Fail);
			CmdListener listener(CmdListener::CallbackType::Failed);
			Assert::ExpectException<std::logic_error>([test, &listener]() { test->AsyncExecute(&listener); }, L"Called back on same thread");
		}

		TEST_METHOD(BadAsyncCommand_TestBadAbort)
		{
			BadAsyncCommand::Ptr test = BadAsyncCommand::Create(BadAsyncCommand::FinishType::Abort);
			CmdListener listener(CmdListener::CallbackType::Aborted);
			Assert::ExpectException<std::logic_error>([test, &listener]() { test->AsyncExecute(&listener); }, L"Called back on same thread");
		}
	};
}
