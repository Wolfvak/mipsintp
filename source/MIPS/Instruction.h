#pragma once

#include "MIPS/perf.h"
#include <iostream>

namespace MIPS {
	enum InstructionClass {
		Register,
		Immediate,
		Jump,
		Special
	};

	enum InstructionType {
		Arithmetic,
		Logical,
		Shift,
		LoadStore,
		Branch,
	};

	class Instruction {
	public:
		Instruction(uint32_t i, uint32_t *gprBase, const void **tgtBase);

		Instruction(uint32_t *gprBase, const void **tgtBase)
		 : Instruction(0, gprBase, tgtBase) {}
		/* nop */

		const void *dispatchTarget() { return target; }

		uint32_t *rrd() { return r.rd; }
		uint32_t *rrt() { return r.rt; }
		uint32_t *rrs() { return r.rs; }
		unsigned rshamt() { return r.imm; }

		uint32_t *irs() { return i.rs; }
		uint32_t *irt() { return i.rt; }
		int32_t isimm() { return i.simm; }
		uint32_t iuimm() { return i.uimm; }

		uint32_t jtgt() { return j.tgt; }

	private:
		union {
			struct {
				uint32_t *rd, *rt;
				union { uint32_t *rs; uint8_t imm; };
				// uses either immediate for constant shift or rs
			} r; // R-Type instruction (three operands)

			struct {
				uint32_t *rs;
				uint32_t *rt;
				union { uint16_t uimm; int16_t simm; };
			} i; // I-Type instruction (immediates)

			struct {
				uint32_t tgt;
			} j; // J-Type instruction (28-bit offset)
		};

		const void *target; // dispatch target
	};
}
