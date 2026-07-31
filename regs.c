// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

/***************************************************************************
 *     Copyright Motorola, Inc. 1989-2001 ALL RIGHTS RESERVED
 *
 *  $Id: regs.c,v 1.1.1.1 2002/04/25 22:20:35 maurie Exp $
 *
 * You are hereby granted a copyright license to use, modify, and
 * distribute the SOFTWARE, also know as DINK32 (Dynamic Interactive Nano 
 * Kernel for 32-bit processors) solely in conjunction with the development 
 * and marketing of your products which use and incorporate microprocessors 
 * which implement the PowerPC(TM) architecture manufactured by 
 * Motorola and provided you comply with all of the following restrictions 
 * i) this entire notice is retained without alteration in any
 * modified and/or redistributed versions, and 
 * ii) that such modified versions are clearly identified as such. 
 * No licenses are granted by implication, estoppel or
 * otherwise under any patents or trademarks of Motorola, Inc.
 * 
 * The SOFTWARE is provided on an "AS IS" basis and without warranty. To
 * the maximum extent permitted by applicable law, MOTOROLA DISCLAIMS ALL
 * WARRANTIES WHETHER EXPRESS OR IMPLIED, INCLUDING IMPLIED WARRANTIES OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE AND ANY WARRANTY 
 * AGAINST INFRINGEMENT WITH REGARD TO THE SOFTWARE 
 * (INCLUDING ANY MODIFIED VERSIONS THEREOF) AND ANY ACCOMPANYING 
 * WRITTEN MATERIALS.
 * 
 * To the maximum extent permitted by applicable law, IN NO EVENT SHALL
 * MOTOROLA BE LIABLE FOR ANY DAMAGES WHATSOEVER (INCLUDING WITHOUT 
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS 
 * INTERRUPTION, LOSS OF BUSINESS INFORMATION,
 * OR OTHER PECUNIARY LOSS) ARISING OF THE USE OR INABILITY TO USE THE
 * SOFTWARE.
 * Motorola assumes no responsibility for the maintenance and support of
 * the SOFTWARE.
 ************************************************************************/

#include "ppcsim.h"


extern int	optind;


//--------------------------------------------------------------------------------
// Register files.
//
ULONG	GPR[ MAX_GPR ];
FREG	FPR[ MAX_FPR ];
VREG	VPR[ MAX_VPR ];


ULONG GPR_get( int no )
{
	return( GPR[no] );
}

void GPR_set( int no, ULONG v )
{
	GPR[no] = v;
}

FPDOUBLE FPR_get( int no )
{
	return( FPR[no].f );
}

void FPR_set( int no, FPDOUBLE *v )
{
	FPR[no].f = *v;
}


void VPR_get( PPCSIM *ppcsim, int reg_no, VREG *v )
{

	if (ppcsim->verbose)
		printf("getVPR: %d = %08x %08x %08x %08x\n", reg_no,
				VPR[reg_no].w[0], VPR[reg_no].w[1],
				VPR[reg_no].w[2], VPR[reg_no].w[3] );

	v->w[0] = VPR[reg_no].w[0];
	v->w[1] = VPR[reg_no].w[1];
	v->w[2] = VPR[reg_no].w[2];
	v->w[3] = VPR[reg_no].w[3];
}


void VPR_set( PPCSIM *ppcsim, int reg_no, VREG *v )
{

	VPR[reg_no].w[0] = v->w[0];
	VPR[reg_no].w[1] = v->w[1];
	VPR[reg_no].w[2] = v->w[2];
	VPR[reg_no].w[3] = v->w[3];

	if (ppcsim->verbose)
		printf("putVPR: %d = %08x %08x %08x %08x\n", reg_no,
				v->w[0],v->w[1],v->w[2],v->w[3]);
}


//---------------------------------------------------------------------------
//	regs_gpr_rd -- display one or more general purpose registers.
//
//  0->0		00000->00000
//  1->8		00001->01000
//  2->16		00010->10000
//  3->24		00011->11000
//  4->1		00100->00001
//  5->9		00101->01001
//  6->17		00110->10001
//  7->25		00111->11001
//
//				Cool!
//---------------------------------------------------------------------------
int regs_gpr_rd( PPCSIM *ppcsim, int start_index, int end_index )
{
	ULONG	rd;
	int		rno, n, cols, col_limit;


// ALIGN_GPR_REGS Used to display registers on an even boundary.
//
#ifdef ALIGN_GPR_REGS
	start_index &= 0xfffe;
	end_index |= 0x0001;
#endif
				
	col_limit = (start_index == 0  &&  end_index == 31) ? 3 : 1;
	cols 	  = 0;
	rno 	  = start_index;
	for (rno = start_index; rno <= end_index; rno++) {
			
		if (col_limit == 3) {
			n = ((rno & 0x03) << 3) | ((rno & 0x1C) >> 2);	// see above
			rd = GPR_get( n );
			printf( "  R%-2d=%08X", n, rd );
		}
		else {
			rd = GPR_get( rno );
			printf( "  R%-2d=%08X", rno, rd );
		}

		if (++cols > col_limit) {
			printf("\n");
			cols = 0;
		}
		else if (col_limit == 3)
			printf( "    " );
		else
			printf( "\t\t" );
	}

	if (cols)
		printf( "\n" );

	return( 0 );
}



#define	RCLASS_GPR  1
#define	RCLASS_FPR	2
#define	RCLASS_VPR 	3
#define	RCLASS_SPR 	4
#define	RCLASS_NB   5
#define	RCLASS_PCI  6
#define	RCLASS_BAT 	7


//---------------------------------------------------------------------------
// REG_getinfo -- extract register family/number information from the text.
//				  r/f/v usually start register names, but watch out for
//				  named SPRs.
//---------------------------------------------------------------------------
int REG_getinfo( char **ri, int *class, int *regno )
{
	int status;
	char *rs;

	rs = *ri;

// See if it is an SPR.  For example, 'vscr' or 'sr0'.

	if ((status = SPR_search( ri, regno )) == 0) {
		*class = RCLASS_SPR;
		return( status );
	}

	if (tolower(*rs) == 'r') {
		rs++;
		if (('0' <= *rs)  &&  (*rs <= '9')) {
			*regno  = strtol( rs, &rs, 10 );
			*class = RCLASS_GPR;
			if (*regno >= MAX_GPR)
				return( 1 );
		}
		else
			return( 1 );
	}

	else if (tolower(*rs) == 'f') {
		rs++;
		if (('0' <= *rs)  &&  (*rs <= '9')) {
			*regno  = strtol( rs, &rs, 10 );
			*ri     = rs;
			*class = RCLASS_FPR; 
			if (*regno >= MAX_FPR)
				return( 1 );
		}
		else
			return( 1 );
	}

	else if (tolower(*rs) == 'v') {
		rs++;
		if (('0' <= *rs)  &&  (*rs <= '9')) {
			*regno  = strtol( rs, &rs, 10 );
			*ri     = rs;
			*class = RCLASS_VPR;
			if (*regno >= MAX_VPR)
				return( 1 );
		}
		else
			return( 1 );
	}

// Don't know what it is...

	else
		return( 1 );

	*ri = rs;
	return( 0 );
}


//-------------------------------------------------------------------------
//	REG_get_range -- parse command line into a range (1..n) of registers
//					 to work on.
//-------------------------------------------------------------------------
int REG_get_range( PPCSIM *ppcsim, char **arg_term, int *class, int *sreg, int *ereg,
					  int *sfield, int *efield, char *dispopts )
{
	int    eclass, status;
	char  *term;
  

// First, try to match the special register names:
//
//	 r f v s bats sprs
//
	term    = *arg_term;
	*sreg   =  0;
	*ereg   =  0;
	*sfield =  0;
	*efield = 31;


// If "BAT" is entered, show all the BAT registers.

	if (!strcasecmp(term, "bats")) {
		*arg_term = term + 4;
		*class = RCLASS_BAT;
	    return( 0 );
	}

// List all registers in a particular family?  R, F, V or S?

	if (!strcasecmp(term, "r")) {
		*arg_term = ++term;
		*class = RCLASS_GPR;
		*ereg   = 31;
	    return( 0 );
	}

	if (!strcasecmp(term, "f")) {
		*arg_term = ++term;
		*class = RCLASS_FPR;
		*ereg   = 31;
	    return( 0 );
	}

	if ((strcasecmp(term, "s") == 0) || (strncasecmp(term, "spr", 3) == 0)) {
		*arg_term = ++term;
		*class = RCLASS_SPR;
		*ereg  = 9999;					// Sufficient to trigger SPR list
	    return( 0 );
	}


// Check for vector registers, if supported.

	if (!strcasecmp(term, "v")) {
		if (ppcsim->process_type == CPU_MPC7400  ||  ppcsim->process_type == CPU_MPC7410  
		||  ppcsim->process_type == CPU_MPC7450) {
			*arg_term = ++term;
			*class = RCLASS_VPR;
			*ereg  = MAX_VPR-1;
			return( 0 );

		} else
			return( ERR_NOVECTOR );
	}


//---------------------------------------------------------------------------
// Not generic like "r" or "f", try "rxx" or "fxx" individual
// register names.

	if ((status = REG_getinfo( &term, class, sreg )) != 0) {
		return( status );
	}


//---------------------------------------------------------------------------
// If SPR, allow field widths.  If found, force the display into
// field mode.

	if (*class == RCLASS_SPR) {
		if (*term == '.'  ||  *term == '[') {
			term++;
			if (SPR_get_fields( ppcsim, *sreg, &term, sfield, efield ) == 0) {
				*dispopts = RD_OPT_FIELDS;
			}
			if (*term == ']')
				term++;
		}
		*arg_term = term;
	}


//---------------------------------------------------------------------------
// Plus indicates a start at the specified register, if any,
// to the end.  So, 'rd r', 'rd r0+' and 'rd r0-r31' are all
// the same.

	if (*term == '+') {
		switch (*class) {
		case RCLASS_SPR:		*ereg = MAX_SPR - 1;	break;
		case RCLASS_GPR:		*ereg = MAX_GPR - 1;	break;
		case RCLASS_FPR:		*ereg = MAX_FPR - 1;	break;
		case RCLASS_VPR:		*ereg = MAX_VPR - 1;	break;
		default:				return( 1 );
		}

		*arg_term = ++term;
		return( 0 );
	}


//-----------------------------------------------------------------------
// Lastly, consider rxx-ryy.

	if (*term != '-') {					// No '-', just one reg.
		*ereg = *sreg;
		*arg_term = term;
		return( 0 );
	}

	term++;
	if ((status = REG_getinfo( &term, &eclass, ereg )) != 0)
		return( status );


// If it is a range, make sure we don't cross families (viz, 'rd r0-f5').

	if (*class != eclass)
		return( 2 );			// RANGE_CROSS_FAMILY

	*arg_term = term;
	return( 0 );
}


//---------------------------------------------------------------------------
//	regs_gpr_rm -- modify registers from start to end.
//---------------------------------------------------------------------------
int regs_gpr_rm( PPCSIM *ppcsim, char **args, int start_index, int end_index)
{
	int	    new_data;
	int     i, action;
	char   *jam;

	jam = *args;			// Save argument.

	for (i = start_index; i <= end_index; i++) {
		*args = jam;
		new_data = GPR_get( i );

		if (*jam == '=')			// Command-line set?
			;
		else
			printf("  R%02d = %08X", i, new_data );

	
// Collect from command line or user.  Write only if something obtained.
//	
		if ((action = shell_getarg_or_prompt( ppcsim, args, " ? ", &new_data )) == -1) {
			printf("\n");
			break;
		}
		else if (action == 1)
			GPR_set( i, new_data );
	}

	return( 0 );
}

		
//---------------------------------------------------------------------------
// reg_rd_f -- display FP register.
//---------------------------------------------------------------------------
int reg_rd_f( PPCSIM *ppcsim, int start_index, int end_index )
{
   union r2i {
			FPDOUBLE register_data;
			long l[2];
	} FPR;
	int     rno, n, cols, col_limit;


// ALIGN_FPR_REGS Used to display registers on an even boundary.

#ifdef ALIGN_FPR_REGS
	start_index &= 0xfffe;
	end_index |= 0x0001;
#endif

	col_limit = (start_index == 0  &&  end_index == 31) ? 1 : 0;
	cols      = 0;
	rno       = start_index;
	for (rno = start_index; rno <= end_index; rno++) {

//  0->0                00000->00000            // see below
//  1->16               00001->10000
//  2->1                00010->00001
//  3->17               00011->10001
//  4->2                00100->00010
//  5->18               00101->10010
//  6->3                00110->00011
//  7->19               00111->10011

		if (col_limit == 1)
			n = ((rno & 0x01) << 4) | ((rno & 0x1E) >> 1);  // see rd_r
		else
			n = rno;

		FPR.register_data = FPR_get( n );

		printf("  F%-2d=%08lX_%08lx", n, FPR.l[0], FPR.l[1]);
		if (col_limit == 0)
				printf("=%20.8f", FPR.register_data);

		if (++cols > col_limit) {
				printf("\n");
				cols = 0;
		}
		else if (col_limit == 1)
				printf( "    " );
			else
				printf( "\t\t" );
	}
	if (cols)
		printf( "\n" );

	return( 0 );
}



//---------------------------------------------------------------------------
//	reg_rm_f -- modify registers from start to end.
//---------------------------------------------------------------------------
int reg_rm_f( PPCSIM *ppcsim, int start_index, int end_index )
{
	int     stat, i;
	char    string[30], *s, *end;
	union r2i {
		FPDOUBLE register_data;
		long l[2];
	} FPR;
	int		string_length;
	char	temp_string[BUFSIZ];


	i = start_index;
	while (1) {
		FPR.register_data = FPR_get( i );

		printf("  F%-2d=%08lX_%08lX ? ", i, FPR.l[0], FPR.l[1] );

		shell_read_line( string, "rmf>" );
		if (!*string  ||  (*string == '\n'))    // blank -- next reg #
			;

		else if (*string == 0x1B)
			break;

		else {                                                                          // non-blank, a value

// Convert text into 1 or 2 ULONGs.  If the string is > 8 characters,
// we need two ULONGS.
//
			FPR.l[0] = 0;
			s = string;
			string_length = strlen(s);
			if (string_length > 8) {

// Put n-8 in a temp buffer for the upper half.
//
				strncpy( temp_string, s, string_length-8 );
				temp_string[ string_length-8 ] = 0;
				FPR.l[0] = strtoul( temp_string, &end, 16 );

				s += (string_length - 8);
			}

// Now there are 8 chars or less, and it goes in the second ULONG.
//
			FPR.l[1] = strtoul( s, &end, 16 );

			if (end != string)
				FPR_set( i, &FPR.register_data );
		}
		if (++i > end_index)
			break;
	}

	return( 0 );
}


//---------------------------------------------------------------------------
// reg_vprint -- print a vector.
//---------------------------------------------------------------------------
void reg_vprint( int rno, VREG *v, int size, int partial )
{
	int i;


	printf("  V%02d", rno);
	if (partial >= 0)
		printf(".%d= ", partial);
	printf("= ");

	if (size == ACCESS_V)
		printf("0x%08X_%08X_%08X_%08X", v->w[0], v->w[1], v->w[2], v->w[3]);

	else if (size == ACCESS_W) {
		for (i = 0; i <= 3; i++)
			if (partial < 0  ||  i == partial)
				printf("%08X ", v->w[i]);
	}

	else if (size == ACCESS_H) {
		for (i = 0; i <= 7; i++)
			if (partial < 0  ||  i == partial)
				printf("%04X ", v->h[i]);
	}

	else if (size == ACCESS_B) {
		for (i = 0; i <= 15; i++)
			if (partial < 0  ||  i == partial)
				printf("%02X ", v->b[i]);
	}
	else if (size == ACCESS_FS) {
		for (i = 0; i <= 3; i++)
			if (partial < 0  ||  i == partial)
				printf("%16.8f ", v->f[i]);
	}
}


//---------------------------------------------------------------------------
// reg_vprint_no -- get and print v
//---------------------------------------------------------------------------
void reg_vprint_no( int reg_num )
{
	VREG *v;

	v = &VPR[ reg_num ];
	reg_vprint( reg_num, v, ACCESS_V, -1 );
}


//---------------------------------------------------------------------------
// reg_rd_v --
//---------------------------------------------------------------------------
int reg_rd_v( PPCSIM *ppcsim, int start_index, int end_index, int size )
{
	VREG	vr;
	int	    rno;
	
	for (rno = start_index; rno <= end_index; rno++) {
		VPR_get( ppcsim, rno, &vr );
		reg_vprint( rno, &vr, size, -1 );
		printf("\n");
	}

	return( 0 );
}


//---------------------------------------------------------------------------
//	reg_rm_v -- modify registers from start to end.
//---------------------------------------------------------------------------
int reg_rm_v( PPCSIM *ppcsim, int start_index, int end_index, int size )
{
	int     i, sub_index, n, x, stat;
	VREG    vr;
	char    string[80], *s, tmp_str[30], *end;

	stat	  = 0;
	sub_index = 0;
	for (i = start_index; i <= end_index; ) {
		VPR_get( ppcsim, i, &vr );
		reg_vprint( i, &vr, size, (size == ACCESS_V) ? -1 : sub_index );

		shell_read_line( string, " ?" );
		s = string;

		if (!*s  ||  (*s == '\n'))                              // blank -- next reg #
			;

		else if (*s == 0x1B)                                    // ESC -- done.
			break;

		else if ((stat = is_valid_hex_input(s)) == 0) {
			if (size != ACCESS_V) {                         // Sized modify
				if (size == ACCESS_B)
					vr.b[sub_index] = strtoul( s, &end, 16 );
				else if (size == ACCESS_H)
					vr.h[sub_index] = strtoul( s, &end, 16 );
				else
					vr.w[sub_index] = strtoul( s, &end, 16 );
			}
			else {                                                          // Unsized -- all data used
				for (n = 0; n < 4; n++) {
					for (x = 0; x < 8  &&  *s; ) {
						if (*s == '_'  ||  *s == '.') {
							s++;                            // Stop early on '_'
							break;
						}
						tmp_str[x++] = *s++;
					}
					tmp_str[x] = 0;
					if (x == 0)
						break;

					vr.w[0] = vr.w[1];
					vr.w[1] = vr.w[2];
					vr.w[2] = vr.w[3];
					vr.w[3] = strtoul( tmp_str, &end, 16 );
				}

				VPR_set( ppcsim, i, &vr );
			}
		}
		else                                                                    // bad input
			return( ERR_BADVAL );

// Point to the next vector, or the next piece of it.
//
		if (size == ACCESS_V)
			i++;
		else if (++sub_index >= (16/(size & 0x1F))) {
			i++;
			sub_index = 0;
		}
	}

	return( stat );
}


//--------------------------------------------------------------------------------


char *rd_help[] = {
	"usage: rd [-vebhw]{[r|rx|rx+|rx-ry|f|fx|fx+|fx-fy|sx|v|vx|vx+|vx-vy|\n"
	"             sx|'bats|<spr_name>|<spr_name>'.'<field_name]} ...",
	"",
	"  The RD command displays the contents of the specified registers.",
	"'R', 'F', 'V', 'S' or 'NB' will display all registers of the",
	"corresponding register files.",
	"",
	"  If the \"+\" form is used, the command will display the",
	"registers in the family starting with the specified register.",
	"  If the \"-\" form is used, the command will display all the",
	"registers specified in the range.",
	"  If the \"'.'field_name\" option is used, display is limited to",
	"the field named.",
	"",
	"Options:",
	"  '-e'   expands and explains SPR/NB fields",
	"  '-v'   forces the verbose mode if overridden by ENV RDOPT",
	"  '-b'   displays registers as a byte",
	"  '-h'   displays registers as a halfword",
	"  '-w'   displays registers as a word",
	"",
	"Examples:",
	"  \"rd r6\"        displays the contents of general register 6.",
	"  \"rd f4-f7, s8\" displays the contents of floating point registers",
	"                 4 through 7 and special purpose register 8.",
	"  \"rd v3\"        display vector V3.",
	"  \"rd bats\"      displays and decodes all BAT registers",
	"  \"rd sprs\"      displays all SPR registers",
	NULL
};


//-------------------------------------------------------------------------
//	reg_rdm -- register display/modify
//-------------------------------------------------------------------------
int reg_rdm( PPCSIM *ppcsim, int argc, char *argv[], int r_w )
{
	int	   class;				// register family - spr, gpr, fpr
	int	   stat, size, vsize;
	int    sreg, ereg, opt;
	int    sfield, efield, do_help;
	char   dopt;
	char   *r, rterm[40];


// Collect arguments.
//	
	size    = ACCESS_W;
	vsize	= ACCESS_V;
	dopt    = 0;		// regdisp_opt;
	optind  = 0;		// POSIX
	do_help	= 0;
	if (argc > 1  &&  strcmp( argv[1], "--help" ) == 0)
		do_help++;

	else while ((opt = getopt( argc, argv, "bdhwefv" )) != -1) {
		switch (opt) {
		case 'b':	size = ACCESS_B;			break;
		case 'h':	size = ACCESS_H;			break;
		case 'w':	size = ACCESS_W;			break;
		case 'd':	vsize = ACCESS_FD;			break;

		case 'v':
		case 'e':	dopt = RD_OPT_VERBOSE;		break;
		case 'f':	dopt = RD_OPT_FIELDS;		break;

		default:	do_help = 1;				break;
		}
	}


// For each additional term, display register(s).
//	
	stat = 0;
	for (;!do_help  &&  optind < argc; optind++) {
		r = argv[optind];

		// Otherwise, must be one or more registers.
		//
		stat = REG_get_range( ppcsim, &r, &class,
								&sreg, &ereg, &sfield, &efield, &dopt );

		if (stat == 0)
			switch( class ) {
			case RCLASS_GPR:
				if (r_w == 0)
					stat = regs_gpr_rm( ppcsim, argv, sreg, ereg );
				else
					stat = regs_gpr_rd( ppcsim, sreg, ereg );
				break;

			case RCLASS_FPR:
				if (r_w == 0)
					return( reg_rm_f( ppcsim, sreg, ereg ) );
				else
					return( reg_rd_f( ppcsim, sreg, ereg ) );
				break;

			case RCLASS_VPR:
				if (ppcsim->process_type == CPU_MPC7400  ||  ppcsim->process_type == CPU_MPC7410  
				||  ppcsim->process_type == CPU_MPC7450) {
					if (r_w == 0)
						return( reg_rm_v( ppcsim, sreg, ereg, vsize ) );
					else
						return( reg_rd_v( ppcsim, sreg, ereg, vsize ) ); 
				}
				else
					stat = 1; 
				break;

			case RCLASS_SPR: 
				if (ereg == 9999)
					stat = SPR_list( ppcsim );
				else if (ereg != sreg)
					;				// range not supported
				else {
					stat = SPR_rw( ppcsim, &r, sreg, r_w, sfield, efield, dopt ); 
				}
				break;

			case RCLASS_NB:
			     //stat = rdm_nb_regs( &argv, ws_modify, dopt, size );
			     break;

			case RCLASS_BAT:
				if (!r_w)
					return( 7 );
				stat = SPR_bats_rd( ppcsim, dopt );
				break;
			}

		if (stat != 0)
			break;
	}

	if (do_help)
		shell_show_help( rd_help );

	return( stat );
}

