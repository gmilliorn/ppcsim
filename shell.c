// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

// SPDX-License-Identifier: MIT

/*
 * Copyright (c) 2026 Gary Milliorn
 * All rights reserved.
 */


//--------------------------------------------------------------------------------
//	SHELL -- SHELL 
//
//		shell takes user input or script input and dispatches it to command
//		processors.
//
//		shell runs in RAW mode so that the simulator can trap on control-C.
//--------------------------------------------------------------------------------


#include "ppcsim.h"


static short	ppcsim_Xdata[] = { 0x7257,0x7469,0x6574,0x206e,0x7962,0x4720,0x7261,
								 0x2079,0x694d,0x6c6c,0x6f69,0x6e72,0x0a00 };

char		   *str_dis_ena[] = { "disabled", "enabled" };



//================================================================================
//									HISTORY
//================================================================================


// History State
//
char   *history[ CFG_HISTORY_SIZE ];
int		hist_idx = 0;
int		hist_count = 0;
int		hist_pos = 0;


void history_clear( void )
{
	int i;

    for (int i = 0; i < CFG_HISTORY_SIZE; i++)
		history[i] = NULL;
}


void history_free( void )
{
	int i;

    for (int i = 0; i < CFG_HISTORY_SIZE; i++) 
		if (history[i]) free(history[i]);
}


void add_history( const char *cmd ) 
{
	int prv;

    if (!cmd || strlen(cmd) == 0) return;

	// Don't add if same as last command.
	//
	prv = (hist_idx - hist_pos + CFG_HISTORY_SIZE) % CFG_HISTORY_SIZE;
	if (prv != hist_idx) {
		if (strcmp(cmd, history[prv] ) == 0)
			return;
	}


    if (history[hist_idx]) free(history[hist_idx]);
    history[hist_idx] = strdup(cmd);
    hist_idx = (hist_idx + 1) % CFG_HISTORY_SIZE;
    if (hist_count < CFG_HISTORY_SIZE) hist_count++;
    hist_pos = 0;
}


//================================================================================
//								RAW terminal IO
//================================================================================
//
// Needed for VT100 history access as well as control-C to stop debugging steps.
	

// Terminal State
//
struct termios orig_termios;
struct timeval shell_timeval;
int	   raw_mode_enabled = 0;



void disable_raw_mode() {

    if (raw_mode_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_mode_enabled = 0;
    }
}


void enable_raw_mode() {
    // Check if stdin is a terminal
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Error: stdin is not a terminal (pipe/IDE?). Run in a real shell.\n");
        exit(1);
    }

    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(1);
    }
    
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); 
    raw.c_cc[VMIN] = 1; // Block until at least 1 byte is available
    raw.c_cc[VTIME] = 0; // No timeout
    
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(1);
    }
    raw_mode_enabled = 1;
}


int kbhit() {
    fd_set set;
    struct timeval timeout;
    int retval;

    // 1. Initialize the set
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);

    // 2. Set zero timeout for non-blocking check
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    // 3. Call select
    // First arg is max_fd + 1. We only care about read set (2nd arg).
    retval = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);

    // 4. Check result
    if (retval == -1) {
        //perror("select()");
        return 0; // Error
    } else if (retval > 0) {
        // Data is available
        return 1;
    }
    
    // No data available
    return 0;
}


//--------------------------------------------------------------------------------
// CTL-C detection.
//
int ctlc_detect = 0;

void key_handler(int sig) {
    puts("\n--------------------------------------------\n");fflush(stdout);
	ctlc_detect = 1;

	tcflush(STDIN_FILENO, TCIFLUSH);
}


int raw_isbreak( void )
{
	int edge;

	edge = ctlc_detect;
	ctlc_detect = 0;
	return( edge );
}


int raw_getkey( void )
{
    int			  n;
    unsigned char c;

	n = read(STDIN_FILENO, &c, 1);

	return( n == -1 ? 0 : c );
}


//--------------------------------------------------------------------------------
// shell_read_line -- command line entry with VT100 history.
//--------------------------------------------------------------------------------
int shell_read_line( char *buf, char *prompt ) {
    int len = 0;
    unsigned char c;
    ssize_t nread;
    

	printf(prompt); fflush(stdout);

    while (1) {
        nread = read(STDIN_FILENO, &c, 1);
        
        // Handle read errors or EOF
        if (nread == -1) {
            if (errno == EINTR) continue; // Interrupted, try again
            perror("read");
            return(-1);
        }
        if (nread == 0) { // EOF (Ctrl+D)
            printf("\n");
            return(-1);
        }

        // Handle Escape Sequence (Arrow keys)
        if (c == 27) { 
            char seq[2];
            // Read '['
            if (read(STDIN_FILENO, &seq[0], 1) != 1) continue;
            // Read final code (A/B)
            if (read(STDIN_FILENO, &seq[1], 1) != 1) continue;
            
            // Up Arrow (ESC [ A)
			//
            if (seq[0] == '[' && seq[1] == 'A') { 
                if (hist_count > 0) {
                    printf("\r\033[K"); // Clear line
                    hist_pos++;
                    if (hist_pos > hist_count) hist_pos = 1;
                    int h_idx = (hist_idx - hist_pos + CFG_HISTORY_SIZE) % CFG_HISTORY_SIZE;
                    if (history[h_idx]) {
                        strncpy(buf, history[h_idx], BUFSIZ-1);
                        len = strlen(buf);

						printf(prompt);
                        printf("%s", buf);
                    }
					fflush(stdout);
                }
                continue;
            }

            // Down Arrow (ESC [ B)
			//
            if (seq[0] == '[' && seq[1] == 'B') {
                if (hist_pos > 0) {
                    printf("\r\033[K");
                    hist_pos--;
                    int h_idx = (hist_idx - hist_pos + CFG_HISTORY_SIZE) % CFG_HISTORY_SIZE;
                    
                    if (hist_pos == 0) {
                        buf[0] = '\0';
                        len = 0;
                    } else if (history[h_idx]) {
                        strncpy(buf, history[h_idx], BUFSIZ-1);
                        len = strlen(buf);
						printf(prompt);
                        printf("%s", buf);
                    }
                    fflush(stdout);
                }
                continue;
            }
        }
        
        // Handle Backspace
		//
        if (c == 127 || c == 8) {
            if (len > 0) {
                len--;
                buf[len] = '\0';
                printf("\b \b");
                fflush(stdout);
            }
            continue;
        }

        // Handle Enter
		//
        if (c == 13 || c == 10) {
            printf("\n");
            buf[len] = '\0';
            return( 0 );
        }

        // Handle Printable Characters
		//
        if (isprint(c)) {
            if (len < BUFSIZ - 1) {
                buf[len++] = c;
                buf[len] = '\0';
                printf("%c", c);
                fflush(stdout);
            }
        }
    }
}


static int default_more_lines = 24;

//--------------------------------------------------------------------------
// shell_more_lines -- return user-defined screen size.
//--------------------------------------------------------------------------
int shell_more_lines( int n )
{


	return( n < default_more_lines ? n : default_more_lines );
}


//--------------------------------------------------------------------------
// shell_more -- pause until a CR/space/etc. character is entered.  Return
//			     1 (for more) unless 'N'/'Q'/'X' or ESC is entered.
//--------------------------------------------------------------------------
int shell_more( void )
{
	int			  ans, nread;
	unsigned char c;

	printf("--more--");
	fflush(stdout);

	for (ans = 0; !ans; ) {
        nread = read(STDIN_FILENO, &c, 1);

		// Currently, anything but n/q/x/ESC is considered a request for more.
		//
		if (nread <= 0)
			ans = 1;
		else if (c == 'n'  || c == 'N' || c == 'q'  ||  c == 'Q'
			 ||  c == 0x1B || c == 'x' || c == 'X')
			ans = -1;
		else
			ans = 1;
	}


// Wipe out prompt.
//
	printf("\b\b\b\b\b\b\b\b        \b\b\b\b\b\b\b\b");
	fflush(stdout);

	return( ans < 0 ? 0 : 1 );
}


//--------------------------------------------------------------------------------
// shell_show_help -- print out help text or other long text arrays, until NULL is found.
//--------------------------------------------------------------------------------
int shell_show_help( char *help[] )
{
	int	i, l, lim;

	i	= 0;
	l	= 1;
	lim = shell_more_lines( 20 );
	while (help[i]) {
		printf("  %s\n", help[i]);
		if (++l >= lim) {
			if (!shell_more())
				break;
			l = 0;
		}
		i++;
	}
	printf("\n");
	return( 0 );
}


//================================================================================
//								COMMANDS
//================================================================================

//--------------------------------------------------------------------------------
// ppcsim_status -- show status.
//--------------------------------------------------------------------------------
int ppcsim_status( PPCSIM *ppcsim )
{

	printf("  PPCSIM STATUS\n"
           "  ===========================\n");
	printf("    CIA         : %08X     NIA: %08X\n", ppcsim->CIA, ppcsim->NIA) ;
	printf("    reservation :");
	if (ppcsim->reserve_valid)
		printf(" %08X\n", ppcsim->rsvd_addr);
	else
		printf(" none\n");

	printf("    dot_cmd     : %s\n", ppcsim->dot_cmd );
	printf("    ss_cmd      : %s\n", ppcsim->tr_cmd );
	printf("    flow tracing: %s\n", str_dis_ena[ ppcsim->flowtrace ] );
	printf("    verbose     : %d\n", ppcsim->verbose);

	printf("    issued      : %lld\n", ppcsim->issued);
	printf("\n");
	printf("    CPU name    : %s\n", ppcsim->CPU_name);
	printf("    CPU class   : %d\n", ppcsim->CPU_class);

	if (ppcsim->CPU_pll_bits) {
	printf("    CPU PLL     : " ); 
				print_binary( ppcsim->CPU_pll, ppcsim->CPU_pll_bits );
	printf("\n");
	}
	printf("    NB name     : %s\n", ppcsim->bridge.name);
	//printf("    NB class    : %d\n", ppcsim->NB_class);

	return( 0 );
}


//------------------------------------------------------------------------      
// is_valid_hex_input -- verifies that the input characters are 0-9 or
//                       A-F or a-f    OR   '_'.
//------------------------------------------------------------------------      
int is_valid_hex_input( char string[] )
{
	int index;

	for (index = 0; index <= (strlen(string)-1); index++) {
		if (((string[index] >= 'A' && string[index] <= 'F')
		||   (string[index] >= 'a' && string[index] <= 'f')
		||   (string[index] == '_')
		||   (string[index] >= '0' && string[index] <= '9')) != 1)
		return( 1 );
	}

	return( 0 );
}


//---------------------------------------------------------------------------
// shell_getvalue -- collect a value, one of:
//						a hex value 
//						a symbol value.
//						'.' meaning the current location (CIA)
//---------------------------------------------------------------------------
int shell_getvalue( PPCSIM *ppcsim, char **cmdline, ULONG *a1, int base )
{       
	SYMBOL *sym;
	char   *s, *e;
	int     stat;

	s = *cmdline;                           // Skip whitespace
	while (*s == ' ')
		s++;    

// '.' = PC.
//
	if (*s == '.') {
		*a1 = ppcsim->CIA;
		return( 0 );
	}

// symbol.
//
	if ((sym = symbol_get_byname( ppcsim, s, a1 )) != NULL)
		return( 0 );
				

// Hex value.
//	
	if (a1 != (ULONG *) NULL) {
        *a1 = (ULONG) strtoul( s, &e, base );
        if (s != e) {
            *cmdline = e;
            return( 0 );
        }
	}               
			
	return( ERR_BADVAL );
}                               


//---------------------------------------------------------------------------
//	shell_getarg_or_prompt -- if the arg starts with '=', collect a
//							  value from the argument.  Otherwise, prompt and 
//							  collect a value.
//
//	Return: 1   -- something entered
//			0   -- nothing entered
//          -1  -- ESC/etc. entered or error.
//---------------------------------------------------------------------------
int shell_getarg_or_prompt( PPCSIM *ppcsim, char **args, char *prompt, ULONG *newval )
{
	char *s, *end;
	int   base;
	char  buf[20];


// Does it start with '='?
//
	s    = *args;
	base = 16;                                      // Seems more useful a default
	if (*s == '=') {
		s++;

		// If starts with '0x', switch to hex.
		//
		if (*s == '0'  &&  *(s+1) == 'x') {
			s += 2;
			base = 16;
		}
		*args = s;
		return( shell_getvalue( ppcsim, args, newval, base ) == 0 ? 1 : -1 );
	}

// Nothing on the command line, prompt.
//
	shell_read_line( buf, prompt );

	if (*buf == '\n')
		return( 0 );
	else if (*buf == 0x1b)
		return( -1 );

	*newval = strtoul( buf, &end, 16 );

	return( (end == buf) ? 0 : 1 );
}


//--------------------------------------------------------------------------------
// shell_getaddrs -- many commands need 1 or 2 address values, where if addr #2 is 
//					 missing, it is set to addr1+0x10 or a copy of addr1, as indicated.
//--------------------------------------------------------------------------------
int shell_getaddrs( PPCSIM *ppcsim, int req, int opt, int base, int argc, char *argv[], 
								   ULONG *arg1, ULONG *arg2 )
{
	int   stat;
	char *e;

	// always at least one.
	//
	if (argc < req)	
		return( ERR_NOARG );
	e = argv[1];
	if ((stat = shell_getvalue( ppcsim, &e, arg1, 16 )) != 0)
		return( ERR_BADVAL );

	// opt!=0 and 1..2 args, get the first now.
	//
	if (opt  &&  (argc <= req)) {		// missing arg2, set to arg1
		if (opt == SHELL_GETARG_COPY)
			*arg2 = *arg1;
		else
			*arg2 = *arg1 + 0x20;
	}

	// opt=0 and 1 arg: fine, just the one then.
	//
	else if (opt == 0) {
		return( 0 );
	}

	// opt!=0 and <argc> matches <reg>, get next.
	//
	else {
		e = argv[2];	
		if ((stat = shell_getvalue( ppcsim, &e, arg2, 16 )) != 0)
			return( ERR_BADVAL );
	}

	return( 0 );
}


char *ppcsim_help[] = {
	 "  PPCSIM COMMANDS",
     "  ====================================================================",
	 "    .     [n]      single step 1 or n instructions.",
	 "    as     a       assemble code at <address>.",
	 "    bp    (cmds)   breakpoint commands",
	 "    bt             show backtrace of change of flow.",
	 "    cont           continue running.",
	 "    do     s       run a script file",
	 "    d      a [e]   disassemble code at <address>",
	 "    echo  [s]      print args to console.",
	 "    fl     s       load file to memory.",
	 "    ft             toggle code flow tracing.",
	 "    go     a       start running at <address>.",
	 "    hist           show command history.",
	 "    mss   (cmds)   MemorySubSystem commands.",
	 "    nb    (cmds)   NorthBridge subsystem commands.",
	 "    over           run past sub call at location.",
	 "    rd    (regs)   register display.",
	 "    reset          reset simulator and drivers (symbols and user",
     "                     code are preserved).",
	 "    rm     r[=a]   register modify.",
	 "    ss    [n]      single step 1 (or n) steps.",
	 "    tc    [s]      set command(s) to run after each single-step.",
	 "    td    [n]      control printing of GPRs that changes.",
	 "    tr     a       start tracing at address a.",
	 "    stat           show internal status.",
	 "    u      a [e]   unassemble code between range.",
	 "    up             run until next BLR.",
	 "    v     [n]      toggle verbose level (or set to <n> for more).",
	 "    where          report current code label.",
     "",
     "where:",
     "    a, e           addresses which are hex values or symbols.",
     "    n              a decimal integer.",
     "    s              string of chars.",
	 "",
	 NULL
};


//--------------------------------------------------------------------------------
// shell_run_one_command -- run the single command in <line>.
//--------------------------------------------------------------------------------
int shell_run_one_command( PPCSIM *ppcsim, char *line ) 
{
	int		 i, n, argc, stat;
	ULONG	 v, start, end;
	char	*e, *token, *cmd;
	char     buf[BUFSIZ+2];
    char	*argv[ CFG_ARGV_SIZE ];


// if <line> starts with '.', repeat or extend a previous command.
//
	cmd = line;
	if (strcmp(cmd, ".") == 0) {
		if (ppcsim->dot_cmd[0] == 0) {							// No previous command, do "ss".
			strcpy( ppcsim->dot_cmd, "ss" );
		}
		cmd = ppcsim->dot_cmd;
	}


// Reasonably sure this command is usable, add to history.
//
    if (strlen(cmd) == 0)
		return(0);
    add_history(line);


// Create argc/argv data from command.
// 
	strcpy(buf, cmd);
    token = strtok(buf, " \t");
	if (token == NULL)
		return( 0 );
	argc  = 0;
    while (token && argc < CFG_ARGV_SIZE - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }
    argv[argc] = NULL;

	if (ppcsim->verbose > 2) {
		printf("  ppcsim.shell: argc    = %d\n", argc);
		for (i = 0; i < argc; i++)
			printf("  ppcsim.shell: argv[%d] = '%s'\n", i, argv[i]);
	}


// Run the command.
//

	// as -- assemble
	//
	if (strcmp(argv[0], "as") == 0) {
		if ((stat = shell_getaddrs( ppcsim, 1, 0, 16, argc, argv, &start, &end )) == 0)
			stat = assemble( ppcsim, start, end );
		return( stat );
	}

	// bp -- breakpoints
	//
	else if ((strcmp(argv[0], "bp") == 0)  ||  (strcmp(argv[0], "br") == 0))
	 	return( BRK_cmd( ppcsim, argc, argv ) );

	// bt -- show backtrace
	//
	else if (strcmp(argv[0], "bt") == 0)
		return( ppcsim_backtrace( ppcsim ) );

	// cont -- continue running
	//
	else if (strcmp(argv[0], "cont") == 0)
		return( ppcsim_run( ppcsim, 0, 0 ) );

	// cpu -- cpu subcommands.
	//
	else if (strcmp(argv[0], "cpu") == 0) {
		return( CPU_cmd( ppcsim, argc, argv ) );
	}

	// do -- run a script
	//
	else if (strcmp(argv[0], "do") == 0) {
		if (argc < 2)
			return( ERR_NOARG );
		return( shell_script( ppcsim, argv[1] ) );
	}
	
	// ds -- disassemble
	// u  -- unassemble
	//
	else if ((strcmp(argv[0], "ds") == 0)  ||  (strcmp(argv[0], "u") == 0)) {
		if ((stat = shell_getaddrs( ppcsim, 2, SHELL_GETARG_LINE, 16, argc, argv, &start, &end )) == 0) {
			if ((stat = disassemble( ppcsim, start, end )) == 0)
				sprintf(ppcsim->dot_cmd, "u %x", end+4 );
		}
		return( stat );
	}

	// ea -- calculate effective address.
	//
	else if (strcmp(argv[0], "ea") == 0) {
		if ((stat = shell_getaddrs( ppcsim, 2, 0, 16, argc, argv, &start, &end )) == 0) {
			start = (start & 0xFFFF) << 16;
			end   &= 0xFFFF;
			i = (int) (end & 0x8000) ? (end | 0xFFFF0000) : end;	
			v = start + i;
			printf("  EA: %08X\n", v);
		}
		return( stat );
	}

	// echo -- print args.
	//
	else if (strcmp(argv[0], "echo") == 0) {
		for (i = 1; i < argc; i++)
			printf("%s ", argv[i]);
		printf("\n");
		return( 0 );
	}

	// fl [-a addr] -- load file.
	//
	else if (strcmp(argv[0], "fl") == 0) {
		if (argc < 2)
			return( ERR_NOARG );
		start = 0;
		i = 1;
		if ((argc >= 4) &&  (strcmp(argv[1], "-a") == 0)) {
			e = argv[2];
			if ((stat = shell_getvalue( ppcsim, &e, &start, 16 )) != 0)
				return( stat );
			start = strtoul( argv[2], &e, 16 );
			i = 3;
		}
		return( file_load( ppcsim, start, argv[i] ) );
	}

	// ft -- toggle flow-tracing.
	//
	else if (strcmp(argv[0], "ft") == 0) {
		ppcsim->flowtrace = 1-ppcsim->flowtrace;
		printf("  ppcsim: flow trace is %s.\n", str_dis_ena[ ppcsim->flowtrace ] );
		return( 0 );
	}

	// go a -- start running in run mode.
	//
	else if (strcmp(argv[0], "go") == 0) {
		if ((stat = shell_getaddrs( ppcsim, 2, SHELL_GETARG_COPY, 0, argc, argv, &start, &end )) == 0)
			return( ppcsim_run( ppcsim, start, 1 ) );
	}

	// he -- help
	//
	else if (strncmp(argv[0], "he", 2) == 0)
		return( shell_show_help( ppcsim_help ) );

	// hist	-- show history.
	//
    else if (strncmp(argv[0], "hi", 2) == 0) {
        for (i = 0; i < hist_count; i++) {
            n = (hist_idx - hist_count + i + CFG_HISTORY_SIZE) % CFG_HISTORY_SIZE;
            printf("  %d: %s\n", n + 1, history[n]);
        }
        return( 0 );
    }

	// md	-- memory display
	//
    else if (strncmp(argv[0], "md", 2) == 0)
		return( MEM_cmd( ppcsim, argc, argv, MODE_RD ) );

	// mm	-- memory modify
	//
    else if (strncmp(argv[0], "mm", 2) == 0)
		return( MEM_cmd( ppcsim, argc, argv, MODE_WR ) );

	// mss	-- memory manager
	//
    else if (strcmp(argv[0], "mss") == 0)
		return( MSS_cmd( ppcsim, argc, argv ) );

	// nb	-- northbridge commands.
	//
    else if (strcmp(argv[0], "nb") == 0)
		return( NB_cmd( ppcsim, argc, argv ) );

	// over -- step over the current subroutine call.
	//
	else if (strcmp(argv[0], "over") == 0) {
		if ((stat = BRK_set( ppcsim, ppcsim->CIA + 4, 0, 1 )) != 0)		// set breakpoint past BL.
			return( stat );
		ppcsim_run( ppcsim, 0, 0 );
		if ((stat = BRK_del_addr( ppcsim, start, 1 )) != 0)
			return( stat );
	}

	// rd	-- register display
	//
    else if (strncmp(argv[0], "rd", 2) == 0)
		return( reg_rdm( ppcsim, argc, argv, MODE_RD ) );

	// reset -- reset simulator
	//
    else if (strcmp(argv[0], "reset") == 0) {
		ppcsim_init( ppcsim );
		BRK_init( ppcsim );
		NB_reset( ppcsim );
		NB_ENV_preset( ppcsim, 1 );
		UART_reset( ppcsim );
		return( 0 );
	}

	// rm	-- register modify
	//
    else if (strncmp(argv[0], "rm", 2) == 0)
		return( reg_rdm( ppcsim, argc, argv, MODE_WR ) );

	// ss [n] -- single-step.
	//
	else if (strcmp(argv[0], "ss") == 0) {
		n = 1;
		if (argc > 1) {
			n = strtol( argv[1], &e, 10 );
			if (*e)
				return( ERR_BADVAL );
		}
		sprintf(ppcsim->dot_cmd, "ss %d", n);
		return( ppcsim_ss( ppcsim, n ) );
	}

	// stat	-- show status.
	//
    else if (strncmp(argv[0], "status", 2) == 0)
		return( ppcsim_status( ppcsim ) );

	// syms	-- list symbols
	//
    else if (strcmp(argv[0], "sym") == 0)
		return( SYM_cmd( ppcsim, argc, argv ) );

	// sys	-- system options.
	//
    else if (strcmp(argv[0], "sys") == 0)
		return( SYS_cmd( ppcsim, argc, argv ) );

	// tc s -- set trace command.
	//
	else if (strcmp(argv[0], "tc") == 0) {
		ppcsim->tr_cmd[0] = 0;
		for (i = 1; i < argc; i++) {
			strcat( ppcsim->tr_cmd, argv[i] );
			strcat( ppcsim->tr_cmd, " " );
		}
		return( 0 );
	}

	// td [n] -- control register delta check
	//
    else if (strcmp(argv[0], "td") == 0) {
		if (argc > 1)
			ppcsim->show_reg_delta	= strtoul( argv[1], &e, 10 );
		else
			ppcsim->show_reg_delta	= (ppcsim->show_reg_delta) ? 0 : 1;
		return( 0 );
	}

	// tr a -- start running in trace mode.
	//
	else if (strcmp(argv[0], "tr") == 0) {
		if ((stat = shell_getaddrs( ppcsim, 2, SHELL_GETARG_COPY, 0, argc, argv, &start, &end )) == 0)
			ppcsim_set_PC( ppcsim, start );
		return( ppcsim_ss( ppcsim, 1 ) );
	}

	// uart	-- uart commands.
	//
    else if (strcmp(argv[0], "uart") == 0)
		return( UART_cmd( ppcsim, argc, argv ) );

	// up -- trace (silently) until BLR executed.
	//
	else if (strcmp(argv[0], "tr") == 0) {
		printf("TBD\n");
		return( 0 );
	}

	// v [n] -- set verbose level
	//
    else if (strcmp(argv[0], "v") == 0) {
		char *e;

		if (argc > 1)
			ppcsim->verbose = strtoul( argv[1], &e, 10 );
		else
			ppcsim->verbose = (ppcsim->verbose) ? 0 : 1;

		if (ppcsim->verbose > 7)
			printf("  %s\n", (char *) &ppcsim_Xdata[0] );
		return( 0 );
	}

	// where -- report function we are currently in (most recent label).
	//
    else if (strcmp(argv[0], "where") == 0) {
		if (SYM_where( ppcsim, ppcsim->CIA, 0 ) == -1)
			printf("  ppcsim: unknown location\n");
		return( 0 );
	}

	// exit -- quit ppcsim
	//
    else if ((strcmp(argv[0], "exit") == 0)  ||  (strncmp(argv[0], "quit", 1) == 0))
		return( -1 );

	printf("  ppcsim: unknown command: '%s'\n", line);

	ppcsim->dot_cmd[0] = 0;

	return( ERR_UNKNOWN );
}


//--------------------------------------------------------------------------------
// shell_run_command -- run the single command in <line>.
//--------------------------------------------------------------------------------
int shell_run_command( PPCSIM *ppcsim, char *line ) 
{
	int   stat;
	char *token, *save;
	char  bx[BUFSIZ];

	if (line[0] == 'q')	exit(1);

	stat  = 0;
	save  = NULL;
    token = strtok_r(line, ";", &save);
    while (token) {
		strcpy( bx, token );
		if ((stat = shell_run_one_command( ppcsim, bx )) != 0)
			break;
        token = strtok_r(NULL, ";", &save);
    }
	return( stat );
}


//--------------------------------------------------------------------------------
// shell_script -- run a script file.
//--------------------------------------------------------------------------------
int shell_script( PPCSIM *ppcsim, char *script )
{
	FILE *fd;
	int   n, stat, line;
	char  buf[BUFSIZ+2];

	if ((fd = fopen( script, "r" )) == NULL)
		return( ERR_FILE );

	stat = 0;
	line = 0;
	while (fgets(buf, BUFSIZ, fd) != NULL) {
		line++;
		n = strlen(buf);
		if (n == 0)
			continue;
		if (buf[0] == '#')
			continue;
		buf[n-1] = 0;

		if (n == 0)
			continue;

		if ((stat = shell_run_command( ppcsim, buf )) != 0) {
			printf("  ppcsim: script error %d on line %d.\n", stat, line);
			printf("        '%s'\n", buf);
			fflush(stdout);
			break;
		}
	}

	fclose( fd );

	return( stat );
}


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void shell_init( void )
{

	shell_timeval.tv_sec	= 0;
	shell_timeval.tv_usec	= 0;

	history_clear();

	ctlc_detect = 0;
    signal(SIGINT, key_handler);

    enable_raw_mode();
}


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void shell_deinit( PPCSIM *ppcsim )
{

	disable_raw_mode();
	history_free();
}
