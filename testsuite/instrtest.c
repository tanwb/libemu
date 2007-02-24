#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>


#include <emu/emu.h>
#include <emu/emu_memory.h>
#include <emu/emu_cpu.h>
#include <emu/emu_log.h>
#include <emu/emu_cpu_data.h>

#define CODE_OFFSET 4711

#define FAILED "\033[31;1mfailed\033[0m"
#define SUCCESS "\033[32;1msuccess\033[0m"

#define F(x) (1 << (x))


static struct run_time_options
{
	int verbose;
	int nasm_force;
} opts;

static const char *regm[] = {
	"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"
};


	                         /* 0     1     2     3      4       5       6     7 */
static const char *flags[] = { "CF", "  ", "PF", "  " , "AF"  , "    ", "ZF", "SF", 
	                           "TF", "IF", "DF", "OF" , "IOPL", "IOPL", "NT", "  ",
	                           "RF", "VM", "AC", "VIF", "RIP" , "ID"  , "  ", "  ",
	                           "  ", "  ", "  ", "   ", "    ", "    ", "  ", "  "};


struct instr_test
{
	const char *instr;

	char  *code;
	uint16_t codesize;

	struct 
	{
		uint32_t reg[8];
		uint32_t		mem_state[2];
		uint32_t	eflags;
	} in_state;

	struct 
	{
		uint32_t reg[8];
		uint32_t		mem_state[2];
		uint32_t	eflags;
		uint32_t eip;
	} out_state;
};

#define FLAG(fl) (1 << (fl))

struct instr_test tests[] = 
{

/*  {
        .instr = "instr",
        .in_state.reg  = {0,0,0,0,0,0,0,0 },
        .in_state.mem_state = {0, 0},
        .out_state.reg  = {0,0,0,0,0,0,0,0 },
        .out_state.mem_state = {0, 0},
    },*/
	{
		.instr = "shellcode",
		.code =                 "\xeb\x03\x59\xeb\x05\xe8\xf8\xff\xff\xff\x4f\x49\x49\x49\x49\x49"
                "\x49\x51\x5a\x56\x54\x58\x36\x33\x30\x56\x58\x34\x41\x30\x42\x36"
                "\x48\x48\x30\x42\x33\x30\x42\x43\x56\x58\x32\x42\x44\x42\x48\x34"
                "\x41\x32\x41\x44\x30\x41\x44\x54\x42\x44\x51\x42\x30\x41\x44\x41"
                "\x56\x58\x34\x5a\x38\x42\x44\x4a\x4f\x4d\x4e\x4f\x4c\x56\x4b\x4e"
                "\x4d\x44\x4a\x4e\x49\x4f\x4f\x4f\x4f\x4f\x4f\x4f\x42\x46\x4b\x48"
                "\x4e\x46\x46\x52\x46\x52\x4b\x38\x45\x54\x4e\x33\x4b\x38\x4e\x47"
                "\x45\x30\x4a\x57\x41\x30\x4f\x4e\x4b\x38\x4f\x54\x4a\x51\x4b\x38"
                "\x4f\x45\x42\x52\x41\x50\x4b\x4e\x49\x44\x4b\x58\x46\x43\x4b\x58"
                "\x41\x50\x50\x4e\x41\x53\x42\x4c\x49\x49\x4e\x4a\x46\x48\x42\x4c"
                "\x46\x37\x47\x50\x41\x4c\x4c\x4c\x4d\x50\x41\x50\x44\x4c\x4b\x4e"
                "\x46\x4f\x4b\x33\x46\x45\x46\x52\x4a\x42\x45\x47\x45\x4e\x4b\x48"
                "\x4f\x55\x46\x52\x41\x30\x4b\x4e\x48\x56\x4b\x48\x4e\x50\x4b\x34"
                "\x4b\x48\x4f\x35\x4e\x41\x41\x50\x4b\x4e\x43\x50\x4e\x52\x4b\x38"
                "\x49\x58\x4e\x36\x46\x32\x4e\x31\x41\x36\x43\x4c\x41\x33\x4b\x4d"
                "\x46\x56\x4b\x38\x43\x54\x42\x33\x4b\x48\x42\x34\x4e\x30\x4b\x58"
                "\x42\x57\x4e\x41\x4d\x4a\x4b\x38\x42\x54\x4a\x30\x50\x55\x4a\x46"
                "\x50\x48\x50\x54\x50\x30\x4e\x4e\x42\x45\x4f\x4f\x48\x4d\x48\x56"
                "\x43\x55\x48\x46\x4a\x46\x43\x33\x44\x43\x4a\x46\x47\x57\x43\x57"
                "\x44\x53\x4f\x55\x46\x35\x4f\x4f\x42\x4d\x4a\x46\x4b\x4c\x4d\x4e"
                "\x4e\x4f\x4b\x53\x42\x35\x4f\x4f\x48\x4d\x4f\x45\x49\x48\x45\x4e"
                "\x48\x36\x41\x58\x4d\x4e\x4a\x30\x44\x50\x45\x55\x4c\x56\x44\x30"
                "\x4f\x4f\x42\x4d\x4a\x46\x49\x4d\x49\x50\x45\x4f\x4d\x4a\x47\x45"
                "\x4f\x4f\x48\x4d\x43\x35\x43\x55\x43\x35\x43\x45\x43\x35\x43\x54"
                "\x43\x45\x43\x34\x43\x55\x4f\x4f\x42\x4d\x48\x46\x4a\x46\x41\x51"
                "\x4e\x35\x48\x36\x43\x35\x49\x58\x41\x4e\x45\x49\x4a\x36\x46\x4a"
                "\x4c\x41\x42\x37\x47\x4c\x47\x45\x4f\x4f\x48\x4d\x4c\x46\x42\x41"
                "\x41\x55\x45\x35\x4f\x4f\x42\x4d\x4a\x56\x46\x4a\x4d\x4a\x50\x52"
                "\x49\x4e\x47\x45\x4f\x4f\x48\x4d\x43\x35\x45\x45\x4f\x4f\x42\x4d"
                "\x4a\x56\x45\x4e\x49\x44\x48\x58\x49\x34\x47\x45\x4f\x4f\x48\x4d"
                "\x42\x55\x46\x35\x46\x55\x45\x35\x4f\x4f\x42\x4d\x43\x39\x4a\x46"
                "\x47\x4e\x49\x37\x48\x4c\x49\x47\x47\x35\x4f\x4f\x48\x4d\x45\x45"
                "\x4f\x4f\x42\x4d\x48\x36\x4c\x46\x46\x46\x48\x46\x4a\x56\x43\x46"
                "\x4d\x56\x49\x48\x45\x4e\x4c\x36\x42\x55\x49\x45\x49\x42\x4e\x4c"
                "\x49\x38\x47\x4e\x4c\x46\x46\x54\x49\x38\x44\x4e\x41\x43\x42\x4c"
                "\x43\x4f\x4c\x4a\x50\x4f\x44\x44\x4d\x42\x50\x4f\x44\x34\x4e\x52"
                "\x43\x39\x4d\x48\x4c\x37\x4a\x33\x4b\x4a\x4b\x4a\x4b\x4a\x4a\x56"
                "\x44\x37\x50\x4f\x43\x4b\x48\x31\x4f\x4f\x45\x37\x46\x34\x4f\x4f"
                "\x48\x4d\x4b\x55\x47\x55\x44\x55\x41\x45\x41\x55\x41\x45\x4c\x56"
                "\x41\x50\x41\x35\x41\x55\x45\x55\x41\x55\x4f\x4f\x42\x4d\x4a\x56"
                "\x4d\x4a\x49\x4d\x45\x50\x50\x4c\x43\x45\x4f\x4f\x48\x4d\x4c\x36"
                "\x4f\x4f\x4f\x4f\x47\x33\x4f\x4f\x42\x4d\x4b\x48\x47\x45\x4e\x4f"
                "\x43\x38\x46\x4c\x46\x56\x4f\x4f\x48\x4d\x44\x45\x4f\x4f\x42\x4d"
                "\x4a\x46\x42\x4f\x4c\x48\x46\x50\x4f\x35\x43\x35\x4f\x4f\x48\x4d"
                "\x4f\x4f\x42\x4d\x5a",

		.codesize = 709,
		.in_state.reg  = {0,0,0,0,0xffffffff,0,0,0},
		.in_state.mem_state = {0, 0},
	},

    /* 00 */
	{
		.instr = "add ah,al",
		.code = "\x00\xc4",
		.codesize = 2,
		.in_state.reg  = {0xff01,0,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x01,0,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags = FLAG(f_cf) | FLAG(f_pf) | FLAG(f_zf),
	},
	{
		.instr = "add ch,dl",
		.code = "\x00\xd5",
		.codesize = 2,
		.in_state.reg  = {0,0x1000,0x20,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0,0x3000,0x20,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "add [ecx],al",
		.code = "\x00\x01",
		.codesize = 2,
		.in_state.reg  = {0x10,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x10101010},
		.out_state.reg  = {0x10,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x10101020},
	},
	/* 01 */
	{
		.instr = "add ax,cx",
		.code = "\x66\x01\xc8",
		.codesize = 3,
		.in_state.reg  = {0xffff1111,0xffff2222,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0xffff3333,0xffff2222,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf), 
	},
	{
		.instr = "add [ecx],ax",
		.code = "\x66\x01\x01",
		.codesize = 3,
		.in_state.reg  = {0xffff1111,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x22224444},
		.out_state.reg  = {0xffff1111,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x22225555},
		.out_state.eflags =  FLAG(f_pf), 
	},
	{
		.instr = "add eax,ecx",
		.code = "\x01\xc8",
		.codesize = 2,
		.in_state.reg  = {0x11112222,0x22221111,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x33333333,0x22221111,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf), 
	},
	{
		.instr = "add [ecx],eax",
		.code = "\x01\x01",
		.codesize = 2,
		.in_state.reg  = {0x22221111,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x22224444},
		.out_state.reg  = {0x22221111,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x44445555},
		.out_state.eflags =  FLAG(f_pf), 
	},
	/* 02 */
	{
		.instr = "add cl,bh",
		.code = "\x02\xcf",	/* add cl,bh */
		.codesize = 2,
		.in_state.reg  = {0,0xff,0,0x100,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0,0,0,0x100,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_cf) | FLAG(f_pf) | FLAG(f_zf), 
	},
	{
		.instr = "add al,[ecx]",
		.code = "\x02\x01",
		.codesize = 2,
		.in_state.reg  = {0x3,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x30303030},
		.out_state.reg  = {0x33,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x30303030},
		.out_state.eflags =  FLAG(f_pf),
	},
	/* 03 */
	{
		.instr = "add cx,di",
		.code = "\x66\x03\xcf",	/* add cx,di */
		.codesize = 3,
		.in_state.reg  = {0,0x10101010,0,0,0,0,0,0x02020202},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0,0x10101212,0,0,0,0,0,0x02020202},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "add ax,[ecx]",
		.code = "\x66\x03\x01",
		.codesize = 3,
		.in_state.reg  = {0x11112222,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x44443333},
		.out_state.reg  = {0x11115555,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x44443333},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "add ecx,edi",
		.code = "\x03\xcf",	/* add ecx,edi */
		.codesize = 2,
		.in_state.reg  = {0,0x10101010,0,0,0,0,0,0x02020202},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0,0x12121212,0,0,0,0,0,0x02020202},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "add eax,[ecx]",
		.code = "\x03\x01",
		.codesize = 2,
		.in_state.reg  = {0x11112222,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x44443333},
		.out_state.reg  = {0x55555555,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x44443333},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "add ecx,[ebx+eax*4+0xdeadbeef]",
		.code = "\x03\x8c\x83\xef\xbe\xad\xde",
		.codesize = 7,
		.in_state.reg  = {0x2,0x1,0,0x1,0,0,0,0},
		.in_state.mem_state = {0xdeadbef8, 0x44443333},
		.out_state.reg  = {0x2,0x44443334,0,0x1,0,0,0,0},
		.out_state.mem_state = {0xdeadbef8, 0x44443333},
	},
	/* 04 */
	{
		.instr = "add al,0x11",
		.code = "\x04\x11",
		.codesize = 2,
		.in_state.reg  = {0x22222222,0,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x22222233,0,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},
	/* 05 */
	{
		.instr = "add ax,0x1111",
		.code = "\x66\x05\x11\x11",
		.codesize = 4,
		.in_state.reg  = {0x22222222,0,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x22223333,0,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "add eax,0x11111111",
		.code = "\x05\x11\x11\x11\x11",
		.codesize = 5,
		.in_state.reg  = {0x22222222,0,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x33333333,0,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},

	





	/* 08 */
	{
		.instr = "or ah,al",
//		.code = "\x00\xc4",
//		.codesize = 2,
		.in_state.reg  = {0xff01,0,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x0ff01,0,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags = FLAG(f_sf) | FLAG(f_pf),
	},
	{
		.instr = "or ch,dl",
//		.code = "\x00\xd5",
//		.codesize = 2,
		.in_state.reg  = {0,0x1000,0x20,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0,0x3000,0x20,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "or [ecx],al",
//		.code = "\x00\x01",
//		.codesize = 2,
		.in_state.reg  = {0x10,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x10101010},
		.out_state.reg  = {0x10,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x10101010},
	},
	/* 09 */
	{
		.instr = "or ax,cx",
//		.code = "\x66\x01\xc8",
//		.codesize = 3,
		.in_state.reg  = {0xffff1111,0xffff2222,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0xffff3333,0xffff2222,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf), 
	},
	{
		.instr = "or [ecx],ax",
//		.code = "\x66\x01\x01",
//		.codesize = 3,
		.in_state.reg  = {0xffff1111,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x22224444},
		.out_state.reg  = {0xffff1111,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x22225555},
		.out_state.eflags =  FLAG(f_pf), 
	},
	{
		.instr = "or eax,ecx",
//		.code = "\x01\xc8",
//		.codesize = 2,
		.in_state.reg  = {0x11112222,0x22221111,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x33333333,0x22221111,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf), 
	},
	{
		.instr = "or [ecx],eax",
//		.code = "\x01\x01",
//		.codesize = 2,
		.in_state.reg  = {0x22221111,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x22224444},
		.out_state.reg  = {0x22221111,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x22225555},
		.out_state.eflags =  FLAG(f_pf), 
	},
	/* 0a */
	{
		.instr = "or cl,bh",
//		.code = "\x02\xcf",	/* or cl,bh */
//		.codesize = 2,
		.in_state.reg  = {0,0xff,0,0x100,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0,0xff,0,0x100,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf) | FLAG(f_sf), 
	},
	{
		.instr = "or al,[ecx]",
//		.code = "\x02\x01",
//		.codesize = 2,
		.in_state.reg  = {0x3,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x30303030},
		.out_state.reg  = {0x33,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x30303030},
		.out_state.eflags =  FLAG(f_pf),
	},
	/* 0b */
	{
		.instr = "or cx,di",
//		.code = "\x66\x03\xcf",	/* or cx,di */
//		.codesize = 3,
		.in_state.reg  = {0,0x10101010,0,0,0,0,0,0x02020202},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0,0x10101212,0,0,0,0,0,0x02020202},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "or ax,[ecx]",
//		.code = "\x66\x03\x01",
//		.codesize = 3,
		.in_state.reg  = {0x11112222,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x44443333},
		.out_state.reg  = {0x11113333,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x44443333},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "or ecx,edi",
//		.code = "\x03\xcf",	/* or ecx,edi */
//		.codesize = 2,
		.in_state.reg  = {0,0x10101010,0,0,0,0,0,0x02020202},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0,0x12121212,0,0,0,0,0,0x02020202},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "or eax,[ecx]",
//		.code = "\x03\x01",
//		.codesize = 2,
		.in_state.reg  = {0x11112222,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x44443333},
		.out_state.reg  = {0x55553333,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x44443333},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "or ecx,[ebx+eax*4+0xdeadbeef]",
//		.code = "\x03\x8c\x83\xef\xbe\xad\xde",
//		.codesize = 7,
		.in_state.reg  = {0x2,0x1,0,0x1,0,0,0,0},
		.in_state.mem_state = {0xdeadbef8, 0x44443333},
		.out_state.reg  = {0x2,0x44443333,0,0x1,0,0,0,0},
		.out_state.mem_state = {0xdeadbef8, 0x44443333},
		.out_state.eflags =  FLAG(f_pf),
	},
	/* 0c */
	{
		.instr = "or al,0x11",
//		.code = "\x04\x11",
//		.codesize = 2,
		.in_state.reg  = {0x22222222,0,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x22222233,0,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},
	/* 0d */
	{
		.instr = "or ax,0x1111",
//		.code = "\x66\x05\x11\x11",
//		.codesize = 4,
		.in_state.reg  = {0x22222222,0,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x22223333,0,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "or eax,0x11111111",
//		.code = "\x05\x11\x11\x11\x11",
//		.codesize = 5,
		.in_state.reg  = {0x22222222,0,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x33333333,0,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},









    /* 10 */
	{
		.instr = "adc ah,al",
//		.code = "\x00\xc4",
//		.codesize = 2,
		.in_state.reg  = {0xff01,0,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0x101,0,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags = FLAG(f_cf) ,
	},
	{
		.instr = "adc ch,dl",
//		.code = "\x00\xd5",
//		.codesize = 2,
		.in_state.reg  = {0,0x1000,0x20,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0,0x3100,0x20,0,0,0,0,0},
		.out_state.mem_state = {0, 0},

	},
	{
		.instr = "adc [ecx],al",
//		.code = "\x00\x01",
//		.codesize = 2,
		.in_state.reg  = {0x10,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x10101010},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0x10,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x10101021},
		.out_state.eflags = FLAG(f_pf) ,
	},
	/* 11 */
	{
		.instr = "adc ax,cx",
//		.code = "\x66\x01\xc8",
//		.codesize = 3,
		.in_state.reg  = {0xffff1111,0xffff2222,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0xffff3334,0xffff2222,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
//		.out_state.eflags =  FLAG_SET(f_pf), 
	},
	{
		.instr = "adc [ecx],ax",
//		.code = "\x66\x01\x01",
//		.codesize = 3,
		.in_state.reg  = {0xffff1111,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x22224444},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0xffff1111,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x22225556},
		.out_state.eflags =  FLAG(f_pf), 
	},
	{
		.instr = "adc eax,ecx",
//		.code = "\x01\xc8",
//		.codesize = 2,
		.in_state.reg  = {0x11112222,0x22221111,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0x33333334,0x22221111,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
//		.out_state.eflags =  FLAG_SET(f_pf), 
	},
	{
		.instr = "adc [ecx],eax",
//		.code = "\x01\x01",
//		.codesize = 2,
		.in_state.reg  = {0x22221111,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x22224444},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0x22221111,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x44445556},
		.out_state.eflags =  FLAG(f_pf), 
	},
	/* 12 */
	{
		.instr = "adc cl,bh",
//		.code = "\x02\xcf",	/* adc cl,bh */
//		.codesize = 2,
		.in_state.reg  = {0,0xff,0,0x100,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0,0x1,0,0x100,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_cf), 
	},
	{
		.instr = "adc al,[ecx]",
//		.code = "\x02\x01",
//		.codesize = 2,
		.in_state.reg  = {0x3,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x30303030},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0x34,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x30303030},

	},
	/* 13 */
	{
		.instr = "adc cx,di",
//		.code = "\x66\x03\xcf",	/* adc cx,di */
//		.codesize = 3,
		.in_state.reg  = {0,0x10101010,0,0,0,0,0,0x02020202},
		.in_state.mem_state = {0, 0},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0,0x10101213,0,0,0,0,0,0x02020202},
		.out_state.mem_state = {0, 0},

	},
	{
		.instr = "adc ax,[ecx]",
//		.code = "\x66\x03\x01",
//		.codesize = 3,
		.in_state.reg  = {0x11112222,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x44443333},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0x11115556,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x44443333},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "adc ecx,edi",
//		.code = "\x03\xcf",	/* adc ecx,edi */
//		.codesize = 2,
		.in_state.reg  = {0,0x10101010,0,0,0,0,0,0x02020202},
		.in_state.mem_state = {0, 0},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0,0x12121213,0,0,0,0,0,0x02020202},
		.out_state.mem_state = {0, 0},
//		.out_state.eflags =  FLAG_SET(f_pf),
	},
	{
		.instr = "adc eax,[ecx]",
//		.code = "\x03\x01",
//		.codesize = 2,
		.in_state.reg  = {0x11112222,0x40000,0,0,0,0,0,0},
		.in_state.mem_state = {0x40000, 0x44443333},
		.in_state.eflags = FLAG(f_cf),
		.out_state.reg  = {0x55555556,0x40000,0,0,0,0,0,0},
		.out_state.mem_state = {0x40000, 0x44443333},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "adc ecx,[ebx+eax*4+0xdeadbeef]",
//		.code = "\x03\x8c\x83\xef\xbe\xad\xde",
//		.codesize = 7,
		.in_state.reg  = {0x2,0x1,0,0x1,0,0,0,0},
		.in_state.mem_state = {0xdeadbef8, 0x44443333},
		.out_state.reg  = {0x2,0x44443334,0,0x1,0,0,0,0},
		.out_state.mem_state = {0xdeadbef8, 0x44443333},
	},
	/* 14 */
	{
		.instr = "adc al,0x11",
//		.code = "\x04\x11",
//		.codesize = 2,
		.in_state.reg  = {0x22222222,0,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x22222233,0,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},
	/* 15 */
	{
		.instr = "adc ax,0x1111",
//		.code = "\x66\x05\x11\x11",
//		.codesize = 4,
		.in_state.reg  = {0x22222222,0,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x22223333,0,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},
	{
		.instr = "adc eax,0x11111111",
//		.code = "\x05\x11\x11\x11\x11",
//		.codesize = 5,
		.in_state.reg  = {0x22222222,0,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0x33333333,0,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eflags =  FLAG(f_pf),
	},




	{
		.instr = "jmp ecx",
		.in_state.reg  = {0,0xdeadbeef,0,0,0,0,0,0},
		.in_state.mem_state = {0, 0},
		.out_state.reg  = {0,0xdeadbeef,0,0,0,0,0,0},
		.out_state.mem_state = {0, 0},
		.out_state.eip = 0xdeadbeef,
	},
	{
		.instr = "jmp [eax]",
		.in_state.reg  = {0xdeadbabe,0,0,0,0,0,0,0},
		.in_state.mem_state = {0xdeadbabe, 0xdeafcafe},
		.out_state.reg  = {0xdeadbabe,0,0,0,0,0,0,0},
		.out_state.mem_state = {0xdeadbabe, 0xdeafcafe},
		.out_state.eip = 0xdeafcafe,
	},
	{
		.code = "\xeb\x10", /* jmp +16*/
		.codesize = 2,
		.out_state.eip = (CODE_OFFSET + 2 + 0x10),
	},
	{
		.code = "\xeb\xff", /* jmp -1 */
		.codesize = 2,
		.out_state.eip = (CODE_OFFSET + 2 + -1),
	},
	{
		.code = "\xe9\x00\x00\x00\x01", /* jmp +0x01000000 */
		.codesize = 5,
		.out_state.eip = (CODE_OFFSET + 5 + 0x1000000),
	},
	
	{
		.instr = "mov al, 0xff",
		.out_state.reg = {0xff,0,0,0,0,0,0,0},
	},
	{
		.instr = "mov ah, 0xff",
		.out_state.reg = {0xff00,0,0,0,0,0,0,0},
	},
	{
		.instr = "mov ax, 0xffff",
		.out_state.reg = {0xffff,0,0,0,0,0,0,0},
	},
	{
		.instr = "mov eax, 0xffffffff",
		.out_state.reg = {0xffffffff,0,0,0,0,0,0,0},
	},
};

int prepare()
{
	int i;
	for (i=0;i<sizeof(tests)/sizeof(struct instr_test);i++)
	{
		if ( opts.nasm_force == 0 && tests[i].code != NULL )
		{ // dup it so we can free it
			char *c = (char *)malloc(tests[i].codesize);
			memcpy(c,tests[i].code,tests[i].codesize);
			tests[i].code = c;
		} else
		{
			const char *use = "USE32\n";
			FILE *f=fopen("/tmp/foo.S","w+");

			if (f == NULL)
			{
				printf("failed to create asm file for nasm instruction %s\n\n\t%s",tests[i].instr,strerror(errno));
				return -1;
			}

			fwrite(use,strlen(use),1,f);
			fwrite(tests[i].instr,1,strlen(tests[i].instr),f);
			fclose(f);
			system("cd /tmp/; nasm foo.S");
			f=fopen("/tmp/foo","r");
			if (f == NULL)
			{
				printf("failed to open compiled nasm file for read for instruction %s\n\n\t%s",tests[i].instr,strerror(errno));
				return -1;
			}

			fseek(f,0,SEEK_END);

			tests[i].codesize = ftell(f);
			tests[i].code = malloc(tests[i].codesize);
			fseek(f,0,SEEK_SET);
			fread(tests[i].code,1,tests[i].codesize,f);
			fclose(f);

			unlink("/tmp/foo.S");
			unlink("/tmp/foo");
		}
	}
	return 0;
}


int test()
{
	int i=0;
	struct emu *e = emu_new();
	struct emu_cpu *cpu = emu_cpu_get(e);
	struct emu_memory *mem = emu_memory_get(e);

	for (i=0;i<sizeof(tests)/sizeof(struct instr_test);i++)
	{
		int failed = 0;


		printf("testing '%s' \t",tests[i].instr);
		int j=0;

		if ( opts.verbose == 1 )
		{
			printf("code '");
			for ( j=0;j<tests[i].codesize;j++ )
			{
				printf("%02x ",(uint8_t)tests[i].code[j]);
			}
			printf("' ");
		}


		/* set the registers to the initial values */
		for ( j=0;j<8;j++ )
		{
			emu_cpu_reg32_set(cpu,j ,tests[i].in_state.reg[j]);
		}
   	

		/* set the flags */
		emu_cpu_eflags_set(cpu,tests[i].in_state.eflags);


		/* write the code to the offset */
		int static_offset = CODE_OFFSET;
		for( j = 0; j < tests[i].codesize; j++ )
		{
			emu_memory_write_byte(mem, static_offset+j, tests[i].code[j]);
		}

		if (tests[i].in_state.mem_state[0] != 0 && tests[i].in_state.mem_state[1] != 0)
			emu_memory_write_dword(mem, tests[i].in_state.mem_state[0], tests[i].in_state.mem_state[1]);

		if (opts.verbose)
		{
			printf("memory at 0x%08x = 0x%08x (%i %i)\n",
				   tests[i].in_state.mem_state[0], 
				   tests[i].in_state.mem_state[1],
				   (int)tests[i].in_state.mem_state[1],
				   (uint32_t)tests[i].in_state.mem_state[1]);
		}


		/* set eip to the code */
		emu_cpu_eip_set(emu_cpu_get(e), static_offset);

		/* run the code */
		if (opts.verbose == 1 )
		{
        	emu_log_level_set(emu_logging_get(e),EMU_LOG_DEBUG);
			emu_cpu_debug_print(cpu);
			emu_log_level_set(emu_logging_get(e),EMU_LOG_NONE);
		}
		
		int ret;
		while ((ret = emu_cpu_run(emu_cpu_get(e))) == 0)
			if (opts.verbose == 1)
			{
				emu_log_level_set(emu_logging_get(e),EMU_LOG_DEBUG);
				emu_cpu_debug_print(cpu);
				emu_log_level_set(emu_logging_get(e),EMU_LOG_NONE);
			}


		if ( ret != 0 )
		{
			printf("cpu error %s\n", emu_strerror(e));
		}
   

		if (opts.verbose == 1)
		{
			emu_log_level_set(emu_logging_get(e),EMU_LOG_DEBUG);
			emu_cpu_debug_print(cpu);
			emu_log_level_set(emu_logging_get(e),EMU_LOG_NONE);
		}
        	

		/* check the registers for the exptected values */

		for ( j=0;j<8;j++ )
		{
			if ( emu_cpu_reg32_get(cpu, j) ==  tests[i].out_state.reg[j] )
			{
				if (opts.verbose == 1)
					printf("\t %s "SUCCESS"\n",regm[j]);
			} else
			{
				printf("\t %s "FAILED" got 0x%08x expected 0x%08x\n",regm[j],emu_cpu_reg32_get(cpu, j),tests[i].out_state.reg[j]);
				failed = 1;
			}
		}


		/* check the memory for expected values */
		uint32_t value;

		if ( tests[i].out_state.mem_state[0] != -1 )
		{
			if ( emu_memory_read_dword(mem,tests[i].out_state.mem_state[0],&value) == 0 )
			{
				if ( value == tests[i].out_state.mem_state[1] )
				{
					if (opts.verbose == 1)
						printf("\t memory "SUCCESS" 0x%08x = 0x%08x\n",tests[i].out_state.mem_state[0], tests[i].out_state.mem_state[1]);
				}
				else
				{
					printf("\t memory "FAILED" at 0x%08x got 0x%08x expected 0x%08x\n",tests[i].out_state.mem_state[0],value, tests[i].out_state.mem_state[1]);
					failed = 1;
				}

			} else
			{
				printf("\tmemory "FAILED" emu says: '%s' when accessing %08x\n", strerror(emu_errno(e)),tests[i].out_state.mem_state[0]);
				failed = 1;
			}

		}

		/* check the cpu flags for expected values */
		if ( tests[i].out_state.eflags != emu_cpu_eflags_get(cpu) )
		{
			printf("\t flags "FAILED" got %08x expected %08x\n",emu_cpu_eflags_get(cpu),tests[i].out_state.eflags);
			for(j=0;j<32;j++)
			{
				uint32_t f = emu_cpu_eflags_get(cpu);
				if ( (tests[i].out_state.eflags & (1 << j)) != (f & (1 <<j)))
					printf("\t flag %s (bit %i) failed, expected %i is %i\n",flags[j], j, 
						   (tests[i].out_state.eflags & (1 << j)),
						   (f & (1 <<j)));
			}

			failed = 1;
		}else
		{
			if (opts.verbose == 1)
				printf("\t flags "SUCCESS"\n");
		}

		
		if( tests[i].out_state.eip != 0 && tests[i].out_state.eip != emu_cpu_eip_get(cpu) )
		{
			printf("\t %s "FAILED" got 0x%08x expected 0x%08x\n", "eip", emu_cpu_eip_get(cpu), tests[i].out_state.eip);
			failed = 1;
		}


		/* bail out on *any* error */
		if (failed == 0)
		{
			printf(SUCCESS"\n");
		}else
		{
			return -1;
		}
		
	}
	emu_free(e);
	return 0;
}

void cleanup()
{
	int i;
	for (i=0;i<sizeof(tests)/sizeof(struct instr_test);i++)
    	if (tests[i].code != NULL)
    		free(tests[i].code);
		
}

int main(int argc, char *argv[])
{
	memset(&opts,0,sizeof(struct run_time_options));

	while ( 1 )
	{	
		int c;
		int option_index = 0;
		static struct option long_options[] = {
			{"verbose"			, 0, 0, 'v'},
			{"nasm-force"		, 0, 0, 'n'},
			{0, 0, 0, 0}
		};

		c = getopt_long (argc, argv, "vn", long_options, &option_index);
		if ( c == -1 )
			break;

		switch ( c )
		{
		case 'v':
			opts.verbose = 1;
			break;

		case 'n':
			opts.nasm_force = 1;
			break;


		default:
			printf ("?? getopt returned character code 0%o ??\n", c);
			break;
		}
	}



	if ( prepare() != 0)
		return -1;

	if ( test() != 0 )
		return -1;

	cleanup();

	return 0;
}
