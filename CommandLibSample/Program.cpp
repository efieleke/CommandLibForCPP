#include "RobotArm.h"
#include "CommandLogger.h"
#include "CommandTracer.h"
#include "PeriodicCommand.h"
#include "SequentialCommands.h"
#include "ParallelCommands.h"
#include "TimeLimitedCommand.h"
#include "RetryableCommand.h"
#include "CommandTimeoutException.h"
#include "ReportPositionCommand.h"
#include "MoveRobotArmCommand.h"
#include <iostream>
#include <algorithm>

// This application demonstrates how to wrap an aysnchronous operation (RobotArm::Move) with an AsyncCommand-derived class,
// and makes use of ParallelCommands, SequentialCommands, PeriodicCommand, TimeLimitedCommand and RetryableCommand.
class RetryHandler : public CommandLib::RetryableCommand::RetryCallback
{
public:
	virtual bool OnCommandFailed(size_t, const std::exception& reason, long long* waitTimeMS) override final
	{
		*waitTimeMS = 0;

		if (dynamic_cast<const CommandLib::CommandTimeoutException*>(&reason) != nullptr)
		{
			std::cout << "Would you like to give more time to move the robot arm to the origin (y/n)? ";
			char letter;
			std::cin >> letter;
			return letter == 'y';
		}

		return false;
	}
};

int main(int argc, char* argv[])
{
	try
	{
		std::unique_ptr<CommandLib::CommandLogger> logger;

		if (argc > 1)
		{
			// Output all the command activity to the file specified. This is a simple text file, and can be viewed using CommandLogViewer
			// from the C# CommandLib project.
			logger.reset(new CommandLib::CommandLogger(argv[1]));
			CommandLib::Command::sm_monitors.push_back(logger.get());
		}

		RobotArm robotArm(100, 126);

		// Create a command to move the robot arm to position 0,0
		MoveRobotArmCommand::Ptr moveToOriginCmd = MoveRobotArmCommand::Create(&robotArm, 0, 0);

		// Create a command to report the robot arm's position every second
		CommandLib::PeriodicCommand::Ptr periodicReportPositionCmd = CommandLib::PeriodicCommand::Create(
			ReportPositionCommand::Create(robotArm), // the command to execute
			std::numeric_limits<size_t>::max(), // no fixed upper limit on repetitions
			1000, // execute the command every second
			CommandLib::PeriodicCommand::IntervalType::PauseBefore, // wait a second before executing the command the first time
			true, // the second to wait is inclusive of the time it actually takes to report the position
			moveToOriginCmd->DoneEvent()); // stop when this command is finished (in other words, when both robots reach 0,0)

		// Create a command that will concurrently move the robot arm and periodically report its position
		CommandLib::ParallelCommands::Ptr moveAndReportCmd = CommandLib::ParallelCommands::Create(true);
		moveAndReportCmd->Add(moveToOriginCmd);
		moveAndReportCmd->Add(periodicReportPositionCmd);

		// Create a command that will first report the starting position, then perform the simulataneous move
		// and position reporting, then report the final position.
		CommandLib::SequentialCommands::Ptr moveAndGreetCmd = CommandLib::SequentialCommands::Create();
		moveAndGreetCmd->Add(ReportPositionCommand::Create(robotArm));
		moveAndGreetCmd->Add(moveAndReportCmd);
		moveAndGreetCmd->Add(ReportPositionCommand::Create(robotArm));

		// Wrap the above command in a command that throws a TimeoutException if it takes longer than 20 seconds.
		CommandLib::TimeLimitedCommand::Ptr timeLimitedCmd = CommandLib::TimeLimitedCommand::Create(moveAndGreetCmd, 20000);

		// Allow retries, because we will time out
		RetryHandler retryHandler;
		CommandLib::RetryableCommand::Ptr retryableCmd = CommandLib::RetryableCommand::Create(timeLimitedCmd, &retryHandler);

		// Execute our top-level command. Every command created by this app is a ultimately owned by this command.
		try
		{
			retryableCmd->SyncExecute();
			std::cout << "Robot arm successfully moved to the origin" << std::endl;
		}
		catch (std::exception& err)
		{
			std::cerr << err.what() << std::endl;
		}

		return 0;
	}
	catch (std::exception& exc)
	{
		std::cerr << exc.what() << std::endl;
		throw;
	}
}
