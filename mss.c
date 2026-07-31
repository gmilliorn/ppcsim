// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

// SPDX-License-Identifier: MIT

/*
 * Copyright (c) 2026 Gary Milliorn
 * All rights reserved.
 */


//--------------------------------------------------------------------------------
//	MSS -- Memory SubSystem
//
//	MSS is responsible for mapping memory read/write cycles to the appropriate
//  device for the targeted system.
//
//	SYSTEM is responsible for allocating memory and placement.
//
//  System definitions of note:
//
//		WINBOND
//			CFG_ADDR	0x8000_0000		addr
//			CFG_ADDR_H	0x800F_0000		higher registers
//
//		NORTHBRIDGE
//			VENDOR+DEV_ID	1057_0002	= MPC106 / Grackle
//							1057_0003	= MPC8240 / Kahlua
//							1057_0004	= MPC107 / Chaparal
//
//		PREP (pre CHRP)
//			IO			0x8000_0000		IO_BASE
//			CFG_ADDR	0x8000_0CF8		addr
//			CFG_DATA	0x8000_0CFC		data
//			
//		CHRP
//			IO			0xFE00_0000		IO_BASE
//			CFG_ADDR	0xFEC0_0000		addr
//			CFG_DATA	0xFEE0_0000		data
//
//  NOT SUPPORTED
//		FLASH programming.
//			ONFI etc. is complicated, you can trap writes to FLASH and model
//			it.

//	YK: UART CommWait	5=70, 6=D0
//
//--------------------------------------------------------------------------------


#include "ppcsim.h"


//================================================================================
//================================================================================


//---------------------------------------------------------------------------
// MSS_MapAddress -- return SYSMAP entry matching an address.
//---------------------------------------------------------------------------
SYSMAP *MSS_MapAddress( PPCSIM *ppcsim, ULONG addr )
{
	CList  *iter, *safe;
	SYSMAP *s;

	c_list_for_each_safe( iter, safe, (CList *) ppcsim->system) {
		s = (SYSMAP *) iter;
		if ((s->start <= addr)  &&  (addr <= s->end)) {
			return( s );
		}
	}

	return( NULL );
}


//---------------------------------------------------------------------------
//	MSS -- memory sub-system.  
//
//
//		   exit with (-1) on an error, with <exc_addr> and <exc_type>
//		   properly set.
//---------------------------------------------------------------------------
static int MSS( PPCSIM *ppcsim, ULONG addr, ULONG *v, int size, int mode )
{
	SYSMAP	*tgt;
	int		 stat, no, i, index;
	ULONG	 map_a, lv;
	UCHAR	*s, *d;


	if ((mode == MODE_RD) && (ppcsim->verbose))	
		printf("MSS: RD A=%08X  size=%d\n", addr, size );
	if ((mode == MODE_WR) && (ppcsim->verbose))	
		printf("MSS: WR A=%08X <= %08X  size=%d\n", addr, *v, size );
		

// Check for data breakpoints.
//
#if 0
	index = -1;
	for (i = 0; i < MAX_BRKPTS; i++) {
		if (ppcsim->brkpt[i].valid  &&  (ppcsim->brkpt[i].addr == a)  &&  (ppcsim->brkpt[i].is_data == 1)) {

			// If count is non-zero, decrement and pretend there was no match.
			//
			if (ppcsim->brkpt[i].count) {
				ppcsim->brkpt[i].count -= 1;
				continue;
			}
			*index = i;
			break;
		}
	}
	if (index >= 0) {
		ppcsim->exc_type = 0x1300;
		ppcsim->exc_addr = addr;
		return( -1 );
	}
#endif


//---------------------------------------------------------------------------
// Insert cache simulation code here.
//


//---------------------------------------------------------------------------
// Map the address to the correct targeted device, based upon the selected
// platform.
//
	tgt = MSS_MapAddress( ppcsim, addr );


// IGNORE
//		some code relies on unsupported memory addresses behaving rationally,
//		which is "unboundedly unguaranteed".  Reads return 0.
//
	if ((tgt  &&  (tgt->mclass == SYS_CLASS_IGNORE))			// match, but ignored.
	||  ((tgt == NULL)  &&  ppcsim->mss_ignore_undef)) {			// no match, but ignored
		if (ppcsim->mss_logging & MM_LOG_IGNORED)
			printf("  ppcsim.MSS: %08X ignored\n", addr);
		if (mode == MODE_RD)
			*v = 0;
		return( 0 );
	}


// TRAP
//		signal a bus exception.
//
	else if ((tgt == NULL)										// no match, trap
		 ||  (tgt->mclass == SYS_CLASS_TRAP)) {					// match, cfg to trap

		if (ppcsim->mss_logging & MM_LOG_TRAP)
			printf("  ppcsim.MSS: bus exception trap on %08X\n", addr);
		ppcsim->exc_type = 0x300;
		ppcsim->exc_addr = addr;
		return( -1 );
	}


// IO
//	IO has several subclasses using drivers.

//	IO devices like the Excimer UART direct-attach to the system bus.
//
	else if (tgt->mclass == SYS_CLASS_IO) {

		if ((tgt->driver == SYS_DRV_UART_EXC))
			return( UART_io( ppcsim, tgt->driver, addr, v, size, mode ) );


//	Other IO devices are indirect through a north-bridge.
//	IONB - INDIRECT IO
//			DRV_NB is the config space.
//			DRV_NBIO is the (configured) IO
//
		else if (tgt->driver == SYS_DRV_NB) {
			if ((stat = NB_cfg( ppcsim, tgt, addr, v, size, mode )) == 0)
				return( stat );
			ppcsim->exc_type = 0x300;
			ppcsim->exc_addr = addr;
			return( -1 );
		}
		else if (tgt->driver == SYS_DRV_NBIO) {
			if ((stat = NB_io( ppcsim, tgt, addr, v, size, mode )) == 0)
				return( stat );
			ppcsim->exc_type = 0x300;
			ppcsim->exc_addr = addr;
			return( -1 );
		}

		else {
			printf("  ppcsim.MSS: %08X requires driver, none set.\n", addr);
			ppcsim->exc_type = 0x300;
			ppcsim->exc_addr = addr;
			return( -1 );
		}
	}


// (DEFAULT)
//		must be mem.
//
	else if (tgt->mclass != SYS_CLASS_MEM) {
		printf("  ppcsim.MSS: unknown class %d\n", tgt->mclass);
		ppcsim->exc_type = 0x300;
		ppcsim->exc_addr = addr;
		return( -1 );
	}

	map_a = addr & tgt->mask;				// After mask, address should be referenced to zero.
	if (map_a >= tgt->memsize) {			// so comparison to memsize will fail.
		ppcsim->exc_type = 0x300;
		ppcsim->exc_addr = addr;
	}

	s = tgt->mem + map_a;
	d = (UCHAR *) v;

	if (mode == 1) {							// read
		switch (size) {
		case ACCESS_B:	* d    = *s++;			break;
		case ACCESS_H:	*(d+1) = *s++;
						*(d  ) = *s++;			break;
		case ACCESS_W:	*(d+3) = *s++;
						*(d+2) = *s++;
						*(d+1) = *s++;
						*(d  ) = *s++;			break;
		}
		if (ppcsim->mss_logging & MM_LOG_MEMRD)
			printf("  ppcsim.MSS: MEMRD %08X => %08X TSIZ=%d\n", (ULONG) map_a, (ULONG) *d, size);

	} else {								// write
		if (ppcsim->mss_logging & MM_LOG_MEMWR)
			printf("  ppcsim.MSS: WR %08X <= %08X TSIZ=%dn", (ULONG) map_a, (ULONG) *d, size);
		switch (size) {
		case ACCESS_B:	* s    = *d++;			break;
		case ACCESS_H:	*(s+1) = *d++;
						*(s  ) = *d++;			break;

		case ACCESS_W:	*(s+3) = *d++;
						*(s+2) = *d++;
						*(s+1) = *d++;
						*(s  ) = *d++;			break;
		}
	}

	return( 0 );
}


//--------------------------------------------------------------------------------
// MSS_* -- map PPCSIM calls to equivalent MSS handler.
//
int MSS_read( PPCSIM *ppcsim, ULONG addr, ULONG *data, int size )
{
	return( MSS( ppcsim, addr, data, size, MODE_RD ) );
}
int MSS_write( PPCSIM *ppcsim, ULONG addr, ULONG *data, int size )
{
	return( MSS( ppcsim, addr, data, size, MODE_WR ) );
}


//--------------------------------------------------------------------------------
//
char *mss_help[] = {
	"  usage: mss [-ans][-l s=v]",
	"  where:",
	"          -a     - turn on all logging.",
	"          -l f=n - enable/disable MSS logging for:",
	"                     mr   - mem reads",
	"                     mw   - mem writes",
	"                     ior  - io reads",
	"                     iow  - io writes",
	"                     ign  - ignores",
	"                     t    - traps",
	"          -n     - turn off all logging.",
	"          -s     - show current log settings.",
	NULL
};


static NAMEVAL flag_tbl[] = {
	{	"mr",	MM_LOG_MEMRD	},
	{	"mw",	MM_LOG_MEMWR	},
	{	"iow",	MM_LOG_IOWR		},
	{	"ior",	MM_LOG_IORD		},
	{	"nbw",	MM_LOG_NBWR		},
	{	"nbr",	MM_LOG_NBRD		},
	{	"cfgw",	MM_LOG_CFGWR	},
	{	"cfgr",	MM_LOG_CFGRD	},
	{	"ign",	MM_LOG_IGNORED	},
	{	"trap",	MM_LOG_TRAP		}
};


//--------------------------------------------------------------------------------
// MSS_cmd -- MSS commands.
//--------------------------------------------------------------------------------
int MSS_cmd( PPCSIM *ppcsim, int argc, char *argv[] )
{
	int i;
	int		opt, stat;
	int		do_help, do_uart;
	ULONG   v;
	char   *s, *e;
	char	logarg[16];

	optind	 = 0;		// POSIX
	do_help	 = 0;
	do_uart	 = 0;
	if (argc > 1  &&  strcmp( argv[1], "--help" ) == 0)
		do_help++;
	else while ((opt = getopt( argc, argv, "al:ns" )) != -1) {
		switch (opt) {
		case 'a':	ppcsim->mss_logging = 0xFF;		
					break;

		case 'l':	if ((s = strtok( optarg, "=" )) == NULL)
						return( ERR_BADVAL );
					e = strtok( NULL, "" );
					v = strtoul( e, &e, 10 );
					for (i = 0; i < sizeof(flag_tbl)/sizeof(NAMEVAL); i++) {
						if (strcmp(s, flag_tbl[i].name) == 0)
							if (e == 0)		ppcsim->mss_logging &= ~flag_tbl[i].val;
							else			ppcsim->mss_logging |=  flag_tbl[i].val;
					}
					break;

		case 'n':	ppcsim->mss_logging = 0;		
					break;

		case 's':	printf("  MEM-RD logging: %d\n", ppcsim->mss_logging & MM_LOG_MEMRD   ? 1 : 0 );
					printf("  MEM-WR logging: %d\n", ppcsim->mss_logging & MM_LOG_MEMWR   ? 1 : 0 );
					printf("  IO-RD  logging: %d\n", ppcsim->mss_logging & MM_LOG_IORD    ? 1 : 0 );
					printf("  IO-WR  logging: %d\n", ppcsim->mss_logging & MM_LOG_IOWR    ? 1 : 0 );
					printf("  NB-RD  logging: %d\n", ppcsim->mss_logging & MM_LOG_NBRD    ? 1 : 0 );
					printf("  NB-WR  logging: %d\n", ppcsim->mss_logging & MM_LOG_NBWR    ? 1 : 0 );
					printf("  CFG-RD logging: %d\n", ppcsim->mss_logging & MM_LOG_CFGRD   ? 1 : 0 );
					printf("  CFG-WR logging: %d\n", ppcsim->mss_logging & MM_LOG_CFGWR   ? 1 : 0 );
					printf("  IGNORE logging: %d\n", ppcsim->mss_logging & MM_LOG_IGNORED ? 1 : 0 );
					printf("  TRAP   logging: %d\n", ppcsim->mss_logging & MM_LOG_TRAP    ? 1 : 0 );
					break;

		default:	return( ERR_INVARG );
		}
	}


// --help	show help
//
	if (do_help)
		return( shell_show_help( mss_help ) );

	return( 0 );
}


//--------------------------------------------------------------------------------
// MSS_init -- allocate MSS resources.
//--------------------------------------------------------------------------------
int MSS_init( PPCSIM *ppcsim )
{

	ppcsim->mss_logging	= 0;	//MM_LOG_IGNORED;

	return( 0 );
}


//--------------------------------------------------------------------------------
// MSS_deinit -- free up MSS resources.
//--------------------------------------------------------------------------------
int MSS_deinit( PPCSIM *ppcsim )
{

	return( 0 );
}
