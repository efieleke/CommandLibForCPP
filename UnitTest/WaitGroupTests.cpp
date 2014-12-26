#include "CppUnitTest.h"
#include "Event.h"
#include "WaitGroup.h"
#include <thread>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(WaitGroupTests)
	{
	public:
		TEST_METHOD(WaitGroupTests_TestWaitForAny)
		{
			{
				CommandLib::WaitGroup group;
				CommandLib::Event::Ptr ev1(new CommandLib::Event());
				CommandLib::Event::Ptr ev2(new CommandLib::Event());
				group.AddWaitable(ev1);
				group.AddWaitable(ev2);

				std::thread thread([ev2]()
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					ev2->Set();
					ev2->Reset(); // as long as the event was signaled at some point during the wait, we should be fine
				});

				Assert::AreEqual(group.WaitForAny(), 1);
				ev2->Reset();
				thread.join();

				std::thread thread2([]()
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100000));
				});

				Assert::AreEqual(group.WaitForAny(std::chrono::milliseconds(100)), -1);
				thread2.detach();
			}

			{
				CommandLib::WaitGroup group;
				CommandLib::Event::Ptr ev1(new CommandLib::Event(true));
				CommandLib::Event::Ptr ev2(new CommandLib::Event());
				group.AddWaitable(ev1);
				group.AddWaitable(ev2);
				Assert::AreEqual(group.WaitForAny(), 0);
			}

			{
				CommandLib::WaitGroup group;
				CommandLib::Event::Ptr ev1(new CommandLib::Event(false));
				CommandLib::Event::Ptr ev2(new CommandLib::Event(false));
				group.AddWaitable(ev1);
				group.AddWaitable(ev2);
				ev2->Set();
				Assert::AreEqual(group.WaitForAny(), 1);
			}
		}

		TEST_METHOD(WaitGroupTests_TestWaitForAll)
		{
			{
				CommandLib::WaitGroup group;
				CommandLib::Event::Ptr ev1(new CommandLib::Event());
				CommandLib::Event::Ptr ev2(new CommandLib::Event());
				group.AddWaitable(ev1);
				group.AddWaitable(ev2);

				std::thread thread([ev1, ev2]()
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
					ev2->Set();
					ev2->Reset(); // as long as the event was signaled at some point during the wait, we should be fine
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
					ev1->Set();
				});

				group.WaitForAll();
				ev2->Reset();
				ev1->Reset();
				thread.join();

				std::thread thread2([ev1]()
				{
					ev1->Set();
					std::this_thread::sleep_for(std::chrono::milliseconds(100000));
				});

				Assert::IsFalse(group.WaitForAll(std::chrono::milliseconds(100)));
				thread2.detach();
				ev2->Set();
				Assert::IsTrue(group.WaitForAll(std::chrono::milliseconds(100)));
			}

			{
				CommandLib::WaitGroup group;
				CommandLib::Event::Ptr ev1(new CommandLib::Event(true));
				CommandLib::Event::Ptr ev2(new CommandLib::Event());
				group.AddWaitable(ev1);
				group.AddWaitable(ev2);
				ev2->Set();
				group.WaitForAll();
			}
		}
	};
}
