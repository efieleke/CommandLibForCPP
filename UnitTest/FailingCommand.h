#include "SyncCommand.h"

namespace CommandLibTests
{
    class FailingCommand : public CommandLib::SyncCommand
    {
	public:
        class FailException : public std::exception
        {
		public:
			FailException(const char* what) : std::exception(what) {}
		};

		typedef std::shared_ptr<FailingCommand> Ptr;
		static FailingCommand::Ptr Create() { return Ptr(new FailingCommand()); }

		virtual std::string ClassName() const override { return "FailingCommand";  }
	private:
		FailingCommand()
		{
		}

        virtual void SyncExeImpl() override final
        {
            throw FailException("boo hoo");
        }
	};
}
