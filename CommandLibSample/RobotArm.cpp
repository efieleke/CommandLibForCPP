#include "RobotArm.h"
#include <thread>

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

RobotArm::RobotArm(int xPos, int yPos) : m_xPos(xPos), m_yPos(yPos)
{
}

void RobotArm::GetPosition(int* x, int* y) const
{
	std::unique_lock<std::mutex> lock(m_mutex);
	*x = m_xPos;
	*y = m_yPos;
}

std::shared_ptr<RobotArm::Operation> RobotArm::Move(int destination, OperationCompleteHandler* handler, int* value)
{
	std::shared_ptr<Operation> operation(new Operation());

	std::thread ([=]()
	{
		operation->m_startedEvent.Set();
		bool aborted = false;

		while (!aborted)
		{
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
			handler->Completed(aborted);
		}
	}).detach();

	operation->m_startedEvent.Wait();
	return operation;
}

std::shared_ptr<RobotArm::Operation> RobotArm::MoveX(int destination, OperationCompleteHandler* handler)
{
	return Move(destination, handler, &m_xPos);
}

std::shared_ptr<RobotArm::Operation> RobotArm::MoveY(int destination, OperationCompleteHandler* handler)
{
	return Move(destination, handler, &m_yPos);
}
