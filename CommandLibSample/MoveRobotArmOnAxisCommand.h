#pragma once
#include "RobotArm.h"
#include "AsyncCommand.h"

class MoveRobotArmOnAxisCommand : public CommandLib::AsyncCommand, private RobotArm::OperationCompleteHandler
{
public:
	typedef std::shared_ptr<MoveRobotArmOnAxisCommand> Ptr;
	static Ptr Create(RobotArm* robot, int destination, RobotArm::Axis axis);
	virtual std::string ClassName() const override;
protected:
	MoveRobotArmOnAxisCommand(RobotArm* robotArm, int destination, RobotArm::Axis axis);
private:
	virtual void AbortImpl() override final;
	virtual void AsyncExecuteImpl(CommandLib::CommandListener* listener) override final;
	void Completed(bool aborted, const std::exception* error) override final;

	RobotArm* const m_robotArm;
	const int m_destination;
	const RobotArm::Axis m_axis;
	std::shared_ptr<RobotArm::Operation> m_operation;
    CommandLib::CommandListener* m_listener = nullptr;
	std::mutex m_mutex;
};
