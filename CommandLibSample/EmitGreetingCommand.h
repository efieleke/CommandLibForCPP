#pragma once
#include "SyncCommand.h"
#include "Robot.h"

class EmitGreetingCommand : public CommandLib::SyncCommand
{
public:
	typedef std::shared_ptr<EmitGreetingCommand> Ptr;
	static Ptr Create(const Robot& robot);
	virtual std::string ClassName() const override;
private:
	explicit EmitGreetingCommand(const Robot& robot);
	virtual void SyncExeImpl() override final;

	const Robot& m_robot;
};
