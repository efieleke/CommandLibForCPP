#include "ReportPositionCommand.h"
#include <iostream>

std::string ReportPositionCommand::ClassName() const
{
	return "ReportPositionCommand";
}

ReportPositionCommand::Ptr ReportPositionCommand::Create(const RobotArm& robot)
{
	return Ptr(new ReportPositionCommand(robot));
}

ReportPositionCommand::ReportPositionCommand(const RobotArm& robot) : m_robot(robot)
{
}

void ReportPositionCommand::SyncExeImpl()
{
    int x, y;
    m_robot.GetPosition(&x, &y);
	std::cout << "Robot arm is at position " + std::to_string(x) + "," + std::to_string(y) << std::endl;
}
