// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

// SPDX-License-Identifier: MIT

/*
 * Copyright (c) 2026 Gary Milliorn
 * All rights reserved.
 */


//--------------------------------------------------------------------------------
//	MAIN -- MAIN 
//
//		startup/main loop/shutdown
//
//--------------------------------------------------------------------------------


#include "ppcsim.h"


//--------------------------------------------------------------------------------
// PPCSIM -- struct holding simulator controls.
//--------------------------------------------------------------------------------

PPCSIM ppcsimctl;


//--------------------------------------------------------------------------------
// ppcsim_init -- initialze system.
//
PPCSIM *ppcsim_init( PPCSIM *ppcsim )
{
	int i;

	ppcsim->headless			= 0;
	ppcsim->dink_mode			= 0;

	ppcsim->process_type		= CPU_MPC603;
	ppcsim->CPU_name[0]			= 0;
	ppcsim->CPU_override[0]		= 0;
	ppcsim->CPU_class			= 0;
	ppcsim->CPU_pll				= 0;
	ppcsim->CPU_pll_bits		= 0;

	ppcsim->system				= NULL;

	ppcsim->mss_ignore_undef	= 1;
	ppcsim->mss_logging			= 0;

	ppcsim->uart_io_coloring	= 1;
	ppcsim->uart_io_color   	= 208;
	ppcsim->uart_io_delkey		= 1;

	ppcsim->symbols				= NULL;
	ppcsim->sym_head.name		= NULL;
	ppcsim->sym_mask			= 0x000FFFFF;

	ppcsim->flowtrace			= 0;
	ppcsim->dec_tick			= 0;
	ppcsim->dec_limit			= 0;
	ppcsim->btsp				= -1;
	ppcsim->max_stack			= 0;
	ppcsim->max_R1				= 0;

	ppcsim->issued				= 0;
	ppcsim->verbose				= 0;

	ppcsim->rsvd_addr			= 0;
	ppcsim->reserve_valid		= 0;

	BRK_init( ppcsim );
	ppcsim->brkpt_hwmodel		= 1;
	ppcsim->brkpt_bypass		= 0;

	ppcsim->exc_type			= 0;
	ppcsim->exc_addr			= 0;

	ppcsim->show_reg_delta		= 0;
	ppcsim->tr_cmd[0]			= 0;
	ppcsim->dot_cmd[0]			= 0;

	return( ppcsim );
}


//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
int main( int argc, char *argv[] ) 
{
	PPCSIM *ppcsim;
	int		 stat, opt;
	char	*startup, *prompt;
	char	 cmdline[ BUFSIZ ];
	char	 script[  BUFSIZ ];


	ppcsim = ppcsim_init( &ppcsimctl );

	shell_init();
	SYM_init( ppcsim );


	printf("  ppcsim: PowerPC Simulator V%d.%d, Copyright 1999+, 2026\n"
		   "          by Motorola Inc and Gary Milliorn.  See LICENSE.\n",
			VER_MAJOR,
			VER_MINOR );


// Get arguments in advance of target/memory setup.  Some settings/setup
// are needed before others, so mostly just connect now.
//
	optind	 = 0;		// POSIX
	startup	 = NULL;
	while ((opt = getopt( argc, argv, "c:dhf:s:t:v" )) != -1) {
		switch (opt) {
		case 'c':	strcpy( ppcsim->CPU_override, optarg );
					break;
		case 'd':	ppcsim->dink_mode = 1;
					break;
		case 'f':	sprintf( script, "fl %s", optarg );
					startup = script;
					break;
		case 'h':	ppcsim->headless = 1;
					break;
		case 's':	sprintf( script, "do %s", optarg );
					startup = script;
					break;
		case 'v':	ppcsim->verbose++;
					break;
		default:	fprintf(stderr, "usage: ppcsim [-c cpu][-f file][-h][-s scr][-t tgt][-v]\n"
									"       -c s - force CPU to emulate, one of:\n"
									"                MPC601,\n"
									"                MPC603, MPC603e, MPC603ev,\n"
									"                MPC604, MPC604e,\n"
									"                MPC740, MPC745, MPC750, MPC755,\n"
									"                MPC7400, MPC7410\n"
									"                MPC7440, MPC7450,\n"
									"                MPC8240, MPC8245\n"
									"              (overrides all other CPU selection)\n"
									"       -f s - load file to memory\n"
									"       -h   - set headless mode\n"
									"       -s s - run script file\n"
									"       -v   - verbose\n"
									"\n"
						   );
					exit( 1 );
		}
	}


// Setup for CPU/BOARD/IO specific options.
//
	CPU_init( ppcsim );
	SYS_init( ppcsim );
	NB_init( ppcsim );
	MSS_init( ppcsim );
	UART_init( ppcsim );
	DASM_init( ppcsim );

	if (ppcsim->dink_mode) {
		NB_ENV_preset( ppcsim, 1 );
	}


// Interpret commands until done.
//
	prompt = "ppcsim> ";
    for (stat = 0; stat >= 0; ) {

		// If startup script was specified, run it now.
		//
		if (startup)
			strcpy( cmdline, startup );
		else
			stat = shell_read_line( cmdline, prompt );
		if (stat < 0)
			break;

        stat = shell_run_command( ppcsim, cmdline );
		if (stat > 0)
			printf("  ppcsim: error: %d\n", stat);

		startup = NULL;
    }


// Cleanup and exit.
//
	DASM_deinit( ppcsim );
	UART_deinit( ppcsim );
	MSS_deinit( ppcsim );
	NB_deinit( ppcsim );
	SYS_deinit( ppcsim );
	CPU_deinit( ppcsim );
	SYM_deinit( ppcsim );
	shell_deinit( ppcsim );

    return( 0 );
}
