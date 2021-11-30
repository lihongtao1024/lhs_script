#include "lhs_code.h"
#include "lhs_load.h"
#include "lhs_variable.h"

#define lhscode_castop(s) ((int)(s))

static const char* markname[LHS_MARKMAX] =
{
	0, "L", "G", 0, 0, 0, "C", "S"
};

static const char* opname[OP_MAX] =
{
	"nop", "add", "sub", "mul", "div",
	"mod", "band", "bor", "bxor", "less",
	"great", "eq", "ne", "ge", "le",
	"and", "or", "lsht", "rsht", "not", 
	"bnot", "push", "pop", "mov",
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
			return false;
		}
		lhsbuf_pushlstr(vm, buf, temp, l);
		break;
	}
	case LHS_MARKNUMBER:
	{
		int l = sprintf(temp, "%lf", code->code.n);
		if (l == -1)
		{ 
			return false;
		}
		lhsbuf_pushlstr(vm, buf, temp, l);
		break;
	}
	case LHS_MARKBOOLEAN:
	{
		lhsbuf_pushstr(vm, buf, code->code.b ? "true" : "false");
		break;
	}
	default:
	{
		int l = sprintf(temp, "%s[%d]", markname[code->mark], 
			code->code.index);
		lhsbuf_pushlstr(vm, buf, temp, l);
		break;
	}
	}

	return true;
}

static int lhscode_dumpinstruction(LHSVM* vm, LHSInstruction* instruction)
{
	LHSSTRBUF buf;
	lhsbuf_init(vm, &buf);

	switch (instruction->op)
	{
	case OP_NOT:
	case OP_BNOT:
	case OP_PUSH:
	case OP_POP:
	{
		lhsbuf_pushstr(vm, &buf, opname[instruction->op]);
		lhsbuf_pushchar(vm, &buf, ' ');
		lhscode_dumpcode(vm, &instruction->body.unary.code, &buf);
		break;
	}
	default:
	{
		lhsbuf_pushstr(vm, &buf, opname[instruction->op]);
		lhsbuf_pushchar(vm, &buf, ' ');
		lhscode_dumpcode(vm, &instruction->body.binary.code1, &buf);
		lhsbuf_pushstr(vm, &buf, ", ");
		lhscode_dumpcode(vm, &instruction->body.binary.code2, &buf);
		break;
	}
	}

	printf("%s\n", buf.data);
	lhsbuf_uninit(vm, &buf);
	return true;
}

int lhscode_unaryexpr(LHSVM* vm, char symbol, LHSCode* code)
{
	LHSInstruction instruction;
	instruction.op = lhscode_castop(symbol);
	memcpy(&instruction.body.unary.code, code, sizeof(LHSCode));

	lhscode_dumpinstruction(vm, &instruction);
	return true;
}

int lhscode_binaryexpr(LHSVM* vm, char symbol, LHSCode* code1,
	LHSCode* code2)
{
	LHSInstruction instruction;
	instruction.op = lhscode_castop(symbol);
	memcpy(&instruction.body.binary.code1, code1, sizeof(LHSCode));
	memcpy(&instruction.body.binary.code2, code2, sizeof(LHSCode));

	lhscode_dumpinstruction(vm, &instruction);
	return true;
}
