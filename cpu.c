// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

// SPDX-License-Identifier: MIT

/*
 * Copyright (c) 2026 Gary Milliorn
 * All rights reserved.
 */


//--------------------------------------------------------------------------------
//	CPU -- CPU 
//
//		functions to manage CPU selection.
//--------------------------------------------------------------------------------


#include "ppcsim.h"


//================================================================================
//								CPU INIT
//================================================================================


// DB used for selection and initialization.
//
typedef struct vcpu_db_st {
	int		device;
	int		class;
	char	name[12];

	ULONG	PVR;
	ULONG	SVR;
	ULONG	HID0;
	ULONG	HID1;
	char	nick[20];
	int		PLL_msb, PLL_bits;
} CPU_DB;



// Marker for info needed:
//
#define NEED_HID	0x00000000


CPU_DB cpu_list[] = {
//																has
//	ID			  CLASS			NAME		PVR			SVR		HID0		HID1		Nickname		PLL
{	CPU_MPC601,   CCLS_601,		"MPC601",	PVR_601,	0x0100,	NEED_HID,	0x00000000,	"",				0, 0	},
{	CPU_MPC603,   CCLS_603,		"MPC603",	PVR_603,	0x0100,	NEED_HID,	0x00000000,	"",				0, 0	},
{	CPU_MPC603e,  CCLS_603,		"MPC603e",	PVR_603e,	0x0000,	0x0010C000,	NEED_HID,	"",				0, 0	},
{	CPU_MPC603ev, CCLS_603,		"MPC603ev",	PVR_603ev,	0x0000,	NEED_HID,	NEED_HID,	"",				0, 0	},
{	CPU_MPC604,   CCLS_604,		"MPC604",	PVR_604,	0x0100,	NEED_HID,	NEED_HID,	"Sirocco",		0, 0	},
{	CPU_MPC604e,  CCLS_604,		"MPC604e",	PVR_604e,	0x0202,	NEED_HID,	NEED_HID,	"",				0, 0	},
{	CPU_MPC750,	  CCLS_750,		"MPC750",	PVR_750,	0x0100,	0x0010C1A4,	0x00000000,	"Arthur",		0, 4	},
{	CPU_MPC740,	  CCLS_750,		"MPC740",	PVR_750,	0x0100,	0x0010C1A4,	0x00000000,	"Arthur",		0, 4	},
{	CPU_MPC755,	  CCLS_750,		"MPC755",	PVR_755,	0x3203,	0x0010C1A4,	0x00000000,	"Goldfinger",	0, 4	},
{	CPU_MPC745,	  CCLS_750,		"MPC745",	PVR_755,	0x3203,	0x0010C1A4,	0x00000000,	"Goldfinger",	0, 4	},
{	CPU_MPC7400,  CCLS_7400,	"MPC7400",	PVR_7400,	0x0100,	0x0010C0A4,	NEED_HID,	"Max",			0, 0	},
{	CPU_MPC7410,  CCLS_7400,	"MPC7410",	PVR_7410,	0x0100,	0x0010C0A4,	NEED_HID,	"Max",			0, 0	},
{	CPU_MPC7450,  CCLS_7450,	"MPC7450",	PVR_7450,	0x0100,	0x0410C0BC,	NEED_HID,	"Vger",		   15, 5	},
{	CPU_MPC7455,  CCLS_7450,	"MPC7455",	PVR_7455,	0x0100,	0x0410C1BC,	NEED_HID,	"Vger",		   15, 5	},
{	CPU_MPC7447A, CCLS_7450,	"MPC7447",	PVR_7447A,	0x0100,	0x0410C19C,	NEED_HID,	"Vger",		   15, 5	},
{	CPU_MPC7448,  CCLS_7450,	"MPC7448",	PVR_7448,	0x0100,	NEED_HID,	NEED_HID,	"",			   15, 5	},
{	CPU_MPC8420,  CCLS_603,		"MPC8240",	PVR_8240,	0x0000,	0x0010C000,	NEED_HID,	"Kahlua",		0, 0	},
{	CPU_MPC8425,  CCLS_603,		"MPC8245",	PVR_8245,	0x0000,	0x0010C000,	NEED_HID,	"Kahlua II",	0, 0	}
};


//--------------------------------------------------------------------------------
// CPU_reset -- set SPR registers to values based on selected CPU.
//--------------------------------------------------------------------------------
int CPU_reset( PPCSIM *ppcsim )
{
	int   i;
	ULONG v;

	for (i = 0; i < sizeof(cpu_list)/sizeof(CPU_DB); i++) {
		if (cpu_list[i].device == ppcsim->process_type) {

			v = ((cpu_list[i].PVR) << 16) | cpu_list[i].SVR;
			SPR_set( SPR_PVR, v );
	
			v = cpu_list[i].HID1;
			if (ppcsim->CPU_pll)
				v = v | ppcsim->CPU_pll << ((32 - ppcsim->CPU_pll_msb) - ppcsim->CPU_pll_bits);
			SPR_set( SPR_HID1, v );
		}
	}


// These registers are common across devices.
//
	SPR_set( SPR_MSR,	0x00000040 );
	SPR_set( SPR_FPSCR,	0x00000000 );
	SPR_set( SPR_XER,	0x00000000 );
	SPR_set( SPR_TBU,	0x00000000 );
	SPR_set( SPR_TBL,	0x00000000 );
	SPR_set( SPR_LR,	0x00000000 );
	SPR_set( SPR_CTR,	0x00000000 );
	SPR_set( SPR_DEC,	0xFFFFFFFF );
	SPR_set( SPR_IABR,	0x00000000 );

	return( 0 );
}


//--------------------------------------------------------------------------------
// CPU_set -- set CPU to desired type.
//--------------------------------------------------------------------------------
int CPU_set( PPCSIM *ppcsim, char *cpu, int pll_set, int verbose )
{
	int i;

	for (i = 0; i < sizeof(cpu_list)/sizeof(CPU_DB); i++) {
		if (strcasecmp( cpu, cpu_list[i].name ) == 0) {

			ppcsim->process_type = cpu_list[i].device;
			strncpy( ppcsim->CPU_name, cpu_list[i].name, 16 );

			ppcsim->CPU_class = cpu_list[i].class;

			if (verbose)
				printf("  ppcsim.SYS: CPU is '%s'.\n", cpu);

			// If PLL_bits > 0, it describes which bits of HID starting at MSB
			// describe the PLL settings.
			//
			if (cpu_list[i].PLL_bits  &&  pll_set) {
				ppcsim->CPU_pll		= pll_set;
				ppcsim->CPU_pll_bits	= cpu_list[i].PLL_bits;
				ppcsim->CPU_pll_msb	= cpu_list[i].PLL_msb;

				if (ppcsim->verbose)
					printf("  ppcsim.SYS: HID1[0:%d] <= %d.\n", ppcsim->CPU_pll_bits-1, ppcsim->CPU_pll);
			}

			CPU_reset( ppcsim );

			return( 0 );
		}
	}

	if (verbose)
		printf("  ppcsim.SYS: unknown CPU '%s'\n", cpu);

	return( ERR_UNKNOWN );
}


//--------------------------------------------------------------------------------
// CPU_list -- list all CPUs.
//--------------------------------------------------------------------------------
int CPU_list( PPCSIM *ppcsim )
{
	int  i;
	char rterm[20];

	printf("    CPU       Nickname      PVR  SVR\n"
		   "    ========  ============  =========\n");
	for (i = 0; i < sizeof(cpu_list)/sizeof(CPU_DB); i++) {
		rterm[0] = 0;
		if (cpu_list[i].nick)
			sprintf(rterm, "\"%s\"", cpu_list[i].nick);

		printf(" %c  %-8s  %-12s  %04hX_%04hX\n", 
			ppcsim->process_type == cpu_list[i].device ? '>' : ' ',
			cpu_list[i].name, rterm,
			cpu_list[i].PVR,
			cpu_list[i].SVR
			);
	}

	return( 0 );
}


//================================================================================
//================================================================================


char *cpu_help[] = {
	"  usage: cpu [-c s][-l][-p n]",
	"  where:",
	"          -l     - list CPUs available.",
	"          -p n   - set PLL multiplier to 'N' (if supported).",
	"          -s s   - set CPU model to use.",
	NULL
};


//--------------------------------------------------------------------------------
// CPU_cmd -- CPU management commands.
//--------------------------------------------------------------------------------
int CPU_cmd( PPCSIM *ppcsim, int argc, char *argv[] )
{
	int    opt, i, stat;
	char   dopt, do_list, do_help, do_set;
	char  *r, cpu_name[40];


// Collect arguments.
	
	optind		= 0;		// POSIX
	do_list		= 0;
	do_set		= 0;
	do_help		= 0;
	strcpy( cpu_name, ppcsim->CPU_name );

	if (argc > 1  &&  strcmp( argv[1], "--help" ) == 0)
		do_help++;
	else while ((opt = getopt( argc, argv, "lp:s:" )) != -1) {
		switch (opt) {
		case 'l':	do_list = 1;						break;

		case 'p':	i = ppcsim->CPU_pll = atoi( optarg );
					if (0 <= i  &&  i <= 128) {
						ppcsim->CPU_pll = i;
						do_set = 1;
					}
					break;

		case 's':	strcpy(cpu_name,optarg);
					do_set = 1;
					break;

		default:	return( ERR_UNKNOWN );
		}
	}


// --help or errors.
//
	if (do_help)
		return( shell_show_help( cpu_help ) );


// -c s		-- set CPU.  
//			   if <CPU_override> was set, always (silently) use it.  This allows
//			   minor alterations of existing setup scripts.
//
	if (do_set) {
		if (*ppcsim->CPU_override) {
			printf("  ppcsim.CPU : CPU override '%s'\n", ppcsim->CPU_override);
			return( 0 );
		}

		return( CPU_set( ppcsim, cpu_name, ppcsim->CPU_pll, 1 ) );
	}


// -l		-- list known cpus
//
	if (do_list)
		return( CPU_list( ppcsim ) );

	return( ERR_INVARG );
}


//--------------------------------------------------------------------------------
// CPU_init -- initialize CPU.
//--------------------------------------------------------------------------------
int CPU_init( PPCSIM *ppcsim )
{

	return( 0 );
}


//--------------------------------------------------------------------------------
// CPU_deinit --
//--------------------------------------------------------------------------------
int CPU_deinit( PPCSIM *ppcsim )
{

	return( 0 );
}
