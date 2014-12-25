#include "EmitGreetingCommand.h"
#include <iostream>

std::string EmitGreetingCommand::ClassName() const
{
	return "EmitGreetingCommand";
}

EmitGreetingCommand::Ptr EmitGreetingCommand::Create(const Robot& robot)
{
	return Ptr(new EmitGreetingCommand(robot));
}

EmitGreetingCommand::EmitGreetingCommand(const Robot& robot) : m_robot(robot)
{
}

void EmitGreetingCommand::SyncExeImpl()
{
	std::cout << m_robot.Greeting() << std::endl;
}
