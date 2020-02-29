#pragma once

#include "common.h"

typedef uint32_t mips_inst_t;

enum {
	MIPS_INST_R,
	MIPS_INST_I,
	MIPS_INST_J
};

static inline uint32_t m_inst_raw(mips_inst_t i) { return i; }
static inline uint32_t m_inst_op(mips_inst_t i) { return ExtractBits(&i, 26, 6); }
static inline uint32_t m_inst_funct(mips_inst_t i) { return ExtractBits(&i, 0, 6); }

static inline uint32_t m_inst_type(mips_inst_t i) {
	switch(m_inst_op(i)) {
		case 0: return MIPS_INST_R;
		default: return MIPS_INST_I;
		case 2: case 3: return MIPS_INST_J;
	}
}

static inline uint32_t m_inst_rd(mips_inst_t i) { return ExtractBits(&i, 11, 5); }
static inline uint32_t m_inst_rt(mips_inst_t i) { return ExtractBits(&i, 16, 5); }
static inline uint32_t m_inst_rs(mips_inst_t i) { return ExtractBits(&i, 21, 5); }

static inline uint32_t m_inst_shamt(mips_inst_t i) { return ExtractBits(&i, 6, 5); }
static inline uint32_t m_inst_jmp(mips_inst_t i) { return ExtractBits(&i, 0, 26); }

static inline int32_t m_inst_simm(mips_inst_t i) { return (int16_t)ExtractBits(&i, 0, 16); }
static inline uint32_t m_inst_uimm(mips_inst_t i) { return ExtractBits(&i, 0, 16); }

static inline uint32_t m_inst_id(mips_inst_t i) {
	return m_inst_op(i) ? m_inst_op(i) : (0x40 | m_inst_funct(i));
}

void m_inst_disasm(mips_inst_t i, char *out);
