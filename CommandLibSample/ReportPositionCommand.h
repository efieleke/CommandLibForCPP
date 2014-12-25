#pragma once
#include "RobotArm.h"
#include "SyncCommand.h"

class ReportPositionCommand : public CommandLib::SyncCommand
{
public:
	typedef std::shared_ptr<ReportPositionCommand> Ptr;
	static Ptr Create(const RobotArm& robot);
	virtual std::string ClassName() const override;
private:
	explicit ReportPositionCommand(const RobotArm& robot);
	virtual void SyncExeImpl() override final;

	const RobotArm& m_robot;
};
