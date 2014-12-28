#pragma once;
#include "Command.h"

namespace CommandLib
{
	/// <summary>
	/// A <see cref="Command"/> which is a simple holder for another <see cref="Command"/> object. It's only useful purpose is to aid in preventing
	/// many temporary <see cref="Command"/> objects from collecting in memory during the lifetime of their owner.
	/// </summary>
	/// <remarks>
	/// If you find that you need to create a Command object within the execution method of its owning command
	/// (perhaps because which type of Command to create depends upon runtime conditions), there are some things to
	/// consider. Owned commands are not destroyed until the owner is destroyed. If the owner is executed many times
	/// before it is destroyed, and you create a new child command upon every execution, resource usage will grow unbounded.
	/// The better approach is to assign this locally created command to a <see cref="VariableCommand"/> object,
	/// which would be a member variable of the owner. The assignment will take care of destroying any previously assigned
	/// command.
	/// </remarks>
	class VariableCommand final : public Command
    {
	public:
		/// <summary>Shared pointer to a non-modifyable VariableCommand object</summary>
		typedef std::shared_ptr<const VariableCommand> ConstPtr;

		/// <summary>Shared pointer to a VariableCommand object</summary>
		typedef std::shared_ptr<VariableCommand> Ptr;

		/// <summary>
		/// Creates a VariableCommand object
		/// </summary>
		static Ptr Create();

		/// <summary>Gets the underlying command to run.</summary>
		Command::ConstPtr GetCommandToRun() const;

		/// <summary>Sets the underlying command to run.</summary>
		/// <remarks>
		/// <para>
		/// This object takes ownership of any command assigned to this property, so the passed command must not already have
		/// an owner.
		/// </para>
		/// <para>
		/// It is acceptable to assign null, but a runtime error will occur if this is null and this VariableCommand instance
		/// is executed.
		/// </para>
		/// <para>Behavior is undefined if this property is changed while this command is executing.</para>
		/// </remarks>
		void SetCommandToRun(Command::Ptr command);

		/// <inheritdoc/>
		virtual std::string ClassName() const override;
	protected:
		/// <summary>
		/// This constructor is not public so as to enforce creation using the Create() methods.
		/// </summary>
		VariableCommand();
	private:
		virtual void SyncExecuteImpl() final;
		virtual void AsyncExecuteImpl(CommandListener* listener) final;

        Command::Ptr m_commandToRun;
	};
}
