// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

// SPDX-License-Identifier: MIT

/*
 * Copyright (c) 2026 Gary Milliorn
 * All rights reserved.
 */


//--------------------------------------------------------------------------------
//	UART -- UART IO model
//
//	UART is responsible for emulating UART peripherals and mapping them to console/shell
//	IO functions where possible.
//
//	YK: UART CommWait	5=70, 6=D0
//
//--------------------------------------------------------------------------------


#include "ppcsim.h"


//================================================================================
//							UART Modelling
//================================================================================


//---------------------------------------------------------------------------
//	UART -- UART modelling.
//---------------------------------------------------------------------------

static UCHAR MSS_UART_regs[  16];				// RW or WO registers
static UCHAR MSS_UART_shadow[16];		// RO registers

#define UART_RXTX		0
#define UART_DIV_LSB	0		// when DLAB=1		(shadow 0)
#define UART_INTEN		1
#define UART_DIV_MSB	1		// when DLAB=1		(shadow 1)
#define UART_IID		2		// RO				(shadow 2)
#define UART_FCR 		2		// FIFO CTL, WR
#define UART_LCR 		3		// LINE CTL
#define UART_MCR 		4		// MODEM CTL
#define UART_LSR  		5		// LINE STATUS
#define UART_MSR  		6
#define UART_SCR		7



//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
int UART_reset( PPCSIM *ppcsim )
{

	MSS_UART_regs[UART_LCR] = 0x00;
	MSS_UART_regs[UART_LSR] = 0x60;			// TEMT=1 THRE=1 (always ready to send).
	MSS_UART_regs[UART_MCR] = 0xD0;			// DCD=1 DSR=1 CTS=1 (always ready to send).

	MSS_UART_shadow[0] = 0x01;
	MSS_UART_shadow[1] = 0x56;
	MSS_UART_shadow[2] = 0x01;
}


extern int kbhit();


//--------------------------------------------------------------------------------
// MSS_UART_PC16552 -- emulate PC16552 DUART.
//
static int MSS_UART_PC16552( PPCSIM *ppcsim, int port, int reg_a, UCHAR *pc, ULONG *v, int size, int r_w )
{
	int		DLAB, loop;
	UCHAR	c;


	loop = MSS_UART_regs[UART_MCR] & 0x10;		// Monitor loopback mode.
	DLAB = MSS_UART_regs[UART_LCR] & 0x80 ? 1 : 0;


// TX: we link to sync putc(), so the transmit ready bits are always valid.
//
	MSS_UART_regs[UART_LSR] |= 0x60;			// TEMT=1 THRE=1 (always ready to send).
	MSS_UART_regs[UART_MSR] |= 0xD0;			// DCD=1 DSR=1 CTS=1 (always ready to send).


// RX: Set LSR[DR] (bit 0) if a key has been pressed.
//
	MSS_UART_regs[UART_LSR] &= ~0x01;
	if (kbhit()) {
		MSS_UART_regs[UART_LSR] |= 0x01;		// LSR[DR] => 1;
	}


// Map CHAN, DLAB and ADDR to pick register.
// Note that the NS16552 has CHAN=1: port 0, CHAN=0: port 1
//		so CHAN=1 is needed.  Currently all go to the one UART.
//
	c = *pc & 0xFF;
	if (!r_w) {
		if (ppcsim->mss_logging & MM_LOG_IOWR)
			printf("  ppcsim.UART: WR(%08X) SIZ=%d :: port=%d  rega=%02X <= %02X\n", 
					reg_a, size, port, reg_a, c);
	}


	// DLAB=0 ADDR=0 RW=1:	RBR- Receive Buffer Register (RO)
	//
	if ((DLAB == 0)  &&  (reg_a == 0)  &&  (r_w == 1)) {
//		if (MSS_UART_regs[UART_LSR] & 0x01) {		// previously set DR on a key ready or loopback.
			if (loop)
				c = MSS_UART_shadow[7];
			else {
				c = raw_getkey();
	
				if ((c == 0x7F)  &&  (ppcsim->uart_io_delkey == 1))
					c = 0x08;
			}
//		}

		//MSS_UART_regs[UART_LSR] & ~0x01;			// clear DR.
	}
	// DLAB=0 ADDR=0 RW=0:	THR- Transmit Holding Register (WO)
	//
	else if ((DLAB == 0)  &&  (reg_a == 0)  &&  (r_w == 0)) {
		if (loop)
			MSS_UART_shadow[7] = c;					// record for loopback
		else if (ppcsim->uart_io_coloring) {
			//printf("\x1b[36m%c\x1b[0m", c);
			printf("\e[38;5;%dm%c\e[0m", ppcsim->uart_io_color, c);
		}
		else
			printf("%c", c);
		fflush(stdout);
	}

	// DLAB=1 ADDR=0 RW=x:	DLL- Divisor Latch (LSB)
	//
	else if ((DLAB == 1)  &&  (reg_a == 0)) {
		if (r_w == 0)	MSS_UART_shadow[0] = c;
		else			c = MSS_UART_shadow[0];
	}

	// DLAB=1 ADDR=1 RW=x:	DLM- Divisor Latch (MSB)
	// 
	else if ((DLAB == 1)  &&  (reg_a == 1)) {
		if (r_w == 0)	MSS_UART_shadow[1] = c;
		else			c = MSS_UART_shadow[1];
	}

	// DLAB=0 ADDR=2 RW=1:	IID- Interrupt ID (RO)
	// 
	else if ((DLAB == 0)  &&  (reg_a == 2)  &&  (r_w == 1)) {
		c = 0x01;
	}

	// DLAB=0 ADDR=2 RW=0:	FCR- FIFO Control Register (WO)
	// 
	else if ((DLAB == 0)  &&  (reg_a == 2)  &&  (r_w == 0)) {
		MSS_UART_regs[reg_a] = c;
	}

	// DLAB=1 ADDR=2 RW=x:	AFR- Alternate Function Register
	// 
	else if ((DLAB == 1)  &&  (reg_a == 2)  &&  (r_w == 0)) {
		if (r_w == 0)	MSS_UART_shadow[1] = c;
		else			c = MSS_UART_shadow[1];
	}

	// All others: 
	//
	else {
		if (r_w == 0)	MSS_UART_regs[reg_a] = c;
		else			c = MSS_UART_regs[reg_a];
	}

// For reads, splat the data across all lanes and let <ppcsim> pick it.
//	
	*pc = c;
	if (r_w) {
//		*v = (c << 24) | (c << 16) | (c << 8) | c;

		if (ppcsim->mss_logging & MM_LOG_IORD)
			printf("  ppcsim.UART: RD (%08X) SIZ=%d :: port=%d  rega=%02X => %02X\n", 
					reg_a, size, port, reg_a, c);
	}

	return( 0 );	
}



//-------------------------------------------------------------------------------- 
// UART_io -- adapt UART emulation to various models.  Most are 1655x-based.
//-------------------------------------------------------------------------------- 
int UART_io( PPCSIM *ppcsim, int utype, ULONG addr, ULONG *v, int size, int r_w )
{
	int		stat, port, reg_addr;
	UCHAR	c;
	ULONG   unused;
	

// 0:UNDEFINED
//
	if (utype == 0)
		return( -1 );


// Excimer 
//		Excimer places UART address on bits 18..31 of the address bus.
//		Data is on D0-7 (MSB).
//		The PC16552 has PORT=0: CHAN=1 and PORT=1: CHAN=0.
//
	else if (utype == SYS_DRV_UART_EXC) {
		reg_addr = (addr >> 19) & 0xFF;
		reg_addr =  reg_addr & 0x7;					// Reg address: 0..7 for each port
		port	 = (reg_addr & 0x8) ? 0 : 1;		// port

		// Byte-write data is on all lanes.
		//
		c = *v & 0xFF;

		stat = MSS_UART_PC16552( ppcsim, port, reg_addr, &c, &unused, size, r_w );

		// Splat byte-read data to all lanes.
		//
		*v = (c << 24) | (c << 16) | (c << 8) | c;
		return( stat );
	}


// Yellowknife X2/X4
//		National PC87308 via Winbond 83C553
//
	else if (utype == SYS_DRV_UART_SIO) {
		reg_addr = (addr & 0x07);
		reg_addr =  reg_addr & 0x7;					// Reg address: 0..7 for each port
		port	 = (reg_addr & 0x8) ? 0 : 1;		// port

		if (!r_w)
			; //printf("PC UART: WR PORT=%d  A=%x  D=%x  R_W=%d\n", port, reg_addr, *v, r_w);

		// Byte-write data is on all lanes.
		//
		c = *v & 0xFF;

		stat = MSS_UART_PC16552( ppcsim, port, reg_addr, &c, &unused, size, r_w );

		// Splat byte-read data to all lanes.
		//
		*v = (c << 24) | (c << 16) | (c << 8) | c;
		if (r_w)
			; //printf("PC UART: RD PORT=%d  A=%x  D=%x  R_W=%d\n", port, reg_addr, *v, r_w);
		return( stat );
	}

	return( -1 );
}


//================================================================================
//================================================================================


//--------------------------------------------------------------------------------
//
char *uart_help[] = {
	"  usage: uart [-cdu][-s n]",
	"  where:",
	"          -c     - toggle IO coloring.",
	"          -d     - toggle BS=DEL handling (default on).",
	"          -s n   - set IO color to n (n=0..255).",
	"          -u     - dump UART registers.",
	"",
	"where:",
	"   n is an ANSI 256-color set code, such as:",
    "          1   - maroon",
    "          2   - office green",
    "          3   - yellow",
    "          4   - blue",
    "          6   - cyan",
    "          7   - argent",
    "        136   - dark amber",
    "        196   - red",
    "        208   - amber",
	NULL
};


//--------------------------------------------------------------------------------
// UART_cmd -- UART commands.
//--------------------------------------------------------------------------------
int UART_cmd( PPCSIM *ppcsim, int argc, char *argv[] )
{
	int		i, opt, stat;
	int		do_help, do_uart;

	optind	 = 0;		// POSIX
	do_help	 = 0;
	do_uart	 = 0;
	if (argc > 1  &&  strcmp( argv[1], "--help" ) == 0)
		do_help++;
	else while ((opt = getopt( argc, argv, "cds:u" )) != -1) {
		switch (opt) {
		case 'c':	ppcsim->uart_io_coloring = 1 - ppcsim->uart_io_coloring;
					break;

		case 'd':	ppcsim->uart_io_delkey   = 1 - ppcsim->uart_io_delkey;
					break;

		case 's':	ppcsim->uart_io_color = atoi( optarg );
					break;

		case 'u':	do_uart = 1;
					break;

		default:	return( ERR_INVARG );
		}
	}


// --help	show help
//
	if (do_help)
		return( shell_show_help( uart_help ) );


// -u		show UART shadow registers.
//
	if (do_uart) {
		printf("  UART:\n");
		for (i = 0; i < 16; i++)
			printf("  %02X", MSS_UART_regs[i]);
		printf("\n");
		printf("  UART shadow:\n");
		for (i = 0; i < 16; i++)
			printf("  %02X", MSS_UART_shadow[i]);
		printf("\n");
	}

	return( 0 );
}


//--------------------------------------------------------------------------------
// UART_init -- allocate UART resources.
//--------------------------------------------------------------------------------
int UART_init( PPCSIM *ppcsim )
{

	ppcsim->uart_io_coloring = 1;
	ppcsim->uart_io_color    = 208;
	ppcsim->uart_io_delkey   = 1;

	return( 0 );
}


//--------------------------------------------------------------------------------
// UART_deinit -- free up UART resources.
//--------------------------------------------------------------------------------
int UART_deinit( PPCSIM *ppcsim )
{

	return( 0 );
}
