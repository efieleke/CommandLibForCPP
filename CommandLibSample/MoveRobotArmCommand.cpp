#include "MoveRobotArmCommand.h"
#include "MoveRobotArmOnAxisCommand.h"
#include <iostream>

MoveRobotArmCommand::Ptr MoveRobotArmCommand::Create(RobotArm* robotArm, int x, int y, int z)
{
	Ptr result(new MoveRobotArmCommand());

	result->Add(
		CommandLib::RetryableCommand::Create(
			MoveRobotArmOnAxisCommand::Create(robotArm, x, RobotArm::Axis::X),
			result.get()));

	result->Add(
		CommandLib::RetryableCommand::Create(
			MoveRobotArmOnAxisCommand::Create(robotArm, y, RobotArm::Axis::Y),
			result.get()));

	result->Add(
		CommandLib::RetryableCommand::Create(
			MoveRobotArmOnAxisCommand::Create(robotArm, z, RobotArm::Axis::Z),
			result.get()));

	return result;
}

MoveRobotArmCommand::MoveRobotArmCommand() : ParallelCommands(true)
{
}

std::string MoveRobotArmCommand::ClassName() const
{
	return "MoveRobotArmCommand";
}

bool MoveRobotArmCommand::OnCommandFailed(size_t, const std::exception& reason, long long* waitMS)
{
	std::cerr << reason.what() << std::endl;

	if (dynamic_cast<const RobotArm::OverheatedException*>(&reason) != nullptr)
	{
		*waitMS = 5000;
		std::cerr << "Will retry moving that axis after waiting 5 seconds..." << std::endl;
		return true;
	}

	return false;
}
