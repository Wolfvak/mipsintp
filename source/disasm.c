#include "disasm.h"

static const char *name_tbl[0x80] = {
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

void m_inst_disasm(mips_inst_t i, char *buf)
{
	uint32_t id, rd, rs, rt, imm, jmp;

	id = m_inst_id(i);
	rd = m_inst_rd(i);
	rs = m_inst_rs(i);
	rt = m_inst_rt(i);
	imm = m_inst_uimm(i);
	jmp = m_inst_jmp(i);

	switch(m_inst_type(i)) {
		case MIPS_INST_R:
			sprintf(buf,
				"%s %02X $%d, $%d, $%d", name_tbl[id], id, rd, rs, rt
			);
			break;

		case MIPS_INST_I:
			sprintf(buf,
				"%s %02X $%d, $%d, 0x%04X\n", name_tbl[id], id, rt, rs, imm
			);
			break;

		case MIPS_INST_J:
			sprintf(buf,
				"%s %02X 0x%08X\n", name_tbl[id], id, jmp
			);
			break;
	}
}
