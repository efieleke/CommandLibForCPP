#include "GetToyCommand.h"
#include "MoveAndReportPositionCommand.h"
#include <iostream>

GetToyCommand::Ptr GetToyCommand::Create(RobotArm* robotArm)
{
	Ptr result(new GetToyCommand(robotArm));

	// Notice that 'this' is passed as the owning command to this child command.
	// If we didn't do that, abort requests would not be honored (and we'd
	// also have a resource leak because we never dispose this object).
	return result;
}

GetToyCommand::GetToyCommand(RobotArm* robotArm) :
	m_robotArm(robotArm),
	m_moveToToyCmd(MoveAndReportPositionCommand::Create(robotArm, 45, 55, -60)),
	m_moveToChuteCmd(MoveAndReportPositionCommand::Create(robotArm, 100, 100, 0)),
	m_moveToHomeCmd(MoveAndReportPositionCommand::Create(robotArm, 0, 0, 0))
{
	// Notice that we take ownership of these commands.
	// If we didn't do so, abort requests would not be honored.
	TakeOwnership(m_moveToToyCmd);
	TakeOwnership(m_moveToChuteCmd);
	TakeOwnership(m_moveToHomeCmd);
}

std::string GetToyCommand::ClassName() const
{
	return "GetToyCommand";
}

void GetToyCommand::SyncExeImpl()
{
	std::cout << "Attempting to grab a toy with the robot arm..." << std::endl;
	std::cout << "Opening clamp" << std::endl;
	m_robotArm->OpenClamp();

	// Run the command that will move the robot arm to 45,55,60 (where we hope it will grab a toy)
	// and periodically report at the same time
	m_moveToToyCmd->SyncExecute();

	std::cout << "Closing clamp on toy" << std::endl;

	if (m_robotArm->CloseClamp())
	{
		std::cout << "Got a toy! Will now move to the chute and drop it." << std::endl;
		m_moveToChuteCmd->SyncExecute();

		// Drop the toy
		std::cout << "Opening clamp" << std::endl;
		m_robotArm->OpenClamp();
		std::cout << "Dropped the toy down the chute!" << std::endl;
		std::cout << "Closing clamp" << std::endl;
		m_robotArm->CloseClamp();
	}
	else
	{
		std::cout << "Too bad. Failed to grasp a toy." << std::endl;
	}

	std::cout << "Now returning to the home position" << std::endl;
	m_moveToHomeCmd->SyncExecute();
}
