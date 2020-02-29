#include "common.h"
#include "disasm.h"
#include "mipsint.h"

#define ZERO_R0() do{ctx->r[0]=0;}while(0)

#define CHAIN_NEXT() \
	do{ \
		(ctx->inst_ctr)++; \
		curi = nexti; \
		nexti = MR32(*pc); \
		*pc += 4; \
		goto *dispatch_tbl[m_inst_op(curi)]; \
	}while(0)
	// m_inst_op should be replaced by a dispatch to m_inst_id on
	// archs with poor branch predictors and fast bit operations (ARM, PPC)

#define I_SHF()	m_inst_shamt(curi)
#define I_RD()	m_inst_rd(curi)
#define I_RS()	m_inst_rs(curi)
#define I_RT()	m_inst_rt(curi)
#define I_JMP()	m_inst_jmp(curi)
#define I_UIMM()	m_inst_uimm(curi)
#define I_SIMM()	m_inst_simm(curi)

#define MR8(x)	m_mem_read8(ctx, (x))
#define MR16(x)	m_mem_read16(ctx, (x))
#define MR32(x)	m_mem_read32(ctx, (x))

#define MW8(x,b)	m_mem_write8(ctx, (x), (b))
#define MW16(x,h)	m_mem_write16(ctx, (x), (h))
#define MW32(x,w)	m_mem_write32(ctx, (x), (w))

// Type-R
#define START_TYPE_R_D()	do{rd = &ctx->r[I_RD()];}while(0)
#define START_TYPE_R_S()	do{rs = &ctx->r[I_RS()];}while(0)
#define START_TYPE_R_DS()	do{rd = &ctx->r[I_RD()]; rs = &ctx->r[I_RS()];}while(0)
#define START_TYPE_R_ST()	do{rs = &ctx->r[I_RS()]; rt = &ctx->r[I_RT()];}while(0)
#define START_TYPE_R_DTH()	do{rd = &ctx->r[I_RD()]; rt = &ctx->r[I_RT()]; imm.h = I_SHF();}while(0)
#define START_TYPE_R()	do{rd = &ctx->r[I_RD()]; rs = &ctx->r[I_RS()]; rt = &ctx->r[I_RT()];}while(0)

// Type-I / Signed
#define START_TYPE_SI_SI()	do{rs = &ctx->r[I_RS()]; imm.s = I_SIMM();}while(0)
#define START_TYPE_SI()	do{rs = &ctx->r[I_RS()]; rt = &ctx->r[I_RT()]; imm.s = I_SIMM();}while(0)

// Type-I / Unsigned
#define START_TYPE_UI_SI()	do{rs = &ctx->r[I_RS()]; imm.u = I_UIMM();}while(0)
#define START_TYPE_UI_TI()	do{rt = &ctx->r[I_RT()]; imm.u = I_UIMM();}while(0)
#define START_TYPE_UI()	do{rs = &ctx->r[I_RS()]; rt = &ctx->r[I_RT()]; imm.u = I_UIMM();}while(0)

// Type-J
#define START_TYPE_J()	do{jmp = I_JMP();}while(0)

void mips_start_exec(mips_ctx *ctx)
{
	ctx->inst_ctr = 0;
	static const void *dispatch_tbl[0x80] = {
		/* BASE OPCODES (0x00 - 0x3F) */
		&&functor, &&bcondz, &&j, &&jal,
		&&beq, &&bne, &&blez, &&bgtz,
		&&addi, &&addiu, &&slti, &&sltiu,
		&&andi, &&ori, &&xori, &&lui,

		&&cop0, &&cop1, &&cop2, &&cop3,
		&&unk, &&unk, &&unk, &&unk,
		&&unk, &&unk, &&unk, &&unk,
		&&unk, &&unk, &&unk, &&unk,

		&&lb, &&lh, &&lwl, &&lw,
		&&lbu, &&lhu, &&lwr, &&unk,
		&&sb, &&sh, &&swl, &&sw,
		&&unk, &&unk, &&swr, &&unk,

		&&lwc0, &&lwc1, &&lwc2, &&lwc3,
		&&unk, &&unk, &&unk, &&unk,
		&&swc0, &&swc1, &&swc2, &&swc3,
		&&unk, &&unk, &&unk, &&unk,

		/* SPECIAL OPCODES (0x40 - 0x7F) */
		&&sll, &&unk, &&srl, &&sra,
		&&sllv, &&unk, &&srlv, &&srav,
		&&jr, &&jalr, &&unk, &&unk,
		&&syscall, &&bkpt, &&unk, &&unk,

		&&mfhi, &&mthi, &&mflo, &&mtlo,
		&&unk, &&unk, &&unk, &&unk,
		&&mult, &&multu, &&div, &&divu,
		&&unk, &&unk, &&unk, &&unk,

		&&add, &&addu, &&sub, &&subu,
		&&iand, &&ior, &&ixor, &&nor,
		&&unk, &&unk, &&slt, &&sltu,
		&&unk, &&unk, &&unk, &&unk,

		&&unk, &&unk, &&unk, &&unk,
		&&unk, &&unk, &&unk, &&unk,
		&&unk, &&unk, &&unk, &&unk,
		&&unk, &&unk, &&unk, &&unk,
	};

	uint64_t mult_res;
	mips_inst_t curi, nexti = 0;

	uint32_t *const pc = &ctx->r[PC], *const al = &ctx->r[AL];
	uint32_t *const lo = &ctx->r[LO], *const hi = &ctx->r[HI];
	uint32_t *rd = NULL, *rs = NULL, *rt = NULL;
	uint32_t jmp = ~0;
	union { uint32_t u; int32_t s; uint32_t h; } imm = {~0};

	CHAIN_NEXT();

	functor:
		goto *dispatch_tbl[0x40 + m_inst_funct(curi)];

	bcondz:
		// bCONDz $s, off
		// cond = I_RT()
		// cond.0 ? >= 0 : < 0
		// cond.4 ? set AL to return address : dont modify AL
		START_TYPE_SI();
		if (((int32_t)*rs < 0) ^ (*rt & 1)) {
			if (*rt & BIT(4))
				*al = *pc;
			*pc += (imm.s - 1) * 4;
		}
		CHAIN_NEXT();

	jal: // jal target
		//START_TYPE_J();
		*al = *pc;
		//*pc = (*pc & 0xF0000000) | (jmp << 2);
		//CHAIN_NEXT();

	j: // j target
		START_TYPE_J();
		*pc = (*pc & 0xF0000000) | (jmp << 2);
		CHAIN_NEXT();

	beq: // beq $s, $t, offset
		START_TYPE_SI();
		if (*rs == *rt)
			*pc += (imm.s - 1) * 4;
		CHAIN_NEXT();

	bne: // bne $s, $t, offset
		START_TYPE_SI();
		if (*rs != *rt)
			*pc += (imm.s - 1) * 4;
		CHAIN_NEXT();

	blez: // blez $s, offset
		START_TYPE_SI_SI();
		if ((int32_t)*rs <= 0)
			*pc += (imm.s - 1) * 4;
		CHAIN_NEXT();

	bgtz: // bgtz $s, offset
		START_TYPE_SI_SI();
		if ((int32_t)*rs > 0)
			*pc += (imm.s - 1) * 4;
		CHAIN_NEXT();

	addi: // no trap handling yet
	addiu: // addi(u) $t, $s, imm
		START_TYPE_SI();
		*rt = *rs + imm.s;
		ZERO_R0();
		CHAIN_NEXT();

	slti: // slti $t, $s, imm
		START_TYPE_SI();
		*rt = (int32_t)*rs < imm.s;
		ZERO_R0();
		CHAIN_NEXT();

	sltiu: // sltiu $t, $s, imm
		START_TYPE_UI();
		*rt = (*rs < imm.u) ? 1 : 0;
		ZERO_R0();
		CHAIN_NEXT();

	andi: // andi $t, $s, imm
		START_TYPE_UI();
		*rt = *rs & imm.u;
		ZERO_R0();
		CHAIN_NEXT();

	ori: // ori $t, $s, imm
		START_TYPE_UI();
		*rt = *rs | imm.u;
		ZERO_R0();
		CHAIN_NEXT();

	xori: // xori $t, $s, imm
		START_TYPE_UI();
		*rt = *rs ^ imm.u;
		ZERO_R0();
		CHAIN_NEXT();

	lui: // lui $t, imm
		START_TYPE_UI_TI();
		*rt = imm.u << 16;
		ZERO_R0();
		CHAIN_NEXT();

	lb: // lb $t, imm($s)
		START_TYPE_SI();
		*rt = (int8_t)MR8(*rs + imm.s);
		ZERO_R0();
		CHAIN_NEXT();

	lh: // lh $t, imm($s)
		START_TYPE_SI();
		*rt = (int16_t)MR16(*rs + imm.s);
		ZERO_R0();
		CHAIN_NEXT();

	lwl: // lwl $t, imm($s) - unimplemented
		goto unk;

	lw: // lw $t, imm($s)
		START_TYPE_SI();
		*rt = MR32(*rs + imm.s);
		ZERO_R0();
		CHAIN_NEXT();

	lbu: // lbu $t, imm($s)
		START_TYPE_SI();
		*rt = MR8(*rs + imm.s);
		ZERO_R0();
		CHAIN_NEXT();

	lhu: // lhu $t, imm($s)
		START_TYPE_SI();
		*rt = MR16(*rs + imm.s);
		ZERO_R0();
		CHAIN_NEXT();

	lwr: // lwr $t, imm($s) - unimplemented
		goto unk;

	sb: // sb $t, imm($s)
		START_TYPE_SI();
		MW8(*rs + imm.s, *rt);
		CHAIN_NEXT();

	sh: // sh $t, imm($s)
		START_TYPE_SI();
		MW16(*rs + imm.s, *rs);
		CHAIN_NEXT();

	swl: // swl $t, imm($s) - unimplemented
		goto unk;

	sw: // sw $t, imm($s)
		START_TYPE_SI();
		MW32(*rs + imm.s, *rt);
		CHAIN_NEXT();

	swr: // swr $t, imm($s) - unimplemented
		goto unk;

	sll: // sll $d, $t, h
		if (m_inst_raw(curi)) { // canonical nop if zero
			START_TYPE_R_DTH();
			*rd = *rt << imm.h;
			ZERO_R0();
		}
		CHAIN_NEXT();

	srl: // srl $d, $t, h
		START_TYPE_R_DTH();
		*rd = *rt >>imm.h;
		ZERO_R0();
		CHAIN_NEXT();

	sra: // sra $d, $t, h
		START_TYPE_R_DTH();
		*rd = ((int32_t)*rt) >> imm.h;
		ZERO_R0();
		CHAIN_NEXT();

	sllv: // sllv $d, $t, $s
		START_TYPE_R();
		*rd = *rt << *rs;
		ZERO_R0();
		CHAIN_NEXT();

	srlv: // srlv $d, $t, $s
		START_TYPE_R();
		*rd = *rt >> *rs;
		ZERO_R0();
		CHAIN_NEXT();

	srav: // srav $d, $t, $s
		START_TYPE_R();
		*rd = (int32_t)*rt >> *rs;
		ZERO_R0();
		CHAIN_NEXT();

	jalr: // jalr $d, $s
		// $d is almost always $31/$AL but it's variable
		START_TYPE_R_DS();
		*rd = *pc;
		ZERO_R0();
		*pc = *rs;
		CHAIN_NEXT();

	jr: // jr $s
		START_TYPE_R_S();
		*pc = *rs;
		CHAIN_NEXT();

	mfhi: // mfhi $d
		START_TYPE_R_D();
		*rd = *hi;
		ZERO_R0();
		CHAIN_NEXT();

	mflo: // mflo $d
		START_TYPE_R_D();
		*rd = *lo;
		ZERO_R0();
		CHAIN_NEXT();

	mthi: // mthi $s
		START_TYPE_R_S();
		*hi = *rs;
		CHAIN_NEXT();

	mtlo: // mtlo $s
		START_TYPE_R_S();
		*lo = *rs;
		CHAIN_NEXT();

	mult: // mult $s, $t
		START_TYPE_R_ST();
		mult_res = (int64_t)((int32_t)*rs) * (int64_t)((int32_t)*rt);
		*lo = mult_res;
		*hi = mult_res >> 32;
		CHAIN_NEXT();

	multu: // multu $s, $t
		START_TYPE_R_ST();
		mult_res = (uint64_t)(*rs) * (uint64_t)(*rt);
		*lo = mult_res;
		*hi = mult_res >> 32;
		CHAIN_NEXT();

	div: // div $s, $t
		START_TYPE_R_ST();
		*lo = (int32_t)*rs / (int32_t)*rt;
		*hi = (int32_t)*rs % (int32_t)*rt;
		CHAIN_NEXT();

	divu: // divu $s, $t
		START_TYPE_R_ST();
		*lo = *rs / *rt;
		*hi = *rs % *rt;
		CHAIN_NEXT();

	add: // no traps yet
	addu: // addu $d, $s, $t
		START_TYPE_R();
		*rd = *rs + *rt;
		ZERO_R0();
		CHAIN_NEXT();

	sub: // no traps yet
	subu: // subu $d, $s, $t
		START_TYPE_R();
		*rd = *rs - *rt;
		ZERO_R0();
		CHAIN_NEXT();

	iand: // and $d, $s, $t
		START_TYPE_R();
		*rd = *rs & *rt;
		ZERO_R0();
		CHAIN_NEXT();

	ior: // or $d, $s, $t
		START_TYPE_R();
		*rd = *rs | *rt;
		ZERO_R0();
		CHAIN_NEXT();

	ixor: // xor $d, $s, $t
		START_TYPE_R();
		*rd = *rs ^ *rt;
		ZERO_R0();
		CHAIN_NEXT();

	nor: // nor $d, $s, $t
		START_TYPE_R();
		*rd = 0xffffffff ^ (*rs | *rt);
		ZERO_R0();
		CHAIN_NEXT();

	slt: // slt $d, $s, $t
		START_TYPE_R();
		*rd = (int32_t)*rs < (int32_t)*rt;
		ZERO_R0();
		CHAIN_NEXT();

	sltu: // sltu $d, $s, $t
		START_TYPE_R();
		*rd = *rs < *rt;
		ZERO_R0();
		CHAIN_NEXT();

	syscall: // not yet implemented
	bkpt:

	cop0:
	cop1:
	cop2:
	cop3:

	swc0:
	swc1:
	swc2:
	swc3:

	lwc0:
	lwc1:
	lwc2:
	lwc3:

	unk: // unknown instruction
		m_ctx_dump(ctx);
		printf("caught unknown instruction: %08X / %d\n", m_inst_raw(curi), m_inst_id(curi));
		fflush(stdout);
		exit(0);
}
