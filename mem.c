// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup


#include "ppcsim.h"


extern	ULONG	pSwap4Bytes( ULONG v );


//--------------------------------------------------------------------------
//	MDEngine -- a generic memory modification engine which allows
//				access to conventional and non-conventional memory
//				(such as NVRAM accesses through indirect registers).
//--------------------------------------------------------------------------
int MDEngine( PPCSIM *ppcsim, ULONG addr, ULONG end, int size, int options )
{
	int		stat;
	short 	dir, linecnt, linesize;
	short 	collect, i, o, k, o2;
	ULONG	mem_data[16];
	ULONG	work;
	char	cbuf[16], c;
	long	va;
	

// Unless prohibited, align the start address to a paragraph (16 byte
// boundary.
//
	if (!(options & MDE_NOALIGN)) {
		addr &= 0xFFFFFFF0;
		end  &= (~size);
	}


// For display, if start == end, show a whole line.
//
	if (addr == end)
		end = addr + 15;


// Determine whether to prompt or not.
//
	linesize = shell_more_lines(999);
	if ((end - addr) > MDE_THRESHOLD_LINES*4*4 )
		linesize = (options & MDE_BIGPAGES) ? 16 : 5;
	else
		linesize = 0;


// How many pieces of data are needed for each line?
//
	if (size == ACCESS_B)
		collect = 16;
	else if (size == ACCESS_H)
		collect = 8;
	else
		collect = 4;

	dir     = 1;
	linecnt = 0;
	while (1) {


// Show address of current memory.
//
		va = (ULONG) addr;
		printf("0x%08lx ", va);
		if (!(options & MDE_DENSER))
			printf("    ");


// Use driver to collect one line of data.  The values are stored
// as longs regardless of size.
//
		for (i = o = 0; i < collect; i++, o += size)
			if (addr+o <= end) {
				MSS_read( ppcsim, addr+o, &mem_data[i], size );
				if (size == ACCESS_B)		mem_data[i] &=   0xFF;
				else if (size == ACCESS_H)	mem_data[i] &= 0xFFFF;
			}


// Print data using our driver (not a supplied driver).
//
		for (i = o = 0; i < collect; i++, o += size)
			if (addr+o <= end) {
				work = mem_data[i];
				if (options & MDE_BRX)
					work = pSwap4Bytes( mem_data[i] );
				switch (size) {
				case ACCESS_B:		printf(" %02x", work);		break;
				case ACCESS_H:		printf(" %04x", work);		break;
				default:
				case ACCESS_W:		printf(" %08x", work);		break;
				}
			}
			else {
				switch (size) {
				case ACCESS_B:		printf("   ");				break;
				case ACCESS_H:		printf("     ");			break;
				default:
				case ACCESS_W:		printf("         ");		break;
				}
			}


// Print again as printable ASCII (not size-dependant).
//
		for (i = o = o2 = 0; i < collect; i++, o += size) {
			if (addr+o <= end) {
				for (k = size; k > 0; k--) {
					c = (mem_data[i] >> (8*(k-1))) & 0xFF;
					if ((c < 0x20)  ||  (c > 0x7E))
						cbuf[o2++] = '.';
					else
						cbuf[o2++] = c;
				}
			}
		}
		cbuf[o2] = '\0';

		switch (size) {
		case ACCESS_B:		printf(" ");						break;
		case ACCESS_H:		printf("     ");					break;
		default:
		case ACCESS_W:		printf("         ");				break;
		}

		printf("%s\n", cbuf);


// Do interactive mode, as needed.
//
		if (options & MDE_NOIO)
			;
		else if (linesize  &&  (++linecnt > linesize)) {
			linecnt = 0;

// Done?
//
			if ((addr + 16*dir) > end)
				break;


// Get commands from user.
//
			if (shell_read_line( cbuf, "  md> " ))
				break;

			if (*cbuf == 0x1B || *cbuf == 'x') {	// Terminate on ESC
				printf("\n");
				break;
			}

			else if (*cbuf == '^')					// Go reverse direction
				dir = -1;

			else if (*cbuf == 'v')					// Go forward direction
				dir = 1;
		}

// Get next location.

		addr += 16 * dir;
		if (addr > end)
			break;
	}

	return( 0 );
}


//---------------------------------------------------------------------------
//	DumpBlock -- dump a block of memory.
//---------------------------------------------------------------------------
int DumpBlock( PPCSIM *ppcsim, ULONG start, ULONG end, int size )
{

	return( MDEngine( ppcsim, start, end, size, MDE_MDOPTS | MDE_NOIO) );
}


//--------------------------------------------------------------------------
// MMEngine -- a generic memory modification engine which allows
//             access to conventional and non-conventional memory
//             (such as NVRAM accesses through indirect registers).
//--------------------------------------------------------------------------
int MMEngine( PPCSIM *ppcsim, ULONG start, ULONG end, short size, short dopts )
{
	int		stat, edit_info, i, step;
	int		dir, store;
	ULONG	addr, lv;
	long	va;
	char   *e;
	char    mmstring[80];


	addr = start;
	dir  = 1;
	while (1) {
		va = (long) addr;
		printf("  0x%08lx :", va);

		// If NOREAD, print nothing.
		//
		if (dopts & MDE_NOREAD) {
			printf(" ");
			for (i = size*2; i >= 0; i--) {
				printf("-");
			}
		}

		// Use driver to access data.
		//
		else {
			MSS_read( ppcsim, addr, &lv, size );
		

			// Print data using the supplied driver, or a generic one if not
			// supplied.
			//
			switch (size) {
			case ACCESS_B:
				printf(" 0x%02x", lv);
				break;

			case ACCESS_H:
				if (dopts & MDE_BRX)
					lv = pSwap4Bytes( lv ) >> 16;
				printf(" 0x%04x", lv);
				break;

			default:
			case ACCESS_W:
				if (dopts & MDE_BRX)
					lv = pSwap4Bytes( lv );
				printf(" 0x%08x", lv);
				break;
			}
		}

		// Process a command from the user.
		//
		if (shell_read_line( mmstring, " ? " )) {
			break;
		}
		e = mmstring;

		if (*e == 0x1B || *e == 'x'||*e == 'q') {							// Terminate on ESC
			break;

		} else if (*e) {
			store = 0;
			if (isxdigit(*e)) {                                             // Collect a value
				lv = strtoul( mmstring, &e, 16);
				if (e != mmstring) {
					store++;
				}
			}
			else if (*e == '\'') {                                          // Collect string
				for (i = lv = 0, e++; i < size  &&  *e != '\''; i++) {
					lv = (lv << 8) + *e++;
				}
				store++;
			}
			 else if (*e == '\"') {                                          // Collect string
				for (e++; *e  &&  *e != '"'; e++) {
					lv = *e;
					stat = MSS_write( ppcsim, addr, &lv, ACCESS_B );
					addr++;
				}
				addr--;
			}

			if (store) {                                                // Anything to do?
				if (dopts & MDE_BRX) {									// Re-reverse before storing.
					if (size == ACCESS_W)
						lv = pSwap4Bytes( lv );
					else if (size == ACCESS_H)
						lv = pSwap4Bytes( lv << 16 );
				}

				stat = MSS_write( ppcsim, addr, &lv, size );
			}
		}

#if 0
		// Otherwise, 
		//
		if (edit_info == 1) {                                           // Down arrow/ctl-F
			dir = 1;
			printf("\n");
		}

		else if (edit_info == 2) {                                      // Up arrow/ctl-B
			dir = -1;
			printf("\n");
		}


		else if (*e == '^') {                                                   // Go reverse direction
			dir = -1;
		}

		else if (*e == 'v')     {                                               // Go forward direction
			dir = 1;
		}

		else if (e  &&  *e == '=') {                                    // Stay here
			dir = 0;
		}

		else if (*e == '?')                                             // Help
			printf("\n"
									"    <h>   writes data to device/memory\n"
									"    '^'   begins reverse direction\n"
									"    'v'   resumes forward direction\n"
									"    <h>=  writes data and reopens location\n"
									"    ESC/x returns to DINK\n"
									"\n"
									);
#endif
		else if (!*e  ||  *e == '\n'  ||  *e == '\r')
			;


		// Determine the next address.
		//
		step = size;
		//if (me->flags & MMEF_HALFWORD)
		//	step = 1;
		if (addr != 0  ||  dir != -1) {
			addr += (step * dir);
			if (start != end) {             // If not single value, check limits.
				if (addr > end) {
					addr = start;
				}
				else if (addr < start) {
					addr = end;
				}
			}
		}
	}
	printf("\n");

	return( 0 );
}


//--------------------------------------------------------------------------------
//
char *mem_help[] = {
	"  usage: mem [-bhw][-r] addr [addr2]",
	"  where:",
	"          -b   - use byte accessing.",
	"          -h   - use halfword accessing.",
	"          -r   - byte-reverse memory.",
	"          -w   - use word accessing.",
	NULL
};


//--------------------------------------------------------------------------
//	MEM_cmd -- display/modify memory.
//--------------------------------------------------------------------------
int MEM_cmd( PPCSIM *ppcsim, int argc, char *argv[], int r_w )
{
    ULONG	start, end, da;
	int		opt, stat, size, do_help;
	int		i, n, dopts;
	char   *s, b;
	char    devname[30], dnext[20];

	start	 = 0;
	end		 = 0;
	size     = 0;
	da       = 0;
	dopts    = MDE_MDOPTS;


// After a device entered, get optional size accessing for it (-b, -h, -w).
//
	optind	 = 0;		// POSIX
	size	 = ACCESS_W;
	do_help	 = 0;
	dnext[0] = 0;
	if (argc > 1  &&  strcmp( argv[1], "--help" ) == 0)
		do_help++;
	else while ((opt = getopt( argc, argv, "bhwr" )) != -1) {
		switch (opt) {
		case 'b':	size = ACCESS_B;	strcpy( dnext, "-b" );			break;
		case 'h':	size = ACCESS_H;	strcpy( dnext, "-h" );			break;
		case 'w':	size = ACCESS_W;	strcpy( dnext, "-w" );			break;
		case 'r':	dopts |= MDE_BRX;
		            size = ACCESS_W;	strcpy( dnext, "-w -r" );		break;
		default:	return( ERR_INVARG );
		}
	}

	if (do_help)
		return( shell_show_help( mem_help ) );

// Else, 1..2 args are needed for start and end address.
//
	if (optind >= argc)
		return( ERR_NOARG );

// Get start..end
//
	start = end = 0;
	dnext[0] = 0;
	for (n = 0, i = optind; i < argc; i++) {
		s = argv[i];
		if (n == 0) {
			if ((stat = shell_getvalue( ppcsim, &s, &start, 16 )) != 0)
				return( stat );
			if (*s == '=')
				strcpy( dnext, s+1 );
			n++;
		} 
		else if (n == 1) {
			if ((stat = shell_getvalue( ppcsim, &s, &end, 16 )) != 0)
				return( stat );
			n++;
		}
	}
	if (n == 1)
		end = start + 0x40-size;

	if (end == start) {
		start &= 0xFFFFFFF0;
		end = (start & 0xFFFFFFF0) + (0xF - size + 1);
	}
	da = end - start + 4;


// Now display the blocks of memory specified on the command
// line.
//
	if (r_w)
		stat = MDEngine( ppcsim, start, end, size, dopts );

// If no '=' specied, work interactively.  Otherwise just the one
// memory write.
//
	else {
		if (dnext[0]) {
			s = dnext;
			if ((stat = shell_getvalue( ppcsim, &s, &end, 16 )) != 0)
				return( stat );
			stat = MSS_write( ppcsim, start, &end, size );
			
		} else {
			stat = MMEngine( ppcsim, start, end, size, dopts );
		}
	}

	i = (size == ACCESS_B) ? -3 
	  : (size == ACCESS_H) ? -2
	  :					     0;

	sprintf(ppcsim->dot_cmd, "%s %s %x %x", argv[0], dnext, end+size, end+da+i );

	return( stat );
}
