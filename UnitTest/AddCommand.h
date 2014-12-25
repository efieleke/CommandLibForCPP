#pragma once
#include "SyncCommand.h"
#include <atomic>

namespace CommandLibTests
{
	class AddCommand :
		public CommandLib::SyncCommand
	{
	public:
		typedef std::shared_ptr<AddCommand> Ptr;
		static Ptr Create(std::atomic_int* toModify, int toAdd);
		virtual std::string ClassName() const override;
	private:
		explicit AddCommand(std::atomic_int* toModify, int toAdd);
		AddCommand(const AddCommand&) = delete;
		AddCommand& operator= (const AddCommand&) = delete;

		virtual void SyncExeImpl() override final;

		std::atomic_int* const m_toModify;
		const int m_toAdd;
	};
}
