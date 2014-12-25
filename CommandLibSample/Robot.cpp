#include "Robot.h"
#include <thread>

Robot::OperationCompleteHandler::~OperationCompleteHandler()
{
}

Robot::Operation::Operation()
{
}

void Robot::Operation::Abort()
{
	m_abortEvent.Set();
}

Robot::Robot(const std::string& name, int xPos, int yPos) : m_name(name), m_xPos(xPos), m_yPos(yPos)
{
}

std::string Robot::Greeting() const
{
	return "Hello, my name is " + m_name;
}

void Robot::GetPosition(int* x, int* y) const
{
	std::unique_lock<std::mutex> lock(m_mutex);
	*x = m_xPos;
	*y = m_yPos;
}

std::shared_ptr<Robot::Operation> Robot::Move(int destination, OperationCompleteHandler* handler, int* value)
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

			aborted = operation->m_abortEvent.Wait(250);
		}

		if (handler != nullptr)
		{
			handler->Completed(aborted);
		}
	}).detach();

	operation->m_startedEvent.Wait();
	return operation;
}

std::shared_ptr<Robot::Operation> Robot::MoveX(int destination, OperationCompleteHandler* handler)
{
	return Move(destination, handler, &m_xPos);
}

std::shared_ptr<Robot::Operation> Robot::MoveY(int destination, OperationCompleteHandler* handler)
{
	return Move(destination, handler, &m_yPos);
}
