#pragma once

#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "MIPS/perf.h"
#include "MIPS/TLB.h"

namespace MIPS {
	enum GPRegister {
		R0 = 0,
		SP = 29,
		AL = 31,
		LO = 32,
		HI = 33,
		PC = 34
	};

	class CPU {
	public:
		CPU();
		~CPU();

		void run();

		uint32_t &accessGpr(unsigned i) {
			if (i >= 35)
				throw new std::runtime_error("cant access gpregister " + std::to_string(i));
			return this->gpRegister[i];
		}

		uint32_t &accessCpr(unsigned i) {
			if (i >= 16)
				throw new std::runtime_error("cant access cpregister " + std::to_string(i));
			return this->cpRegister[i];
		}

		friend std::ostream &operator<<(std::ostream &os, const CPU &cpu);

		// memory access helpers
		template<typename T>
		void loadMemory(uint32_t addr, uint32_t *r) {
			if (addr & (sizeof(T)-1)) {
				std::cout << "unaligned access @ " << addr << std::endl;
				exit(0);
			}

			for (auto i = 0; i < 128; i++) {
				if (this->tlb[i].match(addr, TLBEntryFlags::READ)) {
					*r = this->tlb[i].read<T> (addr);
					if (this->tlb[i].flags() & TLBEntryFlags::BUFFER)
						std::swap(this->tlb[i], this->tlb[0]);
					return;
				}
			}

			std::cout << "unable to load memory @ " << std::to_string(addr) << ", size = " << sizeof(T) << std::endl;
			exit(0);
		}

		template<typename T>
		void storeMemory(uint32_t addr, T *r) {
			if (addr & (sizeof(T)-1)) {
				std::cout << "unaligned access @ " << addr << std::endl;
				exit(0);
			}

			for (auto i = 0; i < 128; i++) {
				if (this->tlb[i].match(addr, TLBEntryFlags::WRITE)) {
					this->tlb[i].write<T> (addr, *r);
					if (this->tlb[i].flags() & TLBEntryFlags::BUFFER)
						std::swap(this->tlb[i], this->tlb[0]);
					return;
				}
			}

			std::cout << "unable to store memory @ " << std::to_string(addr) << ", size = " << sizeof(T) << std::endl;
			exit(0);
		}

	private:
		uint32_t gpRegister[36];
		uint32_t cpRegister[16];
		uint64_t instCounter;

		MIPS::TLBEntry tlb[128];
	};

}
