// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

// SPDX-License-Identifier: MIT

/*
 * Copyright (c) 2026 Gary Milliorn
 * All rights reserved.
 */


//--------------------------------------------------------------------------------
//	FILE -- File load functions.
//
//		functions to load simulation files into memory.
//		supports:
//			binary
//			s-record
//			ELF
//
//		ELF		requires powerpc-linux-gnu or equivalent installed.
//		SREC	requires srec_cat or equivalent installed.
//--------------------------------------------------------------------------------


#include "ppcsim.h"


#define	FILE_IS_BINARY		1
#define	FILE_IS_SREC		2
#define	FILE_IS_ELF			3


static char *ELF_header	= "\x7f""ELF";


//--------------------------------------------------------------------------------
// FILE_info -- given filename, return info on it.
//--------------------------------------------------------------------------------
static int FILE_info( char *filename, int *size, int *type )
{
	int				fd, stat;
	size_t			n;
	struct stat		sb;
	unsigned char	header[10];


// Get size and then look in header bytes.
//
	*size	= 0;
	*type	= 0;
	n		= 0;
	if ((fd = open( filename, O_RDONLY )) < 0)
		return( ERR_FILE );

	if ((stat = fstat(fd, &sb)) == 0)
		n = read( fd, header, 8 );

	close( fd );

	if (stat != 0  ||  n == 0)
		return( -1 );

	*size	= (int) sb.st_size;

	if (strncmp(header, ELF_header, 4) == 0)
		*type = FILE_IS_ELF;
	else if (strncmp(header, "S0", 2) == 0)
		*type = FILE_IS_SREC;
	else
		*type = FILE_IS_BINARY;

	return( 0 );
}


//--------------------------------------------------------------------------------
// FILE_load_bin -- load binary file to memory.
//					memory must be of sufficient size.
//--------------------------------------------------------------------------------
static int FILE_load_bin( PPCSIM *ppcsim, SYSMAP *m, ULONG addr, char *filename, int fsize )
{
	FILE           *fp;
	size_t			fr_size;
	int				fd, stat;
	ULONG			addr_msk;
	void		   *mem;
	unsigned char	buffer[4];


	addr_msk = addr & m->mask;
	printf("  ppcsim.FILE: '%s' size %d (%08X) bytes.\n", filename, fsize, fsize);


// Check size.
//
	if (fsize >= m->memsize - 8) {
		printf("  ppcsim.FILE: filesize of %dB exceeds memory size of %dB\n", fsize, m->memsize );
		return( ERR_NOMEM );
	}

	mem = m->mem + addr_msk;

// Load in as binary.
//
	if ((fp = fopen( filename, "rb")) == NULL) {
		printf("  ppcsim.FILE: cannot open '%s'\n", filename);
		return( ERR_FILE );
	}

	fr_size = fread( mem, 1, fsize, fp );
	if (fr_size != fsize) {
		fclose( fp );
		return( ERR_FILE );
	}

	fclose( fp );

	return( 0 );
}


//--------------------------------------------------------------------------------
// FILE_load_ELF -- load ELF file to memory.
//--------------------------------------------------------------------------------
static int FILE_load_ELF( PPCSIM *ppcsim, SYSMAP *m, ULONG addr, char *filename, int fsize )
{
	int		stat;
	int		new_fsize, new_ft;
	char	buf[BUFSIZ*2], new_f[BUFSIZ];

	sprintf(new_f, "%s.bin", filename);
	sprintf(buf, "powerpc-linux-gnu-objcopy -O binary %s %s", filename, new_f);

// Use GCC tools to do the work.
//
	if (ppcsim->verbose)
		printf("  ppcsim.FILE: ELF converison: '%s'\n", buf);
	stat = system( buf );
	if (stat != 0)
		return( ERR_FILE );

// Update file size.
//
	if ((stat = FILE_info( new_f, &new_fsize, &new_ft )) != 0)
		return( ERR_FILE );

	stat = FILE_load_bin( ppcsim, m, addr, new_f, new_fsize );
	return( stat );
}


//--------------------------------------------------------------------------------
// FILE_load_SREC -- load SREC file to memory.
//--------------------------------------------------------------------------------
static int FILE_load_SREC( PPCSIM *ppcsim, SYSMAP *m, ULONG addr, char *filename, int fsize )
{
	int		stat;
	int		new_fsize, new_ft;
	char	buf[BUFSIZ*2], new_f[BUFSIZ];

	sprintf(new_f, "%s.bin", filename);
	sprintf(buf, "srec_cat %s -o %s -binary", filename, new_f);

// Use GCC tools to do the work.
//
	if (ppcsim->verbose)
		printf("  ppcsim.FILE: SREC converison: '%s'\n", buf);
	stat = system( buf );
	if (stat != 0)
		return( ERR_FILE );
	
// Update file size.
//
	if ((stat = FILE_info( new_f, &new_fsize, &new_ft )) != 0) {
		return( ERR_FILE );
	}

	stat = FILE_load_bin( ppcsim, m, addr, new_f, new_fsize );
	return( stat );
}


//--------------------------------------------------------------------------------
// file_load -- load file into memory
//--------------------------------------------------------------------------------
int file_load( PPCSIM *ppcsim, ULONG addr, char *filename )
{
	SYSMAP *m;
	int		ft, fsize, stat;

	if ((m = MSS_MapAddress( ppcsim, addr )) == NULL)
		return( ERR_NOMEM );
	if (m->mem == NULL)
		return( ERR_NOMEM );

	if (!ppcsim->headless)
		printf("  ppcsim.FILE: loading '%s' to '%s' @%08X.\n", filename, m->name, addr);


// ID filetype
//
	if ((stat = FILE_info( filename, &fsize, &ft )) != 0)
		return( ERR_FILE );


// Finally load.
//
	switch (ft) {
	case FILE_IS_BINARY:	stat = FILE_load_bin( ppcsim, m, addr, filename, fsize );
							break;
	case FILE_IS_ELF:		stat = FILE_load_ELF( ppcsim, m, addr, filename, fsize );
							break;
	case FILE_IS_SREC:		stat = FILE_load_SREC( ppcsim, m, addr, filename, fsize );
							break;
	default:				stat = ERR_FILE;
							break;
	}

	return( stat );
}
