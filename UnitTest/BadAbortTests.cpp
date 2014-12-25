#include "CppUnitTest.h"
#include "CmdListener.h"
#include "PauseCommand.h"
#include "SequentialCommands.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(BadAbortTests)
	{
	public:
		TEST_METHOD(BadAbort_TestAbortChild)
		{
			CommandLib::SequentialCommands::Ptr seqCmd = CommandLib::SequentialCommands::Create();
			CommandLib::PauseCommand::Ptr pauseCmd = CommandLib::PauseCommand::Create(0);
			seqCmd->Add(pauseCmd);
			Assert::ExpectException<std::logic_error>([pauseCmd]() { pauseCmd->Abort(); }, L"Aborted an owned command");
		}
	};
}
