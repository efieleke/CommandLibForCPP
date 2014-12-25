#include "CmdListener.h"
#include <cassert>
#include "CppUnitTestAssert.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

CmdListener::CmdListener(CallbackType expectedCallback) :
	m_expectedCallback(expectedCallback),
	m_actualCallback(CallbackType::None)
{
	assert(expectedCallback != CallbackType::None);
}

void CmdListener::Reset(CallbackType expectedCallback)
{
	m_expectedCallback = expectedCallback;
	m_actualCallback = CallbackType::None;
	m_excPtr == nullptr;
}

void CmdListener::CommandSucceeded()
{
	m_actualCallback = CallbackType::Succeeded;
}

void CmdListener::CommandAborted()
{
	m_actualCallback = CallbackType::Aborted;
}

void CmdListener::CommandFailed(const std::exception&, std::exception_ptr excPtr)
{
	m_actualCallback = CallbackType::Failed;
	m_excPtr = excPtr;
}
