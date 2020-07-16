#include "MIPS/CPU.h"

#include <cstring>
#include <inttypes.h> // todo: replace these with regular cout things

namespace MIPS {

	class ConsoleDevice : MMDevice {
		uint32_t read(uint32_t addr, unsigned sz) { return 0; }
		void write(uint32_t addr, uint32_t val, unsigned sz) {
			std::cout << static_cast<char> (val);
		}
	};

	CPU::CPU() : instCounter(0)
	{
		std::memset(this->gpRegister, 0, sizeof(this->gpRegister));
		std::memset(this->cpRegister, 0, sizeof(this->cpRegister));

		this->gpRegister[GPRegister::PC] = 0;

		this->instCounter = 0;

		uint8_t *mem = new uint8_t[0x1000];
		std::memset(mem, 0, 0x1000);

		this->tlb[0] = MIPS::TLBEntry(0x00000000, 0x00001000, TLBEntryFlags::READ | TLBEntryFlags::WRITE | TLBEntryFlags::EXEC | TLBEntryFlags::BUFFER, mem);
		this->tlb[2] = MIPS::TLBEntry(0x00004000, 0x00000100, TLBEntryFlags::WRITE | TLBEntryFlags::READ, new ConsoleDevice);
	}

	CPU::~CPU()
	{}

	std::ostream &operator<<(std::ostream &os, const CPU &cpu)
	{
		char outstr[1024], *strptr = outstr;

		for (auto i = 0; i < 32; i++) {
			strptr += sprintf(strptr, "R%02d: 0x%08X", i, cpu.gpRegister[i]);
			if ((i & 3) != 3) strptr += sprintf(strptr, "   ");
			else strptr += sprintf(strptr, "\n");
		}

		strptr += sprintf(strptr, "PC:  0x%08X   HI:  0x%08X   LO:  0x%08X\n\nCTR: %" PRIu64 "\n",
			cpu.gpRegister[GPRegister::PC] << 2,
			cpu.gpRegister[GPRegister::HI],
			cpu.gpRegister[GPRegister::LO],
			cpu.instCounter
		);

		os << outstr;
		return os;
	}
}
