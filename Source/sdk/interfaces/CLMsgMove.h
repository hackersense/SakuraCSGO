#pragma once

#include "../../includes.hpp"

struct CLMsgMove
{
	CLMsgMove() {
		netMessageVtable = *reinterpret_cast<std::uint32_t*>(memory->clSendMove + 126);
		clMsgMoveVtable = *reinterpret_cast<std::uint32_t*>(memory->clSendMove + 138);
		allocatedMemory = *reinterpret_cast<void**>(memory->clSendMove + 131);
		unknown = 15;

		flags = 3;

		unknown1 = 0;
		unknown2 = 0;
		unknown3 = 0;
		unknown4 = 0;
		unknown5 = 0;
	}

	~CLMsgMove() {
		memory->clMsgMoveDescontructor(this);
	}

	inline auto setNumBackupCommands(int backupCommands) {
		this->backupCommands = backupCommands;
	}

	inline auto setNumNewCommands(int newCommands) {
		this->newCommands = newCommands;
	}

	inline auto setData(unsigned char* data, int numBytesWritten) {

		flags |= 4;

		//Why does this work??
		if (allocatedMemory == reinterpret_cast<void*>(memory->clSendMove + 131)) {
			void* newMemory = m_memalloc()->Alloc(24);
			if (newMemory) {
				*((unsigned long*)newMemory + 0x14) = 15;
				*((unsigned long*)newMemory + 0x10) = 0;
				*(unsigned char*)newMemory = 0;
			}

			allocatedMemory = newMemory;
		}

		return memory->clMsgMoveSetData(allocatedMemory, data, numBytesWritten);
	}

	std::uint32_t netMessageVtable; // 0x58 88 0
	std::uint32_t clMsgMoveVtable; // 0x54 84 4
	int unknown1; // 0x4c 80 8
	int backupCommands; // 0x4c 76 12
	int newCommands; // 0x48 72 16
	void* allocatedMemory; // 0x44 68 20
	int unknown2; // 0x40 64 24
	int flags; // 0x3c 60 28
	char unknown3; // 0x38 64 32
	std::byte pad[3]; // 65 33
	char unknown4; // 0x34 68 36
	std::byte pad2[15]; // 69 37
	int unknown5; // 0x24 84 52
	int unknown; // 0x20 88 56
	bufferWrite dataOut;
};
