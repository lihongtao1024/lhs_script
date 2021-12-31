#include "lhs_code.h"
#include "lhs_buf.h"
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
	"nop",		"add",		"sub",		"mul",		"div",
	"mod",		"andb",		"orb",		"xorb",		"less",
	"great",	"eq",		"ne",		"ge",		"le",
	"and",		"or",		"shl",		"shr",		"neg",
	"not",		"notb",		"mov",		"concat",	"movs", 
	"push",		"pop",		"jmp",		"jz",		"jnz", 
	"je",		"nop",		"call",		"ret",		"ret1", 
	"swap",		"pushtab",	"instab",	"settab",	"gettab",
	"exit"
};

static const char* lhscode_escapestr(const char* str)
{
	static char buf[256];

	int i = 0, l = sizeof(buf) - sizeof(char) * 2;
	for (; *str && i < l; str++ && i++)
	{
		switch (*str)
		{
		case '\n':
		{
			buf[i++] = '\\';
			buf[i] = 'n';
			break;
		}
		case '\r':
		{
			buf[i++] = '\\';
			buf[i] = 'r';
			break;
		}
		case '\t':
		{
			buf[i++] = '\\';
			buf[i] = 't';
			break;
		}
		case '\f':
		{
			buf[i++] = '\\';
			buf[i] = 'f';
			break;
		}
		case '\v':
		{
			buf[i++] = '\\';
			buf[i] = 'v';
			break;
		}
		case '\"':
		{
			buf[i++] = '\\';
			buf[i] = '"';
			break;
		}
		case '\\':
		{
			buf[i++] = '\\';
			buf[i] = '\\';
			break;
		}
		default:
		{
			buf[i] = *str;
			break;
		}
		}
	}

	buf[i] = 0;
	return buf;
}

static lhscode_dmpfunction(LHSVM* vm, LHSFunction* func)
{
	LHSVar* name = lhsvector_at(vm, &vm->conststrs, func->name);
	printf
	(
		";---function <%s> line:%d column:%d---\n", 
		name->desc->name->data, 
		name->desc->line, 
		name->desc->column
	);

	const char* head = func->code.data;
	const char* tail = head + func->code.usize;
	while (head < tail)
	{
		const char* cur = head;
		char op = lhscode_c(head);
		int line = lhscode_i(head);
		int column = lhscode_i(head);
		int name = lhscode_i(head);
		const char* refer = lhscode_escapestr
		(
			((LHSVar*)lhsvector_at(vm, &vm->conststrs, name))->desc->name->data
		);

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
				"%p\t%s\t%s[%d],\t%s[%d]\t\t;line:%d column:%d ->%s\n", 
				cur, 
				opname[op], 
				markname[mark1], 
				index1,
				markname[mark2], 
				index2,
				line,
				column,
				refer
			);
			break;
		}
		case OP_MOVS:
		{
			char mark = lhscode_c(head);
			int index = lhscode_i(head);
			printf
			(
				"%p\t%s\t%s[%d]\t\t\t;line:%d column:%d ->%s\n", 
				cur, 
				opname[op], 
				markname[mark], 
				index,
				line,
				column,
				refer
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
					"%p\t%s\t%s[%d]\t\t\t;line:%d column:%d ->%s\n", 
					cur, 
					opname[op], 
					markname[mark], 
					index,
					line,
					column,
					refer
				);
				break;
			}
			case LHS_MARKINTEGER:
			{
				long long l = lhscode_l(head);
				printf
				(
					"%p\t%s\t%lld\t\t\t;line:%d column:%d ->%s\n", 
					cur, 
					opname[op], 
					l,
					line,
					column,
					refer
				);
				break;
			}
			case LHS_MARKNUMBER:
			{
				double n = lhscode_n(head);
				printf
				(
					"%p\t%s\t%lf\t\t;line:%d column:%d ->%s\n", 
					cur, 
					opname[op], 
					n,
					line,
					column,
					refer
				);
				break;
			}
			case LHS_MARKBOOLEAN:
			{
				char b = lhscode_b(head);
				printf
				(
					"%p\t%s\t%s\t\t\t;line:%d column:%d ->%s\n", 
					cur, 
					opname[op], 
					b ? "true" : "false",
					line,
					column,
					refer
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
			l += (long long)(func->code.data);
			printf
			(
				"%p\t%s\t%p\t;line:%d column:%d ->%s\n", 
				cur, 
				opname[op], 
				(void*)l,
				line,
				column,
				refer
			);
			break;
		}
		case OP_JE:
		{
			char b = lhscode_b(head);
			long long l = lhscode_i(head);
			l += (long long)(func->code.data);
			printf
			(
				"%p\t%s\t%s,\t%p;line:%d column:%d ->%s\n",
				cur,
				opname[op],
				b ? "true" : "false",
				(void*)l,
				line,
				column,
				refer
			);
			break;
		}
		case OP_CALL:
		{
			char mark = lhscode_c(head);
			int index = lhscode_i(head);
			int argn = lhscode_i(head);
			char wantn = lhscode_c(head);
			printf
			(
				"%p\t%s\t%s[%d],\t%d,\t%d\t;line:%d column:%d ->%s\n", 
				cur, 
				opname[op], 
				markname[mark], 
				index,
				argn,
				wantn,
				line,
				column,
				refer
			);
			break;
		}
		default:
		{
			printf
			(
				"%p\t%s\t\t\t\t;line:%d column:%d ->%s\n", 
				cur, 
				opname[op],
				line,
				column,
				refer
			);
			break;
		}
		}
	}
	printf("\n");
	return LHS_TRUE;
}

int lhscode_dmpcode(LHSVM* vm)
{
	printf(";------------------------------------------------------------------------------------\n");
	printf(";lhscript v1.0\n");
	printf(";programmer: lihong	email: 71164325@qq.com\n");
	printf(";====================================================================================\n\n");
	
	for (LHSFrame* frame = vm->mainframe; frame; frame = frame->next)
	{
		LHSFunction* func = frame->mainfunc;
		lhscode_dmpfunction(vm, func);

		for (func = frame->allfunc;
			func && func != frame->mainfunc;
			func = func->next)
		{
			lhscode_dmpfunction(vm, func);
		}
	}
	return LHS_TRUE;
}
