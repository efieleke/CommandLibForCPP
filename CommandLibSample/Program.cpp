#include "Robot.h"
#include "CommandLogger.h"
#include "CommandTracer.h"
#include "PeriodicCommand.h"
#include "SequentialCommands.h"
#include "ParallelCommands.h"
#include "TimeLimitedCommand.h"
#include "RetryableCommand.h"
#include "CommandTimeoutException.h"
#include "ReportPositionCommand.h"
#include "MoveRobotCommand.h"
#include "EmitGreetingCommand.h"
#include <iostream>
#include <algorithm>

// This application demonstrates how to wrap an aysnchronous operation (Robot::Move) with an AsyncCommand-derived class,
// and makes use of ParallelCommands, SequentialCommands, PeriodicCommand, TimeLimitedCommand and RetryableCommand.
class RetryHandler : public CommandLib::RetryableCommand::RetryCallback
{
public:
	virtual bool OnCommandFailed(size_t, const std::exception& reason, long long* waitTimeMS) override final
	{
		*waitTimeMS = 0;

		if (dynamic_cast<const CommandLib::CommandTimeoutException*>(&reason) != nullptr)
		{
			std::cout << "Would you like to give George and Martha more time to find each other (y/n)? ";
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

		if (argc > 2)
		{
			// Output all the command activity to the file specified. This is a simple text file, and can be viewed using CommandLogViewer
			// from the C# CommandLib project.
			logger.reset(new CommandLib::CommandLogger(argv[1]));
			CommandLib::Command::sm_monitors.push_back(logger.get());
		}

		Robot robotOne("George", 100, 126);
		Robot robotTwo("Martha", 97, 80);

		// Create a command that will concurrently move both robots to position 0,0
		CommandLib::ParallelCommands::Ptr moveRobotsCmd = CommandLib::ParallelCommands::Create(true);
		moveRobotsCmd->Add(MoveRobotCommand::Create(&robotOne, 0, 0));
		moveRobotsCmd->Add(MoveRobotCommand::Create(&robotTwo, 0, 0));

		// Create commands that will periodically report robot positions until they both reach their destination (0,0)
		CommandLib::PeriodicCommand::Ptr reportRobotOnePositionCmd = CommandLib::PeriodicCommand::Create(
			ReportPositionCommand::Create(robotOne), // the command to execute
			std::numeric_limits<size_t>::max(), // no fixed upper limit on repetitions
			1000, // execute the command every second
			CommandLib::PeriodicCommand::IntervalType::PauseBefore, // wait a second before executing the command the first time
			true, // the second to wait is inclusive of the time it actually takes to report the position
			moveRobotsCmd->DoneEvent()); // stop when this command is finished (in other words, when both robots reach 0,0)

		CommandLib::PeriodicCommand::Ptr reportRobotTwoPositionCmd = CommandLib::PeriodicCommand::Create(
			ReportPositionCommand::Create(robotTwo), // the command to execute
			std::numeric_limits<size_t>::max(), // no fixed upper limit on repetitions
			1000, // execute the command every second
			CommandLib::PeriodicCommand::IntervalType::PauseBefore, // wait a second before executing the command the first time
			true, // the second to wait is inclusive of the time it actually takes to report the position
			moveRobotsCmd->DoneEvent()); // stop when this command is finished (in other words, when both robots reach 0,0)

		// Create a command that will move the robots and periodically report at the same time
		CommandLib::ParallelCommands::Ptr moveAndReportCmd = CommandLib::ParallelCommands::Create(true);
		moveAndReportCmd->Add(moveRobotsCmd);
		moveAndReportCmd->Add(reportRobotOnePositionCmd);
		moveAndReportCmd->Add(reportRobotTwoPositionCmd);

		// Create a command that will first report the starting positions, then perform the simulataneous moves
		// and position reporting, then report their final positions, and last of all, greet each other.
		CommandLib::SequentialCommands::Ptr moveAndGreetCmd = CommandLib::SequentialCommands::Create();
		moveAndGreetCmd->Add(ReportPositionCommand::Create(robotOne));
		moveAndGreetCmd->Add(ReportPositionCommand::Create(robotTwo));
		moveAndGreetCmd->Add(moveAndReportCmd);
		moveAndGreetCmd->Add(ReportPositionCommand::Create(robotOne));
		moveAndGreetCmd->Add(ReportPositionCommand::Create(robotTwo));
		moveAndGreetCmd->Add(EmitGreetingCommand::Create(robotOne));
		moveAndGreetCmd->Add(EmitGreetingCommand::Create(robotTwo));

		// Wrap the above command in a command that throws a TimeoutException if it takes longer than 20 seconds.
		CommandLib::TimeLimitedCommand::Ptr timeLimitedCmd = CommandLib::TimeLimitedCommand::Create(moveAndGreetCmd, 20000);

		// Allow retries, because we will time out, and maybe the end user actually wants to see if there are any
		// sparks between George and Martha.
		RetryHandler retryHandler;
		CommandLib::RetryableCommand::Ptr retryableCmd = CommandLib::RetryableCommand::Create(timeLimitedCmd, &retryHandler);

		// Execute our top-level command. Every command created by this app is a ultimately owned by this command.
		try
		{
			retryableCmd->SyncExecute();
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
