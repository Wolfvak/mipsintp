#include "MIPS/CPU.h"
#include "MIPS/Instruction.h"

#include "MIPS/perf.h"

/*
 use macros for all these ops to hint they should be inlined
 gcc might want to merge some together but clang doesnt seem to do this
*/

#define ZERO_R0() do { this->gpRegister[GPRegister::R0] = 0; } while(0)

// adds off to the current PC
// updates the instruction counter
#define UPDATE_PC_REL(off) \
	do { \
		this->instCounter += *pc - last_pc; \
		*pc += off; \
		*gpc = *pc; \
		last_pc = *pc; \
	} while(0)

// same as above but sets the absolute PC
// PC is in word steps (byte address >> 2)
#define UPDATE_PC_ABS(abs) \
	do { \
		this->instCounter += *pc - last_pc; \
		*pc = abs; \
		*gpc = *pc; \
		last_pc = *pc; \
	} while(0)

#define READ_PC() \
	(*pc << 2)

#define INST_IMPL_LOAD(t, destptr, addr) \
	do { \
		/* short circuit to the most likely entry */ \
		if (LIKELY(this->tlb[0].fastMatch(addr, (sizeof(t)-1) | TLBEntryFlags::READ))) { \
			*(destptr) = this->tlb[0].fastRead<t> (addr); \
		} else { \
			this->loadMemory<t> (addr, destptr); \
		} \
	} while(0)

#define INST_IMPL_STORE(t, srcptr, addr) \
	do { \
		if (LIKELY(this->tlb[0].fastMatch(addr, (sizeof(t)-1) | TLBEntryFlags::WRITE))) { \
			this->tlb[0].fastWrite<t> (addr, *(srcptr)); \
		} else { \
			this->storeMemory<t> (addr, reinterpret_cast<t*> (srcptr)); \
		} \
	} while(0)

#define CHAIN_NEXT() \
	do{ \
		ci = ni; \
		ni = &icache[(*pc)++]; \
		goto *ci->dispatchTarget(); \
	} while(0)

#define CI_RIDX(rptr) \
	((rptr) - this->gpRegister)

/* returns garbage on unknown instructions */
#define CI_ID() \
	(ci->dispatchTarget() - cpuDispatchTable)

namespace MIPS {
	void CPU::run()
	{
		static const void *cpuDispatchTable[0x80] = {
			/* BASE OPCODES (0x00 - 0x3F) */
			&&unk, &&bcondz, &&j, &&jal,
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

		Instruction nopi(0, this->gpRegister, cpuDispatchTable);
		Instruction *icache = (Instruction*)malloc(sizeof(Instruction) * 128);
		Instruction *ci = &nopi, *ni = &icache[0];

		for (auto i = 0; i < 128; i++) {
			uint32_t inst = 0;
			this->loadMemory<uint32_t> (i * 4, &inst);
			icache[i] = Instruction(inst, this->gpRegister, cpuDispatchTable);
		}

		uint32_t *const gpc = &this->gpRegister[GPRegister::PC];
		uint32_t local_pc = *gpc, last_pc = *gpc;
		uint32_t *const pc = &local_pc;

		uint32_t *const al = &this->gpRegister[GPRegister::AL];
		uint32_t *const lo = &this->gpRegister[GPRegister::LO];
		uint32_t *const hi = &this->gpRegister[GPRegister::HI];

		ZERO_R0();
		CHAIN_NEXT();

		bcondz:
			// bCONDz $s, off
			// cond = I_RT()
			// cond.0 ? >= 0 : < 0
			// cond.4 ? set AL to return address : dont modify AL
			if ((*ci->irs() >> 31) ^ (CI_RIDX(ci->irt()) & 1)) {
				if (CI_RIDX(ci->irt()) & BIT(4)) {
					*al = READ_PC();
				}
				UPDATE_PC_REL(ci->isimm() - 1);
			}
			CHAIN_NEXT();

		jal: // jal target
			*al = READ_PC();
			// fallthrough to j target

		j: // j target
			UPDATE_PC_ABS((READ_PC() & 0xF0000000) | ci->jtgt());
			CHAIN_NEXT();

		beq: // beq $s, $t, offset
			if (*ci->irs() == *ci->irt())
				UPDATE_PC_REL(ci->isimm() - 1);
			CHAIN_NEXT();

		bne: // bne $s, $t, offset
			if (*ci->irs() != *ci->irt())
				UPDATE_PC_REL(ci->isimm() - 1);
			CHAIN_NEXT();

		blez: // blez $s, offset
			if ((int32_t)*ci->irs() <= 0)
				UPDATE_PC_REL(ci->isimm() - 1);
			CHAIN_NEXT();

		bgtz: // bgtz $s, offset
			if ((int32_t)*ci->irs() > 0)
				UPDATE_PC_REL(ci->isimm() - 1);
			CHAIN_NEXT();

		addi: // no trap handling yet
		addiu: // addi(u) $t, $s, imm
			*ci->irt() = *ci->irs() + ci->isimm();
			ZERO_R0();
			CHAIN_NEXT();

		slti: // slti $t, $s, imm
			*ci->irt() = (int32_t)*ci->irs() < ci->isimm();
			ZERO_R0();
			CHAIN_NEXT();

		sltiu: // sltiu $t, $s, imm
			*ci->irt() = (*ci->irs() < ci->iuimm()) ? 1 : 0;
			ZERO_R0();
			CHAIN_NEXT();

		andi: // andi $t, $s, imm
			*ci->irt() = *ci->irs() & ci->iuimm();
			ZERO_R0();
			CHAIN_NEXT();

		ori: // ori $t, $s, imm
			*ci->irt() = *ci->irs() | ci->iuimm();
			ZERO_R0();
			CHAIN_NEXT();

		xori: // xori $t, $s, imm
			*ci->irt() = *ci->irs() ^ ci->iuimm();
			ZERO_R0();
			CHAIN_NEXT();

		lui: // lui $t, imm
			*ci->irt() = ci->iuimm() << 16;
			ZERO_R0();
			CHAIN_NEXT();

		lb: // lb $t, imm($s)
			INST_IMPL_LOAD(int8_t, ci->irt(), *ci->irs() + ci->isimm());
			CHAIN_NEXT();

		lbu: // lbu $t, imm($s)
			INST_IMPL_LOAD(uint8_t, ci->irt(), *ci->irs() + ci->isimm());
			CHAIN_NEXT();

		lh: // lh $t, imm($s)
			INST_IMPL_LOAD(int16_t, ci->irt(), *ci->irs() + ci->isimm());
			CHAIN_NEXT();

		lhu: // lhu $t, imm($s)
			INST_IMPL_LOAD(uint16_t, ci->irt(), *ci->irs() + ci->isimm());
			CHAIN_NEXT();

		lw: // lw $t, imm($s)
			INST_IMPL_LOAD(uint32_t, ci->irt(), *ci->irs() + ci->isimm());
			CHAIN_NEXT();

		lwr: // lwr $t, imm($s)
		{
			uint32_t addr, word, shift;

			addr = *ci->irs() + ci->isimm();

			INST_IMPL_LOAD(uint32_t, &word, addr & ~3);

			shift = (addr & 3) * 8;
			*ci->irt() &= shift ? (0xFFFFFFFF << (32 - shift)) : 0;
			*ci->irt() |= word >> shift;

			CHAIN_NEXT();
		}

		lwl: // lwl $t, imm($s)
		{
			uint32_t addr, word, shift;

			addr = *ci->irs() + ci->isimm();

			INST_IMPL_LOAD(uint32_t, &word, addr & ~3);

			shift = (3 - (addr & 3)) * 8;
			*ci->irt() &= shift ? (0xFFFFFFFF >> (32 - shift)) : 0;
			*ci->irt() |= word << shift;

			CHAIN_NEXT();
		}

		sb: // sb $t, imm($s)
			INST_IMPL_STORE(uint8_t, ci->irt(), *ci->irs() + ci->isimm());
			CHAIN_NEXT();

		sh: // sh $t, imm($s)
			INST_IMPL_STORE(uint16_t, ci->irt(), *ci->irs() + ci->isimm());
			CHAIN_NEXT();

		swl: // swl $t, imm($s) - unimplemented
		{
			goto unk;
		}

		sw: // sw $t, imm($s)
			INST_IMPL_STORE(uint32_t, ci->irt(), *ci->irs() + ci->isimm());
			CHAIN_NEXT();

		swr: // swr $t, imm($s) - unimplemented
		{
			goto unk;
		}

		sll: // sll $d, $t, h
			*ci->rrd() = *ci->rrt() << ci->rshamt();
			ZERO_R0();
			CHAIN_NEXT();

		srl: // srl $d, $t, h
			*ci->rrd() = *ci->rrt() >> ci->rshamt();
			ZERO_R0();
			CHAIN_NEXT();

		sra: // sra $d, $t, h
			*ci->rrd() = ((int32_t)*ci->rrt()) >> ci->rshamt();
			ZERO_R0();
			CHAIN_NEXT();

		sllv: // sllv $d, $t, $s
			*ci->rrd() = *ci->rrt() << *ci->rrs();
			ZERO_R0();
			CHAIN_NEXT();

		srlv: // srlv $d, $t, $s
			*ci->rrd() = *ci->rrt() >> *ci->rrs();
			ZERO_R0();
			CHAIN_NEXT();

		srav: // srav $d, $t, $s
			*ci->rrd() = ((int32_t)(*ci->rrt())) >> *ci->rrs();
			ZERO_R0();
			CHAIN_NEXT();

		jalr: // jalr $d, $s
			// $d is almost always $31/$AL but it's variable
			*ci->rrd() = READ_PC();
			ZERO_R0();
			UPDATE_PC_ABS(*ci->rrs() >> 2);
			CHAIN_NEXT();

		jr: // jr $s
			UPDATE_PC_ABS(*ci->rrs() >> 2);
			CHAIN_NEXT();

		mfhi: // mfhi $d
			*ci->rrd() = *hi;
			ZERO_R0();
			CHAIN_NEXT();

		mflo: // mflo $d
			*ci->rrd() = *lo;
			ZERO_R0();
			CHAIN_NEXT();

		mthi: // mthi $s
			*hi = *ci->rrs();
			CHAIN_NEXT();

		mtlo: // mtlo $s
			*lo = *ci->rrs();
			CHAIN_NEXT();

		mult: // mult $s, $t
		{
			uint64_t mult_res = (int64_t)((int32_t)*ci->rrs()) * (int64_t)((int32_t)*ci->rrt());
			*lo = mult_res;
			*hi = mult_res >> 32;
			CHAIN_NEXT();
		}

		multu: // multu $s, $t
		{
			uint64_t mult_res = (uint64_t)(*ci->rrs()) * (uint64_t)(*ci->rrt());
			*lo = mult_res;
			*hi = mult_res >> 32;
			CHAIN_NEXT();
		}

		div: // div $s, $t
		{
			// todo: handle special cases
			*lo = (int32_t)*ci->rrs() / (int32_t)*ci->rrt();
			*hi = (int32_t)*ci->rrs() % (int32_t)*ci->rrt();
			CHAIN_NEXT();
		}

		divu: // divu $s, $t
		{
			*lo = *ci->rrs() / *ci->rrt();
			*hi = *ci->rrs() % *ci->rrt();
			CHAIN_NEXT();
		}

		add: // no traps yet
		addu: // addu $d, $s, $t
			*ci->rrd() = *ci->rrs() + *ci->rrt();
			ZERO_R0();
			CHAIN_NEXT();

		sub: // no traps yet
		subu: // subu $d, $s, $t
			*ci->rrd() = *ci->rrs() - *ci->rrt();
			ZERO_R0();
			CHAIN_NEXT();

		iand: // and $d, $s, $t
			*ci->rrd() = *ci->rrs() & *ci->rrt();
			ZERO_R0();
			CHAIN_NEXT();

		ior: // or $d, $s, $t
			*ci->rrd() = *ci->rrs() | *ci->rrt();
			ZERO_R0();
			CHAIN_NEXT();

		ixor: // xor $d, $s, $t
			*ci->rrd() = *ci->rrs() ^ *ci->rrt();
			ZERO_R0();
			CHAIN_NEXT();

		nor: // nor $d, $s, $t
			*ci->rrd() = 0xffffffff ^ (*ci->rrs() | *ci->rrt());
			ZERO_R0();
			CHAIN_NEXT();

		slt: // slt $d, $s, $t
			*ci->rrd() = (int32_t)*ci->rrs() < (int32_t)*ci->rrt();
			ZERO_R0();
			CHAIN_NEXT();

		sltu: // sltu $d, $s, $t
			*ci->rrd() = *ci->rrs() < *ci->rrt();
			ZERO_R0();
			CHAIN_NEXT();

		syscall:
			printf("SYSCALL\n");
			goto unk;

		bkpt:
			printf("BKPT\n");
			goto unk;

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
			uint32_t inst = ~0;
			std::cout << *this << std::endl;
			this->loadMemory<uint32_t> (READ_PC(), &inst);
			printf("caught unknown instruction %08X @ %08X\n", inst, READ_PC());
			fflush(stdout);
			exit(0);
	}
}
