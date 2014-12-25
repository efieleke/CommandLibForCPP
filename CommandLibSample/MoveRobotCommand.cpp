#include "MoveRobotCommand.h"
#include "MoveRobotOnAxisCommand.h"

MoveRobotCommand::Ptr MoveRobotCommand::Create(Robot* robot, int x, int y)
{
	Ptr result(new MoveRobotCommand());
	result->Add(MoveRobotOnAxisCommand::Create(robot, x, true));
	result->Add(MoveRobotOnAxisCommand::Create(robot, y, false));
	return result;
}

MoveRobotCommand::MoveRobotCommand() : ParallelCommands(true)
{
}

std::string MoveRobotCommand::ClassName() const
{
	return "MoveRobotCommand";
}
