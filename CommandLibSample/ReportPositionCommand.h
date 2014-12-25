#pragma once
#include "Robot.h"
#include "SyncCommand.h"

class ReportPositionCommand : public CommandLib::SyncCommand
{
public:
	typedef std::shared_ptr<ReportPositionCommand> Ptr;
	static Ptr Create(const Robot& robot);
	virtual std::string ClassName() const override;
private:
	explicit ReportPositionCommand(const Robot& robot);
	virtual void SyncExeImpl() override final;

	const Robot& m_robot;
};
