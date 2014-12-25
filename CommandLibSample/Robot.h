#pragma once

#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include "Event.h"

class Robot
{
public:
	Robot(const std::string& name, int xPos, int yPos);

	class OperationCompleteHandler
	{
	public:
		virtual ~OperationCompleteHandler();
		virtual void Completed(bool aborted) = 0;
	};

	class Operation
	{
	public:
		void Abort();
	private:
		friend class Robot;
		Operation();

		CommandLib::Event m_startedEvent;
		CommandLib::Event m_abortEvent;
	};

	void GetPosition(int* x, int* y) const;
	std::string Greeting() const;

	std::shared_ptr<Operation> MoveX(int destination, OperationCompleteHandler* handler);
	std::shared_ptr<Operation> MoveY(int destination, OperationCompleteHandler* handler);

	const std::string m_name;
private:
	Robot(const Robot&) = delete;
	Robot& operator=(const Robot&) = delete;
	std::shared_ptr<Operation> Move(int destination, OperationCompleteHandler* handler, int* value);
		
	int m_xPos;
    int m_yPos;
	mutable std::mutex m_mutex;
};
