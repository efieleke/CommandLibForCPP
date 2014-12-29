#include "MoveAndReportPositionCommand.h"
#include "MoveRobotArmCommand.h"
#include "PeriodicCommand.h"
#include "ReportPositionCommand.h"

MoveAndReportPositionCommand::Ptr MoveAndReportPositionCommand::Create(RobotArm* robotArm, int x, int y, int z)
{
	Ptr result(new MoveAndReportPositionCommand(robotArm, x, y, z));

	// Notice that 'this' is passed as the owning command to this child command.
	// If we didn't do that, abort requests would not be honored (and we'd
	// also have a resource leak because we never dispose this object).
	return result;
}

MoveAndReportPositionCommand::MoveAndReportPositionCommand(RobotArm* robotArm, int x, int y, int z) :
	m_moveAndReportCmd(CommandLib::SequentialCommands::Create())
{
	MoveRobotArmCommand::Ptr moveCmd = MoveRobotArmCommand::Create(robotArm, x, y, z);

	// Create a commands that will periodically report robot arm position until it reaches the destination (x,y,z)
	CommandLib::PeriodicCommand::Ptr reportPositionCmd = CommandLib::PeriodicCommand::Create(
		ReportPositionCommand::Create(*robotArm), // the command to execute
		std::numeric_limits<size_t>::max(), // no fixed upper limit on repetitions
		500, // execute the command twice a second
		CommandLib::PeriodicCommand::IntervalType::PauseBefore, // wait a second before executing the command the first time
		true, // the second to wait is inclusive of the time it actually takes to report the position
		moveCmd->DoneEvent()); // stop when this event is signaled (in other words, when the arm reaches 0,0)

	// Create the command that will move the robot arm and periodically report at the same time
	CommandLib::ParallelCommands::Ptr moveAndReportCmd = CommandLib::ParallelCommands::Create(true);
	moveAndReportCmd->Add(moveCmd);
	moveAndReportCmd->Add(reportPositionCmd);

	// Compose a command that will first report the starting position, then perform the simultaneous move
	// and position reporting, then report the final position.
	m_moveAndReportCmd->Add(ReportPositionCommand::Create(*robotArm));
	m_moveAndReportCmd->Add(moveAndReportCmd);
	m_moveAndReportCmd->Add(ReportPositionCommand::Create(*robotArm));

	// Notice that we take ownership of this command.
	// If we didn't do that, abort requests would not be honored.
	TakeOwnership(m_moveAndReportCmd);
}

std::string MoveAndReportPositionCommand::ClassName() const
{
	return "MoveAndReportPositionCommand";
}

void MoveAndReportPositionCommand::SyncExeImpl()
{
	m_moveAndReportCmd->SyncExecute();
}
