#pragma once;
#include "SyncCommand.h"

namespace CommandLib
{
	/// <summary>
	/// Represents a collection of <see cref="Command"/> objects that execute in sequence, wrapped in a <see cref="Command"/> object.
	/// </summary>
	class SequentialCommands : public SyncCommand
    {
	public:
		/// <summary>Shared pointer to a non-modifyable SequentialCommands object</summary>
		typedef std::shared_ptr<const SequentialCommands> ConstPtr;

		/// <summary>Shared pointer to a SequentialCommands object</summary>
		typedef std::shared_ptr<SequentialCommands> Ptr;

		/// <summary>
		/// Creates a SequentialCommands object
		/// </summary>
		static Ptr Create();

		/// <summary>Adds a <see cref="Command"/> to the collection to execute.</summary>
		/// <param name="command">The command to add</param>
		/// <remarks>
		/// This object takes ownership of any commands that are added, so the passed command must not already have an owner.
		/// <para>Behavior is undefined if you add a command while this SeqentialCommands object is executing</para>
		/// </remarks>
		void Add(Command::Ptr command);

		/// <summary>
		/// Empties all commands from the collection. Behavior is undefined if you call this while this command is executing.
		/// </summary>
		void Clear();

		/// <summary>
		/// Returns diagnostic information about this object's state
		/// </summary>
		/// <returns>
		/// The returned text includes the number of commands in the collection
		/// </returns>
		virtual std::string ExtendedDescription() const override;

		/// <inheritdoc/>
		virtual std::string ClassName() const override;
	protected:
		/// <summary>
		/// This constructor is not public so as to enforce creation using the Create() methods.
		/// </summary>
		SequentialCommands();
	private:
		virtual void SyncExeImpl() final;
        std::list<Command::Ptr> m_commands;
	};
}
