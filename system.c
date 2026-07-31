// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

// SPDX-License-Identifier: MIT

/*
 * Copyright (c) 2026 Gary Milliorn
 * All rights reserved.
 */


//--------------------------------------------------------------------------------
//	SYS -- SYSTEM 
//
//		functions to define and manage system archtecture.
//
//		generally SYS loads and parses a system definition file or command
//		line to create a replica of the target system.
//
//--------------------------------------------------------------------------------


#include "ppcsim.h"


//================================================================================
//								SYSTEM DEFINE
//================================================================================


//--------------------------------------------------------------------------------
// get_scalar -- get SI unit scalars for memory size, etc.
//--------------------------------------------------------------------------------
int get_scalar( char **sp )
{
	int   v;
	char *s;

// Pedantry option.
//
#ifdef MB_IS_1024
	v = 1024;
#else
	v = 1000;
#endif

	s = *sp;
	if (strcmp(s, "KB" ) == 0)	{	*sp = s+2;	return(			     v );	}
	if (strcmp(s, "MB" ) == 0)	{	*sp = s+2;	return(         v*   v );	}
	if (strcmp(s, "GB" ) == 0)	{	*sp = s+2;	return(    v*   v*   v );	}
	if (strcmp(s, "KiB") == 0)	{	*sp = s+2;	return(			  1024 );	}
	if (strcmp(s, "MiB") == 0)	{	*sp = s+2;	return(      1024*1024 );	}
	if (strcmp(s, "GiB") == 0)	{	*sp = s+2;	return( 1024*1024*1024 );	}

	return( 1 );
}


//--------------------------------------------------------------------------------
// SYS_new -- create a new (empty) SYSMAP entry.
//--------------------------------------------------------------------------------
SYSMAP *SYS_new( PPCSIM *ppcsim )
{
	SYSMAP *m;

	if ((m = (SYSMAP *) malloc( sizeof(SYSMAP) )) == NULL)
		return( NULL );
	c_list_init( (CList *) m);

	m->mclass	= 0;
	m->name		= NULL;
	m->start	= 0;
	m->end		= 0;
	m->mask		= 0;
	m->mode		= 0;
	m->mem		= NULL;
	m->memsize	= 0;
	m->driver	= 0;
	
	m->s_memsize[0]	= 0;

	c_list_link_tail( (CList *) ppcsim->system, (CList *) m );

	return( m );	
}


//--------------------------------------------------------------------------------
//  SYS_free -- free all system defs.
//--------------------------------------------------------------------------------
int SYS_free( PPCSIM *ppcsim )
{
	CList  *iter, *safe;
	SYSMAP *m;

	c_list_for_each_safe( iter, safe, (CList *) ppcsim->system) {
		c_list_unlink_stale(iter);
		m = (SYSMAP *) iter;
		if (m->mem)
			free(m->mem);
		if (m->name)
			free(m->name);
		free( iter );
	}

	return( 0 );
}


//--------------------------------------------------------------------------------
// Parsing terms.
//
NAMEVAL sys_classes_list[] = {
	{	"MEM",			SYS_CLASS_MEM		},
	{	"IO",			SYS_CLASS_IO		},
	{	"IGNORE",		SYS_CLASS_IGNORE	},
	{	"ALIAS",		SYS_CLASS_ALIAS		},
	{	"TRAP",			SYS_CLASS_TRAP		}
};

NAMEVAL sys_modes_list[] = {
	{	"RW",			SYS_TYPE_RW			},
	{	"RO",			SYS_TYPE_RO			},
	{	"HW",			SYS_TYPE_HW			},
	{	"ALIAS",		SYS_TYPE_ALIAS		}
};

NAMEVAL sys_drivers_list[] = {
	{	"-",			0					},
	{	"UART-EXC",		SYS_DRV_UART_EXC	},
	{	"UART-SIO",		SYS_DRV_UART_SIO	},
	{	"UART-8240",	SYS_DRV_UART_8240	},
	{	"NB",			SYS_DRV_NB			},
	{	"NBIO",			SYS_DRV_NBIO		}
};


//--------------------------------------------------------------------------------
// SYS_parse_def -- parse a system definition and create new SYSMAP entry.
//--------------------------------------------------------------------------------
int SYS_parse_def( PPCSIM *ppcsim, char *buf )
{
	SYSMAP	*m;
	int		 i, stat, lineno, mode;
	char    *s, *e, *s_type, *s_start, *s_end, *s_mask, *s_mode, *s_alloc, *s_driver, *s_name;

	if (!buf[0]  ||  isspace(buf[0])  ||  buf[0] == '#')
		return( 0 );

	stat	= 0;
	s_type	= strtok(buf,  " \t");		// nominal value
	s_start	= strtok(NULL, " \t");		// type
	s_end	= strtok(NULL, " \t");		// type
	s_mask	= strtok(NULL, " \t");		// mask
	s_mode	= strtok(NULL, " \t");		// mode
	s_alloc	= strtok(NULL, " \t");		// alloc
	s_driver= strtok(NULL, " \t");		// driver
	s_name	= strtok(NULL, "");			// name (rest of line)


	if (ppcsim->verbose)
		printf("  ppcsim.SYS :  type=%s  start=%s  end=%s  mask=%s  mode=%s  alloc=%s  driver=%s  name=%s\n",
				s_type, s_start, s_end, s_mask, s_mode, s_alloc, s_driver, s_name );


	if ((m = SYS_new( ppcsim )) == NULL)
		return( ERR_NOMEM );

// Class can be one of: MEM, IO, IGNORE, ALIAS
//
	m->mclass = -1;
	for (i = 0; i < sizeof(sys_classes_list)/sizeof(NAMEVAL); i++) {
		 if (strcasecmp(s_type, sys_classes_list[i].name) == 0) {
			m->mclass = sys_classes_list[i].val;
			break;
		}
	}
	if (m->mclass == -1) {
		printf("  ppcsim.SYS : unknown class '%s'.\n", s_type);
		return( ERR_INVARG );
	}


// All entries follow with 3 hex words.
//
	m->start = strtoul( s_start, &e, 16 );
	m->end	 = strtoul( s_end,   &e, 16 );
	m->mask  = strtoul( s_mask,  &e, 16 );


// Mode can be one of: RW, RO, HW
//
	m->mode = -1;
	for (i = 0; i < sizeof(sys_modes_list)/sizeof(NAMEVAL); i++) {
		 if (strcasecmp(s_mode, sys_modes_list[i].name) == 0) {
			m->mode = sys_modes_list[i].val;
			break;
		}
	}
	if (m->mode == -1) {
		printf("  ppcsim.SYS : unknown mode '%s'\n", s_mode);
		return( ERR_INVARG );
	}

// Malloc memory if required.
//
	if (isdigit( s_alloc[0] )) {
		m->memsize = strtoul( s_alloc, &e, 10 );
		m->memsize = m->memsize * get_scalar( &e );				// Scale value up.

		if ((m->mem = malloc( m->memsize )) == NULL)
			return( ERR_NOMEM );
		if (strcpy( m->s_memsize, s_alloc ) == NULL)
			return( ERR_NOMEM );
	}

// Driver.  Only for IO.
//
	m->driver = 0;
	m->driver = -1;
	for (i = 0; i < sizeof(sys_drivers_list)/sizeof(NAMEVAL); i++) {
		 if (strcasecmp(s_driver, sys_drivers_list[i].name) == 0) {
			m->driver = sys_drivers_list[i].val;
			break;
		}
	}
	if (m->driver == -1) {
		if (s_driver  &&  *s_driver != '-')					// '-' means no driver, which is OK
			m->driver = 0;
		else
			return( ERR_INVARG );
	}

// Name. might be quoted.  Probably has leading spaces due to strtok() "rest of line" 
// feature.
//	
	while (*s_name  &&  isspace(*s_name))
		s_name++;
	if (*s_name == '"') {						// strip quotes
		s_name++;
		if (s_name[strlen(s_name)-1] == '"')
			s_name[strlen(s_name)-1] = 0;
	}
	m->name	= strdup( s_name );

	return( 0 );
}


//--------------------------------------------------------------------------------
// SYS_file_load -- load system definitions from file (old way).
//--------------------------------------------------------------------------------
int SYS_file_load( PPCSIM *ppcsim, char *filename )
{
	FILE	*fd;
	int		 stat, lineno;
	char    *s, buf[BUFSIZ];

	if ((fd = fopen( filename, "r")) == NULL)
		return( ERR_FILE );

	lineno = 0;
	stat   = 0;
	while ((s = fgets( buf, BUFSIZ, fd )) != NULL) {
		buf[strlen(buf)-1] = 0;
		lineno++;
		if (!buf[0]  ||  isspace(buf[0])  ||  buf[0] == '#')
			continue;

		if ((stat = SYS_parse_def( ppcsim, buf )) != 0) {
			printf("  ppcsim.SYS : error on line %d\n", lineno);
			break;
		}
	}

	fclose(fd);

	return( stat );
}


//--------------------------------------------------------------------------------
// SYS_show -- show current map.
//--------------------------------------------------------------------------------
int SYS_show( PPCSIM *ppcsim )
{
	SYSMAP *m;
	CList  *iter;

	printf("  START..END           MASK      MODE   CLASS   DRIVER        MEMORY      NAME\n"
           "  ===================  ========  =====  ======  ============  ==========  ===========\n");

	c_list_for_each( iter, (CList *) ppcsim->system ) {
		m = (SYSMAP *) iter;
		printf("  %08X..%08X   %08X  %-5s  %-6s  %-12s  %c %8s  %s\n",
				m->start, m->end, m->mask, sys_modes_list[ m->mode ].name, 
				sys_classes_list[ m->mclass ].name, 
				sys_drivers_list[ m->driver ].name, 
				m->mem == NULL ? ' ' : '*',
				m->s_memsize[0] ? m->s_memsize : "-",
				m->name );
	}
	return( 0 );
}


//================================================================================
//================================================================================


char *sys_help[] = {
	"  usage: sys [-a s][-d f][-lmr]",
	"  where:",
	"          -a s - add system definition using string 's'.",
	"          -d f - define system using file 's'.",
	"          -l   - list active system definitions.",
	"          -m   - create simple 64KB memory-only system.",
	"          -r   - delete active system definitions.",
	NULL
};


//--------------------------------------------------------------------------------
// SYS_cmd -- system management commands.
//--------------------------------------------------------------------------------
int SYS_cmd( PPCSIM *ppcsim, int argc, char *argv[] )
{
	int    opt, i, stat;
	char   dopt, do_add, do_load, do_help, do_set;
	int	   do_list, do_clist, do_simple;
	char  *r, filename[BUFSIZ], def[BUFSIZ];


// Collect arguments.
	
	optind		= 0;		// POSIX
	do_add		= 0;
	do_list		= 0;
	do_load		= 0;
	do_help		= 0;
	do_simple	= 0;
	def[0]		= 0;

	if (argc > 1  &&  strcmp( argv[1], "--help" ) == 0)
		do_help++;
	else while ((opt = getopt( argc, argv, "a:d:lmr" )) != -1) {
		switch (opt) {
		case 'a':	strcpy( def, optarg );
					do_add = 1;
					break;

		case 'd':	do_load = 1;
					do_list = 1;
					strcpy(filename,optarg);				
					break;

		case 'l':	do_list = 1;
					break;

		case 'm':	do_simple = 1;
					break;

		case 'r':	SYS_free( ppcsim );
					break;

		default:	return( ERR_UNKNOWN );
		}
	}

// --help or errors.
//
	if (do_help)
		return( shell_show_help( sys_help ) );

// -a nn	-- add system definition.
//
	if (do_add) {
		for (;optind < argc; optind++) {
			strcat( def, " " );
			strcat( def, argv[optind] );
		}
		stat = SYS_parse_def( ppcsim, def );
		return( stat );	
	}


// -d nn	-- load system definition.
//
	if (do_load) {
		SYS_free( ppcsim );
		stat = SYS_file_load( ppcsim, filename );
		return( stat );	
	}

// -m		-- create a small memory system definition (no IO)
//
	if (do_simple) {
		SYS_free( ppcsim );
		strcpy( def, "MEM 00000000 0000FFFF 0000FFFF RW 64MiB - SRAM" );
		stat = SYS_parse_def( ppcsim, def );
		return( stat );	
	}

// (default)
// -l		-- list system definitions.
//
	return( SYS_show( ppcsim ) );
}


//--------------------------------------------------------------------------------
// SYS_init -- initialize system.
//--------------------------------------------------------------------------------
int SYS_init( PPCSIM *ppcsim )
{

	ppcsim->sys_head.name = NULL;

	ppcsim->system = (SYSMAP *) c_list_init( (CList *) &ppcsim->sys_head );

	return( 0 );
}


//--------------------------------------------------------------------------------
// SYS_deinit --
//--------------------------------------------------------------------------------
int SYS_deinit( PPCSIM *ppcsim )
{

	SYS_free( ppcsim );

	return( 0 );
}
