#pragma once

#include "common.h"
#include "disasm.h"

enum {
	R0 = 0,
	SP = 29,
	AL = 31,
	PC = 32,
	LO = 33,
	HI = 34,
};

typedef struct {
	uint32_t r[36]; // extra register to keep struct alignment
	uint64_t inst_ctr;
	uint8_t m[4096]; // replace with a proper TLB at some point
} mips_ctx;

void m_ctx_init(mips_ctx *ctx);
void m_ctx_dump(const mips_ctx *ctx);
void m_ctx_load_bootrom(mips_ctx *ctx, const char *path);

static inline uint8_t m_mem_read8(mips_ctx *ctx, uint32_t addr) {
	return ctx->m[addr];
}

static inline uint16_t m_mem_read16(mips_ctx *ctx, uint32_t addr) {
	return *(uint16_t*)(&ctx->m[addr]);
}

static inline uint32_t m_mem_read32(mips_ctx *ctx, uint32_t addr) {
	return *(uint32_t*)(&ctx->m[addr]);
}

static inline void m_mem_write8(mips_ctx *ctx, uint32_t addr, uint8_t v) {
	ctx->m[addr] = v;
}

static inline void m_mem_write16(mips_ctx *ctx, uint32_t addr, uint16_t v) {
	*(uint16_t*)(&ctx->m[addr]) = v;
}

static inline void m_mem_write32(mips_ctx *ctx, uint32_t addr, uint32_t v) {
	*(uint32_t*)(&ctx->m[addr]) = v;
}
