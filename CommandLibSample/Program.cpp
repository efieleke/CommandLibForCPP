#include "RobotArm.h"
#include "CommandLogger.h"
#include "CommandTracer.h"
#include "GetToyCommand.h"
#include <iostream>
#include <algorithm>

// This application demonstrates how to wrap an aysnchronous operation (RobotArm::Move) with an AsyncCommand-derived class,
// and makes use of ParallelCommands, SequentialCommands, PeriodicCommand, TimeLimitedCommand and RetryableCommand.
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

		RobotArm robotArm;
		GetToyCommand::Ptr getToyCmd = GetToyCommand::Create(&robotArm);

		// Execute our top-level command. Every command created by this app is a ultimately owned by this command.
		try
		{
			getToyCmd->SyncExecute();
			std::cout << "Operation complete." << std::endl;
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
