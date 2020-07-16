#pragma once

#include "MIPS/perf.h"

namespace MIPS {
	enum TLBEntryFlags {
		ACCESS_8BIT = 0,
		ACCESS_16BIT = 1,
		ACCESS_32BIT = 3,
		READ = BIT(2), // entry is readable
		WRITE = BIT(3), // entry is writable
		EXEC = BIT(4), // entry is executable
		BUFFER = BIT(5), // entry is a direct memory buffer

		MASK = BIT(6) - 1,
	};

	// memory mapped device, has a bunch of special read/write ops
	// not quite designed for super high speeds, unlike the real thing
	class MMDevice {
	public:
		virtual uint32_t read(uint32_t addr, unsigned sz) = 0;
		virtual void write(uint32_t addr, uint32_t val, unsigned sz) = 0;
	};

	class TLBEntry {
	public:
		TLBEntry()
		 : gaddr(0), hdevice(nullptr) {}

		TLBEntry(uint32_t start, uint32_t size, unsigned flags, void *ptr);

		// the slow ops handle both buffer and device access
		bool match(uint32_t addr, unsigned flags);

		template<typename T>
		T read(uint32_t addr);

		template<typename T>
		void write(uint32_t addr, T val);

		// fast{Match, Read, Write} are meant for RAM/ROM access only
		static constexpr uint32_t guestAddr(uint32_t addr, unsigned flags) {
			unsigned sz = (flags & 3) + 1;
			return (addr & ~(0x100 - sz)) ^ (flags & ~3);
		}

		bool constexpr fastMatch(uint32_t addr, unsigned flags) {
			uint32_t mask = this->gaddr;
			mask ^= guestAddr(addr, flags | TLBEntryFlags::BUFFER);
			mask &= ~this->gaddr;

			return mask == 0;
		}

		template<typename T>
		T fastRead(uint32_t addr) {
			return *(reinterpret_cast<T*> (this->hbase + addr));
		}

		template<typename T>
		void fastWrite(uint32_t addr, T val) {
			*(reinterpret_cast<T*> (this->hbase + addr)) = val;
		}

		TLBEntryFlags constexpr flags() {
			return static_cast<TLBEntryFlags> (this->gaddr & TLBEntryFlags::MASK);
		}

	private:
		uint32_t gaddr;
		union {
			uintptr_t hbase;
			MMDevice *hdevice;
		};
	};
}
