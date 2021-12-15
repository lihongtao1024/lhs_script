#include "lhs_code.h"
#include "lhs_strbuf.h"
#include "lhs_load.h"
#include "lhs_variable.h"

#define lhscode_castop(s) ((int)(s))

static const char* markname[LHS_MARKMAX] =
{
	0, "l", "g", 0, 0, 0, "c", "s",
};

static const char* opname[OP_MAX] =
{
	"nop", "add", "sub", "mul", "div",
	"mod", "andb", "orb", "xorb", "l",
	"g", "eq", "ne", "ge", "le",
	"and", "or", "shl", "shr", "neg",
	"not", "notb", "mov", "push", "pop",
	"pushc", "popc", "jmp", "jz", "jnz", 
	"nop", "call", "ret", "return", "exit"
};

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
		case OP_RET:
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
			l += (long long)(vm->code.data);
			printf("%p\t%s\t%p\n", cur, opname[op], (void*)l);
			break;
		}
		case OP_CALL:
		{
			char mark = *head++;
			int index = *((int*)head)++;
			int argn = *((int*)head)++;
			char retn = *head++;
			printf
			(
				"%p\t%s\t%s[%d],\t%d,\t%d\n", 
				cur, 
				opname[op], 
				markname[mark], 
				index,
				argn,
				(int)retn
			);
			break;
		}
		default:
		{
			printf("%p\t%s\n", cur, opname[op]);
			break;
		}
		}
	}
	return LHS_TRUE;
}
