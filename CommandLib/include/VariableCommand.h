﻿#pragma once;
#include "Command.h"

namespace CommandLib
{
	/// <summary>
	/// A <see cref="Command"/> which is a simple holder for another <see cref="Command"/> object. It's only useful purpose is to aid in preventing
	/// many temporary <see cref="Command"/> objects from collecting in memory during the lifetime of their owner.
	/// </summary>
	/// <remarks>
	/// If you find that you are generating a <see cref="Command"/> object local to the execution method of an owning
	/// <see cref="Command"/>, it's best to not specify the creator as the owner of this local command. Owned commands are
	/// not destructed until the owner is destructed, so if the owner is executed many times before it is destructed,
	/// it's possible for resource usage to grow unbounded. The better approach is to assign this local command
	/// to a VariableCommand object, which would be a member variable of the owner. <see cref="SetCommandToRun"/>
	/// will take care of destructing any previously assigned command.
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
