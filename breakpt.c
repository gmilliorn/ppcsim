// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

// SPDX-License-Identifier: MIT

/*
 * Copyright (c) 2026 Gary Milliorn
 * All rights reserved.
 */


//--------------------------------------------------------------------------------
//	BREAKPT -- BREAKPOINTS
//
//		functions to create/delete simulations breakpoints.
//
//--------------------------------------------------------------------------------


#include "ppcsim.h"


//--------------------------------------------------------------------------------
// BRK_init -- initialize breakpoints.
//--------------------------------------------------------------------------------
int BRK_init( PPCSIM *ppcsim )
{
	int i;

	for (i = 0; i < MAX_BRKPTS; i++)
		ppcsim->brkpt[i].valid = 0;
	return( 0 );
}


//--------------------------------------------------------------------------------
// BRK_list -- list breakpoints.
//--------------------------------------------------------------------------------
int BRK_list( PPCSIM *ppcsim )
{
	int   i;
	char *s;

	printf("  NO  ADDRESS   SYS  DATA  COUNT  NAME\n"
		   "  ===========================================\n");
	for (i = 0; i < MAX_BRKPTS; i++) {
		if (ppcsim->brkpt[i].valid) {
			s = symbol_get_byaddr( ppcsim, ppcsim->brkpt[i].addr );
			printf("  %-2d  %08X  %-3s %-3s    %5d  %-s\n", 
				i,
				ppcsim->brkpt[i].addr, 
				ppcsim->brkpt[i].sys ?	 " Y " : "   ",
				ppcsim->brkpt[i].is_data ? " Y " : "   ",
				ppcsim->brkpt[i].count,
				s == NULL ? "" : s );
		}
	}
	return( 0 );
}


//--------------------------------------------------------------------------------
// BRK_set -- set a breakpoint.
//--------------------------------------------------------------------------------
int BRK_set( PPCSIM *ppcsim, ULONG addr, int count, int sys )
{
	int i;

	for (i = 0; i < MAX_BRKPTS; i++) {
		if (ppcsim->brkpt[i].valid == 0) {
			ppcsim->brkpt[i].valid	= 1;
			ppcsim->brkpt[i].addr		= addr & 0xFFFFFFFC;		// force alignment
			ppcsim->brkpt[i].sys		= sys;
			ppcsim->brkpt[i].is_data	= 0;
			ppcsim->brkpt[i].count	= 0;
			return( 0 );
		}
	}

	return( ERR_LIMITS );
}


//--------------------------------------------------------------------------------
// BRK_del -- delete a breakpoint by index.
//--------------------------------------------------------------------------------
int BRK_del( PPCSIM *ppcsim, ULONG idx )
{

	if (idx < 0  ||  idx >= MAX_BRKPTS)
		return( ERR_BADVAL );

	ppcsim->brkpt[idx].valid	= 0;

	return( 0 );
}


//--------------------------------------------------------------------------------
// BRK_del_addr -- delete a breakpoint by addr.
//--------------------------------------------------------------------------------
int BRK_del_addr( PPCSIM *ppcsim, ULONG addr, int sys )
{
	int i;

	for (i = 0; i < MAX_BRKPTS; i++) {
		if (ppcsim->brkpt[i].valid == 0)
			continue;
		if (ppcsim->brkpt[i].addr == addr) {

			// If sys is 0 or 1, it must match breakpoints <sys> value.
			// If sys is -1, it is ignored.
			//
			if ((sys == -1)  ||  (ppcsim->brkpt[i].sys == sys)) {
				ppcsim->brkpt[i].valid = 0;
				return( 0 );
			}
		}
	}	

	return( ERR_BADVAL );
}


char *bp_help[] = {
	"usage: bp [-c n][-d #][-r] addr",
	"       where:",
	"          -c n   - set breakpoint ignore/skip counter",
	"                   (used on create only)",
	"          -d n   - delete breakpoint by index from list.",
	"          -r     - clear all breakpoints.",
	"          addr   - address of new breakpoint, one of:",
	"                     symbol - address via symbol",
	"                     .      - current execution address.",
	"                     hex    - hex literal.",
	NULL
};


//--------------------------------------------------------------------------------
// BRK_cmd -- breakpoint commands.
//--------------------------------------------------------------------------------
int BRK_cmd( PPCSIM *ppcsim, int argc, char *argv[] ) 
{
	int     i, opt, opt_rst, opt_del, stat;
	int		do_help;
	int		count;
	ULONG	a;
	char   *r, *e;


// Collect arguments.
//	
	optind		= 0;		// POSIX
	opt_rst		= 0;
	opt_del		= -1;
	count		= 0;
	do_help	= 0;
	if (argc > 1  &&  strcmp( argv[1], "--help" ) == 0)
		do_help++;
	else while ((opt = getopt( argc, argv, "c:rd:" )) != -1) {
		switch (opt) {
		case 'c':	count = atoi(optarg);		break;
		case 'd':	opt_del = atoi(optarg);		break;
		case 'r':	opt_rst = 1;				break;
		default:	return( ERR_INVARG );
		}
	}

// -r: reset breakpoints.
//
	if (opt_rst)
		return( BRK_init( ppcsim ) );

// -d: delete a breakpoint by index.
//
	if (opt_del >= 0)
		return( BRK_del( ppcsim, opt_del ) );

	if (do_help)
		return( shell_show_help( bp_help ) );
		
// If no args, list breakpoint status; else set a breakpoint for all the arguments.
//
	stat = 0;
	if (optind >= argc)
		stat = BRK_list( ppcsim );

	for (;optind < argc; optind++) {
		r = argv[optind];
		if ((stat = shell_getvalue( ppcsim, &r, &a, 16 )) != 0)
			return( ERR_BADVAL );
		if ((stat = BRK_set( ppcsim, a, count, 0 )) != 0)
			break;
	}

	return( stat );
}
