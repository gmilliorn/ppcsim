// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

// SPDX-License-Identifier: MIT

/*
 * Copyright (c) 2026 Gary Milliorn
 * All rights reserved.
 */


//--------------------------------------------------------------------------------
//	SYMBOL -- 
//
//		load and access symbols.  typically loaded from a symbol table,
//
//		to get a symbol file:
//			powerpc-linux-gnu-objdump -t dinkyk >dinkyk.sym
//
//		to get a symbol file:
//			powerpc-linux-gnu-objdump -d -S -l dinkyk >dinkyk.lst
//--------------------------------------------------------------------------------


#include "ppcsim.h"


//--------------------------------------------------------------------------------
// Optionally include Excimer DINK symbols.  Just a convenience.
//
#ifdef ADD_DEFAULT_SYMBOLS
SYMBOL default_syms[] = {
	{	{0,0},	"system_reset",	   0x33c8, SYM_ATTR_I	},
	{	{0,0},	"try_excimer",	   0x33ec, SYM_ATTR_I	},
	{	{0,0},	"excimer_main",	   0x3844, SYM_ATTR_I	},
	{	{0,0},	"lp1",			   0x3884, SYM_ATTR_I	},
	{	{0,0},	"verlp",		   0x38c4, SYM_ATTR_I	},
	{	{0,0},	"dink_loop",	   0x4f98, SYM_ATTR_I	},
	{	{0,0},	"init_IO_values",  0x56a4, SYM_ATTR_I	},
	{	{0,0},	"dink_initialize", 0x58c8, SYM_ATTR_I	},
	{	{0,0},	"main",			   0x5d40, SYM_ATTR_I	},
	{	{0,0},	"io_base_addr",	  0x2ca04, SYM_ATTR_D	},
	{	{0,0},	"board_type",	  0x2ca10, SYM_ATTR_D	}
};
#endif


//--------------------------------------------------------------------------------
// symbol_new -- create a new symbol.  It is not linked to anything.
//--------------------------------------------------------------------------------
SYMBOL *symbol_new( char *name, ULONG val )
{
	SYMBOL *s;

	if ((s = (SYMBOL *) malloc( sizeof(SYMBOL) )) == NULL)
		return( NULL );
	c_list_init( (CList *) s);

	s->name	= NULL;
	s->val	= val;

	if (name != NULL)
		s->name = (char *) strdup( name );
	
	return( s );	
}


//--------------------------------------------------------------------------------
// symbol_get_byaddr -- return symbol matching address.
//--------------------------------------------------------------------------------
char *symbol_get_byaddr( PPCSIM *ppcsim, ULONG addr )
{
	SYMBOL *s;
	CList  *iter;

	c_list_for_each( iter, (CList *) ppcsim->symbols ) {
		s = (SYMBOL *) iter;
		if ((s->val & ppcsim->sym_mask) == (addr & ppcsim->sym_mask))
			return( s->name );
	}

	return( NULL );
}


//--------------------------------------------------------------------------------
// symbol_get_byname -- return symbol matching address.
//--------------------------------------------------------------------------------
SYMBOL *symbol_get_byname( PPCSIM *ppcsim, char *name, ULONG *v )
{
	SYMBOL *s;
	CList  *iter;

	c_list_for_each( iter, (CList *) ppcsim->symbols ) {
		s = (SYMBOL *) iter;
		if (strcmp(s->name, name) == 0) {
			*v = s->val;
			return( s );
		}
	}
	return( NULL );
}


//--------------------------------------------------------------------------------
// symbol_add -- create and add a new symbol.
//--------------------------------------------------------------------------------
int symbol_add( PPCSIM *ppcsim, char *str, ULONG val, int attr )
{
	SYMBOL *s, *x;
	ULONG	v;

	if ((x = symbol_get_byname( ppcsim, str, &v )) != NULL) {		// sym already defined.
		ppcsim->sym_dups += 1;
		return( 0 );	
	}
	if ((s = symbol_new( str, val )) == NULL)
		return( ERR_NOMEM );
	s->attr = attr;

	c_list_link_tail( (CList *) ppcsim->symbols, (CList *) s );
	return( 0 );
}


//--------------------------------------------------------------------------------
// symbols_load_gccsym -- load symbols from GCC symbol table.
//	
// the format is not terribly consistent.
//--------------------------------------------------------------------------------
int symbols_load_gccsym( PPCSIM *ppcsim, char *filename, int filter_level )
{
	FILE   *fp;
	int		look_hdr, lineno, stat, newdefs;
	ULONG   v;
	char   *s, *e, *lv, *lt, *f3, *f4, *f5, *f6;
	char	buf[BUFSIZ];

	if ((fp = fopen( filename, "r")) == NULL)
		return( ERR_FILE );

	ppcsim->sym_dups	= 0;

	look_hdr	= 1;
	lineno		= 0;
	stat		= 0;
	newdefs		= 0;
	while ((s = fgets( buf, BUFSIZ, fp )) != NULL) {
		buf[strlen(buf)-1] = 0;
		lineno++;
		if (look_hdr) {
			if (strncmp(buf, "SYMBOL TABLE", 12) == 0) {
				look_hdr = 0;
				continue;
			}
			// header junk, ignore.
		}
		else {
			if (!buf[0]  ||  isspace(buf[0]))
				continue;
			lv  = strtok(buf, " \t");		// nominal value
			lt  = strtok(NULL, " \t");		// type: l
			f3  = strtok(NULL, " \t");
			f4  = strtok(NULL, " \t");
			f5  = strtok(NULL, " \t");
			f6  = strtok(NULL, " \t");
			v   = strtoul( lv, &e, 16 );

			if (ppcsim->verbose)
				printf("  parse lv='%s'  lt='%s'  f3='%s'  f4='%s'  f5='%s'\n", lv, lt, f3, f4, f5);

			// If the third token is "d" or "df" it describes a section.
			// this might be useful, not sure.
			//
			if (strcmp( f3, "d" ) == 0  ||  strcmp(f3, "df") == 0) {
				if (strcmp(f4,"*ABS*") == 0)
					continue;								// not a sect.
				if (ppcsim->verbose)
					printf("  section def: %s starts at %08X\n", f4, v );

			// .text with or without "F" field.
			//
			} else if ( strcmp( f3, ".text" ) == 0) {
				if ((int) v < filter_level)
					continue;
				if ((stat = symbol_add( ppcsim, f5, v, SYM_ATTR_I )) != 0)
					break;
				newdefs++;

			} else if ((strcmp( f3, "F" ) == 0) && (strcmp( f4, ".text" ) == 0)) {
				if ((int) v < filter_level)
					continue;
				if ((stat = symbol_add( ppcsim, f6, v, SYM_ATTR_I )) != 0)
					break;
				newdefs++;

			// .data with or without "G" field.
			//
			} else if ( strcmp( f3, ".data" ) == 0) {
				if ((int) v < filter_level)
					continue;
				if ((stat = symbol_add( ppcsim, f5, v, SYM_ATTR_D )) != 0)
					break;
				newdefs++;

			} else if (((strcmp( f3, "G" ) == 0) && (strcmp( f4, ".data" ) == 0))
			       ||  ((strcmp( f3, "O" ) == 0) && (strcmp( f4, ".data" ) == 0))) {
				if ((int) v < filter_level)
					continue;
				if ((stat = symbol_add( ppcsim, f6, v, SYM_ATTR_D )) != 0)
					break;
				newdefs++;

			// .bss or .sbss with "O" field.
			//
			} else if (((strcmp( f3, "O" ) == 0) && (strcmp( f4, ".bss"  ) == 0))
			       ||  ((strcmp( f3, "O" ) == 0) && (strcmp( f4, ".sbss" ) == 0))) {
				if ((int) v < filter_level)
					continue;
				if ((stat = symbol_add( ppcsim, f6, v, SYM_ATTR_D )) != 0)
					break;
				newdefs++;

			// *ABS* definitions.
			//
			} else if (strcmp( f3, "*ABS*" ) == 0) {
				if ((int) v < filter_level)
					continue;
				if ((stat = symbol_add( ppcsim, f5, v, 0 )) != 0)
					break;
				newdefs++;

			} else {
				if (ppcsim->verbose > 1)
					printf("  ppcsim.SYM : %s/%s/%s/%s/%s/%s ignored\n", lv,lt,f3,f4,f5,f6 );
			}
		}
		if (stat)
			break;
	}
	fclose( fp );

	printf("  ppcsim.SYM : %d symbols added, %d dups skipped.\n", newdefs, ppcsim->sym_dups );

	return( stat );
}



//--------------------------------------------------------------------------------
//  SYM_where -- find closest symbol <= address.
//--------------------------------------------------------------------------------
int SYM_where( PPCSIM *ppcsim, ULONG addr, int opt )
{
	SYMBOL *s, *closest;
	CList  *iter;
	ULONG   opcode;
	int	    i,	d, delta;


	closest	= NULL;
	delta	= 0xFFFFFF;
	c_list_for_each( iter, (CList *) ppcsim->symbols ) {
		s = (SYMBOL *) iter;
		if (s->val < addr) {			// candidates must be < addr
			d = addr - s->val;
			if (d < delta) {			// closer
				closest = s;
				delta	= d;
			}
		}
	}

// If found report code there and here.
//
	if (closest) {
		MSS_read( ppcsim, closest->val, &opcode, 4 );
		disassemble_one( ppcsim, opcode, closest->val, 1 );
		printf("...\n");
		MSS_read( ppcsim, ppcsim->CIA, &opcode, 4 );
		disassemble_one( ppcsim, opcode, ppcsim->CIA, 1 );
		return( 0 );
	}

	return( -1 );
}


//--------------------------------------------------------------------------------
//  SYM_list -- list all/matching symbols.
//					NOTE: s may be NULL.
//--------------------------------------------------------------------------------
int SYM_list( PPCSIM *ppcsim, char *pat )
{
	SYMBOL *s;
	CList  *iter;
	int	    i;


	printf("  NAME                            VAL       VAL(dec)  TYPE\n"
		   "  ==============================  ========  ========  ====\n");
	i = 0;
	c_list_for_each( iter, (CList *) ppcsim->symbols ) {
		i++;
		s = (SYMBOL *) iter;
		if ((pat != NULL)  &&  (strstr(s->name, pat) == NULL))
			continue;
		printf("  %-30s  %8X  %8d    ", s->name, s->val, s->val );
		if (s->attr == SYM_ATTR_I)	printf("I");
		if (s->attr == SYM_ATTR_D)	printf("D");
		printf("\n");
	}
	printf("  ==============================  ========  ========  ====\n");
	printf("  %d symbols\n", i);

	return( 0 );
}


//--------------------------------------------------------------------------------
//  SYM_free -- free all symbols.  Either on shutdown or loading new symbols.
//--------------------------------------------------------------------------------
int SYM_free( PPCSIM *ppcsim )
{
	CList  *iter, *safe;
	SYMBOL *s;

	c_list_for_each_safe( iter, safe, (CList *) ppcsim->symbols) {
		c_list_unlink_stale(iter);
		s = (SYMBOL *) iter;
		if (s->name)
			free(s->name);
		free( iter );
	}

	return( 0 );
}


char *symbols_help[] = {
	"usage: sym [-d name=val] [-D name=val] [-f symfile] [-l[pat]] [-n no] [-r]",
	"where:",
	"          -d name=val   - create code symbol of 'name' with 'val'.",
	"          -D name=val   - create data symbol of 'name' with 'val'.",
	"          -f symfile    - GCC symbol listing file.",
	"          -l [pat]      - list all symbols, or those matching 'pat'.",
	"          -n no         - filter symbols with values < n (default = 100)",
	"          -r            - remove all symbols",
	NULL
};


//--------------------------------------------------------------------------------
//  SYM_cmd -- symbol management.
//--------------------------------------------------------------------------------
int SYM_cmd( PPCSIM *ppcsim, int argc, char *argv[] )
{
	int		opt, stat, sym_filter;
	int		do_def, do_list, do_file, do_help, do_reset;
	ULONG   v;
	char   *name, *def;
	char	newdef[BUFSIZ], filename[BUFSIZ];

	sym_filter	= 100;
	optind		= 0;		// POSIX
	do_def		= 0;
	do_list		= 0;
	do_file		= 0;
	do_help		= 0;
	do_reset	= 0;

	if (argc > 1  &&  strcmp( argv[1], "--help" ) == 0)
		do_help++;
	else while ((opt = getopt( argc, argv, "d:D:f:ln:r" )) != -1) {
		switch (opt) {
		case 'd':	strcpy(newdef, optarg);
					do_def = SYM_ATTR_I;
					break;
		case 'D':	strcpy(newdef, optarg);
					do_def = SYM_ATTR_D;
					break;
		case 'f':	strcpy(filename, optarg);
					do_file = 1;
					break;
		case 'l':	do_list = 1;
					break;
		case 'n':	sym_filter = atoi(optarg);
					break;
		case 'r':	do_reset = 1;
					break;
		default:	return( ERR_BADVAL );
		}
	}

	if (do_help)
		return( shell_show_help( symbols_help ) );

// If nothing specified, do list.
//
	if (do_def == 0)
		do_list = 1;

// -r -- reset symbols list.
//
	if (do_reset) {
		SYM_free( ppcsim );
		return( 0 );
	}

// -d name=value
//
	if (do_def) {
		name = strtok(newdef, "=");
        def  = strtok(NULL, "");
		if ((stat = shell_getvalue( ppcsim, &def, &v, 16 )) != 0)
			return( stat );
		if ((stat = symbol_add( ppcsim, name, v, do_def )) != 0)
			return( stat );
		return( 0 );	
	}

// -f file -- load GCC symbol file.
//
	if (do_file)
		return( symbols_load_gccsym( ppcsim, filename, sym_filter ) );

// Default: list symbols.
//
	stat = SYM_list( ppcsim, argv[optind] );

	return( stat );
}


//--------------------------------------------------------------------------------
// SYM_init --
//--------------------------------------------------------------------------------
int SYM_init( PPCSIM *ppcsim )
{

	ppcsim->sym_head.name = NULL;

	ppcsim->symbols = (SYMBOL *) c_list_init( (CList *) &ppcsim->sym_head );

#ifdef ADD_DEFAULT_SYMBOLS
	SYMBOL *s;
	int     i;

	for (i = 0; i < sizeof(default_syms)/sizeof(SYMBOL); i++) {
		s = &default_syms[i];
		symbol_add( ppcsim, s->name, s->val, s->attr );
	}
#endif

	return( 0 );
}


//--------------------------------------------------------------------------------
// SYM_deinit --
//--------------------------------------------------------------------------------
int SYM_deinit( PPCSIM *ppcsim )
{

	SYM_free( ppcsim );

	return( 0 );
}
