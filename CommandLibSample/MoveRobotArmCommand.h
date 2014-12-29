#pragma once
#include "RobotArm.h"
#include "ParallelCommands.h"
#include "RetryableCommand.h"

class MoveRobotArmCommand : public CommandLib::ParallelCommands, private CommandLib::RetryableCommand::RetryCallback
{
public:
	typedef std::shared_ptr<MoveRobotArmCommand> Ptr;
	static Ptr Create(RobotArm* robotArm, int x, int y, int z);
	virtual std::string ClassName() const override;
protected:
	MoveRobotArmCommand();
private:
	virtual bool OnCommandFailed(size_t failNumber, const std::exception& reason, long long* waitMS) override final;
};
