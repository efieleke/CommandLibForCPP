#pragma once

#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include "Event.h"

class RobotArm
{
public:
	class OverheatedException : public std::runtime_error
	{
	public:
		explicit OverheatedException(const char* message);
	};

	class OperationCompleteHandler
	{
	public:
		virtual ~OperationCompleteHandler();
		virtual void Completed(bool aborted, const std::exception* exc) = 0;
	};

	class Operation
	{
	public:
		void Abort();
	private:
		friend class RobotArm;
		Operation();

		CommandLib::Event m_startedEvent;
		CommandLib::Event m_abortEvent;
	};

	enum Axis
	{
		X,
		Y,
		Z
	};

	RobotArm();
	void GetPosition(int* x, int* y, int* z) const;
	std::shared_ptr<Operation> Move(Axis axis, int destination, OperationCompleteHandler* handler);
	void OpenClamp();

	// Returns true if it successfully grabs something
	bool CloseClamp();
private:
	RobotArm(const RobotArm&) = delete;
	RobotArm& operator=(const RobotArm&) = delete;
	std::shared_ptr<Operation> Move(Axis axis, int destination, OperationCompleteHandler* handler, int* value);
		
	int m_xPos;
    int m_yPos;
	int m_zPos;
	bool m_clampOpen;
	mutable std::mutex m_mutex;
};
