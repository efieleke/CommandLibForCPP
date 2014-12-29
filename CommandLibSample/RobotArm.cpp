#include "RobotArm.h"
#include <thread>
#include <random>
#include <functional>

RobotArm::OverheatedException::OverheatedException(const char* message) : std::runtime_error(message)
{
}

RobotArm::OperationCompleteHandler::~OperationCompleteHandler()
{
}

RobotArm::Operation::Operation()
{
}

void RobotArm::Operation::Abort()
{
	m_abortEvent.Set();
}

RobotArm::RobotArm() : m_xPos(0), m_yPos(0), m_zPos(0), m_clampOpen(false)
{
}

void RobotArm::GetPosition(int* x, int* y, int* z) const
{
	std::unique_lock<std::mutex> lock(m_mutex);
	*x = m_xPos;
	*y = m_yPos;
	*z = m_zPos;
}

std::shared_ptr<RobotArm::Operation> RobotArm::Move(Axis axis, int destination, OperationCompleteHandler* handler, int* value)
{
	std::shared_ptr<Operation> operation(new Operation());

	std::thread ([=]()
	{
		operation->m_startedEvent.Set();
		std::random_device rd;
		bool aborted = false;

		try
		{
			while (!aborted)
			{
				std::default_random_engine random_number_engine(rd());
				std::uniform_int_distribution<int> dice_distribution(1, 100);

				if (std::bind(dice_distribution, random_number_engine)() == 1)
				{
					throw OverheatedException(std::string(
						"Error: axis " + std::string(axis == X ? "X" : (axis == Y ? "Y" : "Z")) + " motor has overheated.").c_str());
				}

				{
					std::unique_lock<std::mutex> lock(m_mutex);

					if (*value > destination)
					{
						--(*value);
					}
					else if (*value < destination)
					{
						++(*value);
					}
					else
					{
						break;
					}
				}

				aborted = operation->m_abortEvent.Wait(125);
			}

			if (handler != nullptr)
			{
				handler->Completed(aborted, nullptr);
			}
		}
		catch (std::exception& exc)
		{
			if (handler != nullptr)
			{
				handler->Completed(false, &exc);
			}
		}

	}).detach();

	operation->m_startedEvent.Wait();
	return operation;
}

std::shared_ptr<RobotArm::Operation> RobotArm::Move(Axis axis, int destination, OperationCompleteHandler* handler)
{
	return Move(axis, destination, handler, axis == X ? &m_xPos : (axis == Y ? &m_yPos : &m_zPos));
}

void RobotArm::OpenClamp()
{
	m_clampOpen = true;
}

bool RobotArm::CloseClamp()
{
	if (m_clampOpen)
	{
		m_clampOpen = false;
		return rand() % 5 == 0;
	}

	return false;
}
