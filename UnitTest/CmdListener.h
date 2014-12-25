#pragma once
#include "CommandListener.h"
#include <string>

class CmdListener :
	public CommandLib::CommandListener
{
public:
	enum class CallbackType
	{
		Succeeded,
		Aborted,
		Failed,
		None
	};

	explicit CmdListener(CallbackType expectedCallback);
	void Reset(CallbackType expectedCallback);

	virtual void CommandSucceeded() override final;
	virtual void CommandAborted() override final;
	virtual void CommandFailed(const std::exception& exc, std::exception_ptr excPtr) override final;

	template<typename T = std::exception>
	void Check() const
	{
		if (m_actualCallback == CallbackType::Failed)
		{
			if (m_expectedCallback == CallbackType::Failed)
			{
				Assert::ExpectException<T>([this](){ std::rethrow_exception(m_excPtr); }, L"Caught unexpected type of exception");
			}
			else
			{
				try
				{
					std::rethrow_exception(m_excPtr);
				}
				catch (std::exception& exc)
				{
					Assert::AreEqual(m_expectedCallback, m_actualCallback, ToString(exc.what()).c_str());
				}
			}
		}
		else
		{
			Assert::AreEqual(m_expectedCallback, m_actualCallback);
		}
	}
private:
	CallbackType m_expectedCallback;
	CallbackType m_actualCallback;
	std::exception_ptr m_excPtr;
};

namespace Microsoft
{
	namespace VisualStudio
	{
		namespace CppUnitTestFramework
		{
			static std::wstring ToString (const CmdListener::CallbackType& t)
			{
				switch (t)
				{
				case CmdListener::CallbackType::None:
					return L"CmdListener::CallbackType::None";
				case CmdListener::CallbackType::Aborted:
					return L"CmdListener::CallbackType::Aborted";
				case CmdListener::CallbackType::Failed:
					return L"CmdListener::CallbackType::Failed";
				case CmdListener::CallbackType::Succeeded:
					return L"CmdListener::CallbackType::Succeeded";
				default:
					return L"Unknown Callback Type";
				}
			}
		}
	}
}
