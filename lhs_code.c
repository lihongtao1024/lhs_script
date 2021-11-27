#include "lhs_code.h"
#include "lhs_load.h"

int lhscode_unarayexpression(char symbol, int index)
{
	int result = true;
	switch (symbol)
	{
	case SYMBOL_ADD:
	case SYMBOL_SUB:
	case SYMBOL_MUL:
	case SYMBOL_DIV:
	case SYMBOL_MOD:
	case SYMBOL_BAND:
	case SYMBOL_BOR:
	case SYMBOL_BXOR:
	case SYMBOL_LESS:
	case SYMBOL_GREAT:
	case SYMBOL_EQUAL:
	case SYMBOL_NOTEQUAL:
	case SYMBOL_GREATEQUAL:
	case SYMBOL_LESSEQUAL:
	case SYMBOL_LOGICAND:
	case SYMBOL_LOGICOR:
	case SYMBOL_BITLEFTSHIFT:
	case SYMBOL_BITRIGHTSHIFT:
	case SYMBOL_BITLEFTSHIFTEQUAL:
	case SYMBOL_BITRIGHTSHIFTEQUAL:
	case SYMBOL_RINCREMENT:
	case SYMBOL_RDECREMENT:
	{
		break;
	}
	default:
	{
		result = false;
		break;
	}
	}
	return result;
}

int lhscode_binaryexpression(char symbol, int index1, int index2)
{
	int result = true;
	switch (symbol)
	{
	case SYMBOL_NOT:
	case SYMBOL_BNOT:
	case SYMBOL_LINCREMENT:
	case SYMBOL_LDECREMENT:
	{
		break;
	}
	default:
	{
		result = false;
		break;
	}
	}
	return result;
}
