CommandLib
=========

CommandLib is a C++ library that simplifies coordination of synchronous and asynchronous activities. Versions for C# and Java exist at https://github.com/efieleke/CommandLib.git and https://github.com/efieleke/CommandLibForJava.git. The library is built upon class Command, which represents an action. A Command may be run synchronously or asynchronously, and may be aborted.

ParallelCommands, itself a Command, executes a collection of commands concurrently, and SequentialCommands executes its commands in sequence. Using these classes, it's possible to create a deep nesting of coordinated actions. For example, SequentialCommands can hold instances of ParallelCommands, SequentialCommands, and any other Command-derived object.

PeriodicCommand repeats its action at a given interval, ScheduledCommand runs once at a specific time, and RecurringCommand runs at times that are provided via a callback.

RetryableCommand provides the option to keep retrying a failed command until the caller decides enough is enough, and TimeLimitedCommand fails with a timeout exception if a given duration elapses before the command finishes execution.

All of the above Command classes are simply containers for other Command objects that presumably do something of interest. It is expected that users of this library will create their own Command-derived classes.

Help File Documentation
----
Documentation is in HTML format, generated using doxygen (www.doxygen.org). Open index.html in the Documentation/hmtl folder for in-depth descriptions of each class and its methods.

Diagnostics
----
The Command class provides a CommandMonitor collection. If a CommandTracer object is added, diagnostic output is written to stdout. If a CommandLogger object is added, diagnostic output is written to a text file. It is intended that this file be compatible with the CommandLogViewer utility included in the C# version of this library, but that is yet to be tested.

Build
----
Included is a solution file that contains three projects: CommandLib itself, a unit test project and a project demonstrating example usage. The solution and project files were created using Microsoft Visual Studio 2013. I haven't tried building them using any other compilers. There are a few Microsoft-specific bits of code in the core library (which I will eventually address), and the unit tests rely upon a Microsoft-provided framework.

Example Usage
----
A sample project is included that moves a simulated robot arm that tries to pick up a toy and drop it down the chute. It demonstrates how to author a naturally asynchronous Command, and makes use of ParallelCommands, SequentialCommands, PeriodicCommand and RetryableCommand.

Author
----
Eric Fieleke
