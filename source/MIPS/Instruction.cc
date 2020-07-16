#include "MIPS/Instruction.h"

namespace MIPS {
	static const char *instNames[0x80] = {
		"unk", "bcondz", "j", "jal",
		"beq", "bne", "blez", "bgtz",
		"addi", "addiu", "slti", "sltiu",
		"andi", "ori", "xori", "lui",

		"cop0", "cop1", "cop2", "cop3",
		"unk", "unk", "unk", "unk",
		"unk", "unk", "unk", "unk",
		"unk", "unk", "unk", "unk",

		"lb", "lh", "lwl", "lw",
		"lbu", "lhu", "lwr", "unk",
		"sb", "sh", "swl", "sw",
		"unk", "unk", "swr", "unk",

		"lwc0", "lwc1", "lwc2", "lwc3",
		"unk", "unk", "unk", "unk",
		"swc0", "swc1", "swc2", "swc3",
		"unk", "unk", "unk", "unk",

		"sll", "unk", "srl", "sra",
		"sllv", "unk", "srlv", "srav",
		"jr", "jalr", "unk", "unk",
		"syscall", "bkpt", "unk", "unk",

		"mfhi", "mthi", "mflo", "mtlo",
		"unk", "unk", "unk", "unk",
		"mult", "multu", "div", "divu",
		"unk", "unk", "unk", "unk",

		"add", "addu", "sub", "subu",
		"and", "or", "xor", "nor",
		"unk", "unk", "slt", "sltu",
		"unk", "unk", "unk", "unk",

		"unk", "unk", "unk", "unk",
		"unk", "unk", "unk", "unk",
		"unk", "unk", "unk", "unk",
		"unk", "unk", "unk", "unk",
	};

	Instruction::Instruction(uint32_t i, uint32_t *gprBase, const void **tgtBase) {
		unsigned op, func, id, rs, rt, rd, shamt, jmp, imm;

		op = ExtractBits(&i, 26, 6);
		func = ExtractBits(&i, 0, 6);
		id = op ? op : (0x40 | func);

		shamt = ExtractBits(&i, 6, 5);
		rd = ExtractBits(&i, 11, 5);
		rt = ExtractBits(&i, 16, 5);
		rs = ExtractBits(&i, 21, 5);

		imm = ExtractBits(&i, 0, 16);
		jmp = ExtractBits(&i, 0, 26);

		this->target = tgtBase[id];

		switch(op) {
			case 0: // R-Type
				this->r.rd = &gprBase[rd];
				this->r.rt = &gprBase[rt];
				if (id >= 0x40 && id <= 0x47)
					this->r.imm = shamt; // shift by constant
				else
					this->r.rs = &gprBase[rs]; // third operand
				break;

			default: // I-Type
				this->i.rs = &gprBase[rs];
				this->r.rt = &gprBase[rt];
				this->i.uimm = imm;
				break;

			case 2: case 3: // J-Type
				this->j.tgt = jmp;
				break;
		}
	}
}