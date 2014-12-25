#include "MoveRobotArmCommand.h"
#include "MoveRobotArmOnAxisCommand.h"

MoveRobotArmCommand::Ptr MoveRobotArmCommand::Create(RobotArm* robotArm, int x, int y)
{
	Ptr result(new MoveRobotArmCommand());
	result->Add(MoveRobotArmOnAxisCommand::Create(robotArm, x, true));
	result->Add(MoveRobotArmOnAxisCommand::Create(robotArm, y, false));
	return result;
}

MoveRobotArmCommand::MoveRobotArmCommand() : ParallelCommands(true)
{
}

std::string MoveRobotArmCommand::ClassName() const
{
	return "MoveRobotArmCommand";
}
