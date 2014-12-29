#pragma once
#include "RobotArm.h"
#include "SyncCommand.h"
#include "SequentialCommands.h"

class MoveAndReportPositionCommand : public CommandLib::SyncCommand
{
public:
	typedef std::shared_ptr<MoveAndReportPositionCommand> Ptr;
	static Ptr Create(RobotArm* robotArm, int x, int y, int z);
	virtual std::string ClassName() const override;
protected:
	MoveAndReportPositionCommand(RobotArm* robotArm, int x, int y, int z);
private:
	virtual void SyncExeImpl() override final;
	
	CommandLib::SequentialCommands::Ptr m_moveAndReportCmd;
};
