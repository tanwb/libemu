/* @header@ */
#include <stdint.h>
#include <errno.h>

#define INSTR_CALC(bits, a, cpu) \
UINT(bits) operand_a = a; \
UINT(bits) operation_result = 0-operand_a; 

#define INSTR_SET_FLAG_OF(cpu, bits) \
{ \
	int64_t sx = (INT(bits))operand_a; \
	int64_t sz = 0; \
 \
	sz = 0-sx; \
 \
	if (sz < max_inttype_borders[sizeof(operation_result)][0][0] || sz > max_inttype_borders[sizeof(operation_result)][0][1] \
	|| sz != (int64_t)operation_result )									    \
	{                                                                           \
		CPU_FLAG_SET(cpu, f_of);                                                 \
	}else                                                                       \
	{                                                                           \
		CPU_FLAG_UNSET(cpu, f_of);                                               \
	}                                                                           \
}

#define INSTR_SET_FLAG_CF(cpu, a)											\
{																				\
	if (a == 0)																\
	{                                                                           \
		CPU_FLAG_SET(cpu, f_cf);                                                 \
	}else                                                                       \
	{                                                                           \
		CPU_FLAG_UNSET(cpu, f_cf);                                               \
	}                                                                           \
}



#include "emu/emu.h"
#include "emu/emu_cpu.h"
#include "emu/emu_cpu_data.h"
#include "emu/emu_cpu_functions.h"
#include "emu/emu_cpu_stack.h"
#include "emu/emu_memory.h"

/*Intel Architecture Software Developer's Manual Volume 2: Instruction Set Reference (24319102.PDF) page 494*/

#define INSTR_CALC_AND_SET_FLAGS(bits, cpu, a)	\
INSTR_CALC(bits, a, cpu) \
INSTR_SET_FLAG_CF(cpu, a) \
INSTR_SET_FLAG_OF(cpu, bits) \
INSTR_SET_FLAG_SF(cpu) \
INSTR_SET_FLAG_ZF(cpu) \
INSTR_SET_FLAG_PF(cpu)





int32_t instr_group_3_f6_neg(struct emu_cpu *c, struct instruction *i)
{
	if ( i->modrm.mod != 3 )
	{
		/* F6 /3 
		 * Two's complement negate r/m8
		 * NEG r/m8  
		 */
		uint8_t m8;
		MEM_BYTE_READ(c, i->modrm.ea, &m8);
		INSTR_CALC_AND_SET_FLAGS(8,
								 c,
								 m8)
		MEM_BYTE_WRITE(c, i->modrm.ea, m8);

	}
	else
	{
		/* F6 /3 
		 * Two's complement negate r/m8
		 * NEG r/m8  
		 */
		INSTR_CALC_AND_SET_FLAGS(8,c,*c->reg8[i->modrm.rm]);
	}
	return 0;
}


int32_t instr_group_3_f7_neg(struct emu_cpu *c, struct instruction *i)
{
	if ( i->modrm.mod != 3 )
	{
		if ( i->prefixes & PREFIX_OPSIZE )
		{
			/* F7 /3 
			 * Two's complement negate r/m16
			 * NEG r/m16 
			 */
			uint16_t m16;
			MEM_WORD_READ(c, i->modrm.ea, &m16);
			INSTR_CALC_AND_SET_FLAGS(16,
									 c,
									 m16)
			MEM_WORD_WRITE(c, i->modrm.ea, m16);

		}
		else
		{
			/* F7 /3 
			 * Two's complement negate r/m32
			 * NEG r/m32 
			 */
			uint32_t m32;
			MEM_DWORD_READ(c, i->modrm.ea, &m32);
			INSTR_CALC_AND_SET_FLAGS(32,
									 c,
									 m32)
			MEM_DWORD_WRITE(c, i->modrm.ea, m32);

		}
	}
	else
	{
		if ( i->prefixes & PREFIX_OPSIZE )
		{
			/* F7 /3 
			 * Two's complement negate r/m16
			 * NEG r/m16 
			 */
			INSTR_CALC_AND_SET_FLAGS(16,
									 c,
									 *c->reg16[i->modrm.rm])
		}
		else
		{
			/* F7 /3 
			 * Two's complement negate r/m32
			 * NEG r/m32 
			 */
			INSTR_CALC_AND_SET_FLAGS(32,
									 c,
									 c->reg[i->modrm.rm])

		}
	}


	return 0;
}



