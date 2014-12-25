#include "AddCommand.h"

using namespace CommandLibTests;

AddCommand::Ptr AddCommand::Create(std::atomic_int* toModify, int toAdd)
{
	return Ptr(new AddCommand(toModify, toAdd));
}

AddCommand::AddCommand(std::atomic_int* toModify, int toAdd) : m_toModify(toModify), m_toAdd(toAdd)
{
}

std::string AddCommand::ClassName() const
{
	return "AddCommand";
}

void AddCommand::SyncExeImpl()
{
	m_toModify->operator+= (m_toAdd);
}
