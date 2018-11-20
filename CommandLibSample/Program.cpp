#include "CommandLogger.h"
#include "CommandTracer.h"
#include "PauseCommand.h"
#include "ParallelCommands.h"
#include "SequentialCommands.h"
#include "CommandAbortedException.h"
#include <iostream>
#include <algorithm>
#include <chrono>

#include <Windows.h>
#include <conio.h>

// This application prepares a spaghetti and salad dinner.

class PretendCmd : public CommandLib::SyncCommand
{
protected:
	PretendCmd(long long milliseconds, const std::string& desc) :
		m_pauseCmd(CommandLib::PauseCommand::Create(milliseconds)),
		m_desc(desc)
	{
		// Notice that we take ownership of the pause command.
		// If we didn't do so, abort requests would not be honored.
		TakeOwnership(m_pauseCmd);
	}
private:
	virtual void SyncExeImpl() override final
	{
		OutputText("Started " + m_desc);
		m_pauseCmd->SyncExecute();
		OutputText("Finished " + m_desc);
	}

	static void OutputText(const std::string& text)
	{
		std::unique_lock<std::mutex> lock(sm_mutex);
		std::cout << text << std::endl;
	}

	static std::mutex sm_mutex;
	CommandLib::PauseCommand::Ptr m_pauseCmd;
	const std::string m_desc;
};

std::mutex PretendCmd::sm_mutex;

class BoilWaterCmd : public PretendCmd
{
public:
	static Ptr Create() { return Ptr(new BoilWaterCmd()); }
	virtual std::string ClassName() const final { return "BoilWaterCmd"; }
private:
	BoilWaterCmd() : PretendCmd(7000, "boiling water") { }
};

class BoilNoodlesCmd : public PretendCmd
{
public:
	static Ptr Create() { return Ptr(new BoilNoodlesCmd()); }
	virtual std::string ClassName() const final { return "BoilNoodlesCmd"; }
private:
	BoilNoodlesCmd() : PretendCmd(10000, "boiling noodles") { }
};

class SauteGarlicCmd : public PretendCmd
{
public:
	static Ptr Create() { return Ptr(new SauteGarlicCmd()); }
	virtual std::string ClassName() const final { return "SauteGarlicCmd"; }
private:
	SauteGarlicCmd() : PretendCmd(3000, "sauteing garlic") { }
};

class HeatSauceCmd : public PretendCmd
{
public:
	static Ptr Create() { return Ptr(new HeatSauceCmd()); }
	virtual std::string ClassName() const final { return "HeatSauceCmd"; }
private:
	HeatSauceCmd() : PretendCmd(7000, "heating sauce") { }
};

class AddGarlicToHeatingSauceCmd : public PretendCmd
{
public:
	static Ptr Create() { return Ptr(new AddGarlicToHeatingSauceCmd()); }
	virtual std::string ClassName() const final { return "AddGarlicToHeatingSauceCmd"; }
private:
	AddGarlicToHeatingSauceCmd() : PretendCmd(1000, "adding garlic to sauce") { }
};

class DrainNoodlesCmd : public PretendCmd
{
public:
	static Ptr Create() { return Ptr(new DrainNoodlesCmd()); }
	virtual std::string ClassName() const final { return "DrainNoodlesCmd"; }
private:
	DrainNoodlesCmd() : PretendCmd(1000, "draining noodles") { }
};

class AddSauceToNoodlesCmd : public PretendCmd
{
public:
	static Ptr Create() { return Ptr(new AddSauceToNoodlesCmd()); }
	virtual std::string ClassName() const final { return "AddSauceToNoodlesCmd"; }
private:
	AddSauceToNoodlesCmd() : PretendCmd(1000, "adding sauce to noodles") { }
};

class ChopVeggiesCmd : public PretendCmd
{
public:
	static Ptr Create() { return Ptr(new ChopVeggiesCmd()); }
	virtual std::string ClassName() const final { return "ChopVeggiesCmd"; }
private:
	ChopVeggiesCmd() : PretendCmd(5000, "chopping veggies") { }
};

class RinseLettuceCmd : public PretendCmd
{
public:
	static Ptr Create() { return Ptr(new RinseLettuceCmd()); }
	virtual std::string ClassName() const final { return "RinseLettuceCmd"; }
private:
	RinseLettuceCmd() : PretendCmd(2000, "rinsing lettuce") { }
};

class TossSaladCmd : public PretendCmd
{
public:
	static Ptr Create() { return Ptr(new TossSaladCmd()); }
	virtual std::string ClassName() const final { return "TossSaladCmd"; }
private:
	TossSaladCmd() : PretendCmd(2000, "tossing salad") { }
};

// This class is a ParallelCommands-derived type that adds Command objects to its
// collection. When the PrepareDinnerCommand instance is executed, its contained
// commands will execute concurrently (those two commands are preparing the spaghetti
// and preparing the salad; see the last few statements in the Create method).
class PrepareDinnerCmd : public CommandLib::ParallelCommands
{
public:
	typedef std::shared_ptr<PrepareDinnerCmd> Ptr;

	static Ptr Create()
	{
		PrepareDinnerCmd::Ptr result = Ptr(new PrepareDinnerCmd());

		// Preparing the noodles consists of three operations that must be performed in sequence.
		CommandLib::SequentialCommands::Ptr prepareNoodlesCmd = CommandLib::SequentialCommands::Create();
		prepareNoodlesCmd->Add(BoilWaterCmd::Create());
		prepareNoodlesCmd->Add(BoilNoodlesCmd::Create());
		prepareNoodlesCmd->Add(DrainNoodlesCmd::Create());

		// The garlic must be sauteed before being added to the sauce, thus these
		// operations are placed into a SequentialCommands instance.
		CommandLib::SequentialCommands::Ptr prepareGarlicCmd = CommandLib::SequentialCommands::Create();
		prepareGarlicCmd->Add(SauteGarlicCmd::Create());
		prepareGarlicCmd->Add(AddGarlicToHeatingSauceCmd::Create());

		// The following operations can be done in tandem (none are dependent upon any
		// of the others being complete before they are initiated), so they are placed into a
		// ParallelCommands instance.
		ParallelCommands::Ptr prepareNoodlesAndSauceCmd = ParallelCommands::Create(true);
		prepareNoodlesAndSauceCmd->Add(HeatSauceCmd::Create());
		prepareNoodlesAndSauceCmd->Add(prepareGarlicCmd);
		prepareNoodlesAndSauceCmd->Add(prepareNoodlesCmd);

		// The noodles and sauce preparation must be complete before the sauce is
		// added to the noodles.
		CommandLib::SequentialCommands::Ptr prepareSpaghettiCmd = CommandLib::SequentialCommands::Create();
		prepareSpaghettiCmd->Add(prepareNoodlesAndSauceCmd);
		prepareSpaghettiCmd->Add(AddSauceToNoodlesCmd::Create());

		// The lettuce doesn't have to be rinsed before the veggies are chopped. (Perhaps
		// there are two chefs in the kitchen, or the one chef likes to alternate rinsing
		// and chopping until both tasks are done.)
		ParallelCommands::Ptr rinseLettuceAndChopVeggiesCmd = ParallelCommands::Create(true);
		rinseLettuceAndChopVeggiesCmd->Add(RinseLettuceCmd::Create());
		rinseLettuceAndChopVeggiesCmd->Add(ChopVeggiesCmd::Create());

		// The salad ingredients need to be ready before it is tossed, so these operations
		// are sequential.
		CommandLib::SequentialCommands::Ptr prepareSaladCmd = CommandLib::SequentialCommands::Create();
		prepareSaladCmd->Add(rinseLettuceAndChopVeggiesCmd);
		prepareSaladCmd->Add(TossSaladCmd::Create());

		// We now call the Add() method to prepare the spaghetti and prepare the salad. Neither task has
		// operations that the other depends upon, so they can be done in parallel.
		result->Add(prepareSpaghettiCmd);
		result->Add(prepareSaladCmd);

		return result;
	}
	virtual std::string ClassName() const final { return "PrepareDinnerCmd"; }
private:
	PrepareDinnerCmd() : ParallelCommands(true)	{}
};

static CommandLib::Command::Ptr MakeDinnerCmd = PrepareDinnerCmd::Create();

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_C_EVENT)
	{
		MakeDinnerCmd->AbortAndWait();
		return TRUE;
	}

	return FALSE;
}

int main(int argc, char* argv[])
{
	// Trap Ctrl-C in to provide an example of aborting a command (see implementation
	// of ConsoleCtrlCheck above)
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

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

		try
		{
			MakeDinnerCmd->SyncExecute();
			std::cout << "Dinner is ready!" << std::endl;
		}
		catch (const CommandLib::CommandAbortedException&)
		{
			std::cout << "Dinner preparation aborted. Let's order pizza instead." << std::endl;
			std::cout << "Press any key to continue..." << std::endl;
			_getch();
		}
	}
	catch (std::exception& exc)
	{
		std::cerr << exc.what() << std::endl;
		throw;
	}

	MakeDinnerCmd.reset();
	return 0;
}
