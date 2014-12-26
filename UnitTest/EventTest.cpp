#include "CppUnitTest.h"
#include "Event.h"
#include "WaitMonitor.h"
#include <thread>
#include <atomic>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	class TestMonitor : public CommandLib::WaitMonitor
	{
	public:
		TestMonitor()
		{
			m_invokedCount = 0;
		}

		virtual void Signaled(const CommandLib::Waitable&) override final
		{
			++m_invokedCount;
		}

		unsigned int InvokedCount() const
		{
			return m_invokedCount;
		}
	private:
		std::atomic_uint m_invokedCount;
	};

	TEST_CLASS(EventTest)
	{
	public:
		TEST_METHOD(EventTest_TestRemoveListener)
		{
			CommandLib::Event ev;
			Assert::IsFalse(ev.IsSignaled());
			std::shared_ptr<TestMonitor> monitor(new TestMonitor());
			Assert::IsFalse(ev.RemoveListener(monitor));
			Assert::IsTrue(ev.AddListener(monitor));
			Assert::IsFalse(ev.AddListener(monitor));
			Assert::IsTrue(ev.RemoveListener(monitor));
			Assert::IsFalse(ev.RemoveListener(monitor));
		}

		TEST_METHOD(EventTest_TestSet)
		{
			CommandLib::Event ev(false);
			Assert::IsFalse(ev.IsSignaled());
			ev.Set();
			Assert::IsTrue(ev.IsSignaled());
			Assert::IsFalse(CommandLib::Event().IsSignaled());
		}

		TEST_METHOD(EventTest_TestReset)
		{
			CommandLib::Event ev(true);
			ev.Reset();
			Assert::IsFalse(ev.IsSignaled());
			ev.Set();
			Assert::IsTrue(ev.IsSignaled());
			ev.Reset();
			Assert::IsFalse(ev.IsSignaled());
		}

		TEST_METHOD(EventTest_TestWaitUntilSignaled)
		{
			CommandLib::Event ev;

			std::thread thread([&ev]()
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				ev.Set();
			});

			ev.Wait();
			ev.Reset();
			thread.join();

			std::thread thread2([]()
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100000));
			});

			Assert::IsFalse(ev.WaitFor(std::chrono::nanoseconds(0)));
			thread2.detach();
			ev.Set();
			Assert::IsTrue(ev.WaitFor(std::chrono::nanoseconds(0)));
		}

		TEST_METHOD(EventTest_TestAddListener)
		{
			std::shared_ptr<TestMonitor> monitor(new TestMonitor());
			CommandLib::Event ev;
			ev.AddListener(monitor);
			ev.Set();
			Assert::AreEqual(monitor->InvokedCount(), 1U);
		}
	};
}
