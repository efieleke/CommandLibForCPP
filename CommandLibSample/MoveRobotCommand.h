#pragma once
#include "Robot.h"
#include "ParallelCommands.h"

class MoveRobotCommand : public CommandLib::ParallelCommands
{
public:
	typedef std::shared_ptr<MoveRobotCommand> Ptr;
	static Ptr Create(Robot* robot, int x, int y);
	virtual std::string ClassName() const override;
protected:
	MoveRobotCommand();
};
