#include "Command.h"
#include "CommandAbortedException.h"
#include "AbortLinkedCommand.h"

using namespace CommandLib;

Command::ListenerProxy::ListenerProxy(Command* command, CommandListener* listener) : m_command(command), m_listener(listener), m_asyncExeThreadId(std::this_thread::get_id())
{
}

void Command::ListenerProxy::CommandSucceeded()
{
	if (std::this_thread::get_id() == m_asyncExeThreadId)
	{
		throw std::logic_error("CommandListener::CommandSucceeded() was called on the same thread as Command::AsyncExecute()");
	}

	m_command->DecrementExecuting(m_listener, nullptr, nullptr);
	delete this;
}

void Command::ListenerProxy::CommandAborted()
{
	if (std::this_thread::get_id() == m_asyncExeThreadId)
	{
		throw std::logic_error("CommandListener::CommandAborted() was called on the same thread as Command::AsyncExecute()");
	}

	CommandAbortedException exc;
	m_command->DecrementExecuting(m_listener, &exc, std::make_exception_ptr(exc));
	delete this;
}

void Command::ListenerProxy::CommandFailed(const std::exception& exc, std::exception_ptr excPtr)
{
	if (std::this_thread::get_id() == m_asyncExeThreadId)
	{
		throw std::logic_error("CommandListener::CommandFailed() was called on the same thread as Command::AsyncExecute()");
	}

	m_command->DecrementExecuting(m_listener, &exc, excPtr);
	delete this;
}

std::list<CommandMonitor*> Command::sm_monitors;
std::atomic_uint Command::sm_nextId;

long Command::Id() const
{
	return m_id;
}

const Command* Command::Parent() const
{
    return Parent(this);
}

int Command::Depth() const
{
    int result = 0;

    for (const Command* parent = Parent(this); parent != nullptr; parent = Parent(parent))
    {
        ++result;
    }

    return result;
}

std::string Command::Description() const
{
    std::string result = ClassName();

    for (const Command* parent = Parent(this); parent != nullptr; parent = Parent(parent))
    {
        result = parent->ClassName() + "=>" + result;
    }

	result += "(" + std::to_string(m_id) + ")";
    const std::string extendedDescription = ExtendedDescription();

    if (!extendedDescription.empty())
    {
        result += " " + extendedDescription;
    }

    return result;
}

std::string Command::ExtendedDescription() const
{
    return std::string();
}

void Command::InformCommandStarting() const
{
	for (CommandMonitor* monitor : sm_monitors)
	{
		monitor->CommandStarting(*this);
	}
}

void Command::InformCommandFinished(const std::exception* exc) const
{
	for (CommandMonitor* monitor : sm_monitors)
	{
		monitor->CommandFinished(*this, exc);
	}
}

void Command::AsyncExecute(CommandListener* listener)
{
    if (!listener)
    {
        throw std::invalid_argument("listener must not be null");
    }

    PreExecute();

    try
    {
		InformCommandStarting();
		std::unique_ptr<CommandListener> proxy(new ListenerProxy(this, listener));
        AsyncExecuteImpl(proxy.get());
		proxy.release(); // The proxy commits suicide in its event callbacks
    }
    catch (std::exception& exc)
    {
        DecrementExecuting(nullptr, &exc, std::current_exception());
        throw;
    }
	catch (...)
	{
		std::exception exc("Unexpected exception type occurred in Command::AsyncExecute");
		DecrementExecuting(nullptr, &exc, std::current_exception());
		throw;
	}
}

void Command::Abort()
{
    // Calling Abort on a child command makes no sense, because children follow the parent in this regard.
    if (m_owner != nullptr)
    {
        throw std::logic_error("Abort can only be called on top-level commands");
    }

	m_abortEvent->Set();
    AbortImplAllDescendents(this);
    AbortImpl();
}

void Command::Wait() const
{
	m_doneEvent->Wait();
}

bool Command::Wait(long long milliseconds) const
{
	return m_doneEvent->Wait(milliseconds);
}

void Command::AbortAndWait()
{
	Abort();
	Wait();
}

bool Command::AbortAndWait(long long milliseconds)
{
    Abort();
    return Wait(milliseconds);
}

Waitable::Ptr Command::DoneEvent() const
{
	return m_doneEvent;
}

Waitable::Ptr Command::AbortEvent() const
{
	return m_abortEvent;
}

Command::Command()
{
	m_abortEvent.reset(new Event(false));
	m_executing = 0;
}

Command::~Command()
{
	// Even though this command may have informed us that it is done by now, it still may not have signaled its done
	// event. That signal must be complete for this command to be considered truly done and destructable.
	Wait();
}

void Command::TakeOwnership(Ptr orphan)
{
    if (orphan->MustBeTopLevel())
    {
        throw std::logic_error(orphan->ClassName() + " objects may only be top level");
    }

	std::unique_lock<std::mutex> lock(m_mutex);

    if (orphan->m_owner != nullptr)
    {
        throw std::logic_error("Attempt to assume ownership of a command that already has an owner.");
    }

    // Only top level owners have the abort event. All of its children share the
    // same single event. Besides saving a bit on resources, it really helps make
    // synchronization easier around aborts.
    SetAbortEvent(orphan);

    // Maintaining children and owner simplifies management of abort and wait operations.
    orphan->m_owner = this;
    m_children.insert(orphan);
}

void Command::RelinquishOwnership(Ptr command)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if (m_children.erase(command) == 0)
    {
        throw std::logic_error("Attempt to relinquish ownership of a command that is not directly owned by this object.");
    }

    command->m_owner = nullptr;
	command->m_abortEvent.reset(new Event(false));

	for (Ptr child : command->m_children)
    {
		SetAbortEvent(child);
    }
}

void Command::CheckAbortFlag() const
{
    if (m_abortEvent->IsSignaled())
    {
        throw CommandAbortedException();
    }
}

bool Command::MustBeTopLevel() const
{
    return false;
}

void Command::AbortImpl()
{
}

const Command* Command::Parent(const Command* command)
{
    if (command->m_owner != nullptr)
    {
        return command->m_owner;
    }

	const AbortLinkedCommand* abortLinkedCommand = dynamic_cast<const AbortLinkedCommand*>(command);
	
	if (abortLinkedCommand != nullptr)
    {
        return abortLinkedCommand->CommandToWatch().get();
    }

    return nullptr;
}

void Command::AbortImplAllDescendents(Command* command)
{
	std::unique_lock<std::mutex> lock(command->m_mutex);

	for (Ptr child : command->m_children)
	{
		AbortImplAllDescendents(child.get());
		child->AbortImpl();
	}
}

void Command::SetAbortEvent(Ptr target) const
{
	target->m_abortEvent = m_abortEvent;

	for (Ptr child : target->m_children)
    {
		target->SetAbortEvent(child);
    }
}

void Command::SyncExecute()
{
    PreExecute();

    try
    {
		InformCommandStarting();
        SyncExecuteImpl();
        DecrementExecuting(nullptr, nullptr, nullptr);
    }
    catch (std::exception& exc)
    {
        DecrementExecuting(nullptr, &exc, std::current_exception());
        throw;
    }
	catch (...)
	{
		std::exception exc("Unexpected exception type in Command::BaseSyncExecute");
		DecrementExecuting(nullptr, &exc, std::current_exception());
		throw;
	}
}

void Command::SyncExecute(Command* owner)
{
	Ptr thisCommand = shared_from_this();

	if (owner != nullptr)
	{
		owner->TakeOwnership(thisCommand);
	}

	try
	{
		SyncExecute();
	}
	catch (...)
	{
		if (owner != nullptr)
		{
			owner->RelinquishOwnership(thisCommand);
		}

		throw;
	}

	if (owner != nullptr)
	{
		owner->RelinquishOwnership(thisCommand);
	}
}

void Command::PreExecute()
{
    // Asynchronously launched commands inform their listener that they are done just before they signal the done event.
    // Owner commands may trigger off these callbacks to help determine when it itself is done (ParallelCommands is an example).
    // When the owner command is done, it will raise its done event, thus signaling the user that it may be relaunched or
    // even disposed. However, the children might not have gotten around yet to signaling their own done events. Thus
    // we take care of that wiggle room here.
	Wait();

	m_doneEvent->Reset();

    // Don't reset the abort event for launched child commands
    if (m_owner == nullptr)
    {
		m_abortEvent->Reset();
	}

	++m_executing;
}

void Command::DecrementExecuting(CommandListener* listener, const std::exception* exc, std::exception_ptr excPtr)
{
    int refCount = --m_executing;

    if (refCount < 0)
    {
		++m_executing;
        throw std::logic_error("Attempt to inform an idle command that it is no longer executing. Command: " + Description());
    }

    if (refCount == 0)
    {
		InformCommandFinished(exc);

        if (listener != nullptr)
        {
            if (exc == nullptr)
            {
                listener->CommandSucceeded();
            }
			else if (dynamic_cast<const CommandAbortedException*>(exc) == nullptr)
            {
				listener->CommandFailed(*exc, excPtr);
            }
            else
            {
				listener->CommandAborted();
			}
        }

		m_doneEvent->Set();
    }
}

bool Command::CommandLess::operator()(const Ptr& a, const Ptr& b) const
{
	return a->Id() < b->Id();
}
