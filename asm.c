// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

/***************************************************************************
 *     Copyright Motorola, Inc. 1989-2001 ALL RIGHTS RESERVED
 *
 *  $Id: asm.c,v 1.1.1.1 2002/04/25 22:20:28 maurie Exp $
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


#define ASM_DSM_AVAILABLE
#ifdef ASM_DSM_AVAILABLE

#define IN_ASSEMBLER_DISASSEMBLER
#include "asm_dsm.h"
#undef IN_ASSEMBLER_DISASSEMBLER
//#include "symtab.h"


extern int spr_pull( char *sname );


label_array_element label_table[MAXBRANCHLABEL] = {
 /*  0 */ {"KEYBOARD",       (ULONG) 0x0,                              DSYM},

#ifdef ON_BOARD

 /*  1 */ {"getchar",        (ULONG) &dink_getchar,                    DSYM},
 /*  2 */ {"putchar",        (ULONG) &dink_putchar,                    DSYM},
 /*  3 */ {"TBaseInit",      (ULONG) &time_base_init,                  DSYM},
 /*  4 */ {"TBaseReadLower", (ULONG) &time_base_read_lower,            DSYM},
 /*  5 */ {"TBaseReadUpper", (ULONG) &time_base_read_upper,            DSYM},
 /*  6 */ {"CacheInhibit",   (ULONG) &cache_flush_disable_all,             DSYM},
 /*  7 */ {"InvEnL1Dcache",  (ULONG) &cache_inval_enable_L1D,       DSYM},
 /*  8 */ {"DisL1Dcache",    (ULONG) &cache_flush_disable_L1D,                 DSYM},
 /*  9 */ {"InvEnL1Icache",  (ULONG) &cache_inval_enable_L1I, DSYM},
 /* 10 */ {"DisL1Icache",    (ULONG) &cache_flush_disable_L1I,                     DSYM},
 /* 11 */ {"dink_loop",      (ULONG) &dink_loop,                       DSYM},
 /* 12 */ {"dink_printf",    (ULONG) &dink_printf,                     DSYM},

#endif

 /* 13 */ {"", (ULONG) 0x0, NSYM},
 /* 14 */ {"", (ULONG) 0x0, NSYM},
 /* 15 */ {"", (ULONG) 0x0, NSYM},
};



extern asm_dasm_element ops_and_mnemonics[];
extern int number_of_mnemonics;


/*

File name:	asm.c
Purpose:	This file contains the "workhorse", or "toolbox" or "module"
		or whatever you want to call it.  The functions in this
		file do the "guts" work of the assembler.

				This function will validate the addresses
				and cycle through each address, calling
				"assemble_a_mnemonic" for each.
assemble( start_address, ending_address )

				This function does the actual encoding
				of a mnemonic string (the work horse).
assemble_a_mnemonic( mnemonic, opcode ,current_address)

				Prints the label_table[] array to 
				the screen
branchtable_list()
				Inits the label_table[] array to 
				name = '\0' and address = 0...called from
				main.c only once!
branchtable_clear()

				Parses string_s and pulls word number word_num
				out of it, returning only that word in
				istring_d.
pull_next_word( word_num, string_d, string_s )

                                Parses string_s and pulls word number word_num
                                out of it, returning only that word in
                                istring_d (doesn't consider '()' whitespace
pull_next_word_2( word_num, string_d, string_s )

				Send in a string containing a register number,
				preceeded by either a "r" (indicating it's
				a general-purpose register), a "f" (indicating
				it's a floating-point register), or an "s"
				(indicating it's a special purpose register).
				This functions chops off the first character
				and returns an unsigned long.
pull_register( string )

				This function takes a string as input, and
				converts that string into an unsigned long.
pull_immediate( string )

				This function is a simple searcher.
				It takes in either a mnemonic and searches
				the data base for a matching mnemonic,
				or takes in an opcode and searches the
				data base for a matching opcode.  The
				index parameter is used to determine what
				to search for (0 indicates searching for
				an opcode, and 1 indicates searching for
				a mnemonic).  The function returns, in
				index, the array entry number which contains
				a matching item.
search_ops_and_mnemonics( opcode, mnemonic, index )
			
				This function is a simple searcher.
			  	The index parameter is used to return the
				location of the matching mnemonic.
				Opcode is the opcode to match and tmp_long
				is an int that can hold info to shorten the
				search.
special_search_ops_and_mnemonics( opcode, index, tmp_long )
				
*/





/*
         1         2         3         4         5         6         7         8
01234567890123456789012345678901234567890123456789012345678901234567890123456789
*/
/*

function:	assemble_a_mnemonic
purpose:	This function will display a disassembled version of
		the memory space requested.
inputs:		opcode:		This is where the 32 bit opcode will be put
		current_address: Current location of PC	
		mnemonic:	This is the string that we are trying to
				assemble.
outputs:	opcode:		This is the 32-bit opcode that I'll be
				returning if the assemble was successful.
return:		Either 0 or some error code telling my caller that
		something went wrong and the assemble was NOT successful.
mod history:	2/2/93	MH
		10/1/95 MM added simplified mnemonics
*/

int	assemble_a_mnemonic( mnemonic, opcode, current_address )
				/* Mnemonic holds the ASCII string which
				   is the mnemonic of the instruction I'm
				   about to assemble.
				*/
char		mnemonic[80];

				/* "opcode" will be the 32-bit piece of
				   data that I just generated, based on
				   the string sent in "mnemonic".
				*/
unsigned long	*opcode;
unsigned long	current_address;
{

				/* This will be the instruction mmemonic
				   itself, no registers, no data references
				   at all, just the instruction mnemonic.
				*/
char		primary_mnemonic[80];

				/* These four strings will hold 4 arguments
				   of an assembly instruction.  Yes, in
				   PowerPC assembly, there can be 4 arguments
				   for an instruction.
				*/
char		operand_1[80];
char		operand_2[80];
char		operand_3[80];
char		operand_4[80];
char		operand_5[80];

				/* This variable acts as a general-purpose
				   index pointer...
				*/
int		index;

				/* This variable I'll use to build the complete
				   32-bit opcode before I send it back out
				   through "opcode".
				*/
unsigned long	tmp_long;

				/* This variable will be used to hold the
				   different fields of the opcode, like
				   register reference numbers, immediates, ..
				*/
unsigned long	tmp_field = 0;

				/* The variable "status" will be used
				   when I call other functions and they
				   return status.  I've typed it as
				   "int" because someone else will
				   define that type.  It will most likely
				   be a long, but who really knows...
				*/
int		status;
				
				/* tmp2_field is used to hold more than
				   field value at a time for the simplified
				   mnemonics
				*/
unsigned long tmp2_field;

				/* First, I want to pull the primary
				   mnemonic from the string passed in.
				   I'll do that by calling a function
				   called "pull_next_word".
				*/
	pull_next_word( 1, primary_mnemonic, mnemonic );

				/* Since I'm here, I might as well pull
				   the other 5 possible arguments for the
				   instruction.
				*/

	status = pull_next_word( 2, operand_1, mnemonic );
	status = pull_next_word( 3, operand_2, mnemonic );
	status = pull_next_word( 4, operand_3, mnemonic );
	status = pull_next_word( 5, operand_4, mnemonic );
	status = pull_next_word( 6, operand_5, mnemonic );
				 
				/* Put a 1 in "index" since we want our
				   search function to search for an
				   mnemonic that matches the "mnemonic"
				   parameter that we're sending in.
				*/
	index = 1;

	status = search_ops_and_mnemonics( *opcode, primary_mnemonic, &index );
	if( status != 0 )
	{
				/* If I get here, then there was no match
				   for the mnemonic.  Return an error.
				*/
		return( 1 );
	}
	else
	{
				/* If I get here, I have a match and life
				   is good!  We can proceed with our work.
				*/
				
				/* We will get the basic opcode out of our
				   data base.  After this, all we need to
				   do is insert the register references,
				   immediate values, ...
				*/
		*opcode = ops_and_mnemonics[index].opcode;

				/* Now I need to insert some specific fields
				   in the opcode.  What fields I put in will
				   depend on the "form" of the instruction.
				   I don't like "switch" statements, so I'm
				   going to use a series of "if-then-else".
				*/
		if( ops_and_mnemonics[index].form == I )
		{
				/* We've got an "I" form instruction.
				   Put the appropriate fields into
				   the opcode.
				*/
			tmp_field = pull_immediate( operand_1 );

/* printf("\n tmp_field = %d\n",tmp_field); */
/* GOOBER */
			tmp_field = pull_immediate_branch( 
			        operand_1,
				EXTRACT_LK_AA(*opcode), current_address);
				*opcode = INSERT_BD( *opcode, tmp_field );


			*opcode = INSERT_LI( *opcode, tmp_field );
				
		}
				/* Branch or simplified mnemonic
				    for a branch...
				*/
		else if( ops_and_mnemonics[index].form == B ||
		         (ops_and_mnemonics[index].form >= smBC &&
                          ops_and_mnemonics[index].form <= (smBCCTRL+4))
		       )
		{
				/* We've got a "B" form instruction
				   or a simplified branch instruction.
				   Put the appropriate fields into
				   the opcode.
				*/
		
			if(ops_and_mnemonics[index].form == B)
			{ 
				tmp_field = pull_immediate( operand_1 );
				*opcode = INSERT_BO( *opcode, tmp_field );
			
				tmp_field = pull_immediate( operand_2 );
				*opcode = INSERT_BI( *opcode, tmp_field );


				tmp_field = pull_immediate_branch( 
			 	   operand_3,
				   EXTRACT_LK_AA(*opcode), current_address);
				*opcode = INSERT_BD( *opcode, tmp_field );
			}
				/* else we have a simplified mnemonic!! */
			else
			{
				/* If we have blr, bctr, blrl, bcctrl then there
				    aren't any parameters to read in.
				*/
				if(ops_and_mnemonics[index].form == 
				(smBCLR-1) ||
				ops_and_mnemonics[index].form == (smBCCTR-1) ||
				ops_and_mnemonics[index].form == (smBCLRL-1) ||
				ops_and_mnemonics[index].form == (smBCCTRL-1)
			    	  )
				{
					/* Do nothing!...no parameters!! */	
				}
				/* Else, if we have an instruction 
				   that doesn't
				   use the condition register 
				   at all (i.e. bdnzlr)
				   and just uses the CTR to 
				   determine if a branch
				   should be taken, do this section.
				*/
			 	else if(ops_and_mnemonics[index].form==
				(smBC+4) ||
				ops_and_mnemonics[index].form==(smBCA+4) ||
				ops_and_mnemonics[index].form==(smBCLR+4) ||
				ops_and_mnemonics[index].form==(smBCCTR+4) ||
				ops_and_mnemonics[index].form==(smBCL+4) ||
				ops_and_mnemonics[index].form==(smBCLA+4) ||
				ops_and_mnemonics[index].form==(smBCLRL+4) ||
				ops_and_mnemonics[index].form==(smBCCTRL+4)
				       )	
				{
		/* If we've got a bc/bca/bcl/bcla simplified
		    mnemonic starting with "bt" or "bf" 
		    (i.e. bta,btl,bt,etc.) then it has 2 operands...
		*/
					if((strncmp(
				        &ops_and_mnemonics[index].mnemonic[0],
					"bt",2)==0 ||
				        strncmp(
	  				&ops_and_mnemonics[index].mnemonic[0],
					"bf",2)==0
	    				   ) &&
	    				(ops_and_mnemonics[index].form == 
					(smBC+4) || 
				        ops_and_mnemonics[index].form == 
					(smBCA+4) ||
				        ops_and_mnemonics[index].form == 
					(smBCL+4) ||
				        ops_and_mnemonics[index].form == 
					(smBCLA+4)
				        )
				          ) 
					{
                                       		tmp_field = pull_immediate_cr( 
							operand_1 );
                                                *opcode = INSERT_BI( 
							*opcode, tmp_field );

						tmp_field = pull_immediate_branch( 
			 	   		operand_2,
				   		EXTRACT_LK_AA(*opcode), current_address);
						*opcode = INSERT_BD( 
							*opcode, tmp_field );
					}
		/* Else, if we have a blcr/bcctr/bclrl/bcctrl
		    mnemonic starting with "bt" or "bf" then
		    we just need to read in the bit of the 
		    condition register to check...the branch location
		    is either the CTR or the LR.
		*/
					else if((strncmp(
					&ops_and_mnemonics[index].mnemonic[0],
					"bt",2)==0 ||
					strncmp(
					&ops_and_mnemonics[index].mnemonic[0],
					"bf",2)==0) &&
					(ops_and_mnemonics[index].form == 
					(smBCLR+4) || 
				        ops_and_mnemonics[index].form == 
					(smBCCTR+4) ||
				        ops_and_mnemonics[index].form == 
					(smBCLRL+4) || 
				        ops_and_mnemonics[index].form == 
					(smBCCTRL+4)
				        )
					      )
					{
						tmp_field = pull_immediate_cr( 
							operand_1 );
						*opcode = INSERT_BI( 
							*opcode, tmp_field );
					}
		/* Else, we've may have one of these instructions...
		    bdnzlr,bdzlr,bdnzlrl,bdzlrl which don't take
		    any operands.
		*/
					else if(ops_and_mnemonics[index].form==
					(smBCLR+4) ||
				        ops_and_mnemonics[index].form==
					(smBCLRL+4)
					       )
					{
					/* Do nothing!! */
					}
		/* Else just pull the offset!! */
					else
					{

					    tmp_field = pull_immediate_branch( 
			 	   		operand_1,
				   		EXTRACT_LK_AA(*opcode), current_address);
						*opcode = INSERT_BD(
							*opcode,tmp_field);
					}
				}
			/* Do this section if you need to read both
			   the CR # and the offset to branch to...
			*/
				else
				{
					tmp_field = pull_register(operand_1);
					tmp_field = tmp_field * 4;
		/* This part turns CR4 into bits 16:19
		    depending on the extension to the
		    form (i.e. Less than instructions always
		    look at the0 bit offset in the 
		    appropriate CR, >:1, =:2,so/un:3)
		*/
					switch(ops_and_mnemonics[index].form)
					{
					case smBC:
					case (smBC+1):	
					case (smBC+2):
					case (smBC+3): tmp_field = tmp_field+
					 (ops_and_mnemonics[index].form-smBC); 
					 break;
					case smBCA:
					case (smBCA+1):
					case (smBCA+2):
					case (smBCA+3): tmp_field = tmp_field+
			  	         (ops_and_mnemonics[index].form-smBCA); 
					 break;
					case smBCLR:
					case (smBCLR+1):
					case (smBCLR+2):
					case (smBCLR+3): tmp_field = tmp_field+
			                 (ops_and_mnemonics[index].form-
						smBCLR); 
					 break;
					case smBCCTR:
					case (smBCCTR+1):
					case (smBCCTR+2):
					case (smBCCTR+3): tmp_field = tmp_field+
					 (ops_and_mnemonics[index].form-
						smBCCTR);
					 break;
					case smBCL:
					case (smBCL+1):
					case (smBCL+2):
					case (smBCL+3): tmp_field = tmp_field+
					 (ops_and_mnemonics[index].form-smBCL);
					 break;
					case smBCLA:
					case (smBCLA+1):
					case (smBCLA+2):
					case (smBCLA+3): tmp_field = tmp_field+
					 (ops_and_mnemonics[index].form-
						smBCLA); 
					 break;
					case smBCLRL:
					case (smBCLRL+1):
					case (smBCLRL+2):
					case (smBCLRL+3):tmp_field=tmp_field+
					 (ops_and_mnemonics[index].form-
						smBCLRL); 
					 break;
					case smBCCTRL:
					case (smBCCTRL+1):
					case (smBCCTRL+2):
					case (smBCCTRL+3):tmp_field=tmp_field+
		   			 (ops_and_mnemonics[index].form-
						smBCCTRL); 
		   			 break;
					}
					*opcode = INSERT_BI( 
						*opcode, tmp_field );
		/* A few instructions don't need the offset
		    so skip that part! 
		*/
					if((ops_and_mnemonics[index].form >= 
					smBCLR &&
				    	ops_and_mnemonics[index].form <= 
					(smBCCTR+4)) ||
				        (ops_and_mnemonics[index].form >=
					smBCLRL &&
				        ops_and_mnemonics[index].form <= 
					(smBCCTRL+4))
	   				  )
					{}
					else
					{

					    tmp_field = pull_immediate_branch( 
			 	   		operand_2,
				   		EXTRACT_LK_AA(*opcode), current_address);
					    *opcode = INSERT_BD( 
						*opcode, tmp_field );	
					}	
				}
			}
		}
		else if( ops_and_mnemonics[index].form == SC )
		{
				/* We've got a "SC" form instruction.
				   Put the appropriate fields into
				   the opcode.
				*/

				/* SC form instructions have no arguments
				*/
		}
		else if( ops_and_mnemonics[index].form == D ||
			 ops_and_mnemonics[index].form == smD
		       )
		{
				/* We've got a "D" form instruction
				   or a simiplifed mnemonic for a
				   trap immediate instruction (smD)
				   or a load simplified mnemonic.
				   Put the appropriate fields into
				   the opcode.
				*/
				
				/* This instruction set is such a %$#&ing
				   KLUDGE!  I need to test a few things on
				   the primary opcode to determine what fields
				   I need to pull out.  That's right, the
				   type of instruction doesn't do that for me!
				*/

				/* I'm going to break the D form instructions
				   into 5 catagories, based on the operands
				   in the mnemonic:
				   1.  rt, ra, si
				   1a. bf, l, ra, si
				   1b. to, ra, si
				   1c. ra, rs, si
				   2.  rt, d(ra)
				   2a. frt, d(ra)
				*/
		
			tmp_long = EXTRACT_PRIMARY( *opcode );
			
			if( ( tmp_long <= 29 ) &&
			    ( tmp_long >= 2  )
			  )
			{
			
				/* We've got all the 1 and 1a catagories
				   in here.  Now we need to seperate those
				   two catagories.
				*/
				if( ( tmp_long == 11 ) ||
				    ( tmp_long == 10 )
				  )
				{
				
				/* Now I've got the 1a. instructions.
				*/
				
					if(ops_and_mnemonics[index].form != smD)
					{	
						tmp_field = pull_immediate( 
							operand_1 );
						*opcode = INSERT_BF( 
							*opcode, tmp_field );
						tmp_field = pull_immediate( 
							operand_2 );
						*opcode = INSERT_L( 
							*opcode, tmp_field );
						tmp_field = pull_register( 
							operand_3 );
						*opcode = INSERT_RA( 									*opcode, tmp_field );
					}
					else 
					{
						tmp_field = pull_register( 
							operand_1 );
						*opcode = INSERT_BF( 
							*opcode,tmp_field );
						tmp_field = pull_register( 
							operand_2);
						*opcode = INSERT_RA(
							*opcode,tmp_field);
					}	
				}
				else if( ( tmp_long == 2 ) ||
				         ( tmp_long == 3 )
				       )
				{
				
				/* Now I've got the 1b. instructions.
				*/
				/* The simplifed mnemonics only have two
				    operands...the register and the value
				    ...so we can skip the
				    reading of the immediate
				*/
					if(ops_and_mnemonics[index].form != smD)
					{	
						tmp_field = pull_immediate( 
							operand_1 );
						*opcode = INSERT_TO( 
							*opcode, tmp_field );
						tmp_field = pull_register( 
							operand_2 );
						*opcode = INSERT_RA( 
							*opcode, tmp_field );
					}
					else
					{
						tmp_field = pull_register(
							operand_1);
						*opcode = INSERT_RA(
							*opcode,tmp_field);
					}	
				}
				else if( ( tmp_long == 28 ) ||
				         ( tmp_long == 29 ) ||
				         ( tmp_long == 24 ) ||
				         ( tmp_long == 25 ) ||
				         ( tmp_long == 26 ) ||
				         ( tmp_long == 27 )
				       )
				{
				/* Ignore the rest of the line if NOP */	
				     if(strncmp(&
                                     ops_and_mnemonics[index].mnemonic[0],
                                     "nop",3)!=0	
				       )
				     {
				/* Now I've isolated out the 1c instructions.
				*/
				
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RA(*opcode,tmp_field);
					
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RS(*opcode,tmp_field);
				     }
				}
				else
				{
				
				/* Now I have to take care of the 1.
				   instructions.
				*/
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RT(*opcode,tmp_field);

				/* If we have a "la" or "li" simplified mnemonic
				    then we only have 2 operands...
				*/
					if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"la",2)==0
					  )
					{	
				/* I had to do something a little wierd for
				   for this one because the format of the
				   operand was different from the rest...so
				   I read it into operand_5 and used it
				   from there.
				*/
						pull_next_word_2(
							3,operand_5,mnemonic);
						tmp_field = pull_register(
							operand_5);
						*opcode = INSERT_RA(
							*opcode,tmp_field);

						tmp_field = pull_immediate(
							operand_2);
						*opcode = INSERT_SI(
							*opcode,tmp_field);
					}
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"li",2)!=0
					       )
					{	
						tmp_field = pull_register( 
							operand_2 );
						*opcode = INSERT_RA( 
							*opcode, tmp_field );
					} 
				}
				
				/* Now take care of the last operand of
				   types 1, 1a, 1b, and 1c instructions.
				*/
				
				/* Note that we have one more compare to do.
				   If this is of type 1a, then this is the 4th
				   argument, otherwise, this is the 3rd.
				*/
				
				/* If we have a simplifed mnemonic at
				    this point it's a load...
				*/
				if( ((tmp_long == 10) || (tmp_long == 11)) &&
				    ops_and_mnemonics[index].form != smD 
				  )
				{
				/* We're in here if we don't have a simplifed
				   mnemonic!!
				*/
					tmp_field = pull_immediate( operand_4 );
				}
				else
				/* We're here if we have a simplified 
				   mnemonic or the primary opcode wasn't
				   10 or 11...
				*/
				{
					if(ops_and_mnemonics[index].form==smD &&
					   tmp_long == 3
					  )
					{
						tmp_field = pull_immediate(
							operand_2);
					}
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"li",2)==0 &&
					 ops_and_mnemonics[index].form == smD
				               )
                                    	{
                                       		tmp_field = pull_immediate(
							operand_2);
                                               	*opcode = INSERT_SI(
							*opcode,tmp_field);
                                        }
					else if(strncmp(&
                                        ops_and_mnemonics[index].mnemonic[0],
                                        "la",2) == 0
                                               )
                                        {
                        /* DO NOTHING IF "la" instruction...already done! */
                                        }
                                        else if(strncmp(&
                                        ops_and_mnemonics[index].mnemonic[0],
                                        "nop",3)==0
                                               )
					{
			/* NO DOTHING IF "nop" instruction!...already done */
					}
					else if(ops_and_mnemonics[index].form==
						smD &&
						(tmp_long == 12 ||
						 tmp_long == 13 ||
						 tmp_long == 15 ||
						 tmp_long == 14)
					       )
					{
						tmp_field = pull_immediate( 								operand_3 );
						tmp_field = -tmp_field;	
					}
					else
					{
						tmp_field = pull_immediate( 
							operand_3 );
					}
				}
				if(strncmp(&
                                ops_and_mnemonics[index].mnemonic[0],
                                "nop",3)!=0
                                  )
                                { 
                        /* NO DOTHING IF "nop" instruction!...
			   already done 
			   ELSE...load the last integer
			*/
				    *opcode = INSERT_SI( *opcode, tmp_field );
                                }
			}
			else
			{
				/* By definition, if we get here, we're
				   dealing with types 2, and 2a instructions.
				*/
				
				/* First thing we'll do is seperate out
				   the type 2a instructions.
				*/
				
				if( ( tmp_long == 50 ) ||
				    ( tmp_long == 51 ) ||
				    ( tmp_long == 48 ) ||
				    ( tmp_long == 49 ) ||
				    ( tmp_long == 54 ) ||
				    ( tmp_long == 55 ) ||
				    ( tmp_long == 52 ) ||
				    ( tmp_long == 53 )
				  )
				{
				
				/* Deal with getting the first operand of
				   type 2a instructions.
				*/
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_FRT(*opcode,tmp_field);
				}
				else
				{
				
				/* If I get here, then by definition, I'm
				   dealing with type 2 instructions.  All I
				   need to do here is get the first operand
				   of these types of instructions.
				*/
				
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RT(*opcode,tmp_field);
				}
				
				/* Now I need to get the last 2 operands for
				   types 2, and 2a instructions.
				*/
				
				tmp_field = pull_immediate( operand_2 );
				*opcode = INSERT_D( *opcode, tmp_field );
				
				tmp_field = pull_register( operand_3 );
				*opcode = INSERT_RA( *opcode, tmp_field );
				
			}
		}
		else if( ops_and_mnemonics[index].form == DS )
		{
				/* We've got a "DS" form instruction.
				   Put the appropriate fields into
				   the opcode.
				*/
				
			tmp_field = pull_register( operand_1 );
			*opcode = INSERT_RT( *opcode, tmp_field );
		
			tmp_field = pull_immediate( operand_2 );
			*opcode = INSERT_DS( *opcode, tmp_field );
			
			tmp_field = pull_register( operand_3 );
			*opcode = INSERT_RA( *opcode, tmp_field );
		}
		else if( ops_and_mnemonics[index].form == X ||
			 ops_and_mnemonics[index].form == smX
		       )
		{
				/* We've got a "X" or smX form instruction.
				   The simplified mnemonics are the compare
				   word and the trap register instructions.
				   Put the appropriate fields into
				   the opcode.
				*/

				/* Here are the 20 cases we need to check for
				   inside the X Form definition.

				bits...
			#       0	6	11	16	21	31
			
			
			1	OPCD	RT	RA	RB	XO	RC
			2	OPCD	BF\L	RA	RB	XO	RC
			3	OPCD	RS	RA	\\\	XO	RC
			4	OPCD	\\\	RA	RB	XO	RC
			5	OPCD	\\\	\\\	\\\	XO	RC
			6	OPCD	FRT	\\\	FRB	XO	RC
			7	OPCD	BF \\	FRA	FRB	XO	RC
			8	OPCD	FRT	RA	RB	XO	RC
			9	OPCD	RS	RA	NB	XO	RC
			10	OPCD	BF\\	BFA\\	\\\	XO	RC
			11	OPCD	BF\\	\\\	\\\	XO	RC
			12	OPCD	RS	\\\	\\\	XO	RC
			13	OPCD	FRT	\\\	\\\	XO	RC
			14	OPCD	RT	BF	\\\	XO	RC
			15	OPCD	RT	\\\	RB	XO	RC
			16	OPCD	BF	\\\	\\\	XO	RC
			17	OPCD	BF\\	\\\	U\	XO	RC
			18	OPCD	\\\	\\\	RB	XO	RC
			19	OPCD	RT	RA	SH	XO	RC
			20	OPCD	TO	RA	RB	XO	RC
			
				*/


				/* The X form of instructions seems to be
				   the "catch-all" form.  There are actually
				   quite a few different things thrown into
				   this sack.  For that reason, this section
				   of code is way-sphagetti-like.  Sorry,
				   just couldn't be avoided...
				*/

				/* First, get the opcode.  We'll have to
				   do some comparisons on it.
				*/
			tmp_long = EXTRACT_PRIMARY( *opcode );

			if( tmp_long == 31 )
			{
				tmp_long = EXTRACT_SECONDARY_X( *opcode );
				
				if( ( tmp_long == 0  ) ||
				    ( tmp_long == 32 )
				  )
				{
				
				/* Take care of case 2
				*/
					
					if(ops_and_mnemonics[index].form != smX)
					{			
						tmp_field = pull_immediate( 
							operand_1 );
						*opcode = INSERT_BF( 
							*opcode, tmp_field );
						tmp_field = pull_immediate( 
							operand_2 );
						*opcode = INSERT_L( 
							*opcode, tmp_field );
						tmp_field = pull_register( 
							operand_3 );
						*opcode = INSERT_RA( 
							*opcode, tmp_field );
						tmp_field = pull_register( 
							operand_4 );
						*opcode = INSERT_RB( 
							*opcode, tmp_field );
					}
			/* Here's the simplified mnemonic part for the
			   compare logical word instructions...
			*/
					else
					{
						tmp_field = pull_register(
							operand_1);
						*opcode = INSERT_BF( 
							*opcode, tmp_field );
						tmp_field = pull_register(
							operand_2);
						*opcode = INSERT_RA(
							*opcode,tmp_field);
						tmp_field = pull_register(
							operand_3);
						*opcode = INSERT_RB(
							*opcode,tmp_field);	
					}
				}
				else if( ( tmp_long == 58 ) ||
				         ( tmp_long == 26 ) ||
				         ( tmp_long == 954 ) ||
				         ( tmp_long == 922 ) ||
				         ( tmp_long == 986 )
				       )
				{
				
				/* Take care of case 3
				*/
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RA(*opcode,tmp_field);

					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RS(*opcode,tmp_field);
				}
				else if( ( tmp_long == 86  ) ||
				         ( tmp_long == 470  ) ||
				         ( tmp_long == 54   ) ||
				         ( tmp_long == 278  ) ||
				         ( tmp_long == 246  ) ||
				         ( tmp_long == 1014 ) ||
				         ( tmp_long == 982 )
				       )
				{
				
				/* Take care of case 4
				*/
				
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RA(*opcode,tmp_field);

					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RB(*opcode,tmp_field);

				}
				else if( ( tmp_long == 854 ) ||
				         ( tmp_long == 498 ) ||
				         ( tmp_long == 598 ) ||
				         ( tmp_long == 370 ) ||
				         ( tmp_long == 566 )
				       )
				{
				
				/* Take care of case 5
				*/
				
				/* Case 5 has no arguments, so print nothing.
				*/
			
				}
				else if( ( tmp_long == 599 ) ||
				         ( tmp_long == 631 ) ||
				         ( tmp_long == 567 ) ||
				         ( tmp_long == 535 ) ||
				         ( tmp_long == 759 ) ||
				         ( tmp_long == 727 ) ||
				         ( tmp_long == 983 ) ||
				         ( tmp_long == 695 ) ||
				         ( tmp_long == 663 )
				       )
				{
				     	
				/* take care of case 8
				*/
					tmp_field = pull_register( operand_1 );				
					*opcode = INSERT_FRT(*opcode,tmp_field);
				     	
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RA(*opcode,tmp_field);
				     	
					tmp_field = pull_register( operand_3 );
					*opcode = INSERT_RB(*opcode,tmp_field);
				}
				else if( ( tmp_long == 597 ) ||
				         ( tmp_long == 725 )
				       )
				{
				
				/* Take care of case 9
				*/
				
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RS(*opcode,tmp_field);
					
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RA(*opcode,tmp_field);
					
					tmp_field = pull_immediate( operand_3 );
					*opcode = INSERT_NB(*opcode,tmp_field);
				
				}
				else if( ( tmp_long == 512 )
				       )
				{
				
				/* Take care of case 11
				*/
				
					tmp_field = pull_immediate( operand_1 );
					*opcode = INSERT_BF(*opcode,tmp_field);
				     	
				}
				else if( ( tmp_long == 19 ) ||
				         ( tmp_long == 83 ) ||
				         ( tmp_long == 146 )
				       )
				{
				
				/* Take care of case 12
				*/
				
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RS(*opcode,tmp_field);
				     	
				}
				else if( ( tmp_long == 210 )	/* mtsr SR,rx */
				       )
				{
				
				/* Take care of case 14, part 1
				*/
					
					tmp_field = pull_immediate( operand_1 );
					
				/* We need to make sure that the field I'm
				   about to insert is only 4 bits (bits
				   12-15).  This case is only for the mtsr
				   and mfsr instructions.
				*/
					tmp_field = tmp_field & 0xF;
					*opcode = INSERT_BI(*opcode,tmp_field); 				     	
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RS(*opcode,tmp_field);
					
				}
				else if( ( tmp_long == 595 )	/* mfsr rx,SR */
				       )
				{
				
				/* Take care of case 14, part 2
				*/
					
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RS(*opcode,tmp_field);
					
					tmp_field = pull_immediate( operand_2 );
					
				/* We need to make sure that the field I'm
				   about to insert is only 4 bits (bits
				   12-15).  This case is only for the mtsr
				   and mfsr instructions.
				*/
					tmp_field = tmp_field & 0xF;  
					*opcode = INSERT_BI(*opcode,tmp_field);
				     	
				}
				else if( ( tmp_long == 659 ) ||
				         ( tmp_long == 242 )
				       )
				{
				
				/* Take care of case 15
				*/
					
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RS(*opcode,tmp_field);
					
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RB(*opcode,tmp_field);
				     	
				}
				else if( ( tmp_long == 434 ) ||
				         ( tmp_long == 466 ) ||
				         ( tmp_long == 306 ) ||
				         ( tmp_long == 338 )
				       )
				{
				
				/* Take care of case 18
				*/
				
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RB(*opcode,tmp_field);
					
				}
				else if( ( tmp_long == 184 ) ||
				         ( tmp_long == 248 ) ||
				         ( tmp_long == 952 ) ||
				         ( tmp_long == 824 ) ||
				         ( tmp_long == 696 ) ||
				         ( tmp_long == 760 )
				       )
				{
				
				/* Take care of case 19
				*/
				
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RA(*opcode,tmp_field);
					
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RS(*opcode,tmp_field);
					
					tmp_field = pull_immediate( operand_3 );
					*opcode = INSERT_SH32(
						*opcode,tmp_field);
					
				}
				else if( ( tmp_long == 68 ) ||
				         ( tmp_long == 4  )
				       )
				{
					
				/* Take care of case 20
				*/
					if(ops_and_mnemonics[index].form != smX)
					{	
						tmp_field = pull_immediate( 
							operand_1 );
						*opcode = INSERT_TO( 
							*opcode, tmp_field );
					
						tmp_field = pull_register( 
							operand_2 );
						*opcode = INSERT_RA( 
							*opcode, tmp_field );
					
						tmp_field = pull_register( 
							operand_3 );
						*opcode = INSERT_RB( 
							*opcode, tmp_field );
					}
					else
				/* Here's the part for the simplified
			    	   mnemonic trap register instructions
				*/
					{
						tmp_field = pull_register(
							operand_1);
						*opcode = INSERT_RA(
							*opcode,tmp_field);
						tmp_field = pull_register(
							operand_2);	
						*opcode = INSERT_RB(
							*opcode,tmp_field);
					}	
				}
			
								
				/* This is the DSS form - it is it's own case.
				*/	
			
				 else if( ( tmp_long == 822 ) )

					{		
					tmp_field = pull_immediate ( operand_1 );
					*opcode = INSERT_STRM(*opcode,tmp_field);
					}
				/* These are the X forms like DST
				*/	
			
				 else if( ( tmp_long == 374 ) ||
				     ( tmp_long == 342 )
				  )

					{		
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_FRA(*opcode,tmp_field);
					
					tmp_field = pull_register ( operand_2 );
					*opcode = INSERT_FRB(*opcode,tmp_field);

					tmp_field = pull_immediate( operand_3 );
					*opcode = INSERT_STRM(*opcode,tmp_field);

					}

				 else if( ( tmp_long == 822 ) )

					{		

					tmp_field = pull_immediate( operand_1 );
					*opcode = INSERT_STRM(*opcode,tmp_field);

					}

				else
				{	
				/* Any other X form instructions that have 31
				   as their primary opcode, are in case 1.
				*/
				
				/* However...
				   If it's a ld or st that's of the X form,
				   are fit in this catagory, then the operands
				   do not get switched, like they do for all
				   other instructions that filter into this
				   cubby hole.
				*/
					tmp_long = EXTRACT_SECONDARY_X(*opcode);
					if( ( tmp_long == 119 ) ||
					    ( tmp_long == 87  ) ||
					    ( tmp_long == 279 ) ||
					    ( tmp_long == 311 ) ||
					    ( tmp_long == 343 ) ||
					    ( tmp_long == 375 ) ||
					    ( tmp_long == 23  ) ||
					    ( tmp_long == 55  ) ||
					    ( tmp_long == 341 ) ||
					    ( tmp_long == 373 ) ||
					    ( tmp_long == 21  ) ||
					    ( tmp_long == 53  ) ||
					    ( tmp_long == 215 ) ||
					    ( tmp_long == 247 ) ||
					    ( tmp_long == 407 ) ||
					    ( tmp_long == 439 ) ||
					    ( tmp_long == 311 ) ||
					    ( tmp_long == 151 ) ||
					    ( tmp_long == 183 ) ||
					    ( tmp_long == 149 ) ||
					    ( tmp_long == 181 ) ||
					    ( tmp_long == 790 ) ||
					    ( tmp_long == 534 ) ||
					    ( tmp_long == 918 ) ||
					    ( tmp_long == 662 ) ||
					    ( tmp_long == 533 ) ||
					    ( tmp_long == 661 ) ||
					    ( tmp_long == 20  ) ||
					    ( tmp_long == 84  ) ||
					    ( tmp_long == 150 ) ||
					    ( tmp_long == 214 ) ||
					    ( tmp_long == 310 ) ||
					    ( tmp_long == 438 ) ||
					    ( tmp_long == 7   ) ||
					    ( tmp_long == 39  ) ||
					    ( tmp_long == 71  ) ||
					    ( tmp_long == 6   ) ||
					    ( tmp_long == 38  ) ||
					    ( tmp_long == 103 ) ||
					    ( tmp_long == 359 ) ||
					    ( tmp_long == 135 ) ||
					    ( tmp_long == 167 ) ||
					    ( tmp_long == 199 ) ||
					    ( tmp_long == 231 ) ||
					    ( tmp_long == 487 )
					  )
					{
						tmp_field = pull_register( 
							operand_1 );
						*opcode = INSERT_RS( 
							*opcode, tmp_field );
	
						tmp_field = pull_register( 
							operand_2 );
						*opcode = INSERT_RA( 
							*opcode, tmp_field );
	
						tmp_field = pull_register( 
							operand_3 );
						*opcode = INSERT_RB( 
							*opcode, tmp_field );
				
					}
					else
					{
				/* We're back to the few X form instructions
				   that fit into this little catagory who
				   have their arguments reversed.  In other
				   words, their arguments appear in the
				   assembly as: RA,RS,RB and are encoded
				   as RS,RA,RB.
				*/
						tmp_field = pull_register( 
							operand_1 );
						*opcode = INSERT_RA( 
							*opcode, tmp_field );
				
						tmp_field = pull_register( 
							operand_2 );
						*opcode = INSERT_RS( 
							*opcode, tmp_field );
						if(ops_and_mnemonics[index].form
							 == smX)
						{
					 	  *opcode = INSERT_RB(
						    	  *opcode,tmp_field);
						}
						else	
						{
						  tmp_field = pull_register( 
							  operand_3 );
						  *opcode = INSERT_RB( 
							  *opcode, tmp_field );
						}
					}
				}
			}

			else if( tmp_long == 63 )
			{
			
				/* Take care of cases where primary opcode is
				   63.
				*/
				tmp_long = EXTRACT_SECONDARY_X( *opcode );
				
				if( ( tmp_long == 264 ) ||
				    ( tmp_long == 846 ) ||
				    ( tmp_long == 814 ) ||
				    ( tmp_long == 815 ) ||
				    ( tmp_long == 14  ) ||
				    ( tmp_long == 15  ) ||
				    ( tmp_long == 72  ) ||
				    ( tmp_long == 136 ) ||
				    ( tmp_long == 40  ) ||
				    ( tmp_long == 12  )
				  )
				{
			
				/* take care of case 6
				*/
				
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_FRT(*opcode,tmp_field);
					
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_FRB(*opcode,tmp_field);
				
				}
				else if( ( tmp_long == 32 ) ||
				         ( tmp_long == 0  )
				       )
				{
				     	
				/* take care of case 7
				*/
				
					tmp_field = pull_immediate( operand_1 );
					*opcode = INSERT_BF(*opcode,tmp_field);
				     	
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_FRA(*opcode,tmp_field);
				     	
					tmp_field = pull_register( operand_3 );
					*opcode = INSERT_FRB(*opcode,tmp_field);
				}
				else if( ( tmp_long == 64 )
				       )
				{
				     	
				/* take care of case 10
				*/
				
					tmp_field = pull_immediate( operand_1 );
					*opcode = INSERT_BF(*opcode,tmp_field);
				     	
					tmp_field = pull_immediate( operand_2 );
					*opcode = INSERT_BFA(*opcode,tmp_field);

				}
				else if( ( tmp_long == 583 )
				       )
				{
				     	
				/* take care of case 13
				*/
				
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_FRT(*opcode,tmp_field);

				}
				else if( ( tmp_long == 70 ) ||
				         ( tmp_long == 38 )
				       )
				{
				     	
				/* take care of case 16
				*/
				
					tmp_field = pull_immediate( operand_1 );
					*opcode = INSERT_BT(*opcode,tmp_field);

				}
				else if( ( tmp_long == 134 )
				       )
				{
				     	
				/* take care of case 17
				*/
				
					tmp_field = pull_immediate( operand_1 );
					*opcode = INSERT_BF(*opcode,tmp_field);
					
					tmp_field = pull_immediate( operand_2 );
					*opcode = INSERT_U(*opcode,tmp_field);

				}
			}

		}

		else if( ( ops_and_mnemonics[index].form == VX ) ||
		       ( ops_and_mnemonics[index].form == VXR ) ||
		       ( ops_and_mnemonics[index].form == smVXR ) ||
		       ( ops_and_mnemonics[index].form == smVX ) ) 
		       
		{
				/* We've got a "VX" form instruction. */
				/* Here are the 20 cases we need to check for
				   inside the VX Form definition.

				bits...
			#       0	6	11	16	21	31
			
			
			1	OPCD	RT	RA	RB	XO	RC
			
						*/


			tmp_long = EXTRACT_PRIMARY( *opcode );

			if( tmp_long == 04 )			/* Just a doublecheck */
			{
				tmp_long = EXTRACT_SECONDARY_V( *opcode );
				/* These are the VX forms like VADDCUW
				*/	
			
				 if( ( tmp_long == 384 ) ||
					(tmp_long == 1284) ||
					(tmp_long == 1156) ||
					( tmp_long == 10 ) ||
					( tmp_long == 768 ) ||
					( tmp_long == 832 ) ||
					( tmp_long == 896 ) ||
					( tmp_long == 0 ) ||
					( tmp_long == 512 ) ||
					( tmp_long == 64 ) ||
					( tmp_long == 576 ) ||
					( tmp_long == 128 ) ||
					( tmp_long == 640 ) ||
					( tmp_long == 1028 ) ||
					( tmp_long == 1092 ) ||
					( tmp_long == 1282 ) ||
					( tmp_long == 1346 ) ||
					( tmp_long == 1410 ) ||
					( tmp_long == 1026 ) ||
					( tmp_long == 1090 ) ||
					( tmp_long == 1154 ) ||
					( tmp_long == 1034 ) ||
					( tmp_long == 258 ) ||
					( tmp_long == 322 ) ||
					( tmp_long == 386 ) ||
					( tmp_long == 2 ) ||
					( tmp_long == 66 ) ||
					( tmp_long == 130 ) ||
					( tmp_long == 1098 ) ||
					( tmp_long == 770 ) ||
					( tmp_long == 834 ) ||
					( tmp_long == 898 ) ||
					( tmp_long == 514 ) ||
					( tmp_long == 578 ) ||
					( tmp_long == 642 ) ||
					( tmp_long == 12 ) ||
					( tmp_long == 76 ) ||
					( tmp_long == 140 ) ||
					( tmp_long == 268 ) ||
					( tmp_long == 332 ) ||
					( tmp_long == 396 ) ||
					( tmp_long == 776 ) ||
					( tmp_long == 840 ) ||
					( tmp_long == 520 ) ||
					( tmp_long == 584 ) ||
					( tmp_long == 264 ) ||
					( tmp_long == 328 ) ||
					( tmp_long == 8 ) ||
					( tmp_long == 72 ) ||
					( tmp_long == 1284 ) ||
					( tmp_long == 1156 ) ||
					( tmp_long == 782 ) ||
					( tmp_long == 398 ) ||
					( tmp_long == 270 ) ||
					( tmp_long == 462 ) ||
					( tmp_long == 334 ) ||
					( tmp_long == 14 ) ||
					( tmp_long == 142 ) ||
					( tmp_long == 78 ) ||
					( tmp_long == 206 ) ||
					( tmp_long == 4 ) ||
					( tmp_long == 68 ) ||
					( tmp_long == 132 ) ||
					( tmp_long == 452 ) ||
					( tmp_long == 260 ) ||
					( tmp_long == 324 ) ||
					( tmp_long == 1036 ) ||
					( tmp_long == 388 ) ||
					( tmp_long == 708 ) ||
					( tmp_long == 772 ) ||
					( tmp_long == 836 ) ||
					( tmp_long == 900 ) ||
					( tmp_long == 516 ) ||
					( tmp_long == 580 ) ||
					( tmp_long == 1100 ) ||
					( tmp_long == 644 ) ||
					( tmp_long == 1408 ) ||
					( tmp_long == 74 ) ||
					( tmp_long == 1792 ) ||
					( tmp_long == 1856 ) ||
					( tmp_long == 1920 ) ||
					( tmp_long == 1024 ) ||
					( tmp_long == 1536 ) ||
					( tmp_long == 1088 ) ||
					( tmp_long == 1600 ) ||
					( tmp_long == 1152 ) ||
					( tmp_long == 1664 ) ||
					( tmp_long == 1672 ) ||
					( tmp_long == 1800 ) ||
					( tmp_long == 1608 ) ||
					( tmp_long == 1544 ) ||
					( tmp_long == 1928 ) ||
					( tmp_long == 1220 )
					  )
				 {
					if( ops_and_mnemonics[index].form == VX )
					{

					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RS( *opcode, tmp_field );
	
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RA( *opcode, tmp_field );
	
					tmp_field = pull_register( operand_3 );
					*opcode = INSERT_RB( *opcode, tmp_field );
					}
					else if( ops_and_mnemonics[index].form == smVX )
					{
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RS( *opcode, tmp_field );
	
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RA( *opcode, tmp_field );
					*opcode = INSERT_RB( *opcode, tmp_field );
					}
				 }

				else if( ( tmp_long == 394 ) ||
					( tmp_long == 458 ) ||
					( tmp_long == 266 ) ||
					( tmp_long == 714 ) ||
					( tmp_long == 522 ) ||
					( tmp_long == 650 ) ||
					( tmp_long == 586 ) ||
					( tmp_long == 330 ) ||
					( tmp_long == 846 ) ||
					( tmp_long == 526 ) ||
					( tmp_long == 590 ) ||
					( tmp_long == 974 ) ||
					( tmp_long == 654 ) ||
					( tmp_long == 718 )
				  ) 

				/* These are the VX forms like VEXPTEFP !! RC --> bit21 !!
				*/				

				{
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_FRT(*opcode,tmp_field);
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_FRB(*opcode,tmp_field);
				}

				else if( ( tmp_long == 966 ) ||
					( tmp_long == 1990 ) ||
					( tmp_long == 198 ) ||
					( tmp_long == 1222 ) ||
					( tmp_long == 6 ) ||
					( tmp_long == 1030 ) ||
					( tmp_long == 70 ) ||
					( tmp_long == 1094 ) ||
					( tmp_long == 134 ) ||
					( tmp_long == 1158 ) ||
					( tmp_long == 454 ) ||
					( tmp_long == 1478 ) ||
					( tmp_long == 710 ) ||
					( tmp_long == 1734 ) ||
					( tmp_long == 774 ) ||
					( tmp_long == 1798 ) ||
					( tmp_long == 838 ) ||
					( tmp_long == 1862 ) ||
					( tmp_long == 902 ) ||
					( tmp_long == 1926 ) ||
					( tmp_long == 518 ) ||
					( tmp_long == 1542 ) ||
					( tmp_long == 582 ) ||
					( tmp_long == 1606 ) ||
					( tmp_long == 1670 ) ||
					( tmp_long == 646 )
				  ) 

				/* These are the VXR forms like VCMPBFPX
				*/
				
				{
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RS(*opcode,tmp_field);
					
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RA(*opcode,tmp_field);
					tmp_field = pull_register( operand_3 );
					*opcode = INSERT_RB(*opcode,tmp_field);
				}
				
				else if( ( tmp_long == 842 ) ||
					( tmp_long == 778 ) ||
					( tmp_long == 970 ) ||
					( tmp_long == 906 ) ||
					( tmp_long == 524 ) ||
					( tmp_long == 588 ) ||
					( tmp_long == 652 )
				  )
 
				/* These are the VX forms with UIMMs like VCFSX
				*/

				{		
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RS(*opcode,tmp_field);
					
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RB(*opcode,tmp_field);
					tmp_field = pull_immediate( operand_3 );
					*opcode = INSERT_VUI(*opcode,tmp_field);
				}

				else if( ( tmp_long == 32 )
				  )
 
				/* These are the VX forms with UIMMs like VCFSX
				*/

				{		
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RS(*opcode,tmp_field);
					
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RB(*opcode,tmp_field);
					tmp_field = pull_immediate( operand_3 );
					*opcode = INSERT_VUI(*opcode,tmp_field);
				}
				else if( ( tmp_long == 780 ) ||
					( tmp_long == 844 ) ||
					( tmp_long == 908 )
				  )
 
				/* These are the VX forms with SIMMs like VSPLTISB
				*/

				{		
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RS(*opcode,tmp_field);
					tmp_field = pull_immediate( operand_2 );
					*opcode = INSERT_VSI(*opcode,tmp_field);
				}


				else if( ( tmp_long == 342 )
				  )
 
				/* These are the X forms for DS touching for AltiVec like VSPLTISB
				*/

				{		
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_FRA(*opcode,tmp_field);
					
					tmp_field = pull_register ( operand_2 );
					*opcode = INSERT_FRB(*opcode,tmp_field);

					tmp_field = pull_immediate( operand_3 );
					*opcode = INSERT_STRM(*opcode,tmp_field);
				}

				else if( ( tmp_long == 1540 ))
 

				/* This is the VX form MTVSCR
				*/
				{		
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RS(*opcode,tmp_field);
				}


				else if( ( tmp_long == 1604 )
				  )
 
				/* This is the VX form MTVSCR
				*/

				{		
					tmp_field = pull_register( operand_1 );
					*opcode = INSERT_RB(*opcode,tmp_field);
				}

			}

		}


		else if( ops_and_mnemonics[index].form == VA )	/* VA type */
		{
			tmp_long = EXTRACT_PRIMARY( *opcode );

			if( tmp_long == 04 )		/* Just a doublecheck - only "04"'s should be VA's */
			{
				tmp_long = EXTRACT_SECONDARY_V( *opcode );

				if( ( tmp_long == 32 ) ||
					( tmp_long == 33 ) ||
					( tmp_long == 34 ) ||
					( tmp_long == 37 ) ||
					( tmp_long == 40 ) ||
					( tmp_long == 41 ) ||
					( tmp_long == 36 ) ||
					( tmp_long == 38 ) ||
					( tmp_long == 39 ) ||
					( tmp_long == 43 ) ||
					( tmp_long == 42 )
			  	)
				{

				tmp_field = pull_register( operand_1 );
				*opcode = INSERT_FRT( *opcode, tmp_field );
			
				tmp_field = pull_register( operand_2 );
				*opcode = INSERT_FRA( *opcode, tmp_field );

				tmp_field = pull_register( operand_3 );
				*opcode = INSERT_FRB( *opcode, tmp_field );
					
				tmp_field = pull_register( operand_4 );
				*opcode = INSERT_FRC( *opcode, tmp_field );

				}

				else if( ( tmp_long == 44 )
			  	)
				{

				tmp_field = pull_register( operand_1 );
				*opcode = INSERT_FRT( *opcode, tmp_field );
			
				tmp_field = pull_register( operand_2 );
				*opcode = INSERT_FRA( *opcode, tmp_field );

				tmp_field = pull_register( operand_3 );
				*opcode = INSERT_FRB( *opcode, tmp_field );
					
				tmp_field = pull_immediate( operand_4 );
				*opcode = INSERT_VAS( *opcode, tmp_field );
				}
				else if( ( tmp_long == 47 ) ||
					 ( tmp_long == 46)
					 )	
			 	 
				{
				/* Form VA, but with last 2 operands
					   switched - VNMSUBFP & vmaddfp only */

				tmp_field = pull_register( operand_1 );
				*opcode = INSERT_FRT( *opcode, tmp_field );
				tmp_field = pull_register( operand_2 );
				*opcode = INSERT_FRA( *opcode, tmp_field );
				tmp_field = pull_register( operand_3 );
				*opcode = INSERT_FRC( *opcode, tmp_field );
				tmp_field = pull_register( operand_4 );
				*opcode = INSERT_FRB( *opcode, tmp_field );

				}
			}

		}

		else if( ops_and_mnemonics[index].form == XL ||
			 ops_and_mnemonics[index].form == smXL
		          )
		{
				/* We've got a "XL" form instruction
				   or a simplified mnemonic for an
				   smXL instruction.
				   Put the appropriate fields into
				   the opcode.
				*/
			
				/* Yet again, we will have to do some
				   checking on the opcodes to get this
				   right.  It turns out that all of the XL
				   form instructions have a primary opcode
				   of 19, so I don't have to even check
				   that.  All I need to do is look at the
				   secondary opcode.
				*/
			
			tmp_long = EXTRACT_SECONDARY_XL( *opcode );
			if( ( tmp_long == 528 ) ||
			    ( tmp_long == 16  ) )
			{
				/* Take care of the case where we have only
				   2 fields to read in, BO and BI.
				*/
				tmp_field = pull_immediate( operand_1 );
				*opcode = INSERT_BO( *opcode, tmp_field );
				
				tmp_field = pull_immediate( operand_2 );
				*opcode = INSERT_BI( *opcode, tmp_field );
			}
			else if( ( tmp_long == 150 ) ||
				 ( tmp_long == 50  ) )
			{
				/* Take care of the case where there are
				   no fields (all three are zeroed-out.  An
				   example would be the isync instruction.
				*/
			}
			else if( ( tmp_long == 0 ) )
			{
				/* Take care of cases where we have two
				   bit fields from the CR as arguments.  An
				   example might be the mcrf instr.
				*/
				tmp_field = pull_immediate( operand_1 );
				*opcode = INSERT_BF( *opcode, tmp_field );
			
				tmp_field = pull_immediate( operand_2 );
				*opcode = INSERT_BFA( *opcode, tmp_field );
			}
			else
			{
			
				/* If we get here, then we are in the general
				   case where we need to get 3 bit-fields.
				*/
				/* This first part is for the simplified 
				    mnemonics which only have 1 or 2 
				    operands and they are duplicated...
				*/
				if(ops_and_mnemonics[index].form==smXL)
				{
					tmp_field = pull_immediate( operand_1 );
					*opcode = INSERT_BT(*opcode,tmp_field);

					if(EXTRACT_SECONDARY_XL(*opcode)==193 ||
					    EXTRACT_SECONDARY_XL(*opcode)==289
					   )
					{
						*opcode = INSERT_BA( 
							*opcode, tmp_field );
						*opcode = INSERT_BB( 
							*opcode, tmp_field );
					}
					else
					{
						tmp_field = pull_immediate( 
							operand_2 );
						*opcode = INSERT_BA( 
							*opcode, tmp_field );
						*opcode = INSERT_BB(
						 	*opcode, tmp_field );
					}
				}
				else
				{
					tmp_field = pull_immediate( operand_1 );
					*opcode = INSERT_BT(*opcode,tmp_field);
				
					tmp_field = pull_immediate( operand_2 );
					*opcode = INSERT_BA(*opcode,tmp_field);
				
					tmp_field = pull_immediate( operand_3 );
					*opcode = INSERT_BB(*opcode,tmp_field);
				}
			}
		}
		else if( ops_and_mnemonics[index].form == XFX ||
			 ops_and_mnemonics[index].form == smXFX
	    	       )
		{
				/* We've got a "XFX" form instruction
				   or a simplified mnemonic for the
				   specail purpose registers.
				   Put the appropriate fields into
				   the opcode.
				*/
				
				/* We'll have to do some massaging
				   to see exactly what to do.
				*/
			tmp_long = EXTRACT_SECONDARY_XFX( *opcode );
			if( ( tmp_long == 339 ) ||
			    ( tmp_long == 371 )
			  )
			{
				/* We have a mfspr or mftb instr, which
				   has operands in the form of:
				   RT,SPR
				*/
				tmp_field = pull_register( operand_1 );
				*opcode = INSERT_RT( *opcode, tmp_field );
			
				/* simplified mnemonics!! mf...  MATT */	
				if(ops_and_mnemonics[index].form != smXFX ||
	   			  strncmp(&
				  ops_and_mnemonics[index].mnemonic[0],
				  "mfsprg",6)==0 ||
	   			  strncmp(&
				  ops_and_mnemonics[index].mnemonic[0],
				  "mfibatu",7)==0 ||
	   			  strncmp(&
				  ops_and_mnemonics[index].mnemonic[0],
				  "mfibatl",7)==0 ||
	   			  strncmp(&
				  ops_and_mnemonics[index].mnemonic[0],
				  "mfdbatu",7)==0 ||
	   			  strncmp(&
				  ops_and_mnemonics[index].mnemonic[0],
				  "mfdbatl",7)==0
				  )
				{
				/* If it's NOT a simplified mnemonic then
				   we need to get the SPR !
				*/
					if(ops_and_mnemonics[index].form != 
						smXFX)
					{

					  tmp_field = spr_pull(operand_2);
					  if(tmp_field > 1023)
					  // bad instruction reassemble this adr
				          return(1);
					}
				/* Else, we can skip the SPR and just get the
				   immediate value!!
				*/
					else tmp_field = pull_immediate( 
						operand_2 );
					if(ops_and_mnemonics[index].form == 
						smXFX)
					{
       					   if(strncmp(&
					   ops_and_mnemonics[index].mnemonic[0],
					   "mfsprg",6)==0
		           		     ) 
					      tmp_field = tmp_field + 272;
       					   else if(strncmp(&
					   ops_and_mnemonics[index].mnemonic[0],
					   "mfibatu",7)==0
		    				  )
					      tmp_field = (2*tmp_field) + 528;
       					   else if(strncmp(&
					   ops_and_mnemonics[index].mnemonic[0],
					   "mfibatl",7)==0
	  	    				  )
					      tmp_field = (2*tmp_field) + 529;
       					   else if(strncmp(&
					   ops_and_mnemonics[index].mnemonic[0],
					   "mfdbatu",7)==0
		    				  )
					      tmp_field = (2*tmp_field) + 536;
       					   else if(strncmp(&
					   ops_and_mnemonics[index].mnemonic[0],
					   "mfdbatl",7)==0
		    				  )
			 		      tmp_field = (2*tmp_field) + 537;
					}
					*opcode=INSERT_SPR(*opcode,tmp_field);
				}
			}
			else if( tmp_long == 144 )
			{
				/* We have an mtcrf instr which has
				   arguments in the form of:
				   FXM,RS
				*/
				tmp_field = pull_immediate( operand_1 );
				*opcode = INSERT_FXM( *opcode, tmp_field );

				tmp_field = pull_register( operand_2 );
				*opcode = INSERT_RS( *opcode, tmp_field );
			}
			else
			{
				/* Finally, we are at the last case, an
				   mtspr instr, which as arguments in the
				   form of:
				   SPR,RS
				*/
				/* simplifed mnemonic!! mt...  MATT */
				if(ops_and_mnemonics[index].form != smXFX ||
				strncmp(&
				ops_and_mnemonics[index].mnemonic[0],
				"mtsprg",6)==0 ||
           		        strncmp(&
				ops_and_mnemonics[index].mnemonic[0],
				"mtibatu",7)==0 ||
           			strncmp(&
				ops_and_mnemonics[index].mnemonic[0],
				"mtibatl",7)==0 ||
           			strncmp(&
				ops_and_mnemonics[index].mnemonic[0],
				"mtdbatu",7)==0 ||
           			strncmp(&
				ops_and_mnemonics[index].mnemonic[0],
				"mtdbatl",7)==0
 				  )
				{
					if(ops_and_mnemonics[index].form != 
						smXFX)
					{
					      tmp_field = spr_pull( operand_1 );
					  if(tmp_field  > 1023)
					  // bad instruction reassemble this adr
				          return(1);
					}
					else tmp_field = pull_immediate(
							 	operand_1 );
					if(ops_and_mnemonics[index].form == 
						smXFX)
					{
					   if(strncmp(&
					   ops_and_mnemonics[index].mnemonic[0],
					   "mtsprg",6)==0
					     )
                                        	tmp_field = tmp_field + 272;
       					   else if(strncmp(&
					   ops_and_mnemonics[index].mnemonic[0],
					   "mtibatu",7)==0
						  )
                                        	tmp_field = (2*tmp_field) + 528;
       					   else if(strncmp(&
					   ops_and_mnemonics[index].mnemonic[0],
					   "mtibatl",7)==0
						  )
                                        	tmp_field = (2*tmp_field) + 529;
       					   else if(strncmp(&
					   ops_and_mnemonics[index].mnemonic[0],
					   "mtdbatu",7)==0
						  )
                                        	tmp_field = (2*tmp_field) + 536;
       					   else if(strncmp(&
					   ops_and_mnemonics[index].mnemonic[0],
					   "mtdbatl",7)==0
						  )
                                        	tmp_field = (2*tmp_field) + 537;
					}
					*opcode = INSERT_SPR( *opcode, tmp_field );
					tmp_field = pull_register( operand_2 );
				}	
				else tmp_field = pull_register( operand_1 );
				*opcode = INSERT_RS( *opcode, tmp_field );
			}
		}
		else if( ops_and_mnemonics[index].form == XFL )
		{
				/* We've got a "XFL" form instruction.
				   Put the appropriate fields into
				   the opcode.
				*/
			
			tmp_field = pull_immediate( operand_1 );
			*opcode = INSERT_FLM( *opcode, tmp_field );
			
			tmp_field = pull_register( operand_2 );
			*opcode = INSERT_FRB( *opcode, tmp_field );
			
		}
		else if( ops_and_mnemonics[index].form == XS )
		{
				/* We've got a "XS" form instruction.
				   Put the appropriate fields into
				   the opcode.
				*/
			
			tmp_field = pull_register( operand_1 );
			*opcode = INSERT_RA( *opcode, tmp_field );
 
			tmp_field = pull_register( operand_2 );
			*opcode = INSERT_RS( *opcode, tmp_field );
			
			tmp_field = pull_immediate( operand_3 );
			*opcode = INSERT_SH64( *opcode, tmp_field );
			
		}
		else if( ops_and_mnemonics[index].form == XO ||
			 ops_and_mnemonics[index].form == smXO
		       )
		{
				/* We've got a "XO" form instruction
				   or simplified mnemonics sub/subc.
				   Put the appropriate fields into
				   the opcode.
				*/
				
				/* Yet again, we must do some
				   comparisons to tell what to do.
				*/
			tmp_long = EXTRACT_SECONDARY_XO( *opcode );
			if( ( tmp_long == 234 ) ||
			    ( tmp_long == 232 ) ||
			    ( tmp_long == 202 ) ||
			    ( tmp_long == 200 ) ||
			    ( tmp_long == 104 ) ||
			    ( tmp_long == 360 ) ||
			    ( tmp_long == 488 )
			  )
			{
				/* We have some instructions that
				   have only 2 arguments.
				*/
				tmp_field = pull_register( operand_1 );
				*opcode = INSERT_RT( *opcode, tmp_field );
				
				tmp_field = pull_register( operand_2 );
				*opcode = INSERT_RA( *opcode, tmp_field );
			}
			else
			{
				/* We have some instructions that
				   have 3 arguments.
				*/
				tmp_field = pull_register( operand_1 );
				*opcode = INSERT_RT( *opcode, tmp_field );
			
				/* sub/subc have 3 registers as operands */
				if(ops_and_mnemonics[index].form == smXO)
				{
					tmp_field = pull_register( operand_2 );
                                	*opcode = INSERT_RB(*opcode,tmp_field);
                                	tmp_field = pull_register( operand_3 );
                                	*opcode = INSERT_RA(*opcode,tmp_field);
				}	
				else
				{
					tmp_field = pull_register( operand_2 );
					*opcode = INSERT_RA(*opcode,tmp_field);
					tmp_field = pull_register( operand_3 );
					*opcode = INSERT_RB(*opcode,tmp_field);
				}
			}
		}
		else if( ops_and_mnemonics[index].form == A )
		{
				/* We've got a "A" form instruction.
				   Put the appropriate fields into
				   the opcode.
				*/
			
			tmp_field = pull_register( operand_1 );
			*opcode = INSERT_FRT( *opcode, tmp_field );
			
			tmp_long = EXTRACT_SECONDARY_A( *opcode );
			if( ( tmp_long == 18 ) ||
			    ( tmp_long == 20 ) ||
			    ( tmp_long == 21 ) ||
			    ( tmp_long == 23 ) ||
			    ( tmp_long == 25 ) ||
			    ( tmp_long == 28 ) ||
			    ( tmp_long == 29 ) ||
			    ( tmp_long == 30 ) ||
			    ( tmp_long == 31 )
			  )
			{
				/* Get FRA for these instructions
				*/
				
				tmp_field = pull_register( operand_2 );
				*opcode = INSERT_FRA( *opcode, tmp_field );
			}

			tmp_long = EXTRACT_SECONDARY_A( *opcode );
			if( ( tmp_long == 18 ) ||
			    ( tmp_long == 20 ) ||
			    ( tmp_long == 21 ) ||
			    ( tmp_long == 23 )
			  )
			{
				/* Get FRB for these instructions
				*/

				tmp_field = pull_register( operand_3 );
				*opcode = INSERT_FRB( *opcode, tmp_field );
			}
			else if( ( tmp_long == 22 ) ||	/* only frD and frB */
			    ( tmp_long == 24 ) ||
			    ( tmp_long == 26 )
			  )
			{
				/* Get FRB for these instructions
				*/

				tmp_field = pull_register( operand_2 );
				*opcode = INSERT_FRB( *opcode, tmp_field );
			}
			else if( ( tmp_long == 28 ) ||
			    ( tmp_long == 29 ) ||
			    ( tmp_long == 30 ) ||
			    ( tmp_long == 31 )
			  )
			{
 				/* Get FRC first, then we will worry about
 				   getting FRB.  These few instructions have
 				   the operands reversed.  
 				*/
				tmp_field = pull_register( operand_3 );
				*opcode = INSERT_FRC( *opcode, tmp_field );
			}
			
			tmp_long = EXTRACT_SECONDARY_A( *opcode );
			if( ( tmp_long == 23 )
			  )
			{
				/* Get FRC for these instructions
				*/

				tmp_field = pull_register( operand_4 );
				*opcode = INSERT_FRC( *opcode, tmp_field );
			}
			else if( ( tmp_long == 25 )	/* only frA and frC */
			  )
			{
				/* Get FRC for these instructions
				*/

				tmp_field = pull_register( operand_3 );
				*opcode = INSERT_FRC( *opcode, tmp_field );
			}
			else if( ( tmp_long == 28 ) ||
			    ( tmp_long == 29 ) ||
			    ( tmp_long == 30 ) ||
			    ( tmp_long == 31 )
			  )
			{
				/* Get FRB as the last operand, eventhough
				   this is located in the opcode as the
				   4th operand.  Yet another kludge in the
				   opcode mapping...
				*/
				tmp_field = pull_register( operand_4 );
				*opcode = INSERT_FRB( *opcode, tmp_field );
			}
			
		}
		else if( ops_and_mnemonics[index].form == M ||
			 ops_and_mnemonics[index].form == smM
		       )
		{
				/* We've got a "M" or "smM" form instruction.
				   (Word rotate and shift instructions)
				   Put the appropriate fields into
				   the opcode.
				*/
			
			tmp_field = pull_register( operand_1 );
			*opcode = INSERT_RA( *opcode, tmp_field );
			
			tmp_field = pull_register( operand_2 );
			*opcode = INSERT_RS( *opcode, tmp_field );
			
			tmp_long = EXTRACT_PRIMARY( *opcode );
			if( tmp_long == 23 )	/* rlwnm */
			{
				tmp_field = pull_register( operand_3 );
				*opcode = INSERT_RB( *opcode, tmp_field );
			}
			else	/* rlwinm,rlwimi */
			{
				tmp_field = pull_immediate( operand_3 );
				if(ops_and_mnemonics[index].form != smM)
				{
					*opcode = INSERT_SH32( 
						*opcode, tmp_field );
				}
				else
				{
				/* If I'm in here, I'm a simplified 
				   mnemonic and I have to take my 3rd
				   operand and do some wacko stuff with
				   it before I put it into the opcode...
				*/
				/* Start by pulling the 4th operand....even
				   if one doesn't exist.
				*/

					tmp2_field = pull_immediate(operand_4);
					
				/* Now, depending on the instruction, I need 
				   to do different things with the operands...
				*/
				 	if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"inslwi",6)==0
						  )
					{
						*opcode = INSERT_SH32(*opcode,
						   (32-tmp2_field));
						*opcode = INSERT_MB_smM(*opcode,
						   tmp2_field);
						*opcode = INSERT_ME_smM(*opcode,
						   (tmp2_field+tmp_field-1));
					}
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"insrwi",6)==0
					       ) 
					{
						*opcode = INSERT_SH32(*opcode,
						   (32-tmp2_field+tmp_field));
						*opcode = INSERT_MB_smM(*opcode,
						   tmp2_field);
						*opcode = INSERT_ME_smM(*opcode,
						   (tmp2_field+tmp_field-1));
					}	
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"extlwi",6)==0
					       )
					{
						*opcode = INSERT_ME_smM(*opcode,
							(tmp_field-1));
						*opcode = INSERT_SH32(*opcode,
							tmp2_field);
					}	
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"extrwi",6)==0
					       )
					{
						*opcode = INSERT_SH32(*opcode,
							(tmp_field+tmp2_field));
						*opcode = INSERT_MB_smM(*opcode,
							(32-tmp_field));
					}
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"inslwi",6)==0
					       )
					{
						*opcode = INSERT_SH32(*opcode,
					  	   (32-tmp2_field));
						*opcode = INSERT_MB_smM(*opcode,
						   tmp2_field);
						*opcode = INSERT_ME_smM(*opcode,
						   (tmp_field+tmp2_field-1));
					}
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"insrwi",6)==0
					       )
					{
						*opcode = INSERT_SH32(*opcode,
						    (32-tmp2_field+tmp_field));
						*opcode = INSERT_MB_smM(*opcode,
						    tmp2_field);
						*opcode = INSERT_ME_smM(*opcode,
						    (tmp2_field+tmp_field-1));
					}
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"slwi",4)==0
					       )
					{
						*opcode = INSERT_SH32(
							*opcode,tmp_field);
						*opcode = INSERT_ME_smM(*opcode,
							(31-tmp_field));
					}
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"srwi",4)==0
					       )
					{
						*opcode = INSERT_SH32(*opcode,
							(32-tmp_field));
						*opcode = INSERT_MB_smM(*opcode,
							tmp_field);
					}
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"clrlwi",6)==0
					       )
					{
						*opcode = INSERT_MB_smM(*opcode,
							tmp_field);
					}
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"clrrwi",6)==0
					       )
					{
						*opcode = INSERT_ME_smM(*opcode,
							(31-tmp_field));
					}
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"clrlslwi",8)==0
					       )

					{
						*opcode = INSERT_SH32(
							*opcode,tmp2_field);
						*opcode = INSERT_MB_smM(*opcode,
							(tmp_field-tmp2_field));
						*opcode = INSERT_ME_smM(*opcode,
							(31-tmp2_field));
					}
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"rotlwi",6)==0
					       )	
					{
						*opcode = INSERT_SH32(
							*opcode,tmp_field);
					}
					else if(strncmp(&
					ops_and_mnemonics[index].mnemonic[0],
					"rotrwi",6)==0
					       )
					{
						*opcode = INSERT_SH32(*opcode,
							(32-tmp_field));
					}
				}
 			}
			if(ops_and_mnemonics[index].form != smM)
			{	
				tmp_field = ((pull_immediate(operand_4)<<5) +
			                    (pull_immediate(operand_5)));
				*opcode = INSERT_MBE(*opcode,tmp_field);
			}
		}
		else if( ops_and_mnemonics[index].form == MD )
		{
				/* We've got a "MD" form instruction.
				   Put the appropriate fields into
				   the opcode.
				*/
		
			tmp_field = pull_register( operand_1 );
			*opcode = INSERT_RA( *opcode, tmp_field );
			
			tmp_field = pull_register( operand_2 );
			*opcode = INSERT_RS( *opcode, tmp_field );
			
			tmp_field = pull_immediate( operand_3 );
			*opcode = INSERT_SH64( *opcode, tmp_field );
			
			tmp_field = pull_immediate( operand_4 );
			*opcode = INSERT_MB( *opcode, tmp_field );
			
		}
		else if( ops_and_mnemonics[index].form == MDS )
		{
				/* We've got a "MDS" form instruction.
				   Put the appropriate fields into
				   the opcode.
				*/

			tmp_field = pull_register( operand_1 );
			*opcode = INSERT_RA( *opcode, tmp_field );
			
			tmp_field = pull_register( operand_2 );
			*opcode = INSERT_RS( *opcode, tmp_field );
			
			tmp_field = pull_register( operand_3 );
			*opcode = INSERT_RB( *opcode, tmp_field );
			
			tmp_field = pull_immediate( operand_4 );
			*opcode = INSERT_MB( *opcode, tmp_field );
		}

				/* End "else search ops_and_mnemonics was
				   correct" block
				*/
	}

	return( 0 );

}


/*
         1         2         3         4         5         6         7         8
01234567890123456789012345678901234567890123456789012345678901234567890123456789
*/
/* --------------------------------------------------------------------- */
/*


function:	pull_next_word
purpose:	This function will copy character from one string into
		the other until any white space is hit.
inputs:		word_num:	This is used to determine which word I'm
				supposed to be pulling.  For example, if
				"word_num" is 1, then I pull the first word.
		string_s:	This is the source string that I'm copying
				from.
outputs:	string_d:	This is the target string that I'm copying
				into.
return:		0 or 1.
mod history:	2/2/93	MH

*/

int	pull_next_word( word_num, string_d, string_s )
				/* This tells me which word to extract.
				*/
int		word_num;

				/* This is the target string that I'm copying
				   into.
				*/
char		*string_d;

				/* This is the source string that I'm copying
				   from.
				*/
char		*string_s;
{
int		i;
char		*tmp_str;
				
				/* this will make sure that tmp_str is
				initialized. */
	tmp_str = 0;

	for( i = 0; i < word_num; i++ )
	{
				/* Initialize the temporary string holder
				*/
		tmp_str = string_d;
	
				/* skip over white space before a word.
				*/
		while( ( *string_s == ' '  ) ||
		       ( *string_s == '\t' ) ||
		       ( *string_s == '\n' ) ||
		       ( *string_s == ','  ) ||
		       ( *string_s == '('  ) ||
		       ( *string_s == ')'  ) ||
		       ( *string_s == '\0' )
		     )
		{
				/* make sure we don't hit an end-o-line
				   before we expect it.
				*/
			if( ( *string_s == '\n' ) ||
			    ( *string_s == '\0' )
			  )
			{
				string_d = '\0';
				return( 1 );
			}
		
			string_s = string_s + 1;
		}

				/* Process until I hit some white
				   space.
				*/
		while( ( *string_s != ' '  ) &&
		       ( *string_s != '\t' ) &&
		       ( *string_s != '\n' ) &&
		       ( *string_s != ','  ) &&
		       ( *string_s != '('  ) &&
		       ( *string_s != ')'  ) &&
		       ( *string_s != '\0' )
		     )
		{
			*tmp_str = *string_s;
			string_s = string_s + 1;
			tmp_str = tmp_str + 1;
		}
	}
	
				/* Now I need to put an end-o-string char
				   at the end of my destination string.
				*/
	*tmp_str = '\0';

	return( 0 );
}


/*
         1         2         3         4         5         6         7         8
01234567890123456789012345678901234567890123456789012345678901234567890123456789
*/
/* --------------------------------------------------------------------- */
/*




function:       pull_next_word_2
purpose:			Same as pull_next_word accept it doesn't 
				consider '(' or ')' whitespace...needed
				this for 'la' simplified mnemonic.
inputs:         word_num:       This is used to determine which word I'm
                                supposed to be pulling.  For example, if
                                "word_num" is 1, then I pull the first word.
                string_s:       This is the source string that I'm copying
                                from.
outputs:        string_d:       This is the target string that I'm copying
                                into.
return:         0 or 1.
mod history:  	9/20/95 MM 


*/


int  pull_next_word_2( word_num, string_d, string_s )
                                /* This tells me which word to extract.
                                */
int             word_num;


                                /* This is the target string that I'm copying
                                   into.
                                */
char            *string_d;


                                /* This is the source string that I'm copying
                                   from.
                                */
char            *string_s;
{
int             i;
char            *tmp_str;


                                /* this will make sure that tmp_str is
                                initialized. */
        tmp_str = 0;


        for( i = 0; i < word_num; i++ )
        {
		while(*string_s == ' ' &&
		      *string_s == ',' &&
		      *string_s == '\n' &&
		      *string_s == '\0' 
		     )
		{
			string_s = string_s + 1;
		}	
		while(*string_s != ' ' &&
                      *string_s != ',' &&
                      *string_s != '\n' &&
                      *string_s != '\0'
                     )
                {
                        string_s = string_s + 1;
                }
	}
                                /* Initialize the temporary string holder
                                */
                tmp_str = string_d;


                                /* skip to first '(' */ 
                while( ( *string_s != '('  ) )
                {
                                /* make sure we don't hit an end-o-line
                                   before we expect it.
                                */
                        if( ( *string_s == '\n' ) ||
                            ( *string_s == '\0' )
                          )
                        {
                                string_d = '\0';
                                return( 1 );
                        }


                        string_s = string_s + 1;
                }
		string_s = string_s + 1;

                                /* Process until I hit ')' */ 
                while( *string_s != ')' ) 
                {
                        *tmp_str = *string_s;
                        string_s = string_s + 1;
                        tmp_str = tmp_str + 1;
                }

                                /* Now I need to put an end-o-string char
                                   at the end of my destination string.
                                */
        *tmp_str = '\0';


        return( 0 );
}

/*
         1         2         3         4         5         6         7         8
01234567890123456789012345678901234567890123456789012345678901234567890123456789
*/
/* --------------------------------------------------------------------- */
/*


function:	pull_register
purpose:	This function will pull register numbers out of a string, then
		return that number in a parameter.
inputs:		string_s:	This is the source string that has my r or f
				number.  I need to pull out the "r"
				or "f" prefix and convert the remaining
				string into a long.
return:		The register number in the string.
mod history:	2/2/93	MH

*/

int	pull_register( string_s )
				/* This is the source string that has my r
				   or f number.  I need to pull out the
				   "r" or "f" prefix and convert the
				   remaining string into a long.
				*/
char		*string_s;

{
				/* Increment the pointer past any characters
				   infront of the register number, like "r" or
				   "f"...
				*/
	while( *string_s >= 0x3a || *string_s <= 0x2f)
	{
		string_s = string_s + 1;
	}
	return( atol( string_s ) );

}

/*
         1         2         3         4         5         6         7         8
01234567890123456789012345678901234567890123456789012345678901234567890123456789
*/
/* --------------------------------------------------------------------- */
/*


function:	pull_immediate_branch
purpose:	This function will pull a number out of a string, then
		return that number in a parameter, or it will match with
		an entry in the branch table and return the address
		preformatted to match what the branch instruction wants
inputs:		string_s:	This is the source string that has my
				number.  All I need to do is convert that
				string into a long.  The problem is that
				the string contains a number in hex
				format, so I can't just use the library
				function "atol".  However, I can use the
				library function called stroul.
		aalk:		The AA and LK bits of the 32 bit opcode 
		current_address: The current address of the assembler. 
return:		The hex number in the string.
mod_history:	3/3/96  MM
		Wrote the original routine search label_table with string_s to 
		see if an entry in the branch table has already been entered...
		even if I'm not executing an branch instruction...this may
		be a bad thing but I can't think of any reason why
*/

int	pull_immediate_branch( string_s, aalk, current_address )
				/* This is the source string that has my
				   number (or branch label).
				*/
char		*string_s;
int		aalk;
unsigned long	current_address;
{
				/* tmpmnemonic holds the ASCII string which
				   is the mnemonic of the instruction I'm
				   about to assemble.
				*/
char		tmpmnemonic[80];
				/* Just a place holder that I'm using for
				   a library call.
				*/
char		*end;
				/* Just hold the length of the 
				   inputted string
				*/
int 		length;
				/* Just a counter for going thru
				   label_table
				*/
int 		count;
				/* Just a counter for going thru
				   an array 
				*/
int 		tmpint;
				/* the address being calculated to 
				   return back
				*/
unsigned long	address;
	if (*string_s == '@')
	{
	  string_s = string_s + 1;
	  for (count=0;count<MAXBRANCHLABEL;count++)
	  {
		if (label_table[count].name[0] == '\0') break;
		if (strcmp(label_table[count].name,string_s) == 0)
		{
		   address = label_table[count].address;
		   if (aalk == 0 || aalk == 1)
		     {
		        address = address - current_address;
		        return(address);
		     } else if (aalk == 2 || aalk == 3)
		     {
			return(address);
		     }
	 	}
	  }
	  strcpy(label_table[count].name,string_s);

			/* If we're in here, the user tried to get
			   out of the assembler...we need to make
			   sure all of the branch labels are
			   verified and are taken care of before
			   exiting!
			*/

	printf("\n `%s' undefined, enter address 0x", label_table[count].name );
	//dink_gets(tmpmnemonic);
	shell_read_line( tmpmnemonic, "ppcsim.dasm> " );

	// Delete whitespace
	tmpint = 0;
	while (tmpmnemonic[tmpint]==' ' || tmpmnemonic[tmpint]=='\t')
		tmpint++;

	label_table[count].address = pull_immediate(&tmpmnemonic[tmpint]);
	printf("`%s' => 0x%08lx\n", label_table[count].name,
							label_table[count].address);

	  address = label_table[count].address;
	  if (aalk == 0 || aalk == 1)
	  {
	      address = address - current_address;
	      return(address);
	  } else if (aalk == 2 || aalk == 3)
	  {
	      return(address);
	  } 

	}
	end = string_s;
	length = strlen(string_s);
	while( *string_s != '0' && length >= 2 )
	{
		length--;
		string_s = string_s + 1;
	}
	string_s = string_s + 1;
	if(length <= 2) 
	{
		string_s = end;
		end = 0;
		return(strtol(string_s,&end,16));
	}
	switch (*string_s) {
		case 'x':
		case 'X': string_s = string_s + 1;
			  return(strtol(string_s, &end, 16));
		case 'b':
		case 'B': string_s = string_s + 1;
			  return(strtol(string_s, &end, 2));
		case 'd':
		case 'D': string_s = string_s + 1;
			  return(strtol(string_s, &end, 10));
		default: string_s = end;
			 end = 0;
			 return(strtol(string_s,&end,16));
 	}	
}

/*
function:	pull_immediate_cr
purpose:	This function will pull a bit number out of a string, then
		return that number in a parameter.
inputs:		string_s:	This is the source string that has my
				number.  All I need to do is convert that
				string into a long.  The problem is that
				the string contains a number in hex
				format, so I can't just use the library
				function "atol".  However, I can use the
				library function called stroul.
return:		The decimal number in the string.
mod history:	
		3/19/96  MM

*/

int	pull_immediate_cr( string_s )
				/* This is the source string that has my
				   number (or branch label).
				*/
char		*string_s;

{
				/* Just a place holder that I'm using for
				   a library call.
				*/
char		*end;
				/* Just hold the length of the 
				   inputted string
				*/
int 		length;
				/* Just a counter for going thru
				   label_table
				*/

	end = string_s;
	length = strlen(string_s);
	while( *string_s != '0' && length >= 2 )
	{
		length--;
		string_s = string_s + 1;
	}
	string_s = string_s + 1;
	if(length <= 2) 
	{
		string_s = end;
		end = 0;
		return(strtol(string_s,&end,10));
	}
	switch (*string_s) {
		case 'x':
		case 'X': string_s = string_s + 1;
			  return(strtol(string_s, &end, 16));
		case 'b':
		case 'B': string_s = string_s + 1;
			  return(strtol(string_s, &end, 2));
		case 'd':
		case 'D': string_s = string_s + 1;
			  return(strtol(string_s, &end, 10));
		default: string_s = end;
			 end = 0;
			 return(strtol(string_s,&end,10));
 	}	
}

/*
function:	pull_immediate
purpose:	This function will pull a number out of a string, then
		return that number in a parameter.
inputs:		string_s:	This is the source string that has my
				number.  All I need to do is convert that
				string into a long.  The problem is that
				the string contains a number in hex
				format, so I can't just use the library
				function "atol".  However, I can use the
				library function called stroul.
return:		The hex number in the string.
mod history:	2/2/93	MH

		3/3/96  MM
		I make this routine search label_table with string_s to 
		see if an entry in the branch table has already been entered...
		even if I'm not executing an branch instruction...this may
		be a bad thing but I can't think of any reason why

*/

int	pull_immediate( string_s )
				/* This is the source string that has my
				   number (or branch label).
				*/
char		*string_s;
{
				/* Just a place holder that I'm using for
				   a library call.
				*/
char		*end;
				/* Just hold the length of the 
				   inputted string
				*/
int 		length;

	end = string_s;
	length = strlen(string_s);
	while( *string_s != '0' && length >= 2 )
	{
		length--;
		string_s = string_s + 1;
	}
	string_s = string_s + 1;
	if(length <= 2) 
	{
		string_s = end;
		end = 0;
		return(strtol(string_s,&end,16));
	}
	switch (*string_s) {
		case 'x':
		case 'X': string_s = string_s + 1;
			 return(strtol(string_s, &end, 16));
		default: string_s = end;
			 end = 0;
			 return(strtol(string_s,&end,16));
 	}	
}

/*
         1         2         3         4         5         6         7         8
01234567890123456789012345678901234567890123456789012345678901234567890123456789
*/
/* --------------------------------------------------------------------- */
/*

function:	search_ops_and_mnemonics
purpose:	This function is just a glorified search function.  No big
		deal.  We are searching a data structure called "ops_and_
		mnemonics for a match.  We are either searching for a
		32-bit opcode match, or a mnemonic (string) match.
inputs:		opcode		This is a 32-bit number which represents
				an instruction opcode.
		mnemonic	This is a character string which represents
				an instruction mnemonic.
		index		This is a pointer to an integer.  The value
				passed in indicates whether we are searching
				for an opcode (index = 0) or searching for
				a mnemonic (index = 1).
outputs:	index		We will return the index into the data
				structure where the matching opcode or
				mnemonic is found.
return:		0 or 1.
mod history:	2/1/93	MH
		9/20/95	MM	(added simplified mnemonic stuff)

*/

int	search_ops_and_mnemonics( opcode, mnemonic, index )

				/* This is a 32-bit number which represents
				   an instruction opcode.
				*/
unsigned long	opcode;

				/* This is a pointer to an integer.  The value
				   passed in indicates whether we are searching
				   for an opcode (index = 0) or searching for
				   a mnemonic (index = 1).
				*/
char		*mnemonic;

				/* This is a pointer to an integer.  The value
				   passed in indicates whether we are searching
				   for an opcode (index = 0) or searching for
				   a mnemonic (index = 1).

				   Later, we will return the index into the
				   data structure where the matching opcode
				   or mnemonic is found.
				*/
int		*index;

{
				/* i will be a simple loop counter for us.
				*/
int		i;

				/* Just a simple flag telling me that the
				   opcode search was successful.
				*/
int		found_it;
				
				/* just a number holder for comparing 
				   secondary opcodes and stuff
				*/
long		tmp_long;

	for( i = 0; i < number_of_mnemonics; i++ )
	{
				/* Check every item in the data structure
				   for a match.  Yes, it's true, a linear
				   search isn't the most efficient way to
				   do this, but searching ~500 items at
				   50MHz, we're talking microseconds here...
				*/

				/* We test to see if we're searching
				   for an opcode.
				*/
		if( *index == 0 )
		{
				/* Initialize "found_it" to be false.
				   I need to redo this everytime I'm
				   looking at a new entry in the data
				   structure because I may have passed
				   one test, and set found_it, but then
				   failed the next test.  Ahh... the
				   joys of a complex instruction set...
				*/
			found_it = 0;

				/* Now we need to check both the primary
				   and secondary opcodes against the ones
				   in the data structure and see if they
				   match.  This isn't too difficult.  First,
				   we'll see if the primary opcodes match,
				   then we'll test the secondary.
				*/
			if( ( ( opcode & ops_and_mnemonics[i].opcode ) ==
			        ops_and_mnemonics[i].opcode ) &&
			    ( EXTRACT_PRIMARY( opcode ) ==
			      EXTRACT_PRIMARY( ops_and_mnemonics[i].opcode )
			    )
			  )
			{
				/* If primary opcode is 16 we have a branch
				   simplified mnemonic and if it's got an
				   opcode of the form 0x42400000 or 0x42000000
				   we need to return an index that has that
				   form also because those 2 opcodes are
				   special.
				*/
				if(EXTRACT_PRIMARY(opcode) == 16)
				{
					if((EXTRACT_SPECIAL(opcode) == 0x4200 ||
					   EXTRACT_SPECIAL(opcode) == 0x4240) &&
					   (EXTRACT_SPECIAL(opcode) == 
				   	   EXTRACT_SPECIAL(
						ops_and_mnemonics[i].opcode))
				 	  )
					{
						if(EXTRACT_SECONDARY_DS(
						opcode)==
						EXTRACT_SECONDARY_DS(
						ops_and_mnemonics[i].opcode)
						  )
						{
							found_it = 1;
						}
					}
					else if(EXTRACT_SPECIAL(opcode) != 
					0x4200 &&
				        EXTRACT_SPECIAL(opcode) != 
					0x4240
					       )
					{
						found_it = 1;
					}
				}		
				/* If primary opcode is 19 we have a branch
			   	    simplified mnemonic and if it's got an
			  	    opcode of the form 0x4E000000 or 0x4D800000
			   	    or 0x4C800000 or 0x4E800000
			   	    we need to return an index that has that
			   	    form also because those 2 opcodes are
			   	    special...careful...isync & rfi are
				    opcode 19 too
				*/
				else if(EXTRACT_PRIMARY(opcode) == 19 &&
				strncmp(&ops_and_mnemonics[i].mnemonic[0],
				"cr",2) != 0 &&
				EXTRACT_SECONDARY_XL(opcode) != 150  &&
				EXTRACT_SECONDARY_XL(opcode) != 50 
				       )
				{
					if((EXTRACT_SPECIAL(opcode) == 0x4E00 ||
                               		   EXTRACT_SPECIAL(opcode) == 0x4E40 ||
					   EXTRACT_SPECIAL(opcode) == 0x4D80 ||
					   EXTRACT_SPECIAL(opcode) == 0x4C80 ||
					   EXTRACT_SPECIAL(opcode) == 0x4E80
					    ) &&
                               		   (EXTRACT_SPECIAL(opcode) ==
                             		    EXTRACT_SPECIAL(
						ops_and_mnemonics[i].opcode)
					   )
                              		  )
                                        {
                                                if(EXTRACT_SECONDARY_LK(							opcode)==
                                                EXTRACT_SECONDARY_LK(
						ops_and_mnemonics[i].opcode)
                                                  )
                                                {
                                                        found_it = 1;
                                                }
                                        }
                                        else if(EXTRACT_SPECIAL(opcode) != 	
					0x4E00 &&
                                        EXTRACT_SPECIAL(opcode) != 
					0x4E40 &&
		   		  	EXTRACT_SPECIAL(opcode) != 
					0x4D80 &&
				    	EXTRACT_SPECIAL(opcode) != 
					0x4C80 &&
				    	EXTRACT_SPECIAL(opcode) != 
					0x4E80
                                               )
                                        {
                                                found_it = 1;
                                        }
				}
				else if(EXTRACT_PRIMARY(opcode) == 19 &&
				strncmp(&ops_and_mnemonics[i].mnemonic[0],
				"cr",2) == 0 &&
				EXTRACT_SECONDARY_XL(opcode) ==
				EXTRACT_SECONDARY_XL(ops_and_mnemonics[i].opcode)
 				       )
				{
					if (EXTRACT_SECONDARY_XL(opcode) == 289)
					{
						/* creqv/crset */
                                		if(strncmp(&
						   ops_and_mnemonics[i].mnemonic[0],
                                		   "crset",5) == 0 &&
						   EXTRACT_RS(opcode) ==
						   EXTRACT_RA(opcode) && 
						   EXTRACT_RA(opcode) ==
						   EXTRACT_RB(opcode)
						  )
						{
							found_it = 1;
						} else if(strncmp(&
						  ops_and_mnemonics[i].mnemonic[0],
                                		  "creqv",5) == 0
							 )
						{
							found_it = 1;
						}
					} else if (EXTRACT_SECONDARY_XL(opcode) == 33)
					{
						/* crnor/crnot */
                                		if(strncmp(&
						   ops_and_mnemonics[i].mnemonic[0],
                                		   "crnot",5) == 0 &&
						    EXTRACT_BA(opcode) ==
						    EXTRACT_RB(opcode) && 
						    EXTRACT_RA(opcode) !=
						    EXTRACT_RS(opcode)
						   )
						{
							found_it = 1;
						} else if(strncmp(&
						  ops_and_mnemonics[i].mnemonic[0],
                                		  "crnor",5) == 0
							 )
						{
							found_it = 1;
						}

					} else if (EXTRACT_SECONDARY_XL(opcode) == 449)
					{
						/* cror/crmove */
                                		if(strncmp(&
						   ops_and_mnemonics[i].mnemonic[0],
                                		   "crmove",6) == 0 &&
						    EXTRACT_BA(opcode) ==
						    EXTRACT_RB(opcode) && 
						    EXTRACT_RA(opcode) !=
						    EXTRACT_RS(opcode)
						   )
						{
							found_it = 1;
						} else if(strncmp(&
						  ops_and_mnemonics[i].mnemonic[0],
                                		  "cror",4) == 0
							 )
						{
							found_it = 1;
						}

					} else if (EXTRACT_SECONDARY_XL(opcode) == 193)
					{
						/* crxor/crclr */
                                		if(strncmp(&
						   ops_and_mnemonics[i].mnemonic[0],
                                		   "crclr",5) == 0 &&
						    EXTRACT_RS(opcode) ==
						    EXTRACT_RA(opcode) && 
						    EXTRACT_RA(opcode) ==
						    EXTRACT_RB(opcode)
						   )
						{
							found_it = 1;
						} else if(strncmp(&
						  ops_and_mnemonics[i].mnemonic[0],
                                		  "crxor",5) == 0
							 )
						{
							found_it = 1;
						}

					}
/* jackass */
				}
				else if(EXTRACT_PRIMARY(opcode) == 19)
				{
                                	if(strncmp(&
					ops_and_mnemonics[i].mnemonic[0],
                                	"isync",5) == 0 &&
					EXTRACT_ISYNC_BITS(opcode) == 150
					  )
                                	{
						found_it = 1;
					} else if(strncmp(&
					ops_and_mnemonics[i].mnemonic[0],
                                	"rfi",3) == 0 &&
					EXTRACT_SECONDARY_XL(opcode) == 50
					  )
                                	{
						found_it = 1;
					}
				}
				/* The primaries match, so we'll need to
				   do a check on the seconaries.  Note that
				   there are some instructions which do not
				   have a secondary opcode, so I don't
				   test for those.
				*/
				else if( ops_and_mnemonics[i].form == SC )
				{
					if( EXTRACT_SECONDARY_SC( opcode ) ==
				        EXTRACT_SECONDARY_SC( 
					ops_and_mnemonics[i].opcode )
				          )
					{
						found_it = 1;
					}
				}
				else if( ops_and_mnemonics[i].form == DS )
				{
					if( EXTRACT_SECONDARY_DS( opcode ) ==
				        EXTRACT_SECONDARY_DS(
				 	ops_and_mnemonics[i].opcode )
				          )
					{
						found_it = 1;
					}
				}
				/* Simplified mnemonic section for cmpw/cmplw */
				else if(strcmp(&
				ops_and_mnemonics[i].mnemonic[0],
				"cmpw")==0 ||
				strcmp(&ops_and_mnemonics[i].mnemonic[0],
				"cmplw")==0
				       )
				{
				/* These mnemonics need to match both
				   the secondary and bit 10 to have a match
				*/
					if( EXTRACT_SECONDARY_X( opcode ) ==
                                  	EXTRACT_SECONDARY_X( 
					ops_and_mnemonics[i].opcode )
                                       	  )
                                        {
                                       		found_it = 1;
                                        }
					if(found_it == 1 &&
					EXTRACT_SECONDARY_BIT10(opcode) ==
					EXTRACT_SECONDARY_BIT10(
					ops_and_mnemonics[i].opcode)
					  )
					{
					}
					else found_it = 0;
				} 
			/* Section for cmpwi/cmplwi simplified mnemonics */
				else if(strcmp(&
				ops_and_mnemonics[i].mnemonic[0],
				"cmpwi")==0 ||
                               	strcmp(&ops_and_mnemonics[i].mnemonic[0],
				"cmplwi")==0
			               )
				{
					/* These just need to match bit 10 */
					if(EXTRACT_SECONDARY_BIT10(opcode) ==
					   EXTRACT_SECONDARY_BIT10(
						ops_and_mnemonics[i].opcode)
					  )
					{
						found_it = 1;
					}
				}	
				else if( ops_and_mnemonics[i].form == X || 
					ops_and_mnemonics[i].form == smX 
				          )
				{
					if( EXTRACT_SECONDARY_X( opcode ) ==
			                (tmp_long = EXTRACT_SECONDARY_X( 
					ops_and_mnemonics[i].opcode ))
				          )
					{
					/* If we've got a "mr" or "not"
					   simplified mnemonic then need
					   to check if RS and RB fields are
					   the same for a match.	
					*/
					   if((strcmp(&
					   ops_and_mnemonics[i].mnemonic[0],
					   "mr")==0 ||
					   strcmp(&
					   ops_and_mnemonics[i].mnemonic[0],
					   "not")==0) &&
					   ops_and_mnemonics[i].form == smX
					     )	
					   {
						if(EXTRACT_FRS(opcode) ==
						EXTRACT_FRB(opcode)
						  )
						{
							found_it = 1;
						}
					   }	
					   else if (tmp_long == 0x004)
					   {
						if(EXTRACT_TO(opcode) ==
						EXTRACT_TO(
						ops_and_mnemonics[i].opcode)
						  )
						{	
							found_it = 1;
						}
				 	   }	
					   else if (ops_and_mnemonics[i].form !=
						 smX)
					   {
						found_it = 1;
					   }	
					}
				}
				else if( ops_and_mnemonics[i].form == I )
				{
					if( ( EXTRACT_AA( opcode ) ==
					EXTRACT_AA(ops_and_mnemonics[i].opcode))
					&& ( EXTRACT_LK( opcode ) ==
					EXTRACT_LK(ops_and_mnemonics[i].opcode))
					  )
					{
						found_it = 1;
					}
				}
				else if( ops_and_mnemonics[i].form == XL )
				{
					if( EXTRACT_SECONDARY_XL( opcode ) ==
				        EXTRACT_SECONDARY_XL(
					ops_and_mnemonics[i].opcode )
				          )
					{
						found_it = 1;
					}
				}
				else if( ops_and_mnemonics[i].form == smXL )
				{
					if( EXTRACT_SECONDARY_XL( opcode ) ==
				        EXTRACT_SECONDARY_XL(
				 	ops_and_mnemonics[i].opcode )
				          )
					{
						if((EXTRACT_SECONDARY_XL(
						opcode)==289 ||
	    					EXTRACT_SECONDARY_XL(
						opcode)==193) &&
	    					(EXTRACT_BO(opcode)==
						EXTRACT_BA(opcode) 
	    					&& EXTRACT_BA(opcode)==
						EXTRACT_BB(opcode))
	   					  )
						{
							found_it = 1;
						}
						else if((EXTRACT_SECONDARY_XL(
						opcode)==449 
	           				|| EXTRACT_SECONDARY_XL(
						opcode)==33)
		     				&& (EXTRACT_BA(opcode)==
						EXTRACT_BB(opcode))
		    			 	       )
						{
							found_it = 1;
						}
					}
				}
				else if( ops_and_mnemonics[i].form == XFX ) 
				{
					if( EXTRACT_SECONDARY_XFX( opcode ) ==
				        EXTRACT_SECONDARY_XFX( 
					ops_and_mnemonics[i].opcode )
				          )
					{
						found_it = 1;
					}
				}
				else if ( ops_and_mnemonics[i].form == smXFX )
				{
					if( EXTRACT_SECONDARY_smXFX( opcode ) ==
				        EXTRACT_SECONDARY_smXFX( 
					ops_and_mnemonics[i].opcode ) 
					  )
					{
						found_it = 1;
					}
				}
				else if( ops_and_mnemonics[i].form == XFL )
				{
					if( EXTRACT_SECONDARY_XFL( opcode ) ==
				        EXTRACT_SECONDARY_XFL( 
					ops_and_mnemonics[i].opcode )
				          )
					{
						found_it = 1;
					}
				}
				else if( ops_and_mnemonics[i].form == XS )
				{
					if( EXTRACT_SECONDARY_XS( opcode ) ==
				        EXTRACT_SECONDARY_XS( 
				 	ops_and_mnemonics[i].opcode )
				          )
					{
						found_it = 1;
					}
				}
				else if( ops_and_mnemonics[i].form == XO ||
				ops_and_mnemonics[i].form == smXO )
				{
					if(ops_and_mnemonics[i].form == XO &&
					EXTRACT_SECONDARY_XO( opcode ) ==
				        EXTRACT_SECONDARY_XO( 
					ops_and_mnemonics[i].opcode )
					  )
					{
						found_it = 1;
					}
					else if(ops_and_mnemonics[i].form==smXO
					&& EXTRACT_SECONDARY_smXO(opcode)==
					EXTRACT_SECONDARY_smXO(
					ops_and_mnemonics[i].opcode)
					       ) 
					{
						found_it = 1;
					}
				}
				else if( ops_and_mnemonics[i].form == A )
				{
					if( EXTRACT_SECONDARY_A( opcode ) ==
				        EXTRACT_SECONDARY_A( 
					ops_and_mnemonics[i].opcode )
				          )
					{
						found_it = 1;
					}
				}
				else if( ops_and_mnemonics[i].form == MD )
				{
					if( EXTRACT_SECONDARY_MD( opcode ) ==
				        EXTRACT_SECONDARY_MD( 
					ops_and_mnemonics[i].opcode )
				          )
					{
						found_it = 1;
					}
				}
				else if( ops_and_mnemonics[i].form == smD )
				{		/* NOP INSTRUCTION... */
					if(EXTRACT_PRIMARY(opcode) == 24)
					{
					   if(EXTRACT_NOP(opcode) == 0)
					   {
					      found_it = 1;
					   } 
					}
				 	else if(EXTRACT_PRIMARY(opcode) >= 12 &&
					   EXTRACT_PRIMARY(opcode) <= 15
					  )
					{
					   if(strncmp(&
					   ops_and_mnemonics[i].mnemonic[0],
					   "li",2)==0 &&
					   (int)EXTRACT_RA(opcode) == 0
					     )
					   { 
						found_it = 1;
					   }
					   else if (strncmp(&
					   ops_and_mnemonics[i].mnemonic[0],
					   "li",2)!=0
					           )
					   {
						found_it = 1;
					   }
					}	
					else if( EXTRACT_TO(opcode) ==
                                        EXTRACT_TO(ops_and_mnemonics[i].opcode)
					       )
					{
						found_it = 1;
					}
				}
				else if( ops_and_mnemonics[i].form == MDS )
				{
					if( EXTRACT_SECONDARY_MDS( opcode ) ==
				            EXTRACT_SECONDARY_MDS( 
						ops_and_mnemonics[i].opcode )
				              )
					{
						found_it = 1;
					}
				}
				else if(ops_and_mnemonics[i].form == smM)
				{
					if(strncmp(&
					ops_and_mnemonics[i].mnemonic[0],
					"rotlwi",6)==0 && 
				 	EXTRACT_SECONDARY1_smM( opcode )==
					EXTRACT_SECONDARY1_smM(
					ops_and_mnemonics[i].opcode )
				    	  )
					{
						found_it = 1;
					}
					else if(strncmp(&
					ops_and_mnemonics[i].mnemonic[0],
					"clrlwi",6)==0 &&
					EXTRACT_SECONDARY2_smM(opcode)==
					EXTRACT_SECONDARY2_smM(
					ops_and_mnemonics[i].opcode) &&
					EXTRACT_SECONDARY5_smM(opcode)==
					EXTRACT_SECONDARY5_smM(
					ops_and_mnemonics[i].opcode)
					       )
					{
						found_it = 1;
					}
					else if(strncmp(&
					ops_and_mnemonics[i].mnemonic[0],
					"clrrwi",6)==0 &&
					EXTRACT_SECONDARY3_smM(opcode)==
					EXTRACT_SECONDARY3_smM(
					ops_and_mnemonics[i].opcode)
					       )
					{
						found_it = 1;
					}
					else if(strncmp(&
					ops_and_mnemonics[i].mnemonic[0],
					"slwi",4)==0 &&
					EXTRACT_SECONDARY4_smM(opcode)==
					EXTRACT_SECONDARY4_smM(
					ops_and_mnemonics[i].opcode)
					       )
					{	
						found_it = 1;
					}
					else if(strncmp(&
					ops_and_mnemonics[i].mnemonic[0],
					"srwi",4)==0 &&
					EXTRACT_SECONDARY5_smM(opcode)==
                                        EXTRACT_SECONDARY5_smM(
					ops_and_mnemonics[i].opcode)
					       )
					{
						found_it = 1;
					}
					else if(strncmp(&
					ops_and_mnemonics[i].mnemonic[0],
					"clrlslwi",8)==0 ||
					EXTRACT_PRIMARY(
					ops_and_mnemonics[i].opcode) == 20 ||
					EXTRACT_PRIMARY(
					ops_and_mnemonics[i].opcode) == 23
					       ) 
					{
						found_it = 1;
					}
				}
				else
				{
				/* If we get here, the instruction is of the
				   form that has no secondary opcode, so we
				   already have our match.  Life is good.
				*/
						found_it = 1;
				}
					
				/* Not so fast, eventhough the primary and
				   secondary opcodes match, we still may have
				   to test for the RC bit.  Yet another kludge
				   in the opcode-mnemonic mapping.
				*/
				if( found_it == 1 )
				{
					if((ops_and_mnemonics[i].form == X) ||
					(ops_and_mnemonics[i].form == XFX) ||
					(ops_and_mnemonics[i].form == XFL) ||
					(ops_and_mnemonics[i].form == XS) ||
					(ops_and_mnemonics[i].form == A) ||
					(ops_and_mnemonics[i].form == M) ||
					(ops_and_mnemonics[i].form == smM) ||
					(ops_and_mnemonics[i].form == MD) ||
					(ops_and_mnemonics[i].form == MDS)
					  )
					{ 
				/* If our instruction is one of these forms,
				   it may have a different RC bit than the
				   instruction we're comparing against.
				   That difference in RC bits wouldn't
				   show up in the tests of the primary and
				   secondary opcodes.  Yeah, it's a kludge.
				*/
						if( EXTRACT_RC( 
						ops_and_mnemonics[i].opcode ) ==
						EXTRACT_RC( opcode )
						  )
						{
				/* The RC bits match, let's go!
				*/
				    
				/* We have a match!  let's update "index"
				   and get outta here.
				*/
							*index = i;
				
							return( 0 );
						}
					}
					else if(ops_and_mnemonics[i].form == XO)
					{
						if( (EXTRACT_RC( 
						ops_and_mnemonics[i].opcode ) ==
						EXTRACT_RC( opcode )) &&
						(EXTRACT_OE( 
						ops_and_mnemonics[i].opcode ) ==
						EXTRACT_OE( opcode ))
						  )
						{
							*index = i;
							return( 0 );
						}
					}
					else if((ops_and_mnemonics[i].form==I)||
					(ops_and_mnemonics[i].form == B)
					       )
					{
						if( (EXTRACT_AA( 
						ops_and_mnemonics[i].opcode ) ==
						EXTRACT_AA( opcode )) &&
						(EXTRACT_LK( 
						ops_and_mnemonics[i].opcode ) ==
						EXTRACT_LK( opcode ))
						  )
						{
							*index = i;
							return( 0 );
						}
					}
					else if(ops_and_mnemonics[i].form == XL)
					{
						if( EXTRACT_LK( 
						ops_and_mnemonics[i].opcode ) ==
						EXTRACT_LK( opcode )
						  )
						{
							*index = i;
							return( 0 );
						}
					}
					else
					{
				/* We have a match!  let's update "index"
				   and get outta here.
				*/
						*index = i;
						return( 0 );
					}
				}
			}
		}
		else
		{
		
			if(strcmp(ops_and_mnemonics[i].mnemonic,mnemonic) == 0)
			{
				/* We have a match!  let's update "index"
				   and get outta here.
				*/
				*index = i;
				return( 0 );
			}
		}
	}
	
				/* If we get here, our search failed.
				   We need to return a 1 to let
				   the caller know we failed.
				*/
	return( 1 );
}				

/* --------------------------------------------------------------------- */
/*

function:	special_search_ops_and_mnemonics
purpose:	This function is just a glorified search function.  No big
		deal.  We are searching a data structure called "ops_and_
		mnemonics for a match.  We are either searching for a
		32-bit opcode match, or a mnemonic (string) match along 
		with a match of bits 11-15 of the hex code thru 
		translating it to a FORM match
inputs:		opcode		This is a 32-bit number which represents
				an instruction opcode.
		index		This is a pointer to an integer.  The value
				passed in indicates whether we are searching
				for an opcode (index = 0) or searching for
				a mnemonic (index = 1).
		tmp_long		This is the bits 11-15 from an opcode
				that must also be matched to the returned
				index, to match correctly, this long must be
				added to the FORM!

outputs:	index		We will return the index into the data
				structure where the matching opcode or
				mnemonic is found.
return:		0 or 1.
mod history:	8/30/95 MM

*/

int special_search_ops_and_mnemonics( opcode, index, tmp_long )

				/* This is a 32-bit number which represents
				   an instruction opcode.
				*/
unsigned long	opcode;

				/* We will return the index into the
				   data structure where the matching opcode
				   or mnemonic is found.
				*/
int		*index;
				/* This is the translation of bits 11-15 of the
				    opcode which is used to match on the FORM
				    as well as the HEX code for a mnemonic
				*/
unsigned long	tmp_long;

{
				/* i will be a simple loop counter for us.
				*/
int		i;
				/* Just a simple flag telling me that the
				   opcode search was successful.
				*/
int		found_it = 0;
	for( i = 0; i < number_of_mnemonics; i++ )
	{
				/* Check every item in the data structure
				   for a match.  Yes, it's true, a linear
				   search isn't the most efficient way to
				   do this, but searching ~500 items at
				   50MHz, we're talking microseconds here...
				*/

				/* We test to see if we've got a match on the
				   previous loop thru this code...
				*/
		if( found_it == 1 ) 
		{
			*index = i-1;
			return( 0 );
		}

				/* First, check if the FORMS match...this will
				    weed out many mnemonics.
				*/
		if( ops_and_mnemonics[i].form == (smBC+tmp_long) ||
	 	     ops_and_mnemonics[i].form == (smBCA+tmp_long) ||
		     ops_and_mnemonics[i].form == (smBCLR+tmp_long) ||
		     ops_and_mnemonics[i].form == (smBCCTR+tmp_long) ||
		     ops_and_mnemonics[i].form == (smBCL+tmp_long) ||
		     ops_and_mnemonics[i].form == (smBCLA+tmp_long) ||
		     ops_and_mnemonics[i].form == (smBCLRL+tmp_long) ||
		     ops_and_mnemonics[i].form == (smBCCTRL+tmp_long) 
		  )
		{
				/* Now we need to check both the primary
				   and secondary opcodes against the ones
				   in the data structure and see if they
				   match.  This isn't too difficult.  First,
				   we'll see if the primary opcodes match,
				   then we'll test the secondary.
				*/
			if( ( ( opcode & 
			ops_and_mnemonics[i].opcode ) ==
		        ops_and_mnemonics[i].opcode 
		            ) &&
		        ( EXTRACT_PRIMARY_B( opcode ) ==
		        EXTRACT_PRIMARY_B(
			ops_and_mnemonics[i].opcode )
		        )
			  )
			{
				/* The primaries match, so we'll need to
				   do a check on the seconaries.  Note that
				   there are some instructions which do not
				   have a secondary opcode, so I don't
				   test for those.
				*/
				if(( ops_and_mnemonics[i].form >= smBCLR &&
				ops_and_mnemonics[i].form <= (smBCCTR+4)) ||
				( ops_and_mnemonics[i].form >= smBCLRL &&
				ops_and_mnemonics[i].form <= (smBCCTRL+4) )
				  )
				{
					if( EXTRACT_SECONDARY_XL(opcode)==
					EXTRACT_SECONDARY_XL(
					ops_and_mnemonics[i].opcode) &&
					EXTRACT_LK(opcode)==
	 				EXTRACT_LK(ops_and_mnemonics[i].opcode)
				          )
					{
						found_it = 1;
					}
				}
				else if(EXTRACT_LK_AA(opcode)== 
				EXTRACT_LK_AA(ops_and_mnemonics[i].opcode)
				       )
				{
					found_it = 1;
				}
			}
		}
	}
	return( 0 );
}


/*
         1         2         3         4         5         6         7         8
01234567890123456789012345678901234567890123456789012345678901234567890123456789
*/
/* --------------------------------------------------------------------- */
/*


function:	branchtable_list()	
purpose: 	This function lists the array of branch lables held
		in label_table[] to the screen 
inputs:		NONE	
outputs: 	NONE
return:		0 or 1.
mod history:	3/4/96 MM	

*/

int	branchtable_list()
{
int		i;

	printf("Current list of DINK branch labels:");
	for( i = 0; i < MAXBRANCHLABEL; i++ )
	{
		if ( label_table[i].label_type == DSYM)
		{
			printf("\n  %15s:    ",label_table[i].name);
			printf("0x%lx",label_table[i].address);
		}
	}
	printf("\n\n");
	printf("Current list of USER branch labels:");
	for( i = 0; i < MAXBRANCHLABEL; i++ )
	{
		if (label_table[i].label_type == USYM)
		{
			printf("\n  %15s:    ",label_table[i].name);
			printf("0x%lx",label_table[i].address);
		}
	}
	printf("\n");
  	return 0;
}

/*
         1         2         3         4         5         6         7         8
01234567890123456789012345678901234567890123456789012345678901234567890123456789
*/
/* --------------------------------------------------------------------- */
/*


function:	branchtable_clear()	
purpose: 	This function initializes the array of branch lables held
		in label_table[] to name = '\0' and address = 0	
inputs:		NONE	
outputs: 	NONE
return:		0 or 1.
mod history:	3/4/96 MM	

*/

int	branchtable_clear()
{
int		i;

	for( i = 0; i < MAXBRANCHLABEL; i++ )
	{
	   if (label_table[i].label_type == USYM)
	     {
		label_table[i].name[0] = '\0';
		label_table[i].address = (long)0;
		label_table[i].label_type = NSYM;
	     }
	}

  	return 0;
}


int	branchtable_clear1(sym)
char *sym;
{

  int i;

  for( i = 0; i < MAXBRANCHLABEL; i++ )
   {
      if ( (label_table[i].label_type == USYM) &&
	   (!strcmp(label_table[i].name, sym)) )
	     {
		label_table[i].name[0] = '\0';
		label_table[i].address = (long)0;
		label_table[i].label_type = NSYM;
  	        return 0;
	     }
   }
  return 1;
}


/*
function:	assemble
purpose:	This function will take, as input, a starting address and an
		ending address.  We will assemble instructions and place the
		32-bit resulting opcode in the specified memory locations.
inputs:		Two parameters are sent in: start_addr and end_addr.
		The caller is telling us to assemble instructions and place
		the 32-bit resulting opcodes in the corresponding addresses.
		start_addr:	This is the starting address in memory
				where we will begin placing the assembled
				instructions.
		end_addr:	This is the ending address in memory
				where we will stop placing the assembled
				istructions.
outputs:	None.
return:		I may return some error codes that I don't even know
		about.  For example, I may check to make sure the addresses
		given are correct, and if they aren't, the function may
		return some strange error code that I'll just pass back
		to my caller.  For this reason, I can't say exactly what
		may be returned.  Although I can say that it will only
		be some kind of status.
mod history:	6/1/93	MH	Modified assembler to produce little-endian
				based opcodes.  This is envoked by a
				"define" in asm_dsm.h.
		2/2/93	MH  							*/

int assemble( PPCSIM *ppcsim, ULONG start, ULONG end )
{
	ULONG	opcode;
	int		status;
	char	mnemonic[100];

	printf("do as start=%08X  end=%08X\n", start, end);
int		tmpint = 0;
				/* This int is used to parse a branch label */
int		tmpint2 = 0;
				/* Counter for checking branch_lable array */
int		count = 0;

int		dummy_var=0;    /* to celar up compiler warnings about dink-printf. */
	
	while (start <= end) {
		status = MSS_read( ppcsim, start, &opcode, 4 );
		if (status != 0)
			return( status );
		printf("opcode %08X\n", opcode);

		disassemble_an_opcode( ppcsim, opcode, start );
		
		printf( "\t" );
		if (shell_read_line( mnemonic, "" ) != 0)
			break;
		printf("do '%s'\n", mnemonic);
		if (!*mnemonic  ||  *mnemonic == 'q'  ||  *mnemonic == '.')
			break;

		tmpint = 0;
		while(mnemonic[tmpint]==' ' || mnemonic[tmpint]=='\t')
		{
			tmpint++;
		}	
		

				/* First we need to make sure the user didn't
				   enter a carrage return only, indicating
				   that the current memory location does
				   not need modification
				*/
				/* Also want to ignore # and .orig and .dc
				   lines so cut and paste is less painful
				*/
		if(strncmp(&mnemonic[tmpint],".orig",5) == 0 || 
			strncmp(&mnemonic[tmpint],".dc",3) == 0 || 
			mnemonic[tmpint] == '#'
		  )
	 	{
			start = start - 4;
		}	

		else if (mnemonic[tmpint] != '\0') {

// Check for exit, either 'x' or ESC.

			if (((mnemonic[tmpint] == 'x') && ((mnemonic[tmpint+1] == '\0')
										    ||(mnemonic[tmpint+1] == '\n')))
				|| (mnemonic[tmpint] == 0x1B)) {

// Verify branch labels, and get values not supplied.

				tmpint2 = 0;
				for (count = 0;count < MAXBRANCHLABEL; count++) {
					if (label_table[count].address == 8675309 
				    && label_table[count].name[0] != '0') {
						if (!tmpint2++)
							printf("\nVERIFYING BRANCH LABELS.....\n");
						printf("\n `%s' undefined, enter address 0x",
							label_table[count].name );
						//dink_gets(mnemonic);
						shell_read_line( mnemonic, "ppcsim.dasm> " );

				      	// Delete whitespace
						tmpint = 0;
				    	while (mnemonic[tmpint]==' ' || mnemonic[tmpint]=='\t')
							tmpint++;
				    	label_table[count].address = pull_immediate(&mnemonic[tmpint]);
						printf("`%s' => 0x%08lx\n", label_table[count].name,
												label_table[count].address);
					}
				}
				if (!tmpint2)
					printf("\n");
				return( 0 );
			}

				/* I have a mnemonic or a lable, now I need to
				   go assemble it.
				*/
			status = assemble_a_mnemonic( mnemonic, &opcode, start );
		
			if( status == 0 )
			{
				/* If we get here, then the assemble
				   operation was a success.  We need to
				   update memory with the new opcode.
				*/

				//status = write_to_memory( start, opcode );			
				status = MSS_write( ppcsim, start, &opcode, 4 );

				/* Make sure the write was successful
				*/
				if( status != 0 )
				{
					return( status );
				}
				
			}
			else
			{

				/* If I got here, I either have an invalid
				   mnemonic OR a label for a branch that I 
				   need to add to my label_table structure
				   to be used during branches
				*/
				tmpint2 = tmpint;
				if (mnemonic[tmpint2] == '.') tmpint2++;
				if (mnemonic[tmpint2] == '@') tmpint2++;
				if (mnemonic[tmpint2] == '.') tmpint2++;
				while (mnemonic[tmpint2] != ':' && 
				       mnemonic[tmpint2] != '\0'
				      )
					tmpint2++;
				if (mnemonic[tmpint2] == ':')
				{
				/* If I get in here, the user entered a lable
				   ending in ':' like it should be...now I 
				   need to enter the new label into my 
				   label_table to be used by branches
				*/
				   mnemonic[tmpint2] = '\0';

				/* Now the lable exists between tmpint and
				   tmpint2 in mnemnonic...first I need to
				   search the label_table to make sure the 
				   lable isn't already in there...then add it
				   and the address (start-4) to 
				   the table
				*/

			   	   tmpint2 = 0;
				   while(tmpint2 < MAXBRANCHLABEL)
				   {
				     if (label_table[tmpint2].name[0] == '\0')
					break;
				     if (strcmp(label_table[tmpint2].name,
					&mnemonic[tmpint]) == 0)
				     {
		
				        printf( "Invalid label...already exists\n" );
					break;
				     }	
				     tmpint2++;
				   }	
				   if (tmpint2 == MAXBRANCHLABEL)
				     printf( "Error...Can't support any more lables!\n" );
				   if (label_table[tmpint2].name[0] == '\0')
				   {
				      strcpy(label_table[tmpint2].name,&mnemonic[tmpint]);
				      label_table[tmpint2].address = start;
				      label_table[tmpint2].label_type = USYM;
				   } 
				}
				else 
				{ 
				  /* If I get here, the user has entered a
				     mnemonic that I can't understand.  I
				     will have to print out a message saying
				     so and give the user another chance.
			  	  */
				  printf( "Unknown instruction\n" );
				}	
				  /* Ha!  Now I need to decrement the pointer
				     to memory because it's about to be
				     incremented, and I want to try to
				     assemble the same address again.
				  */
				start = start - 4;
			}
		}

				/* Increment out pointer to memory.
				*/
		start = start + 4;
		
	}
	
	return( 0 );

}

#endif
