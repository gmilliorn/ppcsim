// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

// SPDX-License-Identifier: MIT

/*
 * Copyright (c) 2026 Gary Milliorn
 * All rights reserved.
 */


//--------------------------------------------------------------------------------
//	NB -- NORTH BRIDGE
//
//		functions to manage northbridge
//
//		generally SYS loads and parses a system definition file to create
//		a replica of the target system.
//
//		as an option, the CPU can be changed to an arbitrary one even if the
//		original platform didn't support that.


#include "ppcsim.h"


//--------------------------------------------------------------------------------
// nb_list -- supported Northbridges.
//
NAMEVAL nb_list[] = {
	{	"MPC105",	NB_MPC105	},
	{	"MPC106",	NB_MPC106	},
	{	"KAHLUA",	NB_KAHLUA	},
	{	"MPC107",	NB_MPC107	}
};


//--------------------------------------------------------------------------------
// NB config space presets.
//
typedef struct nb_reg_defs {
	int		match;
	int		addr;
	int		size;
	int		ro;
	ULONG	def;
	char	name[16];
} NB_REGS;


NB_REGS nb_regs[] = {
	{	NB_MPC106,	NB_DEVID,	  2,	1,	NB_DEVID_MPC106,	"DEVID"		},
	{	NB_KAHLUA,	NB_DEVID,	  2,	1,	NB_DEVID_KAHLUA,	"DEVID"		},
	{	NB_MPC107,	NB_DEVID,	  2,	1,	NB_DEVID_MPC107,	"DEVID"		},
	{	0,			NB_PICR1,	1|2|4,	0,	NB_PICR1_DEF,		"PICR1"		},
	{	0,			NB_PICR2,	1|2|4,	0,	NB_PICR2_DEF,		"PICR2"		},
	{	0,			NB_EUMBBAR,	    4,	0,	0,					"EUMBBAR"	},
	{	0,			NB_MSAR1,	1|2|4,	0,	0,					"MSAR1"		},
	{	0,			NB_MSAR2,	1|2|4,	0,	0,					"MSAR2"		},
	{	0,			NB_EMSAR1,	1|2|4,	0,	0,					"EMSAR1"	},
	{	0,			NB_EMSAR2,	1|2|4,	0,	0,					"EMSAR2"	},
	{	0,			NB_MEAR1,	1|2|4,	0,	0,					"MEAR1"		},
	{	0,			NB_MEAR2,	1|2|4,	0,	0,					"MEAR2"		},
	{	0,			NB_EMEAR1,	1|2|4,	0,	0,					"EMEAR1"	},
	{	0,			NB_EMEAR2,	1|2|4,	0,	0,					"EMEAR2"	},
	{	0,			NB_MCCR1,	1|2|4,	0,	NB_MCCR1_DEF,		"MCCR1"		},
	{	0,			NB_MCCR2,	1|2|4,	0,	NB_MCCR2_DEF,		"MCCR2"		},
	{	0,			NB_MCCR3,	1|2|4,	0,	NB_MCCR3_DEF,		"MCCR3"		},
	{	0,			NB_MCCR4,	1|2|4,	0,	NB_MCCR4_DEF,		"MCCR4"		}
};


//--------------------------------------------------------------------------------
// NB_ENV_preset --preset ENV to zeros, and preset for DINK if needed.
//--------------------------------------------------------------------------------
int NB_ENV_preset( PPCSIM *ppcsim, int for_dink )
{
	int i;

	for (i = 0; i < MAX_NVRAM; i++)
		ppcsim->bridge.nvram[i] = 0;

// DINK header.
//
	if (for_dink) {
		// 0-3: ID header
		//
		ppcsim->bridge.nvram[ 0] = 'D';
		ppcsim->bridge.nvram[ 1] = 'K';
		ppcsim->bridge.nvram[ 2] = 'N';
		ppcsim->bridge.nvram[ 3] = 'V';

		// 4-7: rsvd
		// 8-B: CRC

		// C..: env strings.
		strcpy( &ppcsim->bridge.nvram[12], "ppcsim=hello\n" );
	}

// SPINIT header
//
	if (for_dink) {
		ppcsim->bridge.nvram[0xF00+ 0] = 'S';
		ppcsim->bridge.nvram[0xF00+ 1] = 'P';
		ppcsim->bridge.nvram[0xF00+ 4] = 0x02;	// cL

		ppcsim->bridge.nvram[0xF00+20] =   0;		// Boot addr
		ppcsim->bridge.nvram[0xF00+21] =   0;
		ppcsim->bridge.nvram[0xF00+22] =   0;
		ppcsim->bridge.nvram[0xF00+23] =   0;

		ppcsim->bridge.nvram[0xF00+24] = 192;		// Assigned IP
		ppcsim->bridge.nvram[0xF00+25] = 168;
		ppcsim->bridge.nvram[0xF00+26] =  86;
		ppcsim->bridge.nvram[0xF00+27] =  99;
	}

	return( 0 );
}


//--------------------------------------------------------------------------------
// NB_reset -- reset the NB except for NVRAM contents.
//--------------------------------------------------------------------------------
int NB_reset( PPCSIM *ppcsim )
{
	int i;

	ppcsim->bridge.nv_addr_hi	= 0;
	ppcsim->bridge.nv_addr_lo	= 0;

	ppcsim->bridge.rtc_addr	= 0;

	for (i = 0; i < 16; i++)
		ppcsim->bridge.gpio[i] = 0;		

	return( 0 );
}


//--------------------------------------------------------------------------------
// NB_get_name --
//--------------------------------------------------------------------------------
char *NB_get_name( PPCSIM *ppcsim, int reg_a )
{
	int i;

	for (i = 0; i < (sizeof(nb_regs)/sizeof(NB_REGS)); i++)
		if (nb_regs[i].addr == reg_a)
			return( nb_regs[i].name );
	return( NULL );
}


//--------------------------------------------------------------------------------
// NB_cfg -- do a subset of configuration space accesses.
//--------------------------------------------------------------------------------
int NB_cfg( PPCSIM *ppcsim, SYSMAP *s, ULONG addr, ULONG *v, int size, int r_w )
{
	BRIDGE *nb;
	int		stat;
	ULONG	d, nb_addr, r_addr, rd, wd, lsb;
	ULONG	NB_addr_a, NB_data_a;


	nb = &ppcsim->bridge;
	NB_addr_a	= 0xfec00000;
	NB_data_a	= 0xfee00000;

	lsb	 = addr & 0x03;
	addr = addr & 0xFFFFFFC0;
	stat = 0;


	if ((r_w == 0)  &&  (ppcsim->mss_logging & MM_LOG_CFGWR))
		printf("  ppcsim.MSS.NBcfg: WR %08X <= %08X TSIZ=%d  LSB=%d\n", addr, *v, size, lsb);


// ADDR:	Address value for indexed selection.
//			just save address in drv_data[0].
//
	if (addr == nb->idx_a) {
		s->drv_args[0] = *v;
		ppcsim->bridge.a = pSwap4Bytes( *v );
		return( stat );
	}


// DATA:	Use saved address to complete cycle.
//
	else if (addr == nb->dat_a) {
		wd		= pSwap4Bytes( *v );
		nb_addr = ppcsim->bridge.a & 0xFFFF;	
		r_addr	= ppcsim->bridge.a & 0x00FF;			// Only first 256 tracked in NB struct

		// Reads
		//
		if (r_w == 1) {

			switch (nb_addr) {
			// DeviceID and VendorID
			//
			case 0x0000:	rd = nb->id;
							break;

			case 0x0080:									// MSAR etc.
			case 0x0084:	
			case 0x0088:
			case 0x008C:	
			case 0x0090:									// MEAR etc.
			case 0x0094:	
			case 0x0098:
			case 0x009C:	
			case 0x00A0:									// MBEN
			case 0x00AC:									// PICR2
			case 0x00F0:									// MCCR etc.
			case 0x00F4:	
			case 0x00F8:
			case 0x00FC:	
							rd = nb->regs[ r_addr ];
							break;

			case 0x00A8:									// PICR1
							rd = 0;
							break;

			default:		if (r_addr < 0x100)
								rd = nb->regs[ r_addr ];
							else
								rd = 0xFFFFFFFF;
							//printf("  ppcsim.MSS.NB: unmapped RD %08X <= %08X TSIZ=%d\n", nb_addr, *v, size);
							break;
			}

			*v = pSwap4Bytes( rd );
		}

		// Write
		//
		else {
			switch (nb_addr) {
			case 0x0000:	
						break;

			case 0x00A0:
			case 0x00A1:
			case 0x00A2:
			case 0x00A3:
						if (lsb == 0)	nb->regs[ r_addr ] = PCICFG_SETBYTE_0(nb->regs[ r_addr ], wd);
						if (lsb == 1)	nb->regs[ r_addr ] = PCICFG_SETBYTE_1(nb->regs[ r_addr ], wd);
						if (lsb == 2)	nb->regs[ r_addr ] = PCICFG_SETBYTE_2(nb->regs[ r_addr ], wd);
						if (lsb == 3)	nb->regs[ r_addr ] = PCICFG_SETBYTE_3(nb->regs[ r_addr ], wd);
						break; 

			// PICR1 (select map A/B on bit 16)
			//
			case 0x00A8:
						nb->regs[ r_addr ] = wd;
						break;

			case 0x0080:									// MSAR etc.
			case 0x0084:	
			case 0x0088:
			case 0x008C:	
			case 0x0090:									// MEAR etc.
			case 0x0094:	
			case 0x0098:
			case 0x009C:	
			case 0x00AC:									// PICR2
			case 0x00F0:									// MCCR etc.
			case 0x00F4:	
			case 0x00F8:
			case 0x00FC:	
						nb->regs[ r_addr ] = wd;
						break;

			// Not normally needed.  Print a message to flag for updates.
			// 
			default:	if (r_addr < 0x100)
							nb->regs[ r_addr ] = wd;
						//printf("  ppcsim.MSS.NB: unmapped WR %08X <= %08X TSIZ=%d\n", r_addr, wd, size);
						break;
			}
		}
	}

	if ((r_w == 1)  &&  (ppcsim->mss_logging & MM_LOG_CFGRD))
		printf("  ppcsim.MSS.NBcfg: RD %08X   TSIZ=%d\n", nb_addr, size);

	return( stat );
}


//--------------------------------------------------------------------------------
// NB_io -- emulate NB moderated IO cycles.  Called by MSS for whatever space
//			is allocated to configured IO cycles.
//
//			currently, NB_io maps to all down-stream IO whereas in a typical
//			system there could be a South Bridge and/or a SuperIO.
//--------------------------------------------------------------------------------
int NB_io( PPCSIM *ppcsim, SYSMAP *s, ULONG addr, ULONG *v, int size, int r_w )
{
	BRIDGE *nb;
	int		stat, i, nva, cyear;
	UCHAR	vb;
	ULONG	d, nb_addr, rd, wd, lsb;
	ULONG	NB_addr_a, NB_data_a;
	struct tm lclt;


	nb = &ppcsim->bridge;

	nb_addr = addr & 0xFFF;
	vb		= *v   & 0xFF;
	stat	= 0;


	if ((r_w == 0)  &&  (ppcsim->mss_logging & MM_LOG_NBWR))
		printf("  ppcsim.NBio: WR %04X <= %08X TSIZ=%d  LSB=%d\n", nb_addr, *v, size, lsb);


//---------------------------------------------------------------------------
// Next, ISA IO devices.  By and large, these are ignored, except for
//
// =========   ============================      ===========
// Address     Description                       Action
// =========   ============================      ===========
// FE00_0020   PIC1                              Winbond PIC1
// FE00_0021   PIC1                              Winbond PIC1
// FE00_0070   RTC Address                       Latch
// FE00_0071   RTC Data                          R/W data
// FE00_0074   NVRAM Address High                Latch
// FE00_0075   NVRAM Address Low                 Latch
// FE00_0076   NVRAM Data                        R/W data
// FE00_0080   Port80 Debug Probe                Print on write
// FE00_00A0   PIC2                              Winbond PIC2
// FE00_00A1   PIC2                              Winbond PIC2
// FE00_015C   ISA PnP Enumerator Index          Ignore
// FE00_015D   ISA PnP Enumerator Data           Ignore
// FE00_02FX   Serial port #2                    UART
// FE00_03FX   Serial port #1                    UART
// FE00_0700   GPIO Output for LED/LpBk          Latch + Copy 7->6
// FE00_0701   GPIO Direction                    Ignore
// =========   ============================      ===========


// UARTs.
//
	if (0x2F8 <= nb_addr  &&  nb_addr <= 0x2FF)
		stat = UART_io( ppcsim, SYS_DRV_UART_SIO, nb_addr, v, size, r_w );

	else if (0x3F8 <= nb_addr  &&  nb_addr <= 0x3FF)
		stat = UART_io( ppcsim, SYS_DRV_UART_SIO, nb_addr, v, size, r_w );

// DEBUG
//		ISA Port80 is the Debug probe.
//
	else if (nb_addr == 0x0080) {
		if (!r_w) printf("  ppcsim.NBio: PORT80 <= 0x%02x", vb);
	}

// NVRAM 
//		Emulate a 4KB NVRAM.  Some are smaller, just ignore that.
//
	else if (nb_addr == 0x0074) {				// Addr HI
		if (!r_w)		nb->nv_addr_hi = vb;
		else			*v = SPLAT(nb->nv_addr_hi);
	}
	else if (nb_addr == 0x0075) {				// Addr LO
		if (!r_w)		nb->nv_addr_lo = vb;
		else			*v = SPLAT(nb->nv_addr_lo);
	}
	else if (nb_addr == 0x0076) {				// Data
		nva = (nb->nv_addr_hi << 8) | nb->nv_addr_lo;
		if (!r_w)		nb->nvram[nva] = vb;
		else			*v = SPLAT( nb->nvram[nva] );
	}

// ISA PnP Enumeration
//		Legacy MPC106 ISA PnP enumeration.
//
	else if (nb_addr == 0x015C) {
		*v = 0;
	}
	else if (nb_addr == 0x015D) {
		*v = 0;
	}

// PIC
//		Winbond PIC1/2
//
	else if ((nb_addr == 0x0020)
		 ||  (nb_addr == 0x0021)
		 ||  (nb_addr == 0x00A0)
		 ||  (nb_addr == 0x00A1)) {
		*v = 0;
	}

// RTC simulation.
//
//
	else if (nb_addr == 0x0070) {
		if (!r_w)		nb->rtc_addr = vb;
		else			*v = SPLAT(nb->rtc_addr);
	}
	else if (nb_addr == 0x0071) {
		nb->rtc_time = time( NULL );
		localtime_r( &nb->rtc_time, &lclt );
		cyear = (lclt.tm_year >= 100) ? lclt.tm_year - 100 : lclt.tm_year;

		if (r_w)								// time set not supported
			switch (nb->rtc_addr) {
			case RTC_SECS:	*v = SPLAT( RTC_TOBCD( lclt.tm_sec   ) );	break;
			case RTC_MINS:	*v = SPLAT( RTC_TOBCD( lclt.tm_min   ) );	break;
			case RTC_HRS:	*v = SPLAT( RTC_TOBCD( lclt.tm_hour  ) );	break;
			case RTC_DAY:	*v = SPLAT( RTC_TOBCD( lclt.tm_mday  ) );	break;
			case RTC_MON:	*v = SPLAT( RTC_TOBCD( lclt.tm_mon+1 ) );	break;
			case RTC_YR:	*v = SPLAT( RTC_TOBCD( cyear		 ) );	break;
			case RTC_FLGS:	*v = SPLAT( RTC_TOBCD( RTCF_24HR     ) );	break;
			case RTC_BATT:	*v = SPLAT( RTC_TOBCD(      0        ) );	break;
			}
	}


// GPIO
//		DINK for Sandpoint uses GPIO pins that are looped for board detection.
//		Monitor the SIO GPIO port so that the DINK SP detection code will work.

	else if ((nb_addr == 0x0700)
	     ||  (nb_addr == 0x0701)) {
		i = nb_addr & 1;
		if (!r_w) {
			nb->gpio[i] = vb;

			if (nb->gpio[i] & 0x80)		// 7 on, set 6
				nb->gpio[i] |=  0x40;
			else						// 7 off, clear 6
				nb->gpio[i] &= ~0x40;
		}
		else	
			*v = SPLAT( nb->gpio[i] );
	}


// UNKNOWN
//		ignore
//
	else {
		printf("  ppcsim.NBio: unknown, ADDR=%08X  *v=%08X  r_w=%d\n", addr, *v, r_w);
		stat = 0;
	}

	if ((r_w == 1)  &&  (ppcsim->mss_logging & MM_LOG_NBRD))
		printf("  ppcsim.NBio: RD A=%04X TSIZ=%d => %08X\n", nb_addr, size, *v);

	return( stat );
}


//--------------------------------------------------------------------------------
// NB_set -- set northbridge data.
//--------------------------------------------------------------------------------
int NB_set( PPCSIM *ppcsim, char *name )
{
	int stat, nb_sel, i;

// Find bridge.
//
	for (nb_sel = -1, i = 0; i < sizeof(nb_list)/sizeof(NAMEVAL); i++) {
		if (strcmp( name, nb_list[i].name ) == 0) {
			nb_sel = i;
			break;
		}
	}
	if (nb_sel == -1)
		return( ERR_INVARG );


// Save and apply NB presets.
//
	strncpy( ppcsim->bridge.name, nb_list[nb_sel].name,  12 );
	ppcsim->bridge.id = nb_list[nb_sel].val;

	ppcsim->bridge.idx_a	= 0xFEC00000;
	ppcsim->bridge.dat_a	= 0xFEE00000;


	for (i = 0; i < sizeof(nb_regs)/sizeof(NB_REGS); i++) {
		if ((nb_regs[i].match == 0)								// for all
		||  (nb_regs[i].match == nb_list[nb_sel].val)) {		// for this
			ppcsim->bridge.regs[nb_regs[i].addr] =  BYTEREV( nb_regs[i].def );
		}
	}


// Finish init.
//
	NB_reset( ppcsim );

	return( 0 );
}


char *nb_help[] = {
	"  usage: nb [-lpr][-n a][-s s]",
	"  where:",
	"          -l    - list northbridge registers.",
	"          -n a  - dump 256B of NVRAM contents starting at <a>.",
	"          -p    - preset NVRAM with DINK defaults.",
	"          -r    - reset northbridge registers.",
	"          -s s  - set northbridge type to 's'.",
	NULL
};


//--------------------------------------------------------------------------------
// NB_cmd -- initialize northbridge data.
//--------------------------------------------------------------------------------
int NB_cmd( PPCSIM *ppcsim, int argc, char *argv[] )
{
	int    opt, i, a, stat, brx;
	ULONG  v, addr;
	char   dopt, do_help, do_set, do_reset, do_list, do_nvpreset, do_shownv;
	char  *r, rterm[40], nbname[40];


// Collect arguments.
	
	optind		= 0;		// POSIX
	do_list		= 0;
	do_reset	= 0;
	do_set		= 0;
	do_shownv	= 0;
	do_nvpreset = 0;
	addr		= 0;
	nbname[0]	= 0;
	if (argc > 1  &&  strcmp( argv[1], "--help" ) == 0)
		do_help++;
	else while ((opt = getopt( argc, argv, "ln:prs:" )) != -1) {
		switch (opt) {
		case 'l':	do_list = 1;										break;
		case 'n':	do_shownv = 1; addr = strtoul( optarg, &r, 16 );	break;
		case 'p':	do_nvpreset = 1;									break;
		case 'r':	do_reset = 1;										break;
		case 's':	do_set = 1; strcpy(nbname, optarg);					break;
		default:	return( ERR_UNKNOWN );
		}
	}


// --help or errors.
//
	if (do_help)
		return( shell_show_help( nb_help ) );

// -n	show NVRAM contents (first 256).
//
	if (do_shownv) {
		for (a = addr; a <= addr+0xfc; a += 16) {
			printf("  %04X: ", a );
			for (i = 0; i <= 0xf; i++)
				printf("%02X ", ppcsim->bridge.nvram[ a + i ] );
			printf("\n");
		}
		return( 0 );
	}

// -p	preset NVRAM as for DINK.
//
	if (do_nvpreset)
		return( NB_ENV_preset( ppcsim, 1 ) );

// -r	reset
//
	if (do_reset)
		return( NB_reset( ppcsim ) );

// -s	set NB to value.
//
	if (do_set)
		return( NB_set( ppcsim, nbname ) );


// -l	list.
//
	brx = 0;	

	for (i = 0; i <= 0xfc; i += 4) {
		if ((i & 0xF) == 0)
			printf("%04X: ", i);
		r = NB_get_name( ppcsim, i );
		r = r ? r : "";
		v = ppcsim->bridge.regs[i];
		if (brx)
			v = pSwap4Bytes( v );
		printf("%-8s=%08X ", r, v );
		if ((i & 0xF) == 0xC)
			printf("\n");
	}

	return( 0 );
}


//--------------------------------------------------------------------------------
// NB_init -- initialize northbridge data.
//--------------------------------------------------------------------------------
int NB_init( PPCSIM *ppcsim )
{

	ppcsim->bridge.id			= 0;
	ppcsim->bridge.a			= 0;
	ppcsim->bridge.map		= 0;
	ppcsim->bridge.name[0]	= 0;

	NB_reset( ppcsim );
	
	return( 0 );
}


//--------------------------------------------------------------------------------
// NB_deinit --
//--------------------------------------------------------------------------------
int NB_deinit( PPCSIM *ppcsim )
{

	return( 0 );
}
