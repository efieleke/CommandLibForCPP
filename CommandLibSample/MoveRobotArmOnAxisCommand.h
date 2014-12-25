#pragma once
#include "RobotArm.h"
#include "AsyncCommand.h"

class MoveRobotArmOnAxisCommand : public CommandLib::AsyncCommand, public RobotArm::OperationCompleteHandler
{
public:
	typedef std::shared_ptr<MoveRobotArmOnAxisCommand> Ptr;
	static Ptr Create(RobotArm* robot, int destination, bool xAxis);

	virtual void AsyncExecuteImpl(CommandLib::CommandListener* listener) override final;
	virtual void AbortImpl() override final;
	virtual std::string ClassName() const override;

	void Completed(bool aborted) override final;
protected:
	MoveRobotArmOnAxisCommand(RobotArm* robotArm, int destination, bool xAxis);
private:
	RobotArm* const m_robotArm;
	const int m_destination;
	const bool m_xAxis;
	std::shared_ptr<RobotArm::Operation> m_operation;
    CommandLib::CommandListener* m_listener = nullptr;
	std::mutex m_mutex;
};
