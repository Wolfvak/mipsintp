#include <cassert>
#include "MIPS/TLB.h"

namespace MIPS {

	TLBEntry::TLBEntry(uint32_t start, uint32_t size, unsigned flags, void *ptr)
	{
		// gaddr contains both address region and flag information
		// bits 31-8: set if address is accessible
		// bits 7-0: set if the corresponding flag is enabled

		unsigned mask = size - 1;

		assert(size >= 256);
		assert(!(size & mask));
		assert(!(start & mask));

		this->gaddr = ((start | mask) & ~0xff) | flags;
		if (flags & TLBEntryFlags::BUFFER) {
			this->hbase = reinterpret_cast<uintptr_t> (ptr) - start;
		} else {
			this->hdevice = reinterpret_cast<MMDevice*> (ptr);
		}
	}

	bool TLBEntry::match(uint32_t addr, unsigned flags) {
		uint32_t mask = this->gaddr;
		mask ^= guestAddr(addr, flags);
		mask &= ~this->gaddr;

		return mask == 0;
	}

	template<typename T>
	T TLBEntry::read(uint32_t addr) {
		if (this->gaddr & TLBEntryFlags::BUFFER) {
			return this->fastRead<T> (addr);
		} else {
			return static_cast<T> (this->hdevice->read(addr, sizeof(T)));
		}
	}

	template<typename T>
	void TLBEntry::write(uint32_t addr, T val) {
		if (this->gaddr & TLBEntryFlags::BUFFER) {
			this->fastWrite<T> (addr, val);
		} else {
			this->hdevice->write(addr, val, sizeof(T));
		}
	}

	// running on a 32 bit bus, so provide primitives
	// to load signed and unsigned values <= 32 bits
	// and storing unsigned <= 32 bit integers
	template
	int8_t TLBEntry::read<int8_t> (uint32_t);

	template
	int16_t TLBEntry::read<int16_t> (uint32_t);

	template
	uint8_t TLBEntry::read<uint8_t> (uint32_t);

	template
	uint16_t TLBEntry::read<uint16_t> (uint32_t);

	template
	uint32_t TLBEntry::read<uint32_t> (uint32_t);

	template
	void TLBEntry::write<uint8_t> (uint32_t, uint8_t);

	template
	void TLBEntry::write<uint16_t> (uint32_t, uint16_t);

	template
	void TLBEntry::write<uint32_t> (uint32_t, uint32_t);
}
