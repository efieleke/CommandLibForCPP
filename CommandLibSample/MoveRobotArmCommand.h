#pragma once
#include "RobotArm.h"
#include "ParallelCommands.h"

class MoveRobotArmCommand : public CommandLib::ParallelCommands
{
public:
	typedef std::shared_ptr<MoveRobotArmCommand> Ptr;
	static Ptr Create(RobotArm* robotArm, int x, int y);
	virtual std::string ClassName() const override;
protected:
	MoveRobotArmCommand();
};
