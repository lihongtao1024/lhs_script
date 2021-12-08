#include "lhs_code.h"
#include "lhs_strbuf.h"
#include "lhs_load.h"
#include "lhs_variable.h"

#define lhscode_castop(s) ((int)(s))

static const char* markname[LHS_MARKMAX] =
{
	0, "v", "g", 0, 0, 0, "c", "s",
};

static const char* opname[OP_MAX] =
{
	"nop", "add", "sub", "mul", "div",
	"mod", "andb", "orb", "xorb", "l",
	"g", "eq", "ne", "ge", "le",
	"and", "or", "shl", "shr", "neg",
	"not", "notb", "mov", "push", "pop",
	"pushc", "popc", "jmp", "jz", "jnz", 
	"nop", "call"
};

#ifdef OP_DEBUG
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
	case OP_NOTB:
	case OP_PUSH:
	case OP_POP:
	case OP_PUSHC:
	case OP_JMP:
	case OP_JZ:
	case OP_JNZ:
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
#endif

int lhscode_dmpcode(LHSVM* vm)
{
	const char* head = vm->code.data;
	const char* tail = head + vm->code.usize;
	while (head < tail)
	{
		const char* cur = head;
		char op = *head++;
		switch (op)
		{
		case OP_NEG:
		case OP_NOT:
		case OP_NOTB:
		{
			char mark = *head++;
			int index = *((int*)head)++;
			printf
			(
				"%p\t%s\t%s[%d]\n", 
				cur, 
				opname[op], 
				markname[mark], 
				index
			);
			break;
		}
		case OP_PUSH:
		case OP_PUSHC:
		{
			char mark = *head++;
			switch (mark)
			{
			case LHS_MARKLOCAL:
			case LHS_MARKGLOBAL:
			case LHS_MARKSTRING:
			case LHS_MARKSTACK:
			{
				int index = *((int*)head)++;
				printf
				(
					"%p\t%s\t%s[%d]\n", 
					cur, 
					opname[op], 
					markname[mark], 
					index
				);
				break;
			}
			case LHS_MARKINTEGER:
			{
				long long l = *((long long*)head)++;
				printf("%p\t%s\t%lld\n", cur, opname[op], l);
				break;
			}
			case LHS_MARKNUMBER:
			{
				double n = *((double*)head)++;
				printf("%p\t%s\t%lf\n", cur, opname[op], n);
				break;
			}
			case LHS_MARKBOOLEAN:
			{
				char b = *head++;
				printf
				(
					"%p\t%s\t%s\n", 
					cur, 
					opname[op], 
					b ? "true" : "false"
				);
				break;
			}
			default:
			{
				lhserr_throw(vm, "unexpected byte code.");
			}
			}
			break;
		}
		case OP_JMP:
		case OP_JZ:
		case OP_JNZ:
		{
			char mark = *head++;
			if (mark != LHS_MARKINTEGER)
			{
				lhserr_throw(vm, "unexpected byte code.");
			}

			long long l = *((long long*)head)++;
			printf("%p\t%s\t%p\n", cur, opname[op], (void*)l);
			break;
		}
		case OP_POPC:
		case OP_NOP:
		{
			printf("%p\t%s\n", cur, opname[op]);
			break;
		}
		case OP_CALL:
		{
			char mark = *head++;
			int index = *((int*)head)++;
			int argn = *((int*)head)++;
			printf
			(
				"%p\t%s\t%s[%d],\t%d\n", 
				cur, 
				opname[op], 
				markname[mark], 
				index,
				argn
			);
			break;
		}
		default:
		{
			char mark1 = *head++;
			int index1 = *((int*)head)++;
			char mark2 = *head++;
			int index2 = *((int*)head)++;
			printf
			(
				"%p\t%s\t%s[%d],\t%s[%d]\n", 
				cur, 
				opname[op], 
				markname[mark1], 
				index1, 
				markname[mark2], 
				index2
			);
			break;
		}
		}
	}
	return LHS_TRUE;
}
