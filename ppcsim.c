// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

/***************************************************************************
 *     Copyright Motorola, Inc. 1989-2002 ALL RIGHTS RESERVED
 *
 *  $Id: ppcsim.c,v 1.1.1.1 2002/04/25 22:20:45 maurie Exp $
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

//===========================================================================
// ppcsim -- a virtual DINK instruction-set simulator (ISS).
//
//	 Copyright 2002 by Motorola
//	 Written by Gary W. Milliorn
// 
// Notes:
//  ppcsim is of sufficient quality that VDINK can actually run a
//  binary image of DINK.  The only restrictions are:
//
//    * Must be a recent position-independant build with ROM_RESIDENT
//		defined
//    * You'll want to patch out calls to delay() unless you
//      have something else to do.  Poke a 'blr' at the address
//		of delay(), found in the link map (dink_dir/xref.txt).
//
//  For extra credit, compile a program and have the emulated
//  DINK run it.
//
//  For extra extra credit, compile VDINK to a PPC binary and have
//  VDINK run DINK run VDINK run [...ad nauseaum...].
//
//	Special thanks to Kristi and Alex for putting up with me while I
//  wrote this, way too late at night.
//===========================================================================

//===========================================================================
// Status:
//
// Known bugs:
//   not convinced backtrace is accurate, seems to miss pushes somewhere
//   fp status not recorded.
//   fp exception on operands handling
//   still not sure overflow is correct
//	 rfi interferes with tr+ -- user vs. dink issue?
//   LE mode not supported for altivec 
//   dec/tb should not work if tben not set (vger-class).
//	 ppcsim_dec_limit not based on CPU or PLL or instruction issue rate
//	 VSCR[NJ]=1 not implemented (required: if denorm set op->0.0)
//   self-resetting register bits (L2CR HWF et al) are not modelled 
//   The MSS is more hard-coded than the 824X/10x.
//
// Unknown bugs:
//
// Pending/TODO
//   On exception, setup SRR's better
//   Profiling?
//   STF -- stack frame; tput home and clear possible
//   print on mem access (trace probe)
//   go/tr w/no args prints current info
//	 i2c emulator could be useful.
//	 replace get/put fpr/vpr with direct access?
//
// Note:
//  HOW TO: simulate DINK on VDINK
//  1. Make a Sandpoint DINK with ROM_RESIDENT defined.
//  2. Look in the link map and find the address of delay().
//  3. Start up VDINK.
//  4. Type in 'vhf /somewhere/dink_dir/dink32.src'
//  5. Type in 'dl -h'
//  6. Type in 'as <address of delay>' and assemble 'blr'.
//  7. Type 'go 100'
//  8. Play with DINK on VDINK
//  9. Type Ctl-C to interrupt the simulator
//
// Note: I'm currently working on these as an enjoyable home project, 
//    if you can believe such a thing, so please don't implement them 
//    unless you have a special need to do so.  Thanks!
//
//  Mnemonic          Added           Tested		Bugs
//  =========================================================================
//  fabs[x]											CR/FPSCR recording
//  fadd[x]											"
//  fadds[x]
//  fcmpo
//  fcmpu
//  fctiw[x]
//  fctiwz[x]
//  fdiv[x]											CR/FPSCR
//  fdivs[x]
//  fmadd[x]										CR/FPSCR
//  fmadds[x]
//  fmr[x]											CR/FPSCR
//  fmsub[x]										CR/FPSCR
//  fmsubs[x]
//  fmul[x]											CR/FPSCR
//  fmuls[x]
//  fnabs[x]										CR/FPSCR
//  fneg[x]											CR/FPSCR
//  fnmadd[x]										CR/FPSCR
//  fnmadds[x]
//  fnmsub[x]										CR/FPSCR
//  fnmsubs[x]
//  fres[x]
//  frsp[x]	
//  frsqrte[x]
//  fsel[x]	
//  fsqrt[x]
//  fsqrts[x]
//  fsub[x]											CR/FPSCR
//  fsubs[x]
//	mffs[x]											CR/FPSCR
//	lvebx
//	lvehx
//	lvewx
//	lvsl
//	lvsr
//	lvx					Y
//	lvxl				Y
//	mcrfs				Y
//	mffs[x]				Y
//	mfvscr				Y
//	mtcrf				Y
//	mtfsf[x]			Y
//	mtfsfi[x]			Y
//	mtvscr				Y
//	mulhw[x]			Y				Y
//	mulhwu[x]			Y				Y
//	mullw[x]			Y				Y
//	rfi					Y				Y
//	rlwimi[x]			Y				Y
//	rlwnm[x]			Y				Y
//	sraw[x]				Y				Y
//	srawi[x]			Y				Y
//	stfiwx				Y				Y
//	stfs				Y				Y
//	stfsu				Y				Y
//	stfsux				Y				Y
//	stfsx				Y				Y
//	stvebx
//	stvehx
//	stvewx
//	stvx				Y				Y
//	stvxl				Y				Y
//	subf[x]				Y				Y
//	subfc[x]			Y				Y
//	subfe[x]			Y				Y
//	subfic				Y				Y
//	subfme[x]			Y				Y
//	subfze[x]			Y				Y
//	vaddcuw
//	vaddfp
//	vaddsbs
//	vaddshs
//	vaddsws
//	vaddubm
//	vaddubs
//	vadduhm
//	vadduhs
//	vadduwm
//	vadduws
//	vand				Y
//	vandc				Y
//	vavgsb
//	vavgsh
//	vavgsw
//	vavgub
//	vavguh
//	vavguw
//	vcfsx
//	vcfus
//	vcmpbfp[.]
//	vcmpeqfp[.]
//	vcmpequb[.]
//	vcmpequh[.]
//	vcmpequw[.]
//	vcmpgefp[.]
//	vcmpgtfp[.]
//	vcmpgtsb[.]
//	vcmpgtsh[.]
//	vcmpgtsw[.]
//	vcmpgtub[.]
//	vcmpgtuh[.]
//	vcmpgtuw[.]
//	vctsxs
//	vctuxs
//	vexptefp
//	vlogefp
//	vmaddfp
//	vmaxfp
//	vmaxsb
//	vmaxsh
//	vmaxsw
//	vmaxub
//	vmaxuh
//	vmaxuw
//	vmhaddshs
//	vmhraddshs
//	vminfp
//	vminsb
//	vminsh
//	vminsw
//	vminub
//	vminuh
//	vminuw
//	vmladduhm
//	vmsummbm			not right
//	vmsumshs			opdet
//	vnmsubfp
//	vnor				Y
//	vor					Y
//	vperm
//	vpkpx
//	vpkshss
//	vpkshus
//	vrfin				what is rndtonear??
//	vrfiz				- not ok
//	vsubsbs				dest b
//	vsubshs
//	vsubsws
//	vsububm
//	vsububs
//	vsubuhm
//	vsubuhs
//	vsubuwm
//	vsubuws
//	vsumsws
//	vsum2sws
//	vsum4sbs
//	vsum4shs			new
//	vupkhpx
//	vupkhsb
//	vupkhsh
//	vupklpx
//	vupklsb				offset
//===========================================================================

//===========================================================================
// A Note on Carry and Overflow
// * Carry is easy.  For add/sub/etc, use ULONG64 and check that the
//   MSB(63-32) is non-zero (for adds) or if the MSB(63:32) is
//   different from that of the minuend.
// * Overflow is a signed concept, and is set when the MSB+1 (bit
//   1, that is) of the operands force a carry out that is different
//   from the carry out of the MSB.  Since the MSB is the sign bit,
//   this indicates a transition from + to - space, generally a
//   problem with overflowing a signed number space.
//
//	   00     00     00     00		// Case 1: CIMSB=0, COMSB=0
//   + 00   + 01   + 10   + 11		// Case 2: CIMSB=0, COMSB=0
//   =000   =001   =010   =011		// Case 3: CIMSB=0, COMSB=0
//     								// Case 4: CIMSB=0, COMSB=0
//
//	   01     01     01     01		// Case 1: CIMSB=0, COMSB=0
//   + 00   + 01   + 10   + 11		// Case 2: CIMSB=1, COMSB=0 :: OVERFLOW
//   =001   =010   =011   =100		// Case 3: CIMSB=0, COMSB=0
//     								// Case 4: CIMSB=1, COMSB=0	:: OVERFLOW
//
//	   10     10     10     10		// Case 1: CIMSB=0, COMSB=0
//   + 00   + 01   + 10   + 11		// Case 2: CIMSB=0, COMSB=0
//   =010   =011   =100   =101		// Case 3: CIMSB=1, COMSB=0 :: OVERFLOW
//     								// Case 4: CIMSB=1, COMSB=0	:: OVERFLOW
//
//	   11     11     11     11		// Case 1: CIMSB=0, COMSB=0
//   + 00   + 01   + 10   + 11		// Case 2: CIMSB=1, COMSB=1
//   =011   =100   =101   =110		// Case 3: CIMSB=0, COMSB=1 :: OVERFLOW
//     								// Case 4: CIMSB=1, COMSB=1
//===========================================================================



//---------------------------------------------------------------------------
//
#include "ppcsim.h"
#include "regs_spr.h"
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>


#define PPCSIM_LINUX

#ifdef PPCSIM_LINUX
#include <fenv.h>
#endif

#ifdef PPCSIM_CYGWIN
#include <mingw/fenv.h>
#endif

#ifdef PPCSIM_SUNOS
#include <ieeefp.h>
#define fesetround(a)	fpsetround(a)

#define FE_TONEAREST  FP_RN
#define FE_TOWARDZERO FP_RZ 
#define FE_UPWARD     FP_RP 
#define FE_DOWNWARD   FP_RM 

#endif


//===========================================================================
//
// Simulator helper routines.
//

#define GR(a)			(GPR[a])

#define EXTS_16(a)      (((a) & 0x80) ? ((a) | 0xFF00) : (a))
#define EXTS(a)         (((a) & 0x8000) ? ((a) | 0xFFFF0000) : (a))
#define EXTS_64(a)      (((a) & 0x80000000) ? ((a) | 0xFFFFFFFF00000000LL) : (a))

//#define EXTS(a)			(((a) & 0x8000) ? ((a) | 0xFFFF0000) : (a))

//#define EXTS64(a)		(((a) & 0x80000000) ? ((a) | 0xFFFFFFFF00000000) : (a))

#define XER_CA			((SPR_get( SPR_XER ) & 0x20000000) ? 1 : 0)




#define UITOUISAT_8(a) 		(((a) & 0xFFFFFF00) ? 0xFF   : (a))
#define UITOUISAT_16(a) 	(((a) & 0xFFFF0000) ? 0xFFFF : (a))
#define UITOUISAT_32(a) 	(((a) & 0xFFFFFFFF00000000LL) ? 0xFFFFFFFF : (a))

#define SITOSISAT_8(a) (((a) & 0x8000) ?\
			(((a) & 0x7F00) ? 0x80 : (a)) :\
			(((a) & 0x7F00) ? 0x7F : (a)))

#define SITOSISAT_16(a) (((a) & 0x80000000) ?\
			(((a) & 0x7FFF0000) ? 0x8000 : (a)) :\
			(((a) & 0x7FFF0000) ? 0x7FFF : (a)))

#define SITOSISAT_32(a) (((a) & 0x8000000000000000LL) ?\
			(((a) & 0x7FFFFFFF00000000LL) ? 0x80000000 : (a)) :\
			(((a) & 0x7FFFFFFF00000000LL) ? 0x7FFFFFFF : (a)))

#define SITOUISAT_16(a) (((a) & 0x80000000) ?\
			(((a) & 0x7FFF0000) ? 0x0000 : (a)) :\
			(((a) & 0x7FFF0000) ? 0xFFFF : (a)))

#define FPXCHECK	printf("ppcsim: FPXCHECK -- TBD\n");


ULONG64 lvsr_data[] = {
        0x1011121314151617LL, 0x18191A1B1C1D1E1FLL,
        0x0F10111213141516LL, 0x1718191A1B1C1D1ELL,
        0x0E0F101112131415LL, 0x161718191A1B1C1DLL,
        0x0D0E0F1011121314LL, 0x15161718191A1B1CLL,
        0x0C0D0E0F10111213LL, 0x1415161718191A1BLL,
        0x0B0C0D0E0F101112LL, 0x131415161718191ALL,
        0x0A0B0C0D0E0F1011LL, 0x1213141516171819LL,
        0x090A0B0C0D0E0F10LL, 0x1112131415161718LL,
        0x08090A0B0C0D0E0FLL, 0x1011121314151617LL,
        0x0708090A0B0C0D0ELL, 0x0F10111213141516LL,
        0x060708090A0B0C0DLL, 0x0E0F101112131415LL,
        0x05060708090A0B0CLL, 0x0D0E0F1011121314LL,
        0x0405060708090A0BLL, 0x0C0D0E0F10111213LL,
        0x030405060708090ALL, 0x0B0C0D0E0F101112LL,
        0x0203040506070809LL, 0x0A0B0C0D0E0F1011LL,
        0x0102030405060708LL, 0x090A0B0C0D0E0F10LL
};


ULONG64 lvsl_data[] = {
        0x0001020304050607LL, 0x08090A0B0C0D0E0FLL,
        0x0102030405060708LL, 0x090A0B0C0D0E0F10LL,
        0x0203040506070809LL, 0x0A0B0C0D0E0F1011LL,
        0x030405060708090ALL, 0x0B0C0D0E0F101112LL,
        0x0405060708090A0BLL, 0x0C0D0E0F10111213LL,
        0x05060708090A0B0CLL, 0x0D0E0F1011121314LL,
        0x060708090A0B0C0DLL, 0x0E0F101112131415LL,
        0x0708090A0B0C0D0ELL, 0x0F10111213141516LL,
        0x08090A0B0C0D0E0FLL, 0x1011121314151617LL,
        0x090A0B0C0D0E0F10LL, 0x1112131415161718LL,
        0x0A0B0C0D0E0F1011LL, 0x1213141516171819LL,
        0x0B0C0D0E0F101112LL, 0x131415161718191ALL,
        0x0C0D0E0F10111213LL, 0x1415161718191A1BLL,
        0x0D0E0F1011121314LL, 0x15161718191A1B1CLL,
        0x0E0F101112131415LL, 0x161718191A1B1C1DLL,
        0x0F10111213141516LL, 0x1718191A1B1C1D1ELL
};


//---------------------------------------------------------------------------
//      pSwap4Bytes -- swap a 32bit input value from Big Endian to Little Endian,
//                                 or vice versa.
//---------------------------------------------------------------------------
ULONG pSwap4Bytes( ULONG v )
{
	ULONG sw;

	sw = ( (v & 0x000000ff) << 24)
	   | ( (v & 0x0000ff00) <<  8)
	   | ( (v & 0x00ff0000) >>  8) 
	   | ( (v               >> 24) );

	return( sw );
}


//================================================================================
//								FLOW TRACING
//================================================================================


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int ppcsim_backtrace( PPCSIM *ppcsim )
{
	int i;

	printf("  Backtrace Stack Information\n"
		   "  ===========================\n");
	printf("    Maximum stack depth = %d levels\n", ppcsim->max_stack);
	printf("    Maximum R1 at blr   = %08X\n\n", ppcsim->max_R1);

	printf("    Stack backtrace:\n");

	i = (ppcsim->btsp >= ppcsim->backtrace_max) ? ppcsim->backtrace_max-1 : ppcsim->btsp;
	for (i = ppcsim->btsp; i >= 0; i--)
		printf("    [%2d]: %08X\n", i, ppcsim->BTS[i]);
	
	return( 0 );
}


//---------------------------------------------------------------------------
//	ppcsim_backtrace_add -- add an entry to the backtrace stack.
//---------------------------------------------------------------------------
int ppcsim_backtrace_add( PPCSIM *ppcsim, ULONG CIA )
{

// Note that ppcsim->btsp increases even if we don't stack something.  This
// way, calls to pop something don't pop too early.

	if (ppcsim->btsp < BACKTRACE_MAX) {
		ppcsim->btsp += 1;
		ppcsim->BTS[ ppcsim->btsp ] = CIA;

		if (ppcsim->verbose)
			printf("  ppcsim-BTA: CIA = @%08X => BTS[%d]\n", CIA, ppcsim->btsp);

		if (ppcsim->btsp > ppcsim->max_stack) {
			ppcsim->max_stack = ppcsim->btsp;
			ppcsim->max_R1 = GR(1);
		}
	}
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void ppcsim_backtrace_pop( PPCSIM *ppcsim )
{

	if (ppcsim->btsp > -1) {
		ppcsim->btsp -= 1;
		if (ppcsim->verbose)
			printf("  ppcsim-BTP: -> %d\n", ppcsim->btsp);
	}
}


//================================================================================
//								REGISTERS
//================================================================================



//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void XER_setOV( int OV )
{
	ULONG v;

	v = SPR_get( SPR_XER );
	if (OV)
		v |=  0xC0000000;						// Set SO if setting OV
	else
		v &= ~0x40000000;						// But don't clear SO
	SPR_set( SPR_XER, v );
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void XER_setCA( int CA )
{
	ULONG v;

	v = SPR_get( SPR_XER );
	if (CA)
		v |=  0x20000000;
	else
		v &= ~0x20000000;
	SPR_set( SPR_XER, v );
}


//---------------------------------------------------------------------------
//	CR_get -- get a 4-bit CR field from the CR register; no=[0..7]
//---------------------------------------------------------------------------
ULONG CR_get( int no )
{
	ULONG v;

	v = SPR_get( SPR_CR );

	no = (7 - no) * 4;		
	v >>= no;
	return( v & 0x0F );
}


//---------------------------------------------------------------------------
//	CR_put -- put a 4-bit CR field in the CR register; no=[0..7]
//---------------------------------------------------------------------------
void CR_put( int no, ULONG v )
{
	ULONG curr, mask;

	curr = SPR_get( SPR_CR );

	v &= 0x0F;
	mask = ~(0xF <<  ((7 - no) * 4));
	v            <<=  (7 - no) * 4;
	curr = (curr & mask) | v;

	SPR_set( SPR_CR, curr );
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CR_ops( int op, int A, int B, int D )
{
	ULONG cr, mask;
	ULONG bitA, bitB, bitD;

	cr = SPR_get( SPR_CR );

	bitA = (cr >> (31-A)) & 0x01;
	bitB = (cr >> (31-B)) & 0x01;

	switch (op) {
	default:
	case  33:	bitD = ~(bitA |  bitB);					break;
	case 129:	bitD =  (bitA & ~bitB);					break;
	case 193:	bitD =  (bitA ^  bitB);					break;
	case 225:	bitD = ~(bitA &  bitB);					break;
	case 257:	bitD =  (bitA &  bitB);					break;
	case 289:	bitD = ~(bitA ^  bitB);					break;
	case 417:	bitD =  (bitA | ~bitB);					break;
	case 449:	bitD =  (bitA |  bitB);					break;
	}

	mask = ~(1 << (31-D));
	bitD &= 0x1;
	bitD = bitD << (31-D);
	cr   = (cr & mask) | bitD;

	SPR_set( SPR_CR, cr );
}


//---------------------------------------------------------------------------
//	stod -- promote single to double precision as per PEM D.6
//---------------------------------------------------------------------------
void stod( ULONG v, FREG *f )
{
	int w1_8, w9_31, i;
	int exp;
	ULONG64 frac, b;

	w1_8  = (v >> (31-8)) & 0xFF;
	w9_31 = (v          ) & 0x007FFFFF;
	printf("stod of %08x, w1_8=%d\n", v, w1_8);

// Normalized.

	if ((w1_8 > 0)  &&  (w1_8 < 255)) {
		f->ll  = ((ULONG64) (v & 0xC0000000)) << 32;	// frd[0-1]=w[0-1]
		if (!(v & 0x40000000))
			f->ll |= (ULONG64) 0x3800000000000000LL;		// frd[2-4]=~w[1]
		f->ll |= ((ULONG64) (v & 0x3FFFFFFF)) << 29;	// frd[5-63]=w[2-31]
	}

// Denormalized.

	else if (w1_8 == 0  &&  w9_31 != 0) {
		printf("stod: denorm\n");
		exp  = -126;
		frac = ((ULONG64) v) << 29;
		while ((frac & 0x0010000000000000LL) == 0) {
			frac <<= 1;
			exp--;
		}
		f->ll  = (v & 0x80000000) ? ((ULONG64) 1) << 63 : 0;// F[0] = sign
		b = (exp + 1023);
		f->ll |= b << (63-11);								// F[1-11] = exp
		f->ll |= frac;
	}

// Infinity/QNan/SNan/Zero

	else if ((w1_8 == 255)  ||  ((v & 0x7FFFFFFF) == 0)) {
		printf("stod: inf/nan/zer\n");
		f->ll  = ((ULONG64) (v & 0xC0000000)) << 32;	// frd[0-1]=w[0-1]
		if (v & 0x40000000)
			f->ll |= (ULONG64) 0x3800000000000000LL;		// frd[2-4]=w[1]
		f->ll |= ((ULONG64) (v & 0x3FFFFFFF)) << 29;	// frd[5-63]=w[2-31]
	}

// ?

	else {
		f->v[0] = v;
		f->v[1] = 0;
	}
}


//---------------------------------------------------------------------------
//	dtos -- demote double to single precision as per PEM D.7
//---------------------------------------------------------------------------
ULONG dtos( FREG *f )
{
	ULONG v, frs_1_11;
	int exp;
	ULONG64 frac;

	printf("dtos -- not implemented\n");

	printf("dtos of %16.8f (%16llx)\n", f->f, f->ll);
	frs_1_11 = f->v[0] >> (31-11);

	if ((f->ll & (~0x8000000000000000LL))
	||  (frs_1_11 > 896)) {
		printf("dtos: nodenorm\n");
		v = (f->v[0] & 0xC0000000)
		  | (f->ll >> (63-34));
	}

	else {
		printf("dtos: denorm\n");
		v = 0xDEADBEEF;
		if ((874 <= frs_1_11)  &&  (frs_1_11 <= 896)) {
			v = (f->v[0] & 0x80000000);				// Sign
			exp = frs_1_11 - 1023;
			frac = ((ULONG64) 1 << (63-12+1))
				 |  f->ll & 0x000FFFFFFFFFFFFFLL;
			while (exp < -126) {
				frac = (frac >> 1) & ~0x8000000000000000LL;
				exp++;
			}
			v |= (ULONG) (frac >> (63-23));
		}
	}
	
	printf("dtos: return %08x\n", v);
	return( v );
}



//================================================================================
//================================================================================


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int sim_powerpc( PPCSIM *ppcsim, ULONG opcode )
{
	int      inst, A, B, C, D, OE, SC, CA;
	int      SH, MB, ME, BO, BI;
	int		 i, j, cr, AA, LK, OV, setCA;
	int		 ctr_ok, cond_ok, isAV, isFP;
    short    RC, RC1, FRC, FSRC, VRC, SAT;
	char	 cb;
	ULONG 	 EA, SIMM, LI, ac;
	ULONG64  l64;
	LONG64	 sl64A, sl64B;
	LONG	 sA, sB;
	UCHAR	 uc;
	USHORT	 uh;
	int	     status;
	FREG	 fA, fB, fC, fD;
	VREG	 vA, vB, vC, vD;

	
	if (ppcsim->verbose)
		printf("  ppcsim: %08X => ", opcode);

// Decode instruction class.

	inst = (opcode >> 26) & 0x3F;
	SC   = 0;
	RC   = 0;
	RC1  = 0;
	FRC  = 0;
	FSRC = 0;
	VRC  = 0;
	CA   = 0;
	setCA= 0;
	OE   = 0;
	OV   = 0;
	D    = 0;
	A    = 0;
	B    = 0;
	SIMM = 0;
	SAT  = 0;
	isAV = 0;
	isFP = 0;


// Decompose the instruction based on the instruction class value.

	switch (inst) {
	case 00:											// invalid
		return( 0x700 );
		break;

	case 3:												// twi
	case 7:												// mulli
	case 8:												// subfic
	case 10:											// cmpli
	case 11:											// cmpi
	case 12:											// addic
	case 13:											// addic.
	case 14:											// addi
	case 15:											// addis
	case 24:											// ori
	case 25:											// oris
	case 26:											// xori
	case 27:											// xoris
	case 28:											// andi.
	case 29:											// andis.
	case 32:											// lwz
	case 33:											// lwzubz
	case 34:											// lbz
	case 35:											// lbzu
	case 36:											// stw
	case 37:											// stwu
	case 38:											// stb
	case 39:											// stbu
	case 40:											// lhz
	case 41:											// lhzu
	case 42:											// lha
	case 43:											// lhau
	case 44:											// sth
	case 45:											// sthu
	case 46:											// lmw
	case 47:											// stmw
	case 48:											// lfs
	case 49:											// lfsu
	case 50:											// lfd
	case 51:											// lfdu
	case 52:											// stfs
	case 53:											// stfsu
	case 54:											// stfd
	case 55:											// stfdu
		D    = (opcode >> 21) & 0x01F;
		A    = (opcode >> 16) & 0x01F;
		SIMM =  opcode        & 0xFFFF;
		break;

	case 4:												// mfvscr
		D    = (opcode >> 21) & 0x01F;
		A    = (opcode >> 16) & 0x01F;
		B    = (opcode >> 11) & 0x01F;
		SC   =  opcode        & 0x7FF;
		isAV = 1;
		break;

	case 16:											// bc[l][a]
		BO   = (opcode >> 21) & 0x01F;
		BI   = (opcode >> 16) & 0x01F;
		AA   = (opcode >> 1)  & 0x1;
		LK   =  opcode        & 0x1;
		LI   =  opcode        & 0xFFFC;
		break;

	case 17:											// sc
		break;

	case 18:											// b[l][a]
		LI   =  opcode       & 0x03FFFFFC;
		AA   = (opcode >> 1) & 0x1;
		LK   =  opcode       & 0x1;
		break;

	case 19:											// crand
	case 31:											// addi[ce][o][.]
	case 63:											// mtfsb0[x]
		D  = (opcode >> 21) & 0x01F;
		A  = (opcode >> 16) & 0x01F;
		B  = (opcode >> 11) & 0x01F;
		OE = (opcode >> 10) & 0x001;
		SC = (opcode >>  1) & 0x3FF;
		RC =  opcode        & 0x001;
		isFP = (inst == 63);
		break;

	case 20:											// rlwimi[x]
	case 21:											// rlwinm[x]
	case 23:											// rlwnm[x]
		D  = (opcode >> 21) & 0x01F;
		A  = (opcode >> 16) & 0x01F;
		SH = (opcode >> 11) & 0x01F;
		MB = (opcode >>  6) & 0x01F;
		ME = (opcode >>  1) & 0x01F;
		RC =  opcode        & 0x001;
		break;

	default:
		printf("  ppcsim: no inst decoder for %d!\n", inst);
		return( 1 );
	}

	if (ppcsim->verbose)
		printf("op=%d D=%02d A=%02d B=%02d OE=%1d SC=%4d RC=%d SIMM=%08x\n",
				inst, D, A, B, OE, SC, RC, SIMM);


//===========================================================================
// For FP and AV, check that such instructions are permitted.

	if (isFP  ||  isAV) {
		ac = SPR_get( SPR_MSR );
		if (isFP  &&  !(ac & 0x00002000)) {
			ppcsim->exc_type = 0x800;
			return( ppcsim->exc_type );
		}
		if (isAV  &&  !(ac & 0x02000000)) {
			ppcsim->exc_type = 0xF20;
			return( ppcsim->exc_type );
		}
	}
	
//===========================================================================
// Now that we've decomposed the instruction into interesting parts,
// use them to emulate the instruction.
//
// As part of the computation, set:
//
//	RC  to update CR0[LT+GT+EQ+SO] using "ac"
//	RC1 to update CR1["]           using "ac" (nonFP ops)
//	FRC to update CR1["]           using "fD"
//  OE  to update XER[SO+OV]
//  CA  to update XER[CA]
//
// Some of these may be pre-set by the opcode decoding.


	if (inst < 0)
		return( 1 );

//-- CLASS 3 ------------------------------------------------------------

	else if (inst == 3) {                	 			// twi
		sA = GR(A);
		sB = EXTS( SIMM );
		ac = EXTS( SIMM );
		if (ppcsim->verbose)
			printf("sA %x  sB %x\n", (ULONG) sA, (ULONG) sB);
		if (((D & 0x10)  &&  (sA  <  sB))
		||  ((D & 0x08)  &&  (sA  >  sB))
		||  ((D & 0x04)  &&  (sA ==  sB))
		||  ((D & 0x02)  &&  (sA  <  ac))
		||  ((D & 0x01)  &&  (sA  >  ac)))
			return( 0x0700 );
	}

//-- CLASS 4 ------------------------------------------------------------

	else if (inst == 4) {                	 			// mfvscr
		
		// Some opcodes are overloaded with vC operands.  Blecch!

		if (((SC & 0x3F) == 36)
		||  ((SC & 0x3F) == 37)
		||  ((SC & 0x3F) == 38)
		||  ((SC & 0x3F) == 39)
		||  ((SC & 0x3F) == 40)
		||  ((SC & 0x3F) == 41)
		||  ((SC & 0x3F) == 42)
		||  ((SC & 0x3F) == 43)
		||  ((SC & 0x3F) == 44)) {
			C = (SC >> 6) & 0x1F;
			VPR_get( ppcsim, C, &vC );
			SC &= 0x3F;
		}

		// Not always needed, but usually so.  So far not a
		// performance issue.

		VPR_get( ppcsim, A, &vA );
		VPR_get( ppcsim, B, &vB );

		switch (SC) {

		case 0:											// vaddubm
			for (i = 0; i <= 15; i++)
				vD.b[i] = vA.b[i] + vB.b[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 2:											// vmaxub
			for (i = 0; i <= 15; i++)
				vD.b[i] = (vA.b[i] >= vB.b[i]) ? vA.b[i] : vB.b[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 4:											// vrlb
			for (i = 0; i <= 15; i++) {
				j = vB.b[i] & 0x07;
				vD.b[i] = (vA.b[i] << j)
						| (vA.b[i] >> (8-j));
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 6:											// vcmpequb
		case 1030:										// vcmpequb.
			for (i = 0; i <= 15; i++)
				vD.b[i] = (vA.b[i] == vB.b[i]) ? ~0 : 0;
			VRC  = (opcode >> 10) & 0x001;
			ac =  (vD.q[0] == ~0  &&  vD.q[1] == ~0) ? 0x08 : 0x00;
			ac |= (vD.q[0] ==  0  &&  vD.q[1] ==  0) ? 0x02 : 0x00;
			VPR_set( ppcsim, D, &vD );
			break;

		case 8:											// vmuloub
			for (i = 0; i <= 7; i++)
				vD.h[i] = vA.b[i*2+1] * vB.b[i*2+1];
			VPR_set( ppcsim, D, &vD );
			break;

		case 10:										// vaddfp
			for (i = 0; i <= 3; i++)
				vD.f[i] = vA.f[i] + vB.f[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 12:										// vmrghb
			for (i = 0; i <= 7; i++) {
				vD.b[i*2  ] = vA.b[i];
				vD.b[i*2+1] = vB.b[i];
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 14:										// vpkuhum
			for (i = 0; i <= 7; i++)
				vD.b[i]   = vA.b[i*2+1];
			for (i = 0; i <= 7; i++)
				vD.b[i+8] = vB.b[i*2+1];
			VPR_set( ppcsim, D, &vD );
			break;

		case 36:										// vmsumubm
			for (i = 0; i <= 3; i++) {
				for (ac = 0, j = i*4; j <= i*4+3; j++)
					ac += (USHORT) vA.b[j] * (USHORT) vB.b[j];
				vD.w[i] = vC.w[i] + ac;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 37:										// vmsummbm
			for (i = 0; i <= 3; i++) {
				for (sA = 0, j = i*4; j <= i*4+3; j++)
					sA += vA.sb[j] * vB.b[j];
				sA = EXTS( sA & 0xFFFF );
				vD.sw[i] = vC.sw[i] + ac;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 38:										// vmsumuhm
			for (i = 0; i <= 3; i++)
				vD.w[i] = vC.w[i] + (vA.h[i*2  ] * vB.h[i*2  ])
								  + (vA.h[i*2+1] * vB.h[i*2+1]);
			VPR_set( ppcsim, D, &vD );
			break;

		case 39:										// vmsumuhs
			for (i = 0; i <= 3; i++) {
				l64 = (ULONG64) vC.w[i];
				l64 += (vA.h[i*2  ] * vB.h[i*2  ])
					 + (vA.h[i*2+1] * vB.h[i*2+1]);
				vD.w[i] = UITOUISAT_32( l64 );
				if (vD.w[i] != l64)
					SAT++;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 40:										// vmsumshm
			for (i = 0; i <= 3; i++)
				vD.sw[i] = vC.sw[i] + (vA.sh[i*2  ] * vB.sh[i*2  ])
									+ (vA.sh[i*2+1] * vB.sh[i*2+1]);
			VPR_set( ppcsim, D, &vD );
			break;

		case 41:										// vmsumshs
			for (i = 0; i <= 3; i++) {
				sl64A = EXTS_64( vC.w[i] );
				sl64A += (vA.sh[i*2  ] * vB.sh[i*2  ])
					  +  (vA.sh[i*2+1] * vB.sh[i*2+1]);
				vD.w[i] = SITOSISAT_32( sl64A );
				if (vD.w[i] != sl64A)
					SAT++;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 42:										// vsel
			vD.q[0] = (vA.q[0] & ~vC.q[0])
			        | (vB.q[0] &  vC.q[0]);
			vD.q[1] = (vA.q[1] & ~vC.q[1])
			        | (vB.q[1] &  vC.q[1]);
			VPR_set( ppcsim, D, &vD );
			break;

		case 43:										// vperm
			for (i = 0; i <= 15; i++) {
				ac = vC.b[i] & 0x1F;
				vD.b[i] = (ac < 16) ? vA.b[ac] : vB.b[ac-16];
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 44:										// vsldoi
			C &= 0x0F;				// 0 = Va->Vd, 
			for (i = 0; i <= 15; i++)
				if (i+C <= 15)
					vD.b[i] = vA.b[i+C];
				else
					vD.b[i] = vB.b[i+C-16];
			VPR_set( ppcsim, D, &vD );
			break;

		case 64:										// vadduhm
			for (i = 0; i <= 7; i++)
				vD.h[i] = vA.h[i] + vB.h[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 66:										// vmaxuh
			for (i = 0; i <= 7; i++)
				vD.h[i] = (vA.h[i] >= vB.h[i]) ? vA.h[i] : vB.h[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 68:										// vrlh
			for (i = 0; i <= 7; i++) {
				j = vB.h[i] & 0x0F;
				vD.h[i] = (vA.h[i] << j)
						| (vA.h[i] >> (16-j));
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 70:										// vcmpequh
		case 1094:										// vcmpequh.
			for (i = 0; i <= 7; i++)
				vD.h[i] = (vA.h[i] == vB.h[i]) ? ~0 : 0;
			VRC  = (opcode >> 10) & 0x001;
			ac =  (vD.q[0] == ~0  &&  vD.q[1] == ~0) ? 0x08 : 0x00;
			ac |= (vD.q[0] ==  0  &&  vD.q[1] ==  0) ? 0x02 : 0x00;
			VPR_set( ppcsim, D, &vD );
			break;

		case 72:										// vmulouh
			for (i = 0; i <= 3; i++)
				vD.w[i] = vA.h[i*2+1] * vB.h[i*2+1];
			VPR_set( ppcsim, D, &vD );
			break;

		case 74:										// vsubfp
			for (i = 0; i <= 3; i++)
				vD.f[i] = vA.f[i] - vB.f[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 76:										// vmrghh
			for (i = 0; i <= 3; i++) {
				vD.h[i*2  ] = vA.h[i];
				vD.h[i*2+1] = vB.h[i];
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 78:										// vpkuwum
			for (i = 0; i <= 3; i++)
				vD.h[i]   = vA.h[i*2+1];
			for (i = 0; i <= 3; i++)
				vD.h[i+4] = vB.h[i*2+1];
			VPR_set( ppcsim, D, &vD );
			break;

		case 128:										// vadduwm
			for (i = 0; i <= 3; i++)
				vD.w[i] = vA.w[i] + vB.w[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 130:										// vmaxuw
			for (i = 0; i <= 3; i++)
				vD.w[i] = (vA.w[i] >= vB.w[i]) ? vA.w[i] : vB.w[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 132:										// vrlw
			for (i = 0; i <= 3; i++) {
				j = vB.w[i] & 0x1F;
				vD.w[i] = (vA.w[i] << j)
						| (vA.w[i] >> (32-j));
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 134:										// vcmpequw
		case 1158:										// vcmpequw.
			for (i = 0; i <= 3; i++)
				vD.w[i] = (vA.w[i] == vB.w[i]) ? ~0 : 0;
			VRC  = (opcode >> 10) & 0x001;
			ac =  (vD.q[0] == ~0  &&  vD.q[1] == ~0) ? 0x08 : 0x00;
			ac |= (vD.q[0] ==  0  &&  vD.q[1] ==  0) ? 0x02 : 0x00;
			VPR_set( ppcsim, D, &vD );
			break;

		case 140:										// vmrghw
			for (i = 0; i <= 1; i++) {
				vD.w[i*2  ] = vA.w[i];
				vD.w[i*2+1] = vB.w[i];
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 142:										// vpkuhus
			for (i = 0; i <= 7; i++) {
				vD.b[i] = UITOUISAT_8( vA.h[i] );
				if (vD.b[i] != vA.h[i] )
					SAT++;
			}
			for (i = 0; i <= 7; i++) {
				vD.b[i+8] = UITOUISAT_8( vB.h[i] );
				if (vD.b[i] != vB.h[i] )
					SAT++;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 198:										// vcmpeqfp
		case 1222:										// vcmpeqfp.
			for (i = 0; i <= 3; i++)
				vD.w[i] = (vA.f[i] == vB.f[i]) ? ~0 : 0;
			VRC  = (opcode >> 10) & 0x001;
			ac =  (vD.q[0] == ~0  &&  vD.q[1] == ~0) ? 0x08 : 0x00;
			ac |= (vD.q[0] ==  0  &&  vD.q[1] ==  0) ? 0x02 : 0x00;
			VPR_set( ppcsim, D, &vD );
			break;

		case 206:										// vpkuwus
			for (i = 0; i <= 3; i++) {
				vD.h[i]   = UITOUISAT_16( vA.w[i] );
				if (vD.h[i] != vA.w[i] )
					SAT++;
			}
			for (i = 0; i <= 3; i++) {
				vD.h[i+4] = UITOUISAT_16( vB.w[i] );
				if (vD.h[i] != vB.w[i] )
					SAT++;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 258:										// vmaxsb
			for (i = 0; i <= 15; i++)
				vD.sb[i] = (vA.sb[i] >= vB.sb[i]) ? vA.sb[i] : vB.sb[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 260:										// vslb
			for (i = 0; i <= 15; i++) {
				B = vB.b[i] & 0x07;
				vD.b[i] = vA.b[i] << B;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 264:										// vmulosb
			for (i = 0; i <= 7; i++)
				vD.sh[i] = vA.sb[i*2+1] * vB.sb[i*2+1];
			VPR_set( ppcsim, D, &vD );
			break;

		case 266:										// vrefp
			for (i = 0; i <= 3; i++) {
				fA.f = 1.0 / (double) vB.f[i];
				vD.f[i] = (double) fA.f;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 268:										// vmrglb
			for (i = 0; i <= 7; i++) {
				vD.b[i*2  ] = vA.b[i+8];
				vD.b[i*2+1] = vB.b[i+8];
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 322:										// vmaxsh
			for (i = 0; i <= 7; i++)
				vD.sh[i] = (vA.sh[i] >= vB.sh[i]) ? vA.sh[i] : vB.sh[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 324:										// vslh
			for (i = 0; i <= 7; i++) {
				B = vB.h[i] & 0x0F;
				vD.h[i] = vA.h[i] << B;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 328:										// vmulosh
			for (i = 0; i <= 3; i++)
				vD.sw[i] = vA.sh[i*2+1] * vB.sh[i*2+1];
			VPR_set( ppcsim, D, &vD );
			break;

		case 330:										// vrsqrtefp
			for (i = 0; i <= 3; i++) {
				fA.f = 1.0 / sqrt( (double) vB.f[i] );
				vD.f[i] = (double) fA.f;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 332:										// vmrglh
			for (i = 0; i <= 3; i++) {
				vD.h[i*2  ] = vA.h[i+4];
				vD.h[i*2+1] = vB.h[i+4];
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 334:										// vpkswus
			for (i = 0; i <= 3; i++) {
				vD.h[i]   = SITOUISAT_16( vA.w[i] );
				if (vD.h[i] != vA.w[i])
					SAT++;
			}
			for (i = 0; i <= 3; i++) {
				vD.h[i+4] = SITOUISAT_16( vB.w[i] );
				if (vD.h[i+4] != vB.w[i])
					SAT++;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 386:										// vmaxsw
			for (i = 0; i <= 3; i++)
				vD.sw[i] = (vA.sw[i] >= vB.sw[i]) ? vA.sw[i] : vB.sw[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 384:										// vaddcuw
			for (i = 0; i <= 3; i++) {
				vC.q[0] = (ULONG64) vA.w[i] + (ULONG64) vB.w[i];
				printf("vaddcuw: init %16llx\n", vC.q[0]);
				vD.w[i] = vC.w[0];
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 388:										// vslw
			for (i = 0; i <= 3; i++) {
				B = vB.w[i] & 0x1F;
				vD.w[i] = vA.w[i] << B;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 394:										// vexptefp
			for (i = 0; i <= 3; i++) {
				fA.f = pow( (double) 2.0, (double) vB.f[i] );
				vD.f[i] = (double) fA.f;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 396:										// vmrglw
			for (i = 0; i <= 1; i++) {
				vD.w[i*2  ] = vA.w[i+2];
				vD.w[i*2+1] = vB.w[i+2];
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 452:										// vsl
			B = vB.b[15] & 0x07;						// Optimistic interp.
			vD.q[1] =   vA.q[1] << B;
			vD.q[0] =  (vA.q[0] << B)
					| ((vA.q[1] & ~(B-1)) >> 64-B);
			VPR_set( ppcsim, D, &vD );
			break;

		case 454:										// vcmpgefp
		case 1478:										// vcmpgefp.
			for (i = 0; i <= 3; i++)
				vD.w[i] = (vA.f[i] >= vB.f[i]) ? ~0 : 0;
			VRC  = (opcode >> 10) & 0x001;
			ac =  (vD.q[0] == ~0  &&  vD.q[1] == ~0) ? 0x08 : 0x00;
			ac |= (vD.q[0] ==  0  &&  vD.q[1] ==  0) ? 0x02 : 0x00;
			VPR_set( ppcsim, D, &vD );
			break;

		case 458:										// vlogefp
			for (i = 0; i <= 3; i++) {
				fA.f = log( (double) vB.f[i] ) / M_LN2;
				vD.f[i] = (double) fA.f;				// dbl->flt
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 462:										// vpkswss
			for (i = 0; i <= 3; i++) {
				vD.sh[i]   = SITOSISAT_16( vA.w[i] );
				if (vD.sh[i] != vA.w[i])
					SAT++;
			}
			for (i = 0; i <= 3; i++) {
				vD.sh[i+4] = SITOSISAT_16( vB.w[i] );
				if (vD.sh[i+4] != vB.w[i])
					SAT++;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 514:										// vminub
			for (i = 0; i <= 15; i++)
				vD.b[i] = (vA.b[i] <= vB.b[i]) ? vA.b[i] : vB.b[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 516:										// vsrb
			VPR_get( ppcsim, A, &vA );
			VPR_get( ppcsim, B, &vB );
			for (i = 0; i <= 15; i++) {
				B = vB.b[i] & 0x07;
				vD.b[i] = vA.b[i] >> B;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 518:										// vcmpgtub
		case 1542:										// vcmpgtub.
			for (i = 0; i <= 15; i++)
				vD.b[i] = (vA.b[i] > vB.b[i]) ? ~0 : 0;
			VRC  = (opcode >> 10) & 0x001;
			ac =  (vD.q[0] == ~0  &&  vD.q[1] == ~0) ? 0x08 : 0x00;
			ac |= (vD.q[0] ==  0  &&  vD.q[1] ==  0) ? 0x02 : 0x00;
			VPR_set( ppcsim, D, &vD );
			break;

		case 520:										// vmuleub
			for (i = 0; i <= 7; i++)
				vD.h[i] = vA.b[i*2] * vB.b[i*2];
			VPR_set( ppcsim, D, &vD );
			break;

		case 522:										// vrfin
			printf("RNDTOFPINT32NEAR\n");
			VPR_set( ppcsim, D, &vD );
			break;

		case 524:										// vspltb
			VPR_get( ppcsim, B, &vB );
			A &= 0xF;
			for (i = 0; i <= 15; i++)
				vD.b[i] = vB.b[ A ];
			VPR_set( ppcsim, D, &vD );
			break;

		case 526:										// vupkhsb
			for (i = 0; i <= 7; i++)
				vD.sh[i] = EXTS_16( vB.sb[i] );
			VPR_set( ppcsim, D, &vD );
			break;

		case 578:										// vminuh
			for (i = 0; i <= 7; i++)
				vD.h[i] = (vA.h[i] <= vB.h[i]) ? vA.h[i] : vB.h[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 580:										// vsrh
			for (i = 0; i <= 7; i++) {
				B = vB.h[i] & 0x0F;
				vD.h[i] = vA.h[i] >> B;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 582:										// vcmpgtuh
		case 1606:										// vcmpgtuh.
			for (i = 0; i <= 7; i++)
				vD.h[i] = (vA.h[i] > vB.h[i]) ? ~0 : 0;
			VRC  = (opcode >> 10) & 0x001;
			ac =  (vD.q[0] == ~0  &&  vD.q[1] == ~0) ? 0x08 : 0x00;
			ac |= (vD.q[0] ==  0  &&  vD.q[1] ==  0) ? 0x02 : 0x00;
			VPR_set( ppcsim, D, &vD );
			break;

		case 584:										// vmuleuh
			for (i = 0; i <= 3; i++)
				vD.w[i] = vA.h[i*2] * vB.h[i*2];
			VPR_set( ppcsim, D, &vD );
			break;

		case 586:										// vrfiz
			fesetround( FE_TOWARDZERO );						// Round to 0
			for (i = 0; i <= 3; i++)
				if (vB.f[i] >= 0.0)
					vD.f[i] = (float) floor( (double) vB.f[ i ] );
				else
					vD.f[i] = (float) ceil( (double) vB.f[ i ] );
			fesetround( FE_TONEAREST );						// Default rounding.
			VPR_set( ppcsim, D, &vD );
			break;

		case 588:										// vsplth
			VPR_get( ppcsim, B, &vB );
			A &= 0x7;
			for (i = 0; i <= 7; i++)
				vD.h[i] = vB.h[ A ];
			VPR_set( ppcsim, D, &vD );
			break;

		case 590:										// vupkhsh
			for (i = 0; i <= 3; i++)
				vD.w[i] = EXTS( vB.h[i] );
			VPR_set( ppcsim, D, &vD );
			break;

		case 640:										// vadduws
			for (i = 0; i <= 3; i++) {
				l64  = vA.w[i];
				l64 += vB.w[i];
				vD.w[i] = UITOUISAT_32( l64 );
				if (vD.w[i] != l64)
					SAT++;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 642:										// vminuw
			for (i = 0; i <= 3; i++)
				vD.w[i] = (vA.w[i] <= vB.w[i]) ? vA.w[i] : vB.w[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 644:										// vsrw
			for (i = 0; i <= 3; i++) {
				B = vB.w[i] & 0x1F;
				vD.w[i] = vA.w[i] >> B;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 646:										// vcmpgtuw
		case 1670:										// vcmpgtuw.
			for (i = 0; i <= 3; i++)
				vD.w[i] = (vA.w[i] > vB.w[i]) ? ~0 : 0;
			VRC  = (opcode >> 10) & 0x001;
			ac =  (vD.q[0] == ~0  &&  vD.q[1] == ~0) ? 0x08 : 0x00;
			ac |= (vD.q[0] ==  0  &&  vD.q[1] ==  0) ? 0x02 : 0x00;
			VPR_set( ppcsim, D, &vD );
			break;

		case 650:										// vrfip
			fesetround( FE_UPWARD );						// Round to +oo
			for (i = 0; i <= 3; i++)
				vD.f[i] = (float) ceil( (double) vB.f[ i ] );
			fesetround( FE_TONEAREST );						// Default rounding.
			VPR_set( ppcsim, D, &vD );
			break;

		case 652:										// vspltw
			A &= 0x3;
			for (i = 0; i <= 3; i++)
				vD.w[i] = vB.w[ A ];
			VPR_set( ppcsim, D, &vD );
			break;

		case 654:										// vupklsb
			for (i = 0; i <= 7; i++)
				vD.sh[i] = EXTS_16( vB.sb[i+8] );
			VPR_set( ppcsim, D, &vD );
			break;

		case 708:										// vsr
			B = vB.b[15] & 0x07;
			vD.q[0] =   vA.q[0] >> B;
			vD.q[1] =  (vA.q[1] >> B)
					| ((vA.q[0] & ~(B-1)) << 64-B);
			VPR_set( ppcsim, D, &vD );
			break;

		case 710:										// vcmpgtfp
		case 1734:										// vcmpgtfp.
			for (i = 0; i <= 3; i++)
				vD.w[i] = (vA.f[i] > vB.f[i]) ? ~0 : 0;
			VRC  = (opcode >> 10) & 0x001;
			ac =  (vD.q[0] == ~0  &&  vD.q[1] == ~0) ? 0x08 : 0x00;
			ac |= (vD.q[0] ==  0  &&  vD.q[1] ==  0) ? 0x02 : 0x00;
			VPR_set( ppcsim, D, &vD );
			break;

		case 714:										// vrfim
			fesetround( FE_DOWNWARD );						// Round to -oo
			for (i = 0; i <= 3; i++)
				vD.f[i] = (float) floor( (double) vB.f[ i ] );
			fesetround( FE_TONEAREST );						// Default rounding.
			VPR_set( ppcsim, D, &vD );
			break;

		case 718:										// vupklsh
			for (i = 0; i <= 3; i++)
				vD.w[i] = EXTS( vB.sh[i+4] );
			VPR_set( ppcsim, D, &vD );
			break;

		case 768:										// vaddsbs
			for (i = 0; i <= 15; i++) {
				vC.sh[0] = EXTS_16( vA.sb[i] ) + EXTS_16( vB.sb[i] );
				if (vC.sh[0] < -128) {
					vD.sb[i] = 0x80;
					SAT++;
				}
				else if (vC.sh[0] > 127) {
					vD.sb[i] = 0x7F;
					SAT++;
				}
				else
					vD.sb[i] = vC.sh[0];
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 770:										// vminsb
			for (i = 0; i <= 15; i++)
				vD.sb[i] = (vA.sb[i] <= vB.sb[i]) ? vA.sb[i] : vB.sb[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 772:										// vsrab
			VPR_get( ppcsim, A, &vA );
			VPR_get( ppcsim, B, &vB );
			for (i = 0; i <= 15; i++) {
				B = vB.b[i] & 0x07;
				A = vA.b[i];
				if (A & 0x80)	
					A |= 0xFF00;
				vD.b[i] = A >> B;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 774:										// vcmpgtsb
		case 1798:										// vcmpgtsb.
			for (i = 0; i <= 15; i++)
				vD.b[i] = (vA.sb[i] > vB.sb[i]) ? ~0 : 0;
			VRC  = (opcode >> 10) & 0x001;
			ac =  (vD.q[0] == ~0  &&  vD.q[1] == ~0) ? 0x08 : 0x00;
			ac |= (vD.q[0] ==  0  &&  vD.q[1] ==  0) ? 0x02 : 0x00;
			VPR_set( ppcsim, D, &vD );
			break;

		case 776:										// vmulesb
			for (i = 0; i <= 7; i++)
				vD.sh[i] = vA.sb[i*2] * vB.sb[i*2];
			VPR_set( ppcsim, D, &vD );
			break;

		case 780:										// vspltisb
			A = (A & 0x10) ? A | 0xFFFFFFF0 : A;
			for (i = 0; i <= 15; i++)
				vD.b[i] = A;
			VPR_set( ppcsim, D, &vD );
			break;

		case 782:										// vpkpx
			for (i = 0; i <= 7; i++) {
				ac = (i >= 4) ? vB.w[i-4] : vA.w[i];
				j  = ((ac >> (31- 7)) & 0x01) << 15;	// alpha: 1 bit
				j |= ((ac >> (31-12)) & 0x1F) << 10;	// red:   5 bits
				j |= ((ac >> (31-20)) & 0x1F) <<  5;	// green: 5 bits
				j |= ((ac >> (31-28)) & 0x1F);			// blue:  5 bits
				vD.h[i  ] = j;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 832:										// vaddshs
			for (i = 0; i <= 7; i++) {
				vC.sw[0] = EXTS( vA.sh[i] ) + EXTS( vB.sh[i] );
				if (vD.sh[i] != sl64A)
					SAT++;
				if (vC.sw[0] < -32768) {
					vD.sh[i] = 0x8000;
					SAT++;
				}
				else if (vC.sw[0] > 32767) {
					vD.sh[i] = 0x7FFF;
					SAT++;
				}
				else
					vD.sh[i] = vC.sw[0];
			}
			VPR_set( ppcsim, D, &vD );
			break;

			break;

		case 834:										// vminsh
			for (i = 0; i <= 7; i++)
				vD.sh[i] = (vA.sh[i] <= vB.sh[i]) ? vA.sh[i] : vB.sh[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 836:										// vsrah
			VPR_get( ppcsim, A, &vA );
			VPR_get( ppcsim, B, &vB );
			for (i = 0; i <= 7; i++) {
				B = vB.h[i] & 0x0F;
				vD.h[i] = vA.sh[i] >> B;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 838:										// vcmpgtsh
		case 1862:										// vcmpgtsh.
			for (i = 0; i <= 7; i++)
				vD.h[i] = (vA.sh[i] > vB.sh[i]) ? ~0 : 0;
			VRC  = (opcode >> 10) & 0x001;
			ac =  (vD.q[0] == ~0  &&  vD.q[1] == ~0) ? 0x08 : 0x00;
			ac |= (vD.q[0] ==  0  &&  vD.q[1] ==  0) ? 0x02 : 0x00;
			VPR_set( ppcsim, D, &vD );
			break;

		case 840:										// vmulesh
			for (i = 0; i <= 3; i++)
				vD.sw[i] = vA.sh[i*2] * vB.sh[i*2];
			VPR_set( ppcsim, D, &vD );
			break;

		case 844:										// vspltish
			A = (A & 0x10) ? A | 0xFFFFFFF0 : A;
			for (i = 0; i <= 7; i++)
				vD.h[i] = A;
			VPR_set( ppcsim, D, &vD );
			break;

		case 898:										// vminsw
			for (i = 0; i <= 3; i++)
				vD.sw[i] = (vA.sw[i] <= vB.sw[i]) ? vA.sw[i] : vB.sw[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 900:										// vsraw
			VPR_get( ppcsim, A, &vA );
			VPR_get( ppcsim, B, &vB );
			for (i = 0; i <= 3; i++) {
				B  = vB.w[i] & 0x1F;
				sl64A = EXTS_64( vA.w[i] );
				vD.w[i] = sl64A >>= B;
			}
			VPR_set( ppcsim, D, &vD );
			break;
				
		case 902:										// vcmpgtsw
		case 1926:										// vcmpgtsw.
			for (i = 0; i <= 3; i++)
				vD.w[i] = (vA.sw[i] > vB.sw[i]) ? ~0 : 0;
			VRC  = (opcode >> 10) & 0x001;
			ac =  (vD.q[0] == ~0  &&  vD.q[1] == ~0) ? 0x08 : 0x00;
			ac |= (vD.q[0] ==  0  &&  vD.q[1] ==  0) ? 0x02 : 0x00;
			VPR_set( ppcsim, D, &vD );
			break;

		case 908:										// vspltisw
			A = (A & 0x10) ? A | 0xFFFFFFF0 : A;
			for (i = 0; i <= 3; i++)
				vD.w[i] = A;
			VPR_set( ppcsim, D, &vD );
			break;

		case 1024:										// vsububm
			for (i = 0; i <= 15; i++)
				vD.b[i] = vA.b[i] - vB.b[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 1026:										// vavgub
			for (i = 0; i <= 15; i++) {
				ac = vA.b[i] + vB.b[i] + 1;
				vD.b[i] = ac >> 1;
			} 
			VPR_set( ppcsim, D, &vD );
			break;

		case 1028:										// vand
			VPR_get( ppcsim, A, &vA );
			VPR_get( ppcsim, B, &vB );
			vD.q[0] = vA.q[0] & vB.q[0];
			vD.q[1] = vA.q[1] & vB.q[1];
			VPR_set( ppcsim, D, &vD );
			break;

		case 1034:										// vmaxfp
			for (i = 0; i <= 3; i++)
				vD.f[i] = (vA.f[i] > vB.f[i]) ? vA.f[i] : vB.f[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 1036:										// vslo
			B = vB.b[15] >> 3;
			vD.q[0] = vD.q[1] = 0;
			for (i = 0; i <= 15; i++)
				if (i+B <= 15)
					vD.b[i] = vA.b[i+B];
			VPR_set( ppcsim, D, &vD );
			break;

		case 1088:										// vsubuhm
			for (i = 0; i <= 7; i++)
				vD.h[i] = vA.h[i] - vB.h[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 1090:										// vavguh
			for (i = 0; i <= 7; i++) {
				ac = vA.h[i] + vB.h[i] + 1;
				vD.h[i] = ac >> 1;
			} 
			VPR_set( ppcsim, D, &vD );
			break;

		case 1092:										// vandc
			VPR_get( ppcsim, A, &vA );
			VPR_get( ppcsim, B, &vB );
			vD.q[0] = vA.q[0] & ~ vB.q[0];
			vD.q[1] = vA.q[1] & ~ vB.q[1];
			VPR_set( ppcsim, D, &vD );
			break;

		case 1098:										// vminfp
			for (i = 0; i <= 3; i++)
				vD.f[i] = (vA.f[i] < vB.f[i]) ? vA.f[i] : vB.f[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 1100:										// vsro
			B = vB.b[15] >> 3;
			vD.q[0] = vD.q[1] = 0;
			for (i = 0; i <= 15; i++)
				if (i+B <= 15)
					vD.b[i+B] = vA.b[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 1152:										// vsubuwm
			for (i = 0; i <= 3; i++)
				vD.w[i] = vA.w[i] - vB.w[i];
			VPR_set( ppcsim, D, &vD );
			break;

		case 1154:										// vavguw
			for (i = 0; i <= 3; i++) {
				ac = vA.w[i] + vB.w[i] + 1;
				vD.w[i] = ac >> 1;
			} 
			VPR_set( ppcsim, D, &vD );
			break;

		case 1156:										// vor
			VPR_get( ppcsim, A, &vA );
			VPR_get( ppcsim, B, &vB );
			vD.q[0] = vA.q[0] | vB.q[0];
			vD.q[1] = vA.q[1] | vB.q[1];
			VPR_set( ppcsim, D, &vD );
			break;

		case 1220:										// vxor
			VPR_get( ppcsim, A, &vA );
			VPR_get( ppcsim, B, &vB );
			vD.q[0] = vA.q[0] ^ vB.q[0];
			vD.q[1] = vA.q[1] ^ vB.q[1];
			VPR_set( ppcsim, D, &vD );
			break;

		case 1282:										// vavgsb
			for (i = 0; i <= 15; i++) {
				vC.sh[0] = EXTS_16( vA.sb[i] )
				         + EXTS_16( vB.sb[i] )
						 + 1;
				vD.sb[i] = vC.sh[0] >> 1;
			} 
			VPR_set( ppcsim, D, &vD );
			break;

		case 1284:										// vnor
			VPR_get( ppcsim, A, &vA );
			VPR_get( ppcsim, B, &vB );
			vD.q[0] = ~(vA.q[0] | vB.q[0]);
			vD.q[1] = ~(vA.q[1] | vB.q[1]);
			VPR_set( ppcsim, D, &vD );
			break;

		case 1346:										// vavgsh
			for (i = 0; i <= 7; i++) {
				vC.sw[0] = EXTS( vA.sh[i] )
				         + EXTS( vB.sh[i] )
						 + 1;
				vD.sh[i] = vC.sw[0] >> 1;
			} 
			VPR_set( ppcsim, D, &vD );
			break;

		case 1408:										// vsubcuw
			for (i = 0; i <= 3; i++) {
				vC.q[0] = (ULONG64) vA.w[i] - (ULONG64) vB.w[i] + 1;
				vD.w[i] = vC.w[0];
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 1410:										// vavgsw
			for (i = 0; i <= 3; i++) {
				vC.sq[0] = EXTS_64( vA.sw[i] )
				         + EXTS_64( vB.sw[i] )
						 + 1;
				vD.sw[i] = vC.sq[0] >> 1;
			} 
			VPR_set( ppcsim, D, &vD );
			break;

		case 1540:										// mfvscr
			ac = SPR_get( SPR_VSCR );
			vD.q[0] = vD.q[1] = 0;
			vD.w[3] = ac;
			VPR_set( ppcsim, D, &vD );
			break;

		case 1544:										// vsum4ubs
			for (i = 0; i <= 3; i++) {
				vC.q[0] = vB.w[i];
				for (j = i*4; j <= i*4+3; j++)
					vC.q[0] += vA.b[j];
				vD.w[i] = vC.w[1];
				if (vC.w[0])					
					SAT++;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 1604:										// mtvscr
			ac = vB.w[3];
			SPR_set( SPR_VSCR, ac );
			break;

		case 1608:										// vsum4shs
			for (i = 0; i <= 3; i++) {
				vC.q[0] = EXTS_64( vB.w[i] );
				for (j = i*2; j <= i*2+1; j++)
					vC.q[0] += EXTS_64( EXTS( vA.sh[j] ) );
				vD.w[i] = vC.w[1];
				if (vC.w[0])					
					SAT++;
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 1792:										// vsubsbs
			for (i = 0; i <= 15; i++) {
				vC.sh[0] = EXTS_16( vA.sb[i] ) - EXTS_16( vB.sb[i] );
				if (vC.sh[0] < -128) {
					vD.sb[i] = 0x80;
					SAT++;
				}
				else if (vC.sh[0] > 127) {
					vD.sb[i] = 0x7F;
					SAT++;
				}
				else
					vD.sb[i] = vC.sh[0];
			}
			VPR_set( ppcsim, D, &vD );
			break;

		case 1928:										// vsumsws
			vD.q[0] = vD.q[1] = 0;
			sl64A = EXTS_64( vB.w[3] );
			printf("vsumsws: init %16llx\n", vD.q[1]);
			for (i = 0; i <= 3; i++) {
				sl64A += EXTS_64( vA.w[i] );
				printf("vsumsws: %d add %16llx\n", i, sl64A );
				sl64B = SITOSISAT_32( sl64A );
				printf("vsumsws: %d sat %16llx\n", i, sl64B );
				if (sl64A != sl64B)
					SAT++;
			}
			vD.w[3] = sl64B;
			VPR_set( ppcsim, D, &vD );
			break;

		default:
			printf("4: no handler for %d\n", SC);
			return( 1 );
		}
	}

//-- CLASS 7 ------------------------------------------------------------

	else if (inst == 7) {                	 			// mulli

		// dec 16000000 squared = 256000000000000
		// hex   F42400    "    = E8D4A5100000

		ac = GR(A);
		l64 = ac * SIMM;
		ac = (ULONG) l64 & (unsigned long long) 0xffffffff;
		GR(D) = ac;
	}

//-- CLASS 8 ------------------------------------------------------------

	else if (inst == 8) {                	 			// subfic
		sl64A = ~ ((LONG64) GR(A));
		sl64A += SIMM + 1;
		GR(D) = ac = (ULONG) sl64A;
		setCA = 1;
		if (sl64A & 0xFFFFFFFF00000000LL)
			CA = 1;
	}

//-- CLASS 10 -----------------------------------------------------------

	else if (inst == 10) {                	 			// cmpli
		if (GR(A) < SIMM)
			ac = 0x08;
		else if (GR(A) > SIMM)
			ac = 0x04;
		else
			ac = 0x02;
		if (SPR_get( SPR_XER ) & 0x80000000)
			ac |= 0x01;
		D >>= 2;
		CR_put( D, ac );
	}

//-- CLASS 11 -----------------------------------------------------------

	else if (inst == 11) {                	 			// cmpi
		sA = (LONG) GR(A);
		sB = EXTS( SIMM );
		if (sA < sB)
			ac = 0x08;
		else if (sA > sB)
			ac = 0x04;
		else
			ac = 0x02;
		if (SPR_get( SPR_XER ) & 0x80000000)
			ac |= 0x01;
		D >>= 2;
		CR_put( D, ac );
	}

//-- CLASS 12 -----------------------------------------------------------
//-- CLASS 13 -----------------------------------------------------------

	else if (inst == 12                   	 			// addic
		 ||  inst == 13) {                 	 			// addic.
		l64 = ((ULONG64) GR(A)) + ((ULONG64) EXTS( SIMM ));
		GR(D) = ac = (ULONG) l64 & 0xFFFFFFFF;
		setCA = 1;
		CA = (l64 & 0xFFFFFFFF00000000LL) ? 1 : 0;
		RC = (inst == 13) ? 1 : 0;						// RC for "addic."
	}

//-- CLASS 14 -----------------------------------------------------------

	else if (inst == 14) {                	 			// addi
		GR(D) = ac = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
	}

//-- CLASS 15 -----------------------------------------------------------

	else if (inst == 15) {                 	 			// addis
		GR(D) = ac = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM << 16 );
	}

//-- CLASS 16 -----------------------------------------------------------
//			bdnz
//			

	else if (inst == 16) {                 	 			// bc[l][a]
		if (ppcsim->verbose)
			printf("BO = %05x BI = %05x LK=%d AA=%d\n", BO, BI, LK, AA);

		if (LK) {
			SPR_set( SPR_LR, ppcsim->NIA );
			ppcsim_backtrace_add( ppcsim, ppcsim->CIA );
		}

		if ((BO & 0x04) == 0) {
			ac = SPR_get( SPR_CTR );
			ac--;
			SPR_set( SPR_CTR, ac );
		}

		ctr_ok = (BO & 0x04) | ((ac != 0) ^ ((BO >> 1) & 0x01));

		ac = SPR_get( SPR_CR );
		ME = (ac >> (31-BI)) & 0x01;
		cond_ok = (BO & 0x10) | (ME == ((BO & 0x08) ? 1 : 0));
		if (ctr_ok  &&  cond_ok) {
			if (AA)
				ppcsim->NIA = EXTS(LI);
			else
				ppcsim->NIA = ppcsim->CIA + EXTS(LI);
			if (ppcsim->flowtrace)
				printf("  ppcsim: bc/bcl/bca @%08X => %08X\n", ppcsim->CIA, ppcsim->NIA);
		}
	}

//-- CLASS 17 -----------------------------------------------------------

	else if (inst == 17) {                 	 			// sc
		return( 0xC00 );
	}

//-- CLASS 18 -----------------------------------------------------------

	else if (inst == 18) {                 	 			// b[l][a]
		if (LK) {
			SPR_set( SPR_LR, ppcsim->NIA );
			ppcsim_backtrace_add( ppcsim, ppcsim->CIA );
		}

		ac = (LI & 0x02000000) ? 0xFC000000 : 0;		// 24-bit EXTS
		ac |= LI;

		if (AA)
			ppcsim->NIA = ac;
		else
			ppcsim->NIA = ppcsim->CIA + ac;

		if (ppcsim->flowtrace)
			printf("  ppcsim: b/bl/ba @%08X => %08X\n", ppcsim->CIA, ppcsim->NIA);
	}

//-- CLASS 19 -----------------------------------------------------------

	else if (inst == 19) {
		switch (SC) {
		case    0:										// mcrf
			A >>= 2;
			ac = CR_get( A );
			D >>= 2;
			CR_put( D, ac );
			break;

// CRAND:	0-5=19	6-10=BT	11-15=BA 16-20=BB          21-30=129 31=/
// BCCTR:	0-5=19  6-10=BO 11-15=BI 16-18=// 19-20=BH 21-30=528 31=LK
//					D-OK	A-OK	 B if masked		OK
		case  16:										// bclr[l]
		case 528:										// bcctr[l]
			BO = D;			// 6-10
			BI = A;			// 11-15
		 // BH = B			// 16-20	(mask if needed)
			LK =  opcode & 0x1;
			if (ppcsim->verbose)
				printf("BO=D = %d  BI=A = %05x BH=B = %05x LK=%d\n", BO, BI, B, LK);

			// 1z1zz (unconditional) overrides decrement and others.
			//
			if ((BO & 0x14) == 0x14) {
				cond_ok = 1;
			} else {
				if ((BO & 0x04) == 0) {
					ac = SPR_get( SPR_CTR );
					ac--;
					SPR_set( SPR_CTR, ac );
				}

				ac = SPR_get( SPR_CR );
				ME = (ac >> (31-BI)) & 0x01;
				cond_ok = (BO & 0x10) | (ME == ((BO & 0x08) ? 1 : 0));
			}
		
			if (cond_ok) {
				ppcsim->NIA = SPR_get( (SC == 528) ? SPR_CTR : SPR_LR );
				if (SC == 16)
					ppcsim_backtrace_pop( ppcsim );
				if (LK) {
					SPR_set( SPR_LR, ppcsim->CIA+4 );
					ppcsim_backtrace_add( ppcsim, ppcsim->CIA );
				}

				if (ppcsim->flowtrace)
					printf("  ppcsim: blr/bclr/bctr @%08X => %08X\n", ppcsim->CIA, ppcsim->NIA);
			}
			break;

		case   33:										// crnor
		case  129:										// crandc
		case  193:										// crxor
		case  225:										// crnand
		case  257:										// crand
		case  289:										// creqv
		case  417:										// crorc
		case  449:										// cror
			CR_ops( SC, A, B, D );
			break;

		case   50:										// rfi
			ac = SPR_get( SPR_SRR1 );
			LI = SPR_get( SPR_MSR  );

			ac &=  0x0000FF73;
			LI &= ~0x000FF73;
			LI |= ac;
			SPR_set( SPR_MSR, ac );
			EA = SPR_get( SPR_SRR0 );

			ppcsim->NIA = EA & 0xFFFFFFFC;
			if (ppcsim->flowtrace)
				printf("  ppcsim: rfi @%08X => %08X\n", ppcsim->CIA, ppcsim->NIA);
			break;

		case  150:										// isync
			break;

		default:
			printf("19: no handler for %d\n", SC);
			return( 1 );
		}
	}

//-- CLASS 20 -----------------------------------------------------------
//-- CLASS 21 -----------------------------------------------------------
//-- CLASS 23 -----------------------------------------------------------

	else if ((inst == 20)                  	 			// rlwimi[.]
	     ||  (inst == 21)                  	 			// rlwinm[.]
	     ||  (inst == 23)) {               	 			// rlwnm[.]
		if (ppcsim->verbose)
			printf("  ppcsim: rlw-x >> MB = %d ME=%d operand=%08x\n",
				 MB, ME, GR(D));
		for (BO = 0, i = MB; i <= ME; i++)
			BO |= 1 << (31-i);							// BO = mask

		if (inst == 23)
			SH = GR(SH) & 0x1F;

		ac = GR(D) << SH;
		BI = GR(D) >> (32-SH);				// BO = mask
		if (ppcsim->verbose)
			printf("  ppcsim: rlw-x >> mask=%08X SHLoff=%08X recover: %08x\n",
				BO, ac, BI);
		ac |= BI;							// Rotated data.

		ac = (ac & BO);
		if (inst == 20)
			ac |= (GR(A) & ~BO);
		GR(A) = ac;
		if (ppcsim->verbose)
			printf("  ppcsim: rlw-x >> ac = %08x\n", ac);
	}

//-- CLASS 24 -----------------------------------------------------------

	else if (inst == 24) {                	 			// ori
		GR(A) = GR(D) | SIMM;
	}

//-- CLASS 25 -----------------------------------------------------------

	else if (inst == 25) {                	 			// oris
		GR(A) = GR(D) | (SIMM << 16);
	}

//-- CLASS 26 -----------------------------------------------------------

	else if (inst == 26) {                	 			// xori
		GR(A) = GR(D) ^ SIMM;
	}

//-- CLASS 27 -----------------------------------------------------------
  
	else if (inst == 27) {                	 			// xoris
		GR(A) = GR(D) ^ (SIMM << 16);
	}

//-- CLASS 28 -----------------------------------------------------------

	else if (inst == 28) {                	 			// andi.
		GR(A) = ac = GR(D) & SIMM;
		RC = 1;
	}

//-- CLASS 29 -----------------------------------------------------------

	else if (inst == 29) {                	 			// andis.
		GR(A) = ac = GR(D) & (SIMM << 16);
		RC = 1;
	}

//-- CLASS 31 -----------------------------------------------------------
//
// Note: the OPCODE overlays the OE field in many instances; when an
// opcode is >=512 OE is implicitly set above.  Sometimes this is
// good, sometimes it isn't.  Below, OE is reset for those ops which
// do not implement OE.

	else if (inst == 31) {
		switch (SC) {
		case 0:											// cmp
			sA = (LONG) GR(A);
			sB = (LONG) GR(B);
			if (sA < sB)
				ac = 0x08;
			else if (sA > sB)
				ac = 0x04;
			else
				ac = 0x02;
			if (SPR_get( SPR_XER ) & 0x80000000)
				ac |= 0x01;
			D >>= 2;
			CR_put( D, ac );
			break;

		case 4:											// tw
			sA = GR(A);
			sB = GR(B);
			ac = EXTS( SIMM );
			if (ppcsim->verbose)
				printf("sA %x  sB %x\n", (ULONG) sA, (ULONG) sB);
			if (((D & 0x10)  &&  (sA  <  sB))
			||  ((D & 0x08)  &&  (sA  >  sB))
			||  ((D & 0x04)  &&  (sA ==  sB))
			||  ((D & 0x02)  &&  (sA  <  ac))
			||  ((D & 0x01)  &&  (sA  >  ac)))
				return( 0x0700 );
			break;

		case 6:											// lvsl
			VPR_get( ppcsim, A, &vA );
			VPR_get( ppcsim, B, &vB );
			EA = ((A == 0) ? 0 : vA.q[1]) + vB.q[1];
			EA &= 0x0F;
			vD.q[0] = lvsl_data[ EA * 2     ];
			vD.q[1] = lvsl_data[ EA * 2 + 1 ];
			VPR_set( ppcsim, D, &vD );
			break;

		case 7:											// lvebx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			if (MSS_read( ppcsim, EA, (ULONG *) &uc, 1 ))
				return( ppcsim->exc_type );
			i  =  EA & 0x0F;
			vD.q[0] = vD.q[1] = 0xDEADBEEFDEADBEEFLL;		// per spec, alas!
			vD.b[i] = (ULONG) uc;
			VPR_set( ppcsim, D, &vD );
			break;

		case  8:										// subfc[.]
		case 40:										// subf[.]
		case 520:										// subfco[.]
		case 552:										// subfo[.]
			l64 = (~((ULONG64) GR(A))) + ((ULONG64) GR(B)) + 1;
			if (ppcsim->verbose)
				printf("SUBF: l64 result %16llx\n", l64);
			CA = (l64 & 0xFFFFFFFF00000000LL) ? 1 : 0;
			ac = ((~(GR(A) + 1)) & 0x80000000) + (GR(B) & 0x80000000);
			OV = (ac == (l64 & 0x80000000)) ? 0 : 1;
			OV = (OV != CA) ? 1 : 0;
			GR(D) = ac = (ULONG) l64 & 0xFFFFFFFF;
			setCA = (SC == 8  ||  SC == 520) ? 1 : 0;
			break;

		case 10:										// addc[.]
		case 522:										// addco[.]
			l64 = ((ULONG64) GR(A)) + ((ULONG64) GR(B));
			if (ppcsim->verbose)
				printf("ADDC: l64 result %16llx\n", l64);
			CA = (l64 & 0xFFFFFFFF00000000LL) ? 1 : 0;
			if (ppcsim->verbose)
				printf("ADDC: CA %d\n", CA);

			ac = (GR(A) & 0x80000000) + (GR(B) & 0x80000000);
			if (ppcsim->verbose)
				printf("ADDC: co_msb op %08x\n", ac);
			OV = (ac == (l64 & 0x80000000)) ? 0 : 1;
			if (ppcsim->verbose)
				printf("ADDC: co_msb = %d\n", OV);

			OV = (OV != CA) ? 1 : 0;
			if (ppcsim->verbose)
				printf("ADDC: OV %d\n", OV);

			GR(D) = ac = (ULONG) l64 & 0xFFFFFFFF;
			break;

		case 11:										// mulhwu[.]
			l64 = ((ULONG64) GR(A)) * ((ULONG64) GR(B));
			if (ppcsim->verbose)
				printf("MULHWU: l64A = %16llx\n", l64);
			GR(D) = ac = (ULONG) ((l64 >> 32) & 0xffffffff);
			break;

		case 19:										// mfcr
			ac = SPR_get( SPR_CR );
			GR(D) = ac;
			break;

		case 20:										// lwarx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			ppcsim->reserve_valid = 1;
			ppcsim->rsvd_addr = EA;
			if (MSS_read( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			GR(D) = ac;
			break;

		case 23:										// lwzx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			if (MSS_read( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			GR(D) = ac;
			break;

		case 24:										// slw[.]
			AA = GR(B) & 0x3F;							// Read the PEM!
			GR(A) = ac = (AA >= 32) ? 0 : (GR(D) << AA);
			break;

		case 26:										// cntlzw[.]
			B = 0;
			ac = GR(D);
			for (B = 0; B < 32; B++)
				if (ac & (1 << (31-B)))
					break;
			GR(A) = ac = B;
			break;

		case 28:										// and[.]
			GR(A) = ac = GR(D) & GR(B);
			break;

		case 32:										// cmpl
			if (GR(A) < GR(B))
				ac = 0x08;
			else if (GR(A) > GR(B))
				ac = 0x04;
			else
				ac = 0x02;
			if (SPR_get( SPR_XER ) & 0x80000000)
				ac |= 0x01;
			D >>= 2;
			CR_put( D, ac );
			break;

		case 38:										// lvsr
			VPR_get( ppcsim, A, &vA );
			VPR_get( ppcsim, B, &vB );
			EA = ((A == 0) ? 0 : vA.q[1]) + vB.q[1];
			EA &= 0x0F;
			vD.q[0] = lvsr_data[ EA * 2     ];
			vD.q[1] = lvsr_data[ EA * 2 + 1 ];
			VPR_set( ppcsim, D, &vD );
			break;

		case 39:										// lvehx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			if (MSS_read( ppcsim, EA, (ULONG *) &uh, 2 ))
				return( ppcsim->exc_type );
			i  =  EA & 0x0F;
			vD.q[0] = vD.q[1] = 0xDEADBEEFDEADBEEFLL;		// per spec, alas!
			vD.h[i] = (ULONG) uh;
			VPR_set( ppcsim, D, &vD );
			break;

		case 55:										// lwzux
			EA = GR(A) + GR(B);
			if (MSS_read( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			GR(D) = ac;
			GR(A) = EA;
			break;

		case 60:										// andc[.]
			GR(A) = ac = GR(D) & ~GR(B);
			break;

		case 71:										// lvewx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			if (MSS_read( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			i  =  EA & 0x0F;
			vD.q[0] = vD.q[1] = 0xDEADBEEFDEADBEEFLL;		// per spec, alas!
			vD.w[i] = ac;
			VPR_set( ppcsim, D, &vD );
			break;

		case 75:										// mulhw[.]
			sl64A = EXTS_64( GR(A) );
			sl64B = EXTS_64( GR(B) );
			sl64A *= sl64B;
			GR(D) = ac = (sl64A >> 32) & (unsigned long long) 0xffffffff;
			break;

		case 83:										// mfmsr
			ac = SPR_get( SPR_MSR );
			GR(D) = ac;
			break;

		case 87:										// lbzx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			if (MSS_read( ppcsim, EA, (ULONG *) &uc, 1 ))
				return( ppcsim->exc_type );
			GR(D) = (ULONG) uc;
			break;

		case 103:										// lvx
		case 359:										// lvxl
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			EA &= 0xFFFFFFF0;
			if (MSS_read( ppcsim, EA, &vA.w[0], 8 ))
				return( ppcsim->exc_type );
			EA += 0x08;
			if (MSS_read( ppcsim, EA, &vA.w[2], 8 ))
				return( ppcsim->exc_type );
			VPR_set( ppcsim, D, &vA );
			break;

		case 104:										// neg[.]
		case 616:										// nego[.]
			if (GR(A) == 0x80000000) {
				GR(D) = GR(A);
				OV    = 1;
			}
			else
				GR(D) = (~GR(A)) + 1;
			ac = GR(D);
			break;

		case 119:										// lbzux
			EA = GR(A) + GR(B);
			if (MSS_read( ppcsim, EA, (ULONG *) &uc, 1 ))
				return( ppcsim->exc_type );
			GR(D) = (ULONG) ac;
			GR(A) = EA;
			break;

		case 124:										// nor[.]
			GR(A) = ac = ~(GR(D) | GR(B));
			break;

		case 135:										// stvebx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			i  =  EA & 0x0F;
			VPR_get( ppcsim, D, &vA );
			ac = vA.b[i];
			if (MSS_write( ppcsim, EA, &ac, 1 ))
				return( ppcsim->exc_type );
			break;

		case 136:										// subfe[.]
		case 648:										// subfeo[.]
			l64 = (~((ULONG64) GR(A))) + ((ULONG64) GR(B)) + XER_CA;
			CA = (l64 & 0xFFFFFFFF00000000LL) ? 1 : 0;
			ac = ((GR(A) & 0x80000000) ^ 0x80000000)
			   + (GR(B) & 0x80000000) + XER_CA;
			OV = (ac == (l64 & 0x80000000)) ? 0 : 1;
			OV = (OV != CA) ? 1 : 0;
			GR(D) = ac = (ULONG) l64 & 0xFFFFFFFF;
			setCA = 1;
			break;

		case 138:										// adde[.]
		case 650:										// addeo[.]
			l64 = ((ULONG64) GR(A)) + ((ULONG64) GR(B)) + XER_CA;
			if (ppcsim->verbose)
				printf("ADDE[O]: l64 result %16llx\n", l64);
			CA = (l64 & 0xFFFFFFFF00000000LL) ? 1 : 0;
			ac = (GR(A) & 0x80000000) + (GR(B) & 0x80000000);
			if (ppcsim->verbose)
				printf("ADDE: co_msb op %08x\n", ac);
			OV = (ac == (l64 & 0x80000000)) ? 0 : 1;
			if (ppcsim->verbose)
				printf("ADDE: co_msb = %d\n", OV);

			OV = (OV != CA) ? 1 : 0;

			GR(D) = ac = (ULONG) l64 & 0xFFFFFFFF;
			setCA = 1;
			break;

		case 144:										// mtcrf
			B    = (opcode >> 12) & 0x0FF;
			LI = 0;
			for (i = 0; i <= 7; i++)
				if (B & (1 << (7-i)))
					LI |= 0xF << (4*(7-i));
			ac = SPR_get( SPR_CR );
			ac = ac & ~LI;
			ac = ac | (GR(D) & LI);
			SPR_set( SPR_CR, ac );
			OE = 0;
			break;

		case 146:										// mtmsr
			ac = GR(D);
			SPR_set( SPR_MSR, ac );
			break;

		case 150:										// stwcx.
			cr = (SPR_get( SPR_XER ) & 0x80000000) ? 0x1 : 0;		// Do our own CR
			RC = 0;										// management
			OE = 0;
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			if (ppcsim->reserve_valid) {
				if (ppcsim->rsvd_addr == EA) {
					ac = GR(D);
					if (MSS_write( ppcsim, EA, &ac, 4 ))
						return( ppcsim->exc_type );
					cr |= 0x02;							// Set EQ
				}
				else  				// Boundedly undefined; may store
					;				// or not; I elect not to.
				ppcsim->reserve_valid = 0;
			}
			else 										// No rsrv.
				;
			CR_put( 0, cr );
			break;

		case 151:										// stwx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			ac = GR(D);
			if (MSS_write( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			break;

		case 167:										// stvehx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			i  =  EA & 0x07;
			EA &= ~0x1;
			VPR_get( ppcsim, D, &vD );
			ac = vD.h[i];
			if (MSS_write( ppcsim, EA, &ac, 2 ))
				return( ppcsim->exc_type );
			break;

		case 183:										// stwux
			EA = GR(A) + GR(B);
			ac = GR(D);
			if (MSS_write( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			GR(A) = EA;
			break;

		case 199:										// stvewx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			i  =  EA & 0x03;
			EA &= ~0x3;
			VPR_get( ppcsim, D, &vA );
			ac = vA.w[i];
			if (MSS_write( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			break;

		case 200:										// subfze[.]
		case 712:										// subfzeo[.]
			l64 = (ULONG64) ~GR(A);
			l64 = l64 + XER_CA;
			if (ppcsim->verbose > 1)
				printf("SUBFZE: l64 current %16llx\n", l64);
			CA = (l64 & 0xFFFFFFFF00000000LL) ? 1 : 0;
			ac = ((GR(A) & 0x80000000) ^ 0x80000000)
			   + XER_CA;
			OV = (ac == (l64 & 0x80000000)) ? 0 : 1;
			OV = (OV != CA) ? 1 : 0;
			GR(D) = ac = (ULONG) l64 & 0xFFFFFFFF;
			setCA = 1;
			break;

		case 202:										// addze[.]
		case 714:										// addzeo[.]
			l64 = ((ULONG64) GR(A)) + XER_CA;
			if (ppcsim->verbose)
				printf("ADDC: l64 result %16llx\n", l64);
			CA = (l64 & 0xFFFFFFFF00000000LL) ? 1 : 0;
			if (ppcsim->verbose)
				printf("ADDC: CA %d\n", CA);
			setCA = 1;

			ac = (GR(A) & 0x80000000) + (GR(B) & 0x80000000);
			if (ppcsim->verbose)
				printf("ADDC: co_msb op %08x\n", ac);
			OV = (ac == (l64 & 0x80000000)) ? 0 : 1;
			if (ppcsim->verbose)
				printf("ADDC: co_msb = %d\n", OV);

			OV = (OV != CA) ? 1 : 0;
			if (ppcsim->verbose)
				printf("ADDC: OV %d\n", OV);

			GR(D) = ac = (ULONG) l64 & 0xFFFFFFFF;
			break;

		case 210:										// mtsr
			ac = GR(D);
			SPR_set( SPR_SR0+A, ac );
			break;

		case 215:										// stbx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			ac = GR(D);
			if (MSS_write( ppcsim, EA, &ac, 1 ))
				return( ppcsim->exc_type );
			break;

		case 231:										// stvx
		case 487:										// stvxl
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			EA &= 0xFFFFFFF0;
			VPR_get( ppcsim, D, &vA );
			if (MSS_write( ppcsim, EA, &vA.w[0], 8 ))
				return( ppcsim->exc_type );
			EA += 0x08;
			if (MSS_write( ppcsim, EA, &vA.w[2], 8 ))
				return( ppcsim->exc_type );
			break;

		case 232:										// subfme[.]
		case 744:										// subfmeo[.]
			l64 = (ULONG64) ~GR(A);
			l64 = l64 + XER_CA - 1;
			CA = (l64 & 0xFFFFFFFF00000000LL) ? 1 : 0;
			ac = (GR(A) & 0x80000000) + XER_CA - (0x80000000);
			OV = (ac == (l64 & 0x80000000)) ? 0 : 1;
			OV = (OV != CA) ? 1 : 0;
			GR(D) = ac = (ULONG) l64 & 0xFFFFFFFF;
			setCA = 1;
			break;

		case 234:										// addme[.]
		case 746:										// addmeo[.]
			l64 = ((ULONG64) GR(A)) + XER_CA + 0xFFFFFFFF;
			if (ppcsim->verbose)
				printf("ADDME: l64 result %16llx\n", l64);
			CA = (l64 & 0xFFFFFFFF00000000) ? 1 : 0;
			if (ppcsim->verbose)
				printf("ADDME: CA %d\n", CA);
			setCA = 1;

			ac = (GR(A) & 0x80000000) + (GR(B) & 0x80000000);
			if (ppcsim->verbose)
				printf("ADDME: co_msb op %08x\n", ac);
			OV = (ac == (l64 & 0x80000000)) ? 0 : 1;
			if (ppcsim->verbose)
				printf("ADDME: co_msb = %d\n", OV);

			OV = (OV != CA) ? 1 : 0;
			if (ppcsim->verbose)
				printf("ADDME: OV %d\n", OV);

			GR(D) = ac = (ULONG) l64 & 0xFFFFFFFF;
			break;

		case 235:										// mullw[.]
		case 747:										// mullwo[.]
			l64 = ((ULONG64) GR(A)) * ((ULONG64) GR(B));
			if (ppcsim->verbose > 1)
				printf("MULLW: l64 = %16llx\n", l64);
			if (l64 & 0xFFFFFFFF00000000LL)
				OE = OV = 1;
			GR(D) = ac = (ULONG) (l64 & (ULONG64) 0xffffffff);
			break;

		case 242:										// mtsrin
			EA = GR( B & 0xF );
			ac = GR(D);
			SPR_set( SPR_SR0+EA, ac );
			break;

		case 247:										// stbux
			EA = GR(A) + GR(B);
			ac = GR(D);
			if (MSS_write( ppcsim, EA, &ac, 1 ))
				return( ppcsim->exc_type );
			GR(A) = EA;
			break;

		case 266:										// add[.]
		case 778:										// addo[.]
			l64 = ((ULONG64) GR(A)) + ((ULONG64) GR(B));
			if (ppcsim->verbose)
				printf("ADD: l64 result %16llx\n", l64);
			CA = (l64 & 0xFFFFFFFF00000000LL) ? 1 : 0;
			if (ppcsim->verbose)
				printf("ADD: CA %d (fyi, never recorded)\n", CA);

			ac = (GR(A) & 0x80000000) + (GR(B) & 0x80000000);
			if (ppcsim->verbose)
				printf("ADD: co_msb op %08x\n", ac);
			OV = (ac == (l64 & 0x80000000)) ? 0 : 1;
			if (ppcsim->verbose)
				printf("ADD: co_msb = %d\n", OV);

			OV = (OV != CA) ? 1 : 0;
			if (ppcsim->verbose)
				printf("ADD: OV %d\n", OV);

			GR(D) = ac = (ULONG) l64 & 0xFFFFFFFF;

			setCA = 0;
			break;

		case 279:										// lhzx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			if (MSS_read( ppcsim, EA, (ULONG *) &uh, 2 ))
				return( ppcsim->exc_type );
			GR(D) = (ULONG) uh;
			break;

		case 284:										// eqv[.]
			GR(A) = ac = ~( GR(D) ^ GR(B) );
			break;

		case 311:										// lhzux
			EA = GR(A) + GR(B);
			if (MSS_read( ppcsim, EA, (ULONG *) &uh, 2 ))
				return( ppcsim->exc_type );
			GR(D) = (ULONG) uh;
			GR(A) = EA;
			break;

		case 316:										// xor[.]
			GR(A) = ac = GR(D) ^ GR(B);
			break;

		case 339:										// mfspr
			EA = (B << 5) | A;
			SIMM = SPR_get( EA );
			GR(D) = SIMM;
			break;

		case 343:										// lhax
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			if (MSS_read( ppcsim, EA, (ULONG *) &uh, 2 ))
				return( ppcsim->exc_type );
			GR(D) = (ULONG) ac;
			if (GR(D) & 0x8000)
				GR(D) |= 0xFFFF0000;
			break;

		case 371:										// mftb
			EA = (B << 5) | A;
			if (EA == 268)
				EA = SPR_TBL;
			else if (EA == 269)
				EA = SPR_TBU;
			else
				EA = 0;							// boundedly undefined
			ac = SPR_get( EA );
			GR(D) = ac;
			break;

		case 375:										// lhaux
			EA = GR(A) + GR(B);
			if (MSS_read( ppcsim, EA, (ULONG *) &uh, 2 ))
				return( ppcsim->exc_type );
			GR(D) = (ULONG) ac;
			if (GR(D) & 0x8000)
				GR(D) |= 0xFFFF0000;
			GR(A) = EA;
			break;

		case 407:										// sthx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			uh = GR(D);
			if (MSS_write( ppcsim, EA, (ULONG *) &uh, 2 ))
				return( ppcsim->exc_type );
			break;

		case 412:										// orc[.]
			GR(A) = ac = GR(D) | ~GR(B);
			break;

		case 439:										// sthux
			EA = GR(A) + GR(B);
			uh = GR(D);
			if (MSS_write( ppcsim, EA, (ULONG *) &uh, 2 ))
				return( ppcsim->exc_type );
			GR(A) = EA;
			break;

		case 444:										// or[.]
			GR(A) = ac = GR(D) | GR(B);
			break;

		case 459:										// divwu[.]
		case 971:										// divwuo[.]
			if ((GR(B) == 0))
				OE = OV = 1;
			else {
				l64 = (LONG64) GR(A) / GR(B);
				GR(D) = ac = l64;
			}
			break;

		case 467:										// mtspr
			EA = (B << 5) | A;
			SPR_set( EA, GR(D) );
			break;

		case 476:										// nand[.]
			GR(A) = ac = ~(GR(D) & GR(B));
			break;

		case 491:										// divw[.]
		case 1003:										// divwo[.]
			if ((GR(B) == 0)
			||  ((GR(A) == 0x80000000)  &&  GR(B) == 0xFFFFFFFF))
				OE = OV = 1;
			else {
				sl64A = (LONG64) GR(A);
				sl64B = (LONG64) GR(B);
				GR(D) = ac = sl64A / sl64B;
			}
			break;

		case 512:										// mcrxr
			ac = SPR_get( SPR_XER );
			ac = (ac >> 28) & 0xF;
			D >>= 2;
			CR_put( D, ac );
			ac &= 0xF0000000;
			SPR_set( SPR_XER, ac );
			OE = 0;
			break;

		case 533:										// lswx
		case 597:										// lswi
			EA = ((A == 0) ? 0 : GR(A));
			if (SC == 597)
				MB = (B == 0) ? 32 : B;					// n, encoded in B
			else
				MB = SPR_get( SPR_XER ) & 0x7F;			// n, PEM error?

			// Load #MB bytes into GR(D) left-to-right.  PPC zero-fills on the right.
			cr = D;
			i = 0;
			ULONG X1 = 0;
			while (MB > 0) {
				if (MSS_read( ppcsim, EA, (ULONG *) &uc, 1 ))
					return( ppcsim->exc_type );

				switch (i) {
				case 0:	GR(cr) = (GR(cr) & 0x00000000) | (uc << 24);	break;
				case 1:	GR(cr) = (GR(cr) & 0xFF000000) | (uc << 16);	break;
				case 2:	GR(cr) = (GR(cr) & 0xFFFF0000) | (uc <<  8);	break;
				case 3:	GR(cr) = (GR(cr) & 0xFFFFFF00) | (uc      );	break;
				}
			
				EA++;
				MB--;
				if (++i >= 4) {
					i = 0;
					cr = (cr + 1) & 0x1F;
				}
			}
			OE = 0;
			break;

		case 534:										// lwbrx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			if (MSS_read( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			GR(D) = pSwap4Bytes( ac );
			OE = 0;
			break;

		case 535:										// lfsx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			if (MSS_read( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			stod( ac, &fD );
			FPR_set( D, &fD.f );
			break;	

		case 536:										// srw[.]
			AA = GR(B) & 0x1F;
			GR(A) = ac = GR(D) >> AA;
			OE = 0;
			break;

		case 567:										// lfsux
			EA = GR(A) + GR(B);
			if (MSS_read( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			stod( ac, &fD );
			FPR_set( D, &fD.f );
			GR(A) = EA;
			break;	

		case 595:										// mfsr
			ac = SPR_get( SPR_SR0+A );
			GR(D) = ac;
			OE = 0;
			break;

		case 599:										// lfdx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			if (MSS_read( ppcsim, EA, &fD.v[0], 8 ))
				return( ppcsim->exc_type );
			FPR_set( D, &fD.f );
			break;

		case 631:										// lfdux
			EA = GR(A) + GR(B);
			if (MSS_read( ppcsim, EA, &fD.v[0], 8 ))
				return( ppcsim->exc_type );
			FPR_set( D, &fD.f );
			GR(A) = EA;
			break;

		case 659:										// mfsrin
			EA = GR( B & 0xF );
			ac = SPR_get( SPR_SR0+EA );
			GR(D) = ac;
			OE = 0;
			break;

		case 661:										// stswx
		case 725:										// stswi
			EA = ((A == 0) ? 0 : GR(A));
			if (SC == 661)
				MB = SPR_get( SPR_XER ) & 0x7F;			// n, PEM error?
			else
				MB = (B == 0) ? 32 : B;					// n

			// Store #MB bytes from GR(D) left-to-right.  PPC zero-fills on the right.
			cr = D;
			i = 0;
			while (MB > 0) {

				switch (i) {
				case 0:	uc = (GR(cr) >> 24) & 0xFF;	break;
				case 1:	uc = (GR(cr) >> 16) & 0xFF;	break;
				case 2:	uc = (GR(cr) >>  8) & 0xFF;	break;
				case 3:	uc = (GR(cr)      ) & 0xFF;	break;
				}

				if (MSS_write( ppcsim, EA, (ULONG *) &uc, 1 ))
					return( ppcsim->exc_type );

				EA++;
				MB--;
				if (++i >= 4) {
					i = 0;
					cr = (cr + 1) & 0x1F;
				}
			}
			OE = 0;
			break;

		case 662:										// stwbrx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			ac = pSwap4Bytes( GR(D) );
			if (MSS_write( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			OE = 0;
			break;

		case 663:										// stfsx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			fD.f = FPR_get( D );
			ac = dtos( &fD );
			if (MSS_write( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			OE = 0;
			break;

		case 695:										// stfsux
			EA = GR(A) + GR(B);
			fD.f = FPR_get( D );
			ac = dtos( &fD );
			if (MSS_write( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			GR(A) = EA;
			OE = 0;
			break;

		case 727:										// stfdx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			fD.f = FPR_get( D );
			if (MSS_write( ppcsim, EA, &fD.v[0], 8 ))
				return( ppcsim->exc_type );
			OE = 0;
			break;

		case 759:										// stfdux
			EA = GR(A) + GR(B);
			fD.f = FPR_get( D );
			if (MSS_write( ppcsim, EA, &fD.v[0], 8 ))
				return( ppcsim->exc_type );
			GR(A) = EA;
			OE = 0;
			break;

		case 790:										// lhbrx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			if (MSS_read( ppcsim, EA, (ULONG *) &uh, 2 ))
				return( ppcsim->exc_type );
			ac = (ULONG) uh;
			GR(D) = ((ac & 0xFF) << 8) | ((ac >> 8) & 0xFF);
			OE = 0;
			break;

		case 792:										// sraw[x]
			setCA = 1;
			ac    = GR(D);
			B     = GR(B) & 0x3F;
			for (i = CA = 0; i < B; i++) {
				CA |= (ac & 0x01);
				if (ac & 0x80000000)
					ac = (ac >> 1) | 0x80000000;
				else
					ac = (ac >> 1);
			}
			if ((GR(D) & 0x80000000) == 0)				// CA ignored if not
				CA = 0;									// initially neg
			GR(A) = ac;
			break;

		case 824:										// srawi[x]
			setCA = 1;
			ac    = GR(D);
			for (i = CA = 0; i < B; i++) {
				CA |= (ac & 0x01);
				if (ac & 0x80000000)
					ac = (ac >> 1) | 0x80000000;
				else
					ac = (ac >> 1);
			}
			if ((GR(D) & 0x80000000) == 0)				// CA ignored if not
				CA = 0;									// initially neg
			GR(A) = ac;
			break;

		case 918:										// sthbrx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			ac = ((GR(D) & 0xFF) << 8) | ((GR(D) >> 8) & 0xFF);	
			if (MSS_write( ppcsim, EA, &ac, 2 ))
				return( ppcsim->exc_type );
			OE = 0;
			break;

		case 922:										// extsh[x]
			GR(A) = (ac = GR(D)) & 0xFFFF;
			if (GR(D) & 0x8000)
				GR(A) |= 0xFFFF0000;
			OE = 0;
			break;

		case 954:										// extsb[x]
			GR(A) = (ac = GR(D)) & 0xFF;
			if (GR(D) & 0x80)
				GR(A) |= 0xFFFFFF00;
			OE = 0;
			break;

		case 983:										// stfiwx
			EA = ((A == 0) ? 0 : GR(A)) + GR(B);
			fD.f = FPR_get( D );
			if (MSS_write( ppcsim, EA, &fD.v[1], 4 ))
				return( ppcsim->exc_type );
			OE = 0;
			break;

		case 54:										// dcbst
		case 86:										// dcbf
		case 246:										// dcbtst
		case 278:										// dcbt
		case 306:										// tlbia
		case 310:										// eciwx
		case 342:										// dst
		case 370:										// tlbie
		case 374:										// dstst
		case 438:										// ecowx
		case 470:										// dcbi
		case 566:										// tlbsync
		case 598:										// sync
		case 758:										// dcba
		case 822:										// dss
		case 854:										// eieio
		case 982:										// icbi
		case 1014:										// dcbz
			OE = 0;
			break;

		default:
			printf("31: no handler for %d\n", SC);
			return( 1 );
		}
	}

//-- CLASS 32 -----------------------------------------------------------

	else if (inst == 32) {								// lwz
		EA = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
		if (MSS_read( ppcsim, EA, &ac, 4 ))
			return( ppcsim->exc_type );
		GR(D) = ac;
	}

//-- CLASS 33 -----------------------------------------------------------

	else if (inst == 33) {								// lwzu
		EA = GR(A) + EXTS( SIMM );
		if (MSS_read( ppcsim, EA, &ac, 4 ))
			return( ppcsim->exc_type );
		GR(D) = ac;
		GR(A) = EA;
	}

//-- CLASS 34 -----------------------------------------------------------

	else if (inst == 34) {								// lbz
		EA = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
		if (MSS_read( ppcsim, EA, (ULONG *) &uc, 1 )) {
			return( ppcsim->exc_type );
		}
		GR(D) = (ULONG) uc;
	}

//-- CLASS 35 -----------------------------------------------------------

	else if (inst == 35) {								// lbzu
		EA = GR(A) + EXTS( SIMM );
		if (MSS_read( ppcsim, EA, (ULONG *) &uc, 1 ))
			return( ppcsim->exc_type );
		GR(D) = (ULONG) uc;
		GR(A) = EA;
	}

//-- CLASS 36 -----------------------------------------------------------

	else if (inst == 36) {								// stw
		EA = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
		ac = GR(D);
		if (MSS_write( ppcsim, EA, &ac, 4 ))
			return( ppcsim->exc_type );
	}

//-- CLASS 37 -----------------------------------------------------------

	else if (inst == 37) {								// stwu
		EA = GR(A) + EXTS( SIMM );
		ac = GR(D);
		if (MSS_write( ppcsim, EA, &ac, 4 ))
			return( ppcsim->exc_type );
		GR(A) = EA;
	}

//-- CLASS 38 -----------------------------------------------------------

	else if (inst == 38) {								// stb
		EA = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
		uc = GR(D);
		if (MSS_write( ppcsim, EA, (ULONG *) &uc, 1 ))
			return( ppcsim->exc_type );
	}

//-- CLASS 39 -----------------------------------------------------------

	else if (inst == 39) {								// stbu
		EA = GR(A) + EXTS( SIMM );
		uc = GR(D);
		if (MSS_write( ppcsim, EA, (ULONG *) &uc, 1 ))
			return( ppcsim->exc_type );
		GR(A) = EA;
	}

//-- CLASS 40 -----------------------------------------------------------

	else if (inst == 40) {								// lhz
		EA = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
		if (MSS_read( ppcsim, EA, (ULONG *) &uh, 2 ))
			return( ppcsim->exc_type );
		GR(D) = (ULONG) uh;
	}

//-- CLASS 41 -----------------------------------------------------------

	else if (inst == 41) {								// lhzu
		EA = GR(A) + EXTS( SIMM );
		if (MSS_read( ppcsim, EA, (ULONG *) &uh, 2 ))
			return( ppcsim->exc_type );
		GR(D) = (ULONG) uh;
		GR(A) = EA;
	}

//-- CLASS 42 -----------------------------------------------------------

	else if (inst == 42) {								// lha
		EA = ((A == 0) ? 0 : GR(A));
		EA = GR(A) + EXTS( SIMM );
		if (MSS_read( ppcsim, EA, (ULONG *) &uh, 2 ))
			return( ppcsim->exc_type );
		ac = (ULONG) uh;
		ac = EXTS( ac );
		GR(D) = ac;
	}

//-- CLASS 43 -----------------------------------------------------------

	else if (inst == 43) {								// lhau
		EA = GR(A) + EXTS( SIMM );
		if (MSS_read( ppcsim, EA, (ULONG *) &uh, 2 ))
			return( ppcsim->exc_type );
		ac = (ULONG) uh;
		ac = EXTS( ac );
		GR(D) = ac;
		GR(A) = EA;
	}

//-- CLASS 44 -----------------------------------------------------------

	else if (inst == 44) {								// sth
		EA = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
		uh = GR(D);
		if (MSS_write( ppcsim, EA, (ULONG *) &uh, 2 ))
			return( ppcsim->exc_type );
	}

//-- CLASS 45 -----------------------------------------------------------

	else if (inst == 45) {								// sthu
		EA = GR(A) + EXTS( SIMM );
		uh = GR(D);
		if (MSS_write( ppcsim, EA, (ULONG *) &uh, 2 ))
			return( ppcsim->exc_type );
		GR(A) = EA;
	}

//-- CLASS 46 -----------------------------------------------------------

	else if (inst == 46) {								// lmw
		EA = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
		for (i = D; i <= 31; i++) {
			if (MSS_read( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			GR(i) = ac;
			EA += 4;
		}
	}

//-- CLASS 47 -----------------------------------------------------------

	else if (inst == 47) {								// stmw
		EA = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
		for (i = D; i <= 31; i++) {
			ac = GR(i);
			if (MSS_write( ppcsim, EA, &ac, 4 ))
				return( ppcsim->exc_type );
			EA += 4;
		}
	}

//-- CLASS 48 -----------------------------------------------------------

	else if (inst == 48) {								// lfs
		EA = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
		if (MSS_read( ppcsim, EA, &ac, 4 ))
			return( ppcsim->exc_type );
		stod( ac, &fD );
		FPR_set( D, &fD.f );
	}

//-- CLASS 49 -----------------------------------------------------------

	else if (inst == 49) {								// lfsu
		EA = GR(A) + EXTS( SIMM );
		if (MSS_read( ppcsim, EA, &ac, 4 ))
			return( ppcsim->exc_type );
		stod( ac, &fD );
		FPR_set( D, &fD.f );
		GR(A) = EA;
	}

//-- CLASS 50 -----------------------------------------------------------

	else if (inst == 50) {								// lfd
		EA = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
		if (MSS_read( ppcsim, EA, &fD.v[0], 8 ))
			return( ppcsim->exc_type );
		FPR_set( D, &fD.f );
	}

//-- CLASS 51 -----------------------------------------------------------

	else if (inst == 51) {								// lfdu
		EA = GR(A) + EXTS( SIMM );
		if (MSS_read( ppcsim, EA, &fD.v[0], 8 ))
			return( ppcsim->exc_type );
		FPR_set( D, &fD.f );
		GR(A) = EA;
	}

//-- CLASS 52 -----------------------------------------------------------

	else if (inst == 52) {								// stfs
		EA = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
		fD.f = FPR_get( D );
		ac = dtos( &fD );
		if (MSS_write( ppcsim, EA, &ac, 4 ))
			return( ppcsim->exc_type );
	}

//-- CLASS 53 -----------------------------------------------------------

	else if (inst == 53) {								// stfsu
		EA = GR(A) + EXTS( SIMM );
		fD.f = FPR_get( D );
		ac = dtos( &fD );
		if (MSS_write( ppcsim, EA, &ac, 4 ))
			return( ppcsim->exc_type );
		GR(A) = EA;
	}

//-- CLASS 54 -----------------------------------------------------------

	else if (inst == 54) {								// stfd
		EA = ((A == 0) ? 0 : GR(A)) + EXTS( SIMM );
		fD.f = FPR_get( D );
		if (MSS_write( ppcsim, EA, &fD.v[0], 8 ))
			return( ppcsim->exc_type );
	}

//-- CLASS 55 -----------------------------------------------------------

	else if (inst == 55) {								// stfdu
		EA = GR(A) + EXTS( SIMM );
		fD.f = FPR_get( D );
		if (MSS_write( ppcsim, EA, &fD.v[0], 8 ))
			return( ppcsim->exc_type );
		GR(A) = EA;
	}

//-- CLASS 63 -----------------------------------------------------------

	else if (inst == 63) {
		if (((SC & 0x3F) == 25)  						// again! ehh.
		||  ((SC & 0x3F) == 28)
		||  ((SC & 0x3F) == 29)
		||  ((SC & 0x3F) == 30)
		||  ((SC & 0x3F) == 31)) {
			i = (inst >> 6) & 0x1F;
			fC.f = FPR_get( i );
			SC = SC & 0x3F;
		}

// Handle the opcode.  Set D to -1 if result handling is not desired.

		switch (SC) {
		case 18:										// fdiv[.]
			FPXCHECK;
			fA.f = FPR_get( A );
			fB.f = FPR_get( B );
			fD.f = fA.f / fB.f;
			FRC = RC;
			FSRC= 1;
			break;

		case 20:										// fsub[.]
			FPXCHECK;
			fA.f = FPR_get( A );
			fB.f = FPR_get( B );
			fD.f = fA.f - fB.f;
			FRC = RC;
			FSRC= 1;
			break;

		case 21:										// fadd[.]
			FPXCHECK;
			fA.f = FPR_get( A );
			fB.f = FPR_get( B );
			fD.f = fA.f + fB.f;
			FRC = RC;
			FSRC= 1;
			break;

		case 25:										// fmul[.]
			FPXCHECK;
			fA.f = FPR_get( A );
			fD.f = fA.f * fC.f;
			FRC = RC;
			FSRC= 1;
			break;

		case 28:										// fmsub[.]
			FPXCHECK;
			fA.f = FPR_get( A );
			fB.f = FPR_get( B );
			fD.f = (fA.f * fC.f) - fB.f;
			FRC = RC;
			FSRC= 1;
			break;

		case 29:										// fmadd[.]
			FPXCHECK;
			fA.f = FPR_get( A );
			fB.f = FPR_get( B );
			fD.f = (fA.f * fC.f) + fB.f;
			FRC = RC;
			FSRC= 1;
			break;

		case 30:										// fnmsub[.]
			FPXCHECK;
			fA.f = FPR_get( A );
			fB.f = FPR_get( B );
			fD.f = - ((fA.f * fC.f) - fB.f);
			FRC = RC;
			FSRC= 1;
			break;

		case 31:										// fnmadd[.]
			FPXCHECK;
			fA.f = FPR_get( A );
			fB.f = FPR_get( B );
			fD.f = - ((fA.f * fC.f) + fB.f);
			FRC = RC;
			FSRC= 1;
			break;

		case 38:										// mtfsb1[.] 
			ac = SPR_get( SPR_FPSCR );
			ac |= (1 << (31-D));
			SPR_set( SPR_FPSCR, ac );
			RC1 = RC;
			D = -1;
			break;

		case 40:										// fneg[.]
			fB.f = FPR_get( B );
			fD.f = fB.f;
			fD.v[0] ^= 0x80000000;
			FRC = RC;
			break;

		case 64:										// mcrfs
			D >>= 2;
			A >>= 2;
			ac = SPR_get( SPR_FPSCR );
			LI = (ac >> (4*(7-A))) & 0x0F;
			CR_put( D, LI );
			ac &= (~(0xF << (4*(7-A)))) | 0xc0000000;	// Reset except FEX
			SPR_set( SPR_FPSCR, ac );					// and VX
			D = -1;
			break;

		case 70:										// mtfsb0[.] 
			ac = SPR_get( SPR_FPSCR );
			ac &= ~ (1 << (31-D));
			SPR_set( SPR_FPSCR, ac );
			RC1 = RC;
			D = -1;
			break;

		case 72:										// fmr[.]
			fesetround( FE_TONEAREST );
			fD.f = FPR_get( B );
			FRC = RC;
			D = -1;
			break;

		case 134:										// mtfsfi[.] 
			D >>= 2;
			B >>= 1;
			ac = SPR_get( SPR_FPSCR );
			LI = 0xF << (4*(7-D));
			B  =   B << (4*(7-D));
			ac = (ac & ~LI) | B;
			SPR_set( SPR_FPSCR, ac );
			OE = 0;
			FRC = RC + 1;								// lock in FX+OX
			D = -1;
			break;

		case 136:										// fnabs[.]
			FPXCHECK;
			fB.f = FPR_get( B );
			fD.f = fB.f;
			fD.v[0] |= 0x80000000;
			FRC = RC;
			break;

		case 264:										// fabs[.]
			FPXCHECK;
			fB.f = FPR_get( B );
			fD.f = fB.f;
			fD.v[0] &= 0x7FFFFFFF;
			FRC = RC;
			break;

		case 583:										// mffs[.]
			ac = SPR_get( SPR_FPSCR );
			fD.v[0] = 0;
			fD.v[1] = ac;
			FPR_set( D, &fD.f );
			RC1 = RC;
			D = -1;
			break;

		case 711:										// mtfsf[.] 
			A  = (opcode >> 17) & 0x0FF;
			fB.f = FPR_get( B );
			EA = fB.v[1];
			ac = SPR_get( SPR_FPSCR );
			LI = 0xF0000000;
			FRC = RC;
			for (i = 0x80; i; i >>= 1) {
				if (A & i) {
					ac &= ~LI;						// discard current
					ac |=  EA & LI;
				}
				if (i == 0x80)
					FRC = RC + 1;					// lock in FX+OX
				LI >>= 4;
			}
			SPR_set( SPR_FPSCR, ac );
			OE = 0;
			D = -1;
			break;

		default:
			printf("63: no handler for %d\n", SC);
			return( 1 );
		}

		if (D >= 0) {
			FPR_set( D, &fD.f );
			printf("round fD(%d) using RN-- TBD\n", D);
		}
	}

	else
		return( 1 );


//===========================================================================
//
// Update the CR, if needed.  'ac' contains the result to test.

	if (FRC) {
		if (FRC > 1)
			printf("lock FX,OX, update FEX, VX -- TBD\n");
		else
			printf("update FX, FEX, VX, OX -- TBD\n");
	}
	else if (RC || RC1) {
		cr  = (ac & 0x80000000) 				  ? 0x8 : 0;
		cr |= (ac == 0) 						  ? 0x2 : 0;
		cr |= (ac < 0x80000000  &&  !(cr & 0x02)) ? 0x4 : 0;
		cr |= (SPR_get( SPR_XER) & 0x80000000)	  ? 0x1 : 0;

		if (ppcsim->verbose > 1)
			printf("ppcsim: set CR%d using %08x => %x\n", RC1, ac, cr);
		CR_put( RC1, cr );
	}

	if (FSRC) {
		printf("update FPRF, FR, FI, FX, OX, UX, XX,VXSNAN, VXISI-- TBD\n");
	}

	if (VRC) {
		printf("update VRC/CR6 using ac %x\n", ac);
		CR_put( 6, ac );
	}

	if (SAT) {
		if (ppcsim->verbose > 1)
			printf("ppcsim: set VSCR[SAT]\n");
		ac = SPR_get( SPR_VSCR );
		ac |= 0x00000001;
		SPR_set( SPR_VSCR, ac );
	}
	if (OE) {
		if (ppcsim->verbose > 1)
			printf("ppcsim: set SO+OV = %d\n", OV);
		XER_setOV( OV );
	}
	if (setCA) {
		if (ppcsim->verbose > 1)
			printf("ppcsim: set CA = %d\n", CA);
		XER_setCA( CA );
	}

	return( 0 );
}


//--------------------------------------------------------------------------------
// ppcsim_bp_match -- compare <a> to breakpoints
// returns:		0 - no match
//				1 - software match, index is set.
//				2 - IABR match.
//
//--------------------------------------------------------------------------------
int ppcsim_bp_match( PPCSIM *ppcsim, ULONG a, int *index )
{
	int i;


// Check soft breakpoints.
//
	for (i = 0; i < MAX_BRKPTS; i++) {
		if (ppcsim->brkpt[i].valid  &&  (ppcsim->brkpt[i].addr == a)  &&  (ppcsim->brkpt[i].is_data == 0)) {
			*index = i;

			// If count is non-zero, decrement and pretend there was no match.
			//
			if (ppcsim->brkpt[i].count) {
				ppcsim->brkpt[i].count -= 1;
				return( 0 );
			}
			return( 1 );
		}
	}

	return( 0 );
}


//---------------------------------------------------------------------------
// ppcsim_ctl -- simulate one or more PowerPC instructions.
//
//	ncyc:	0=run/cont
//			n=# cycles to run
//---------------------------------------------------------------------------
int ppcsim_ctl( PPCSIM *ppcsim, int ncyc, int dasm )
{
	ULONG  opcode, msr, IABR, v;
	int	   i, stat, toex, x, take_exc;
	int	   status;


// Init shadow GPR registers.
//
	for (i = 0; i < MAX_GPR; i++)
		ppcsim->GPR_shadow[i] = GPR[i];


// If the current address is a breakpoint, we must skip over it to 
// start.
//
	if (ppcsim_bp_match( ppcsim, ppcsim->CIA, &i ) != 0) {
		ppcsim->brkpt_bypass = 1;
	}
	if (ncyc == 0) {				// run/continue
		toex = 1;
	} else {						// step
		toex = 0;
	}


// Calculate the decrementer limit.  When 'ppcsim_dec_tick' exceeds
// ppcsim_dec_limit, the decrementer is updated.

	v = SPR_get( SPR_HID1 );
	v = ((v >> 13) & 0x1F);
	if (v == 0)
		ppcsim->dec_limit = 5;


// Mirror BP#0 to IABR for hardware emulation.
//
	if (ppcsim->brkpt_hwmodel  &&  ppcsim->brkpt[0].addr)
		SPR_set( SPR_IABR, ppcsim->brkpt[0].addr | 0x02 );			//  b2 = ENABLE

	IABR = SPR_get( SPR_IABR ) & 0xFFFFFFFC;


// Start at CIA, and run until done. 
//
	while (toex  ||  (ncyc > 0)) {

// Check for breakpoints.
//
		take_exc = 0;
		if (!ppcsim->brkpt_bypass) {
			if ((stat = ppcsim_bp_match( ppcsim, ppcsim->CIA, &i )) == 1) {	// INS break
				printf("  ppcsim: breakpoint at %08X\n", ppcsim->CIA);
				ppcsim->exc_addr = ppcsim->CIA;
				ppcsim->exc_type = 0x1300;
				take_exc = 1;
			}
			else if (stat == 2) {										// DATA break
			}
			// else no breakpoint.
		}

		if (take_exc) {
			break;
		}


// Get the opcode to simulate.  If we can't get it, force an
// instruction access exception. 

		if (MSS_read( ppcsim, ppcsim->CIA, &opcode, 4 ) == -1) {
			ppcsim->exc_type = 0x400;
			ppcsim->exc_addr = ppcsim->CIA;
			return( status );
		}

		if (dasm  ||  take_exc) 
			disassemble_one( ppcsim, opcode, ppcsim->CIA, 1 );


// Simulate the opcode.  If non-zero returned, there was an error so
// stop by forcing a PROGRAM exception.
//
		ppcsim->NIA = ppcsim->CIA + 4;

		if (x = sim_powerpc( ppcsim, opcode )) {
			printf("  ppcsim: exception %04X @ %08X\n", x, ppcsim->CIA );
			ppcsim->exc_type = x;
			ppcsim->exc_addr = ppcsim->CIA;
			break;
		}


// Post-simulation activities.
//
		// Run user-defined command, if any.
		//
		shell_run_command( ppcsim, ppcsim->tr_cmd );

		// Report on changed registers.
		//
		if (ppcsim->show_reg_delta) {
			for (i = 0; i < MAX_GPR; i++) {
				if (ppcsim->GPR_shadow[i] != GPR[i]) {
					printf("  R%d: %08X -> %08X\n", i, ppcsim->GPR_shadow[i], GPR[i] );
				}
				ppcsim->GPR_shadow[i] = GPR[i];
			}
		}

		// Update the decrementer.
		//
		if (++ppcsim->dec_tick >= ppcsim->dec_limit) {
			v = SPR_get( SPR_DEC );
			SPR_set( SPR_DEC, --v );

			v = SPR_get( SPR_TBL );
			SPR_set( SPR_TBL, ++v );
			if (v == 0) {
				v = SPR_get( SPR_TBU );
				SPR_set( SPR_TBU, ++v );
			}

			ppcsim->dec_tick = 0;
		}

		// Check for an interrupt.
		//
		if (raw_isbreak()) {
			printf("  ppcsim:  break at %08X\n\n", ppcsim->CIA);fflush(stdout);

			if (MSS_read( ppcsim, ppcsim->CIA, &opcode, 4 ) == 0)
				disassemble( ppcsim, ppcsim->CIA, ppcsim->CIA );
			break;
		}


// Next, to continue update the PC and possibly keep going.

		ppcsim->CIA = ppcsim->NIA;
		ppcsim->issued++;
		if (ncyc)
			ncyc--;


// If brkpt_bypass was set, clear it now.  Only usable once.
//
		ppcsim->brkpt_bypass = 0;
	}

	return( 0 );
}


//---------------------------------------------------------------------------
// ppcsim_set_PC -- set simulation execution address.
//---------------------------------------------------------------------------
void ppcsim_set_PC( PPCSIM *ppcsim, ULONG a )
{

	ppcsim->CIA = a;
}


//---------------------------------------------------------------------------
// ppcsim_ss -- single-step 1..n cycles.
//---------------------------------------------------------------------------
int ppcsim_ss( PPCSIM *ppcsim, int n )
{
	int   stat;
	ULONG v;

	stat = ppcsim_ctl( ppcsim, n, 1 );

	return( stat );
}


//---------------------------------------------------------------------------
// ppcsim_run -- run continuously until stopped by breakpoint or interrupt.
//---------------------------------------------------------------------------
int ppcsim_run( PPCSIM *ppcsim, ULONG a, int set_a )
{
	int   stat;
	ULONG v;


	if (set_a)
		ppcsim->CIA = a;

	stat = ppcsim_ctl( ppcsim, 0, 0 );

	return( stat );
}
