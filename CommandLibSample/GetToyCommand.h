#pragma once
#include "RobotArm.h"
#include "SyncCommand.h"
#include "MoveAndReportPositionCommand.h"

class GetToyCommand : public CommandLib::SyncCommand
{
public:
	typedef std::shared_ptr<GetToyCommand> Ptr;
	static Ptr Create(RobotArm* robotArm);
	virtual std::string ClassName() const override;
protected:
	explicit GetToyCommand(RobotArm* robotArm);
private:
	virtual void SyncExeImpl() override final;
	
	RobotArm* const m_robotArm;
	MoveAndReportPositionCommand::Ptr m_moveToToyCmd;
	MoveAndReportPositionCommand::Ptr m_moveToChuteCmd;
	MoveAndReportPositionCommand::Ptr m_moveToHomeCmd;
};
