#include "lhs_code.h"
#include "lhs_load.h"
#include "lhs_variable.h"

#define lhscode_castop(s) ((int)(s))

static const char* markname[LHS_MARKMAX] =
{
	0, "local", "global", 0, 0, 0, "const", "stack",
};

static const char* opname[OP_MAX] =
{
	"nop", "add", "sub", "mul", "div",
	"mod", "band", "bor", "bxor", "less",
	"great", "eq", "ne", "ge", "le",
	"and", "or", "lsht", "rsht", "neg",
	"not", "bnot", "push", "pop", "mov",
	"pushc", "popc", "jmp", "jmpf", "nop"
};

static int lhscode_dumpcode(LHSVM* vm, LHSCode* code, LHSSTRBUF* buf)
{
	char temp[64];
	switch (code->mark)
	{
	case LHS_MARKINTEGER:
	{
		int l = sprintf(temp, "%lld", code->code.i);
		if (l == -1)
		{ 
			return LHS_FALSE;
		}
		lhsbuf_pushls(vm, buf, temp, l);
		break;
	}
	case LHS_MARKNUMBER:
	{
		int l = sprintf(temp, "%lf", code->code.n);
		if (l == -1)
		{ 
			return LHS_FALSE;
		}
		lhsbuf_pushls(vm, buf, temp, l);
		break;
	}
	case LHS_MARKBOOLEAN:
	{
		lhsbuf_pushs(vm, buf, code->code.b ? "true" : "false");
		break;
	}
	default:
	{
		int l = sprintf(temp, "%s[%d]", markname[code->mark], 
			code->code.index);
		lhsbuf_pushls(vm, buf, temp, l);
		break;
	}
	}

	return LHS_TRUE;
}

static int lhscode_dumpinstruction(LHSVM* vm, LHSInstruction* instruction)
{
	LHSSTRBUF buf;
	lhsbuf_init(vm, &buf);

	switch (instruction->op)
	{
	case OP_NEG:
	case OP_NOT:
	case OP_BNOT:
	case OP_PUSH:
	case OP_POP:
	case OP_PUSHC:
	case OP_JMP:
	case OP_JMPF:
	{
		lhsbuf_pushs(vm, &buf, opname[instruction->op]);
		lhsbuf_pushc(vm, &buf, '\t');
		lhscode_dumpcode(vm, &instruction->body.unary.code, &buf);
		break;
	}
	case OP_POPC:
	case OP_NOP:
	{
		lhsbuf_pushs(vm, &buf, opname[instruction->op]);
		break;
	}
	default:
	{
		lhsbuf_pushs(vm, &buf, opname[instruction->op]);
		lhsbuf_pushc(vm, &buf, '\t');
		lhscode_dumpcode(vm, &instruction->body.binary.code1, &buf);
		lhsbuf_pushs(vm, &buf, ",\t");
		lhscode_dumpcode(vm, &instruction->body.binary.code2, &buf);
		break;
	}
	}

	printf("%s\n", buf.data);
	lhsbuf_uninit(vm, &buf);
	return LHS_TRUE;
}

int lhscode_unarydump(LHSVM* vm, char symbol, LHSCode* code)
{
	LHSInstruction instruction;
	instruction.op = lhscode_castop(symbol);
	if (code)
	{
		memcpy(&instruction.body.unary.code, code, sizeof(LHSCode));
	}

	lhscode_dumpinstruction(vm, &instruction);
	return LHS_TRUE;
}

int lhscode_binarydump(LHSVM* vm, char symbol, LHSCode* code1,
	LHSCode* code2)
{
	LHSInstruction instruction;
	instruction.op = lhscode_castop(symbol);
	memcpy(&instruction.body.binary.code1, code1, sizeof(LHSCode));
	memcpy(&instruction.body.binary.code2, code2, sizeof(LHSCode));

	lhscode_dumpinstruction(vm, &instruction);
	return LHS_TRUE;
}
