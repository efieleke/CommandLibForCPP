#include "MoveRobotOnAxisCommand.h"

MoveRobotOnAxisCommand::Ptr MoveRobotOnAxisCommand::Create(Robot* robot, int destination, bool xAxis)
{
	return Ptr(new MoveRobotOnAxisCommand(robot, destination, xAxis));
}

MoveRobotOnAxisCommand::MoveRobotOnAxisCommand(Robot* robot, int destination, bool xAxis) : m_robot(robot), m_destination(destination), m_xAxis(xAxis)
{
}

std::string MoveRobotOnAxisCommand::ClassName() const
{
	return "MoveRobotOnAxisCommand";
}

void MoveRobotOnAxisCommand::AsyncExecuteImpl(CommandLib::CommandListener* listener)
{
    m_listener = listener;

	std::unique_lock<std::mutex>(m_mutex);
	
	if (m_xAxis)
    {
        m_operation = m_robot->MoveX(m_destination, this);
    }
    else
    {
        m_operation = m_robot->MoveY(m_destination, this);
    }
}

void MoveRobotOnAxisCommand::AbortImpl()
{
	std::unique_lock<std::mutex>(m_mutex);
    
	if (m_operation)
    {
		m_operation->Abort();
    }
}

void MoveRobotOnAxisCommand::Completed(bool aborted)
{
    if (aborted)
    {
        m_listener->CommandAborted();
    }
    else
    {
        m_listener->CommandSucceeded();
    }
}
