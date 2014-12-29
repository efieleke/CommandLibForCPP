#include "MoveRobotArmOnAxisCommand.h"

MoveRobotArmOnAxisCommand::Ptr MoveRobotArmOnAxisCommand::Create(RobotArm* robot, int destination, RobotArm::Axis axis)
{
	return Ptr(new MoveRobotArmOnAxisCommand(robot, destination, axis));
}

MoveRobotArmOnAxisCommand::MoveRobotArmOnAxisCommand(RobotArm* robotArm, int destination, RobotArm::Axis axis) :
	m_robotArm(robotArm), m_destination(destination), m_axis(axis)
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
	m_operation = m_robotArm->Move(m_axis, m_destination, this);
}

void MoveRobotArmOnAxisCommand::AbortImpl()
{
	std::unique_lock<std::mutex>(m_mutex);
    
	if (m_operation)
    {
		m_operation->Abort();
    }
}

void MoveRobotArmOnAxisCommand::Completed(bool aborted, const std::exception* error)
{
    if (aborted)
    {
        m_listener->CommandAborted();
    }
	else if (error == nullptr)
    {
        m_listener->CommandSucceeded();
    }
	else
	{
		m_listener->CommandFailed(*error, std::current_exception());
	}
}
