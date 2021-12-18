#include "lhs_code.h"
#include "lhs_strbuf.h"
#include "lhs_load.h"
#include "lhs_variable.h"

#define lhscode_c(ip)								\
(*(ip)++)

#define lhscode_i(ip)                               \
(*((int*)(ip))++)

#define lhscode_l(ip)                               \
(*((long long*)(ip))++)

#define lhscode_n(ip)                               \
(*((double*)(ip))++)

#define lhscode_b(ip)                               \
(*(ip)++)

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
	"not", "notb", "mov", "movs", "push", 
	"pop", "jmp", "jz", "jnz", "nop", 
	"call", "ret", "ret1", "swap", "exit"
};

int lhscode_dmpcode(LHSVM* vm)
{
	const char* head = vm->code.data;
	const char* tail = head + vm->code.usize;
	while (head < tail)
	{
		const char* cur = head;
		char op = lhscode_c(head);
		int line = lhscode_i(head);
		int column = lhscode_i(head);
		LHSVar* name = lhsvector_at(vm, &vm->conststrs, lhscode_i(head));

		switch (op)
		{
		case OP_MOV:
		{
			char mark1 = lhscode_c(head);
			int index1 = 0;
			switch (mark1)
			{
			case LHS_MARKLOCAL:
			case LHS_MARKGLOBAL:
			case LHS_MARKSTRING:
			case LHS_MARKSTACK:
			{
				index1 = lhscode_i(head);
				break;
			}
			default:
			{
				lhserr_throw(vm, "unexpected byte code.");
			}
			}

			char mark2 = lhscode_c(head);
			int index2 = 0;
			switch (mark1)
			{
			case LHS_MARKLOCAL:
			case LHS_MARKGLOBAL:
			case LHS_MARKSTRING:
			case LHS_MARKSTACK:
			{
				index2 = lhscode_i(head);
				break;
			}
			default:
			{
				lhserr_throw(vm, "unexpected byte code.");
			}
			}

			printf
			(
				"%p\t%s\t%s[%d],\t%s[%d]\t\t;l:%d c:%d ->%s\n", 
				cur, 
				opname[op], 
				markname[mark1], 
				index1,
				markname[mark2], 
				index2,
				line,
				column,
				name->desc->name->data
			);
			break;
		}
		case OP_MOVS:
		{
			char mark = lhscode_c(head);
			int index = lhscode_i(head);
			printf
			(
				"%p\t%s\t%s[%d]\t\t\t;l:%d c:%d ->%s\n", 
				cur, 
				opname[op], 
				markname[mark], 
				index,
				line,
				column,
				name->desc->name->data
			);
			break;
		}
		case OP_PUSH:
		{
			char mark = lhscode_c(head);
			switch (mark)
			{
			case LHS_MARKLOCAL:
			case LHS_MARKGLOBAL:
			case LHS_MARKSTRING:
			case LHS_MARKSTACK:
			{
				int index = lhscode_i(head);
				printf
				(
					"%p\t%s\t%s[%d]\t\t\t;l:%d c:%d ->%s\n", 
					cur, 
					opname[op], 
					markname[mark], 
					index,
					line,
					column,
					name->desc->name->data
				);
				break;
			}
			case LHS_MARKINTEGER:
			{
				long long l = lhscode_l(head);
				printf
				(
					"%p\t%s\t%lld\t\t\t;l:%d c:%d ->%s\n", 
					cur, 
					opname[op], 
					l,
					line,
					column,
					name->desc->name->data
				);
				break;
			}
			case LHS_MARKNUMBER:
			{
				double n = lhscode_n(head);
				printf
				(
					"%p\t%s\t%lf\t\t;l:%d c:%d ->%s\n", 
					cur, 
					opname[op], 
					n,
					line,
					column,
					name->desc->name->data
				);
				break;
			}
			case LHS_MARKBOOLEAN:
			{
				char b = lhscode_b(head);
				printf
				(
					"%p\t%s\t%s\t\t\t;l:%d c:%d ->%s\n", 
					cur, 
					opname[op], 
					b ? "true" : "false",
					line,
					column,
					name->desc->name->data
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
			long long l = lhscode_i(head);
			l += (long long)(vm->code.data);
			printf
			(
				"%p\t%s\t%p\t;l:%d c:%d ->%s\n", 
				cur, 
				opname[op], 
				(void*)l,
				line,
				column,
				name->desc->name->data
			);
			break;
		}
		case OP_CALL:
		{
			char mark = lhscode_c(head);
			int index = lhscode_i(head);
			int argn = lhscode_i(head);
			int retn = lhscode_i(head);
			printf
			(
				"%p\t%s\t%s[%d],\t%d,\t%d\t;l:%d c:%d ->%s\n", 
				cur, 
				opname[op], 
				markname[mark], 
				index,
				argn,
				retn,
				line,
				column,
				name->desc->name->data
			);
			break;
		}
		default:
		{
			printf
			(
				"%p\t%s\t\t\t\t;l:%d c:%d ->%s\n", 
				cur, 
				opname[op],
				line,
				column,
				name->desc->name->data
			);
			break;
		}
		}
	}

	return LHS_TRUE;
}
