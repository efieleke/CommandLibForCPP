#include "MoveRobotArmOnAxisCommand.h"

MoveRobotArmOnAxisCommand::Ptr MoveRobotArmOnAxisCommand::Create(RobotArm* robot, int destination, bool xAxis)
{
	return Ptr(new MoveRobotArmOnAxisCommand(robot, destination, xAxis));
}

MoveRobotArmOnAxisCommand::MoveRobotArmOnAxisCommand(RobotArm* robotArm, int destination, bool xAxis) : m_robotArm(robotArm), m_destination(destination), m_xAxis(xAxis)
{
}

std::string MoveRobotArmOnAxisCommand::ClassName() const
{
	return "MoveRobotArmOnAxisCommand";
}

void MoveRobotArmOnAxisCommand::AsyncExecuteImpl(CommandLib::CommandListener* listener)
{
    m_listener = listener;

	std::unique_lock<std::mutex>(m_mutex);
	
	if (m_xAxis)
    {
		m_operation = m_robotArm->MoveX(m_destination, this);
    }
    else
    {
		m_operation = m_robotArm->MoveY(m_destination, this);
    }
}

void MoveRobotArmOnAxisCommand::AbortImpl()
{
	std::unique_lock<std::mutex>(m_mutex);
    
	if (m_operation)
    {
		m_operation->Abort();
    }
}

void MoveRobotArmOnAxisCommand::Completed(bool aborted)
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
