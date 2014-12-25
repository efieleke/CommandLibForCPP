#pragma once
#include "Robot.h"
#include "AsyncCommand.h"

class MoveRobotOnAxisCommand : public CommandLib::AsyncCommand, public Robot::OperationCompleteHandler
{
public:
	typedef std::shared_ptr<MoveRobotOnAxisCommand> Ptr;
	static Ptr Create(Robot* robot, int destination, bool xAxis);

	virtual void AsyncExecuteImpl(CommandLib::CommandListener* listener) override final;
	virtual void AbortImpl() override final;
	virtual std::string ClassName() const override;

	void Completed(bool aborted) override final;
protected:
	MoveRobotOnAxisCommand(Robot* robot, int destination, bool xAxis);
private:
	Robot* const m_robot;
	const int m_destination;
	const bool m_xAxis;
	std::shared_ptr<Robot::Operation> m_operation;
    CommandLib::CommandListener* m_listener = nullptr;
	std::mutex m_mutex;
};
