// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup

/***************************************************************************
 *     Copyright Motorola, Inc. 1989-2001 ALL RIGHTS RESERVED
 *
 *  $Id: spr.c,v 1.1.1.1 2002/04/25 22:20:35 maurie Exp $
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


//--------------------------------------------------------------------------------
// SRR - Special Purpose Register
//		 globally accessible but also via functions.
//

ULONG	SRR[ MAX_SPR ];


ULONG SPR_get( int no )
{
	return( SRR[no] );
}

void SPR_set( int no, ULONG v )
{
	SRR[no] = v;
}


//---------------------------------------------------------------------------
// What follows are the bit-field definitions for each unique SPR register.
// Some registers are constant across machines, while others vary.

REG_FIELDS	sf_ASR[] = {
  {	 0, 20, "=PADDR",	"Physical Address of Segment Table"		},
  {	20, 12, NULL,		NULL              	 					}
};
REG_FIELDS	sf_BAMR[] = {
  {	 0, 32, "MASK",		"Breakpoint Address Mask"				}
};
REG_FIELDS	sf_BATL[] = {
  {	 0, 15, "BRPN",		"Block Real Page Number"				},
  {	15, 10, NULL,		NULL              	 					},
  {	25,  4, "V",		"Valid Bit"								},
  {	30,  6, "MASK",		"Block Size Mask"						}
};
REG_FIELDS	sf_BATU[] = {
  {	 0, 15, "=BEPI",	"Block Effective Page Index"			},
  {	15,  9, NULL,		NULL              	 					},
  {	24,  4, "WIMG",		"Storage Access Controls"				},
  {	28,  1, "VS",		"Valid for Supervisor State"			},
  {	29,  1, "VP",		"Valid for Problem(User) State"			},
  {	30,  2, "PP",		"Block Protection"						}
};
REG_FIELDS	sf_CR[] = {
  {	 0,  1, "LT",		"CR0: Negative"							},
  {	 1,  1, "GT",		"     Positive"							},
  {	 2,  1, "EQ",		"     Zero"								},
  {	 3,  1, "SO",		"     Summary Overflow"					},
  {	 4,  1, "FX",		"CR1: FP Exception"						},
  {	 5,  1, "FEX",		"     FP enabled exception"				},
  {	 6,  1, "VX",		"     FP invalid exception"				},
  {	 7,  1, "OX",		"     FP overflow exception"			},
  {	 8,  1, "LT",		"CR2: Negative"							},
  {	 9,  1, "GT",		"     Positive"							},
  {	10,  1, "EQ",		"     Zero"								},
  {	11,  1, "SO",		"     Summary Overflow/FP Unordered"	},
  {	12,  4, "CR3",		"CR3: Compare Result"					},
  {	16,  4, "CR4",		"CR3: Compare Result"					},
  {	20,  4, "CR5",		"CR4: Compare Result"					},
  {	24,  4, "CR6",		"CR6: Compare Result"					},
  {	28,  4, "CR7",		"CR7: Compare Result"					}
};
REG_FIELDS	sf_CTR[] = {
  {	 0, 32, "COUNT",	"Count Register"						}
};
REG_FIELDS	sf_DABR[] = {
  {	 0, 29, "=ADDR",	"Data Access Breakpoint"				},
  {	29,  1, "TEN",		"Translation Enabled"					},
  {	30,  1, "SEN",		"Store Enabled"							},
  {	31,  1, "LEN",		"Load Enabled"							}
};
REG_FIELDS	sf_DCMP[] = {
  {	 0,  1, "VALID",	"Valid"									},
  {	 1, 24, "VSID",		"Virtual Segment ID"					},
  {	25,  1, "HASH",		"Hash Indicator"						},
  {	26,  6, "API",		"Abbreviated Page Index"				}
};
REG_FIELDS	sf_DEC[] = {
  {	 0, 32, "DEC",		"Decrementer Value"						}
};
REG_FIELDS	sf_DSISR[] = {
  {	 0, 12, "DA_EXC",	"Data Access Exception"					},
  {	12, 20, "AA_EXC",	"Alignment Access Exception"			}
};
REG_FIELDS	sf_EA[] = {
  {	 0, 32, "ADDR",		"Effective Address"						}
};
REG_FIELDS	sf_EAR[] = {
  {	 0,  1, "EN",		"Enable"								},
  {	 1, 25, NULL,		NULL              	 					},
  {	26,  7, "RID",		"Resource ID"							}
};
REG_FIELDS	sf_FPSCR[] = {
  {	 0,  1, "FX",		"Exception"								},
  {	 1,  1, "FEX",		"Enabled exception"						},
  {	 2,  1, "VX",		"Invalid exception"						},
  {	 3,  1, "OX",		"Overflow exception"					},
  {	 4,  1, "UX",		"Underflow exception"					},
  {	 5,  1, "ZX",		"Zero Divide exception"					},
  {	 6,  1, "XX",		"Inexact exception"						},
  {	 7,  1, "VXSNAN",	"Invalid Op. Exception for SNaN"		},
  {	 8,  1, "VXISI",	"Invalid Op. Exception for inf-inf"		},
  {	 9,  1, "VXIDI",	"Invalid Op. Exception for inf+inf"		},
  {	10,  1, "VXZDZ",	"Invalid Op. Exception for 0/0"			},
  {	11,  1, "VXIMZ",	"Invalid Op. Exception for 0*inf"		},
  {	12,  1, "VXVC",		"Invalid Op. Exception for compare"		},
  {	13,  1, "FR",		"Fraction Rounded"						},
  {	14,  1, "FI",		"Fraction Inexact"						},
  {	15,  5, "FPRF",		"Result Flags"							},
  {	20,  1, NULL,		NULL									},
  {	21,  1, "VXSOFT",	"Invalid Op. Exception for software"	},
  {	22,  1, "VXSQRT",	"Invalid Op. Exception for sqrt"		},
  {	23,  1, "VXCVI",	"Invalid Op. Exception for int cvt"		},
  {	24,  1, "VE",		"Invalid Op. Exception Enable"			},
  {	25,  1, "OE",		"Overflow Exception Enable"				},
  {	26,  1, "UE",		"Underflow Exception Enable"			},
  {	27,  1, "ZE",		"Zero Divide Exception Enable"			},
  {	28,  1, "XE",		"Inexact Exception Enable"				},
  {	29,  1, "NI",		"Non-IEEE mode"							},
  {	30,  2, "RN",		"Rounding Control"						},
};
REG_FIELDS	sf_HASH[] = {
  {	 0,  7, "HTABORG",	"High 7-bits of Page Table Addr"		},
  {	 7, 16, "HPA",		"Hashed Page Address"					},
  {	23,  6, NULL,		NULL									}
};
REG_FIELDS	sf_HID0[] = {
  {	 0,  1, "EMCP",		"Enable MCP Check"						},
  {	 1,  1, NULL,		NULL									},
  {	 2,  1, "EBA",		"Enable Address Parity Check"			},
  {	 3,  1, "EBD",		"Enable Data Parity Check"				},
  {	 4,  1, "BCLK",		"Select Bus Clock"						},
  {	 5,  1, "EICE",		"Enable ICE Outputs"					},
  {	 6,  1, "ECLK",		"Enable External Test Clock"			},
  {	 7,  1, "PAR",		"Disable ARTRY/SHD Precharge"			},
  {	 8,  1, "DOZE",		"Doze Mode: PLL/TB/snoop active"		},
  {	 9,  1, "NAP",		"Nap Mode: PLL/TB active"				},
  {	10,  1, "SLEEP",	"Sleep Mode: no clock"					},
  {	11,  1, "DPM",		"Enable Dynamic Power Management"		},
  {	12,  1, "RISEG",	"Read ISEG"								},
  {	13,  2, NULL,		NULL									},
  {	15,  1, "NHR",		"Not Hard Reset"						},
  {	16,  1, "ICE",		"Instruction Cache Enable"				},
  {	17,  1, "DCE",		"Data Cache Enable"						},
  {	18,  1, "ILOCK",	"Instruction Cache Lock"				},
  {	19,  1, "DLOCK",	"Data Cache Lock"						},
  {	20,  1, "ICFI",		"Instruction Cache Flash Inval"			},
  {	21,  1, "DCFI",		"Data Cache Flash Inval"				},
  {	22,  1, NULL,		NULL									},
  {	23,  1, "CIFE",		"Coherent Instruction Fetch Enable"		},
  {	24,  1, "SIED",		"Serial Instruction Exec Enable"		},
  {	25,  2, NULL,		NULL									},
  {	27,  1, "FBIOB",	"Force Branch Indirect on bus"			},
  {	28,  1, "ABE",		"Address Broadcast Enable"				},
  {	29,  1, "BHT",		"Branch History Table Enable"			},
  {	30,  1, "BTACDIS",	"Branch Target Addr. Cache Disable"	    },
  { 31,  1, NULL,		NULL									}
};
REG_FIELDS	sf_HID0_604[] = {
  {	 0,  1, "EMCP",		"Enable MCP Check"						},
  {	 1,  1, "ECPAR",	"Enable Cache Parity Check"				},
  {	 2,  1, "EBA",		"Enable Address Parity Check"			},
  {	 3,  1, "EBD",		"Enable Data Parity Check"				},
  {	 4,  3, NULL,		NULL									},
  {	 7,  1, "PAR",		"Disable ARTRY/SHD Precharge"			},
  {	 8,  4, NULL,		NULL									},
  {	12,  1, "RISEG",	"Read ISEG"								},
  {	13,  2, NULL,		NULL									},
  {	15,  1, "NHR",		"Not Hard Reset"						},
  {	16,  1, "ICE",		"Instruction Cache Enable"				},
  {	17,  1, "DCE",		"Data Cache Enable"						},
  {	18,  1, "ILOCK",	"Instruction Cache Lock"				},
  {	19,  1, "DLOCK",	"Data Cache Lock"						},
  {	20,  1, "ICFI",		"Instruction Cache Flash Inval"			},
  {	21,  1, "DCFI",		"Data Cache Flash Inval"				},
  {	22,  1, NULL,		NULL									},
  {	23,  1, "CIFE",		"Coherent Instruction Fetch Enable"		},
  {	24,  1, "SIED",		"Serial Instruction Exec Enable"		},
  {	25,  4, NULL,		NULL									},
  {	29,  1, "BHT",		"Branch History Table Enable"			},
  {	30,  1, "BTACDIS",	"Branch Target Address Cache Disable"	},
  { 31,  1, NULL,		NULL									}
};
REG_FIELDS	sf_HID0_7xx[] = {
  {	 0,  1, "EMCP",		"Enable MCP Check"						},
  {	 1,  1, "DBP",		"Disable Addr/Data Generation"			},
  {	 2,  1, "EBA",		"Enable Address Parity Check"			},
  {	 3,  1, "EBD",		"Enable Data Parity Check"				},
  {	 4,  1, "BCLK",		"Select Bus Clock"						},
  {	 5,  1, NULL,		NULL									},
  {	 6,  1, "ECLK",		"Enable External Test Clock"			},
  {	 7,  1, "PAR",		"Disable ARTRY/SHD Precharge"			},
  {	 8,  1, "DOZE",		"Doze Mode: PLL/TB/snoop active"		},
  {	 9,  1, "NAP",		"Nap Mode: PLL/TB active"				},
  {	10,  1, "SLEEP",	"Sleep Mode: no clock"					},
  {	11,  1, "DPM",		"Enable Dynamic Power Management"		},
  {	12,  1, "RISEG",	"Read ISEG"								},
  {	13,  2, NULL,		NULL									},
  {	15,  1, "NHR",		"Not Hard Reset"						},
  {	16,  1, "ICE",		"Instruction Cache Enable"				},
  {	17,  1, "DCE",		"Data Cache Enable"						},
  {	18,  1, "ILOCK",	"Instruction Cache Lock"				},
  {	19,  1, "DLOCK",	"Data Cache Lock"						},
  {	20,  1, "ICFI",		"Instruction Cache Flash Inval"			},
  {	21,  1, "DCFI",		"Data Cache Flash Inval"				},
  {	22,  1, "SPD",		"Speculative Cache Disable"				},
  {	23,  1, "CIFE",		"Coherent Instruction Fetch Enable"		},
  {	24,  1, "SGE",		"Store Gathering Enabled"				},
  {	25,  1, "DCFA",		"Data Cache Flush Assist"				},
  {	26,  1, "BTIC",		"Branch Target Instr. Cache Enable"		},
  {	27,  1, NULL,		NULL									},
  {	28,  1, "ABE",		"Address Broadcast Enable"				},
  {	29,  1, "BHT",		"Branch History Table Enable"			},
  { 30,  1, NULL,		NULL									},
  {	31,  1, "NOOPTI",	"NOP all dcbt/dcbtst Instructions"		}
};
REG_FIELDS	sf_HID0_max[] = {
  {	 0,  1, "EMCP",		"Enable MCP Check"						},
  {  1,  1, NULL,		NULL									},
  {	 2,  1, "EBA",		"Enable Address Parity Check"			},
  {	 3,  1, "EBD",		"Enable Data Parity Check"				},
  {	 4,  1, "BCLK",		"Select Bus Clock"						},
  {	 5,  1, NULL,		NULL									},
  {	 6,  1, "ECLK",		"Enable External Test Clock"			},
  {	 7,  1, "PAR",		"Disable ARTRY/SHD Precharge"			},
  {	 8,  1, "DOZE",		"Doze Mode: PLL/TB/snoop active"		},
  {	 9,  1, "NAP",		"Nap Mode: PLL/TB active"				},
  {	10,  1, "SLEEP",	"Sleep Mode: no clock"					},
  {	11,  1, "DPM",		"Enable Dynamic Power Management"		},
  {	12,  1, "RISEG",	"Read ISEG"								},
  {	13,  1, "EIEC",		"Enable Internal Error Checking"		},
  {	14,  1, NULL,		NULL									},
  {	15,  1, "NHR",		"Not Hard Reset"						},
  {	16,  1, "ICE",		"Instruction Cache Enable"				},
  {	17,  1, "DCE",		"Data Cache Enable"						},
  {	18,  1, "ILOCK",	"Instruction Cache Lock"				},
  {	19,  1, "DLOCK",	"Data Cache Lock"						},
  {	20,  1, "ICFI",		"Instruction Cache Flash Inval"			},
  {	21,  1, "DCFI",		"Data Cache Flash Inval"				},
  {	22,  1, "SPD",		"Speculative Cache Disable"				},
  {	23,  1, "IFTT",		"IFetch TTx Encoding"					},
  {	24,  1, "SGE",		"Store Gathering Enabled"				},
  {	25,  1, "DCFA",		"Data Cache Flush Assist"				},
  {	26,  1, "BTIC",		"Branch Target Instr. Cache Enable"		},
  {	27,  2, NULL,		NULL									},
  {	29,  1, "BHT",		"Branch History Table Enable"			},
  { 30,  1, "NOPDST",	"NOP all dst/dstt/dstst/tststt"			},
  {	31,  1, "NOOPTI",	"NOP all dcbt/dcbtst Instructions"		}
};
REG_FIELDS	sf_HID0_vger[] = {
  {  0,  5, NULL,		NULL									},
  {	 5,  1, "TBEN",		"Time Base Enable"						},
  {  6,  1, NULL,		NULL									},
  {	 7,  1, "STEN",		"Software Tablewalk Enable"				},
  {  8,  1, NULL,		NULL									},
  {	 9,  1, "NAP",		"Nap Mode: PLL/TB active"				},
  {	10,  1, "SLEEP",	"Sleep Mode: no clock"					},
  {	11,  1, "DPM",		"Enable Dynamic Power Management"		},
  { 12,  1, NULL,		NULL									},
  {	13,  1, "BHTCLR",	"Branch History Table Clear"			},
  {	14,  1, "XAEN",		"Extended Addressing Enable"			},
  {	15,  1, "NHR",		"Not Hard Reset"						},
  {	16,  1, "ICE",		"Instruction Cache Enable"				},
  {	17,  1, "DCE",		"Data Cache Enable"						},
  {	18,  1, "ILOCK",	"Instruction Cache Lock"				},
  {	19,  1, "DLOCK",	"Data Cache Lock"						},
  {	20,  1, "ICFI",		"Instruction Cache Flash Inval"			},
  {	21,  1, "DCFI",		"Data Cache Flash Inval"				},
  {	22,  1, "SPD",		"Speculative Cache Disable"				},
  {	23,  1, NULL,		NULL									},
  {	24,  1, "SGE",		"Store Gathering Enabled"				},
  {	25,  1, NULL,		NULL									},
  {	26,  1, "BTIC",		"Branch Target Instr. Cache Enable"		},
  {	27,  1, "LRSTK",	"Link Stack Enable"						},
  {	28,  1, "FOLD",		"Branch Folding Enable"					},
  {	29,  1, "BHT",		"Branch History Table Enable"			},
  { 30,  1, "NOPDST",	"NOP all dst/dstt/dstst/tststt"			},
  {	31,  1, "NOOPTI",	"NOP all dcbt/dcbtst Instructions"		}
};
REG_FIELDS	sf_HID1[] = {
  {	 0,  4, "PLL",		"PLL Selection"							},
  {	 4, 28, NULL,		NULL									}
};
REG_FIELDS	sf_HID1_vger[] = {
  {	 0,  1, "EMCP",		"Enable MCP Check"						},
  {  1,  1, NULL,		NULL									},
  {	 2,  1, "EBA",		"Enable Address Parity Check"			},
  {	 3,  1, "EBD",		"Enable Data Parity Check"				},
  {	 4,  1, "BCLK",		"Select Bus Clock"						},
  {	 5,  1, NULL,		NULL									},
  {	 6,  1, "ECLK",		"Enable External Test Clock"			},
  {	 7,  1, "PAR",		"Disable ARTRY/SHD Precharge"			},
  {	 8,  7, NULL,		NULL									},
  {	15,  4, "PLL",		"PLL Selection"							},
  {	19,  1, "PLLEXT",	"PLL Extension"							},
  {	20,  1, "SYNCBE",	"Sync/EIEIO Broadcast Enable"			},
  {	21,  1, "ABE",		"Address Broadcast Enable"				},
  {	22, 10, NULL,		NULL									}
};
REG_FIELDS	sf_HID2[] = {
  {	 0, 11, NULL,		NULL									},
  {	11,  1, "L2AP_EN",	"L2 Addr. Parity Enable"				},
  {	12,  1, "SWT_EN",	"S/W Table Walk Enable"					},
  {	13,  1, "HIGHBAT",  "Enable I/DBAT[4:7]"					},
  {	14,  1, NULL,		NULL									},
  {	15,  1, "SFP",		"Speed For Power"						},
  {	16,  3, "IWLCK",	"ICache Way Lock"						},
  {	19,  5, NULL,		NULL									},
  {	24,  3, "DWLCK",	"DCache Way Lock"						},
  {	27,  5, NULL,		NULL									}
};
REG_FIELDS	sf_IABR[] = {
  {	 0, 30, "=ADDR",	"Instruction Access Breakpoint"			},
  {	30,  1, "BEN",		"Breakpoint Enabled"					},
  {	31,  1, "TEN",		"Translation Enabled"					}
};
REG_FIELDS	sf_ICTC[] = {
  {	 0, 23, NULL,		NULL									},
  {	23,  8, "IFI",		"Instruction Forwarding Interval"		},
  {	31,  1, "CTEN",		"Cache Throttle Enable"					}
};
REG_FIELDS	sf_L2CR[] = {
  {	 0,  1, "L2EN",		"L2 Cache Enable"						},
  {	 1,  1, "L2PEN",	"L2 Cache Parity Enable"				},
  {	 2,  2, "L2SIZ",	"L2 Cache Size"							},
  {	 4,  3, "L2CLK",	"L2 Clock Divisor"						},
  {	 7,  2, "L2RAM",	"L2 Ram Type"							},
  {	 9,  1, "L2DO",		"L2 Data-only"							},
  {	10,  1, "L2I",		"L2 Global Invalidate"					},
  {	11,  1, "L2CTL",	"L2 RAM Control"						},
  {	12,  1, "L2WT",		"L2 Write-through"						},
  {	13,  1, "L2TS",		"L2 Test Mode"							},
  {	14,  2, "L2OH",		"L2 Output Hold"						},
  {	16,  1, "L2SL",		"L2 DLL Slow"							},
  {	17,  1, "L2DF",		"L2 Differential Clock Enable"			},
  {	18,  1, "L2BYP",	"L2 DLL Bypass"							},
  {	19, 12, NULL,       NULL                       				},
  {	31,  1, "L2IIP",	"L2 Invalidate In-progress"				}
};
REG_FIELDS	sf_L2CR_max[] = {
  {	 0,  1, "L2EN",		"L2 Cache Enable"						},
  {	 1,  1, "L2PEN",	"L2 Cache Parity Enable"				},
  {	 2,  2, "L2SIZ",	"L2 Cache Size"							},
  {	 4,  3, "L2CLK",	"L2 Clock Divisor"						},
  {	 7,  2, "L2RAM",	"L2 Ram Type"							},
  {	 9,  1, "L2DO",		"L2 Data-only"							},
  {	10,  1, "L2I",		"L2 Global Invalidate"					},
  {	11,  1, "L2CTL",	"L2 RAM Control"						},
  {	12,  1, "L2WT",		"L2 Write-through"						},
  {	13,  1, "L2TS",		"L2 Test Mode"							},
  {	14,  2, "L2OH",		"L2 Output Hold"						},
  {	16,  1, "L2SL",		"L2 DLL Slow"							},
  {	17,  1, "L2DF",		"L2 Differential Clock Enable"			},
  {	18,  1, "L2BYP",	"L2 DLL Bypass"							},
  {	19,  1, "L2FA",		"L2 Flush Assist"						},
  {	20,  1, "L2HWF",	"L2 HW Flush"							},
  {	21,  1, "L2IO",		"L2 Instruction-only"					},
  {	22,  1, "L2CS",		"L2 Clock Stop"							},
  {	23,  1, "L2ROLL",	"L2 DLL Rollover Checkstop"				},
  {	24,  7, "L2CTR",	"L2 Counter Value"						},
  {	31,  1, "L2IIP",	"L2 Invalidate In-progress"				}
};
REG_FIELDS	sf_L2CR_vger[] = {
  {	 0,  1, "L2EN",		"L2 Cache Enable"						},
  {	 1,  1, "L2PEN",	"L2 Cache Parity Enable"				},
  {	 2,  2, "L2SIZ",	"L2 Cache Size"							},
  {	 4,  6, NULL,       NULL                 					},
  {	10,  1, "L2I",		"L2 Global Invalidate"					},
  {	11,  1, "L2IO",		"L2 Instruction-only"					},
  {	12,  3, NULL,       NULL                 					},
  {	15,  1, "L2DO",		"L2 Data-only"							},
  {	16,  3, NULL,       NULL                 					},
  {	19,  1, "L2REP",	"L2 Replacement Algorithm"				},
  {	20,  1, "L2HWF",	"L2 Hardware Flush"						},
  {	21, 10, NULL,		NULL									}
};
REG_FIELDS	sf_L2DM[] = {
  {	 0, 14, "=ADDR",	"Base Address"							},
  {	14, 16, NULL,       NULL          				 			},
  {	30,  2, "SIZE",		"Memory Address"						}
};
REG_FIELDS	sf_L3DM[] = {
  {	 0, 16, "=ADDR",	"Base Address"							},
  {	16, 16, NULL,       NULL          				 			}
};
REG_FIELDS	sf_L3CR[] = {
  {	 0,  1, "L3EN",		"L3 Cache Enable"						},
  {	 1,  1, "L3PEN",	"L3 Cache Parity Enable"				},
  {	 2,  1, NULL,       NULL          				 			},
  {	 3,  1, "L3SIZ",	"L3 Cache Size"							},
  {	 4,  1, "L3CLKEN",	"L3 Clock Enable"						},
  {	 5,  1, "L3DX",		"L3 64-bit Data Bus"					},
  {	 6,  3, "L3CLK",	"L3 Clock Ratio"						},
  {	 9,  1, "L3IONLY",	"L3 Instruction Only"					},
  {	10,  3, NULL,       NULL          				 			},
  {	13,  1, "L3SPO",	"L3 Sample Point Override"				},
  {	14,  2, "L3CKSP",	"L3 Clock Sample Point"					},
  {	16,  3, "L3PSP",	"L3 P-Clock Sample Point"				},
  {	19,  1, "L3REP",	"L3 Replacement Alg."					},
  {	20,  1, "L3HF",		"L3 HW Flush"							},
  {	21,  1, "L3I",		"L3 Global Invalidate"					},
  {	22,  2, "L3RT",		"L3 SRAM Type"							},
  {	24,  1, "L3CYA",	"L3 Clock Control Bit"					},
  {	25,  1, "L3DONLY",	"L3 Data Only"							},
  {	26,  3, NULL,       NULL          				 			},
  {	29,  1, "L3DMEN",	"L3 Direct Map Enable"					},
  {	30,  1, NULL,       NULL          				 			},
  {	31,  1, "L3DMSIZ",	"L3 Direct Map Size"					}
};
REG_FIELDS	sf_LR[] = {
  {	 0, 32, "ADDR",		"Branch Address"						}
};
REG_FIELDS	sf_MMCR0[] = {
  {	 0,  1, "DCNT",		"Disable Count Options"					},
  {	 1,  1, "DCS",		"Disable Count in Sup. Mode"			},
  {	 2,  1, "DCU",		"Disable Count in User Mode"			},
  {	 3,  1, "DCPMS",	"Disable Count if MSR[PM] set"			},
  {	 4,  1, "DCPMC",	"Disable Count if MSR[PM] clear"		},
  {	 5,  1, "EPMI",		"Enable PM Int."						},
  {	 6,  1, "DCI",		"Disable Count if Interrupt"			},
  {	 7,  2, "64",		"64bit Time Base"						},
  {	 9,  1, "BTIE",		"Bit Transition Int. Enable"			},
  {	11,  6, "THRESH",	"Threshold Value"						},
  {	17,  1, "P1IE",		"PMC1 Overflow Int. Enable"				},
  {	18,  1, "P2IE",		"PMC2 Overflow Int. Enable"				},
  {	19,  1, "P2CC",		"PMC2-4 Count Control Enable"			},
  {	20,  7, "P1S",		"PMC1 Select"							},
  {	27,  5, "P2S",		"PMC2 Select"							}
};
REG_FIELDS	sf_MMCR0_604[] = {
  {	 0,  5, "DCNT",		"Disable Count Options"					},
  {	 6,  1, "EIS",		"Enable Int. Signalling"				},
  {	 7,  1, "DCI",		"Disable Count on Int."					},
  {	 8,  2, "BSE",		"Bit Select Enable"						},
  {	10,  1, "BTIE",		"Bit Transition Int. Enable"			},
  {	11,  6, "THRESH",	"Threshold Value"						},
  {	17,  1, "P1IE",		"PMC1 Int. Enable"						},
  {	18,  1, "P2IE",		"PMC2 Int. Enable"						},
  {	19,  1, "P2CC",		"PMC2 Count Control Enable"				},
  {	20,  7, "P1S",		"PMC1 Select"							},
  {	27,  5, "P2S",		"PMC2 Select"							}
};
REG_FIELDS	sf_MQ[] = {
  {	 0, 32, "MQ",		"Product or Dividend"					}
};
REG_FIELDS	sf_MMCR1[] = {
  {	 0,  5, "PMC3",		"PMC3 Input Select"						},
  {	 5,  5, "PMC3",		"PMC3 Input Select"						},
  {	10, 22, NULL,		NULL              	 					}
};
REG_FIELDS	sf_MMCR2[] = {
  {	 0,  1, "THRMULT",	"Threshold Multiplier"					},
  {	 1,  1, "SMCNTEN",	"SM Count Enable"						},
  {	 2,  1, "SMINTEN",	"SM Int. Enable"						},
  {	 3, 29, NULL,		NULL              	 					}
};
REG_FIELDS	sf_MSR[] = {
  {	 0, 13, NULL,		NULL              	 					},
  {	13,  1, "POW",		"Power Management Enable"				},
  {	14,  1, "TLBGPR",	"TLB GPR Overlay Enable"				},
  {	15,  1, NULL,		NULL              	 					},
  {	16,  1, "EE",		"External Exception Enable"				},
  {	17,  1, "PR",		"Privilege Level"						},
  {	18,  1, "FP",		"Floating-point Available"				},
  {	19,  1, "ME",		"Machine Check Enable"					},
  {	20,  1, "FE0",		"FP Exception Mode 0"					},
  {	21,  1, "SE",		"Single-step Trace Enable"				},
  {	22,  1, "BE",		"Branch Trace Enable"					},
  {	23,  1, "FE1",		"FP Exception Mode 1"					},
  {	24,  1, NULL,		NULL              	 					},
  {	25,  1, "IP",		"Exception Prefix"						},
  {	26,  1, "IR",		"Instruction Address Translation"		},
  {	27,  1, "DR",		"Data Address Translation"				},
  {	28,  2, NULL,		NULL              	 					},
  {	30,  1, "RI",		"Recoverable Exception"					},
  {	31,  1, "LE",		"Little-endian Mode"					}
};
REG_FIELDS	sf_MSR_604[] = {
  {	 0, 13, NULL,		NULL              	 					},
  {	13,  1, "POW",		"Power Management Enable"				},
  {	14,  1, NULL,		NULL              	 					},
  {	15,  1, "ILE",		"Exception Little Endian"				},
  {	16,  1, "EE",		"External Exception Enable"				},
  {	17,  1, "PR",		"Privilege Level"						},
  {	18,  1, "FP",		"Floating-point Available"				},
  {	19,  1, "ME",		"Machine Check Enable"					},
  {	20,  1, "FE0",		"FP Exception Mode 0"					},
  {	21,  1, "SE",		"Single-step Trace Enable"				},
  {	22,  1, "BE",		"Branch Trace Enable"					},
  {	23,  1, "FE1",		"FP Exception Mode 1"					},
  {	24,  1, NULL,		NULL              	 					},
  {	25,  1, "IP",		"Exception Prefix"						},
  {	26,  1, "IR",		"Instruction Address Translation"		},
  {	27,  1, "DR",		"Data Address Translation"				},
  {	28,  2, NULL,		NULL              	 					},
  {	30,  1, "RI",		"Recoverable Exception"					},
  {	31,  1, "LE",		"Little-endian Mode"					}
};
REG_FIELDS	sf_MSR_max[] = {
  {	 0,  6, NULL,		NULL              	 					},
  {	 6,  1, "VEC",		"Altivec Available"						},
  {	 7,  6, NULL,		NULL              	 					},
  {	13,  1, "POW",		"Power Management Enable"				},
  {	14,  1, NULL,		NULL              	 					},
  {	15,  1, "ILE",		"Exception Little Endian"				},
  {	16,  1, "EE",		"External Exception Enable"				},
  {	17,  1, "PR",		"Privilege Level"						},
  {	18,  1, "FP",		"Floating-point Available"				},
  {	19,  1, "ME",		"Machine Check Enable"					},
  {	20,  1, "FE0",		"FP Exception Mode 0"					},
  {	21,  1, "SE",		"Single-step Trace Enable"				},
  {	22,  1, "BE",		"Branch Trace Enable"					},
  {	23,  1, "FE1",		"FP Exception Mode 1"					},
  {	24,  1, NULL,		NULL              	 					},
  {	25,  1, "IP",		"Exception Prefix"						},
  {	26,  1, "IR",		"Instruction Address Translation"		},
  {	27,  1, "DR",		"Data Address Translation"				},
  {	28,  1, NULL,		NULL              	 					},
  {	29,  1, "PM",		"Performance Monitor Marked Mode"		},
  {	30,  1, "RI",		"Recoverable Exception"					},
  {	31,  1, "LE",		"Little-endian Mode"					}
};
REG_FIELDS	sf_MSSCR0[] = {
  {	 0,  1, "SHDEN",	"Shared State Enable"					},
  {	 1,  1, "SHDPEN3",	"SHD0/SHD1 Enable for 3-state MEI"		},
  {	 2,  3, "L1INTVEN", "L1 DCache HIT Intervention Enable"		},
  {	 5,  3, "L2INTVEN", "L2 DCache HIT Intervention Enable"		},
  {	 8,  1, "DL1_HWF",	"L1 DCache Hardware Flush"				},
  {	 9,  1, "DBSIZE",	"System DataBus Size (1=64, 0=128)"		},
  {	10,  1, "EMODE",	"MPXBus Mode Enable"					},
  {	11,  1, "ABD",		"Address Bus Driven Mode Enable"		},
  {	12, 20, NULL,		NULL              	 					}
};
REG_FIELDS	sf_MSSCR0_vger[] = {
  {	 0,  2, NULL,		NULL              	 					},
  {	 3,  3, "DTQ",	    "Data Transaction Queue"				},
  {	 6,  1, "SSM",	    "Strict Sync Mode"						},
  {	 7,  1, "EIDIS",    "Disable Extern. Intervention"			},
  {	 8,  1, "SHB",	    "Snoop Harmonic Backoff"				},
  {	 9,  2, NULL,		NULL              	 					},
  {	11,  1, "ABGD",		"Address Bus Grant Drive"				},
  {	12,  1, "L3TCEN",   "L3 Turnaround Clock Enable"			},
  {	13,  2, "L3TC",     "L3 Turnaround Clocks"					},
  {	15,  1, NULL,		NULL              	 					},
  {	16,  2, "BMODE",	"60XBus (00)/MPXBus (10)"				},
  {	18,  6, NULL,		NULL              	 					},
  {	24,  1, "WOPD",		"Window of Opportunity Disable"    		},
  {	25,  1, NULL,		NULL              	 					},
  {	26,  1, "ID",		"Processor ID"                   		},
  {	27,  3, NULL,		NULL              	 					},
  {	30,  2, "L2PFE",	"L2 PreFetch Enable"					}
};
REG_FIELDS	sf_MSSCR1[] = {
  {	 0, 32, NULL,		NULL              	 					}
};
REG_FIELDS	sf_MSSSR0[] = {
  {	 0, 13, NULL,		NULL              	 					},
  {	13,  1, "L2TAG",	"L2TAG parity error"					},
  {	14,  1, "L2DAT",	"L2 data parity error"					},
  {	15,  1, "L3TAG",	"L3TAG parity error"					},
  {	16,  1, "L3DAT",	"L3 data parity error"					},
  {	17,  1, "APE",      "Address Bus Parity Error"				},
  {	18,  1, "DPE",      "Data Bus Parity Error"					},
  {	19,  1, "TEA",      "TEA* Detected"							},
  {	20, 12, NULL,		NULL              	 					}
};
REG_FIELDS	sf_PID[] = {
  {	 0, 32, "ID",		"ID"									},
};
REG_FIELDS	sf_PIR[] = {
  {	 0, 28, NULL,		NULL              	 					},
  {	28,  4, "ID",		"ID"									}
};
REG_FIELDS	sf_PMCX[] = {
  {	 0, 32, "CTR",		"Counter Value"							}
};
REG_FIELDS	sf_PMCX_new[] = {
  {	 0,  1, "OV",		"Overflow"								},
  {	 1, 31, "CTR",		"Counter Value"							}
};
REG_FIELDS	sf_PVR[] = {
  {	 0, 16, "VER",		"Version"								},
  {	16, 16, "REV",		"Revision"								}
};
REG_FIELDS	sf_PVR_vger[] = {
  {	 0, 16, "TYPE",		"Processor Type"						},
  {	16,  4, "TECH",		"Process Technology Variation"			},
  {	20,  4, "MAJOR",	"Major Revision Number"					},
  {	24,  8, "MINOR",	"Minor Revision Number"					}
};
REG_FIELDS	sf_RPA[] = {
  {	 0, 20, "RPN",		"Real Page Number"						},
  {	20,  3, NULL,		NULL									},
  {	23,  1, "REF",		"Referenced Bit"						},
  {	24,  1, "CHNG",		"Changed Bit"							},
  {	25,  4, "PA",		"Page Attributes"						},
  {	29,  1, NULL,		NULL									},
  {	30,  2, "PP",		"Page Protection"						}
};
REG_FIELDS	sf_RTCL[] = {
  {	 0,  2, NULL,		NULL									},
  {	 2, 24, "XSEC",		"Seconds since begin of current second" },
  {	25, 16, NULL,       NULL        							}
};
REG_FIELDS	sf_RTCU[] = {
  {	 0, 32, "SEC",		"Seconds since specified time"			}
};
REG_FIELDS	sf_SDR1[] = {
  {	 0, 16, "HTABORG",	"High 16-bits of Page Table Addr"		},
  {	16,  7, NULL,		NULL									},
  {	23,  9, "HTABMSK",	"Page Table Address Mask"				}
};
REG_FIELDS	sf_SPRG[] = {
  {	 0, 32, "VALUE",	"Register Contents"						}
};
REG_FIELDS	sf_SR[] = {
  {	 0,  1, "T",		"Type (=0 assumed)"						},
  {	 1,  1, "KS",		"Supervisor State Protection Key"		},
  {	 2,  1, "KP",		"User State Protection Key"				},
  {	 3,  1, "N",		"No-execute Protection Key"				},
  {	 4,  4, NULL,		NULL									},
  {	 8, 24, "VSID",		"Virtual Segment ID"					}
};
REG_FIELDS	sf_SRR0[] = {
  {	 0, 32, "ADDR",		"Exception Address"						}
};
REG_FIELDS	sf_SRR1[] = {
  {	 0, 16, "MISC",		"Exception-Specific"					},
  {	16, 16, "SMSR",		"Save MSR bits 16:31"					}
};
REG_FIELDS	sf_SRR1_max[] = {
  {	 0,  1, NULL,		NULL									},
  {	 1,  1, "IERR",		"ICache Error"							},
  {	 2,  1, "DERR",		"DCache Error"							},
  {	 3,  1, "L2TERR",	"L2 Tag Error"							},
  {	 4,  1, "TLBERR",	"TLB Error"								},
  {	 5,  1, "BTERR",	"BHT/BTIC Error"						},
  {	 6,  4, NULL,		NULL									},
  {	10,  1, "OTHER",	"Other Error"							},
  {	11,  1, "L2PERR",	"L2 Data Parity Error"					},
  {	12,  1, "MCPERR",	"MCP Error"								},
  {	13,  1, "TEAERR",	"TEA Error"								},
  {	14,  1, "DPE",		"Data Parity Error"						},
  {	15,  1, "APE",		"Address Parity Error"					},
  {	16, 16, "SMSR",		"Save MSR bits 16:31"					}
};
REG_FIELDS	sf_TBL[] = {
  {	 0, 32, "TBL",		"Lower Time Base"						}
};
REG_FIELDS	sf_TBU[] = {
  {	 0, 32, "TBU",		"Upper Time Base"						}
};
REG_FIELDS	sf_THRM1[] = {
  {	 0,  1, "TIN",		"Thermal Management Interrupt"			},
  {	 1,  1, "TIV",		"Thermal Management Interrupt Valid"	},
  {	 2,  7, "THRESH",	"Compare Threshold"						},
  {	 9, 20, NULL,		NULL              	 					},
  {	29,  1, "TID",		"Interrupt Direction"					},
  {	30,  1, "TIE",		"Thermal Management Interrupt Enable"	},
  {	31,  1, "V",		"SPR Valid"								}
};
REG_FIELDS	sf_THRM3[] = {
  {	 0, 18, NULL,		NULL              	 					},
  {	18, 13, "SITV",		"Sample Interval Timer"					},
  {	31,  1, "E",		"Thermal Sensor Enable"					}
};
REG_FIELDS	sf_VRSAVE[] = {
  {	 0, 32, "VRSAVE",	"Value"									}
};
REG_FIELDS	sf_VSCR[] = {
  {	 0, 15, NULL,		NULL              	 					},
  {	15,  1, "NJAVA",	"non-Java(tm) Mode"						},
  {	16, 15, NULL,		NULL              	 					},
  {	31,  1, "AVSAT",	"Altivec(tm) Saturation"				}
};
REG_FIELDS	sf_XBATL[] = {
  {	 0, 15, "BRPN",		"Block Real Page Number"				},
  {	15, 10, NULL,		NULL              	 					},
  {	25,  4, "WIMG",		"Storage Access Controls"				},
  {	29,  1, NULL,		NULL              	 					},
  {	30,  2, "PP",		"Block Protection"						}
};
REG_FIELDS	sf_XBATU[] = {
  {	 0, 15, "BEPI",		"Block Effective Page Index"			},
  {	15,  4, NULL,		NULL              	 					},
  {	19, 11, "BL",		"Block Length"							},
  {	30,  1, "VS",		"Valid for Supervisor State"			},
  {	31,  1, "VP",		"Valid for Problem(User) State"			}
};
REG_FIELDS	sf_XER[] = {
  {	 0,  1, "SO",		"Summary Overflow"						},
  {	 1,  1, "OV",		"Overflow"								},
  {	 2,  1, "CA",		"Carry"									},
  {	 3, 13, NULL,		NULL									},
  {	16,  8, "CB",		"Compare Inst. byte"					},
  {	24,  1, NULL,		NULL									},
  {	25,  7, "ECB",		"ECIO transfer byte count"				}
};



//---------------------------------------------------------------------------
// What follows are the definitions for each of the SPR register file.
// Each register is qualified by processor type, and points to a bit-field
// definition in the previous section.


// Terse equivalents of "CCLS_xxx"
//
#define CPU_X       CCLS_601        // ?          -- 601-class device
#define CPU_W       CCLS_603        // Wart       -- 603-class device
#define CPU_S       CCLS_604        // Sirocco    -- 604-class device
#define CPU_A       CCLS_750        // Arthur     -- 750-class device
#define CPU_M       CCLS_7400       // Max        -- 7400-class device
#define CPU_G       CCLS_755        // Goldfinger -- 755-class device
#define CPU_V       CCLS_7450       // Vger       -- 7450-class device
#define CPU_N       CCLS_7410       // NITRO      -- 7410-class device


#define FIELDS(x)	x, (sizeof(x)/sizeof(REG_FIELDS))


REGFIELD_TABLE spr_tab[] = {
  {    0, SPR_MQ,	"MQ",		(CPU_X										    ), FIELDS(sf_MQ),
	      "Multiplication/Quotient Register"						},
  {    1, SPR_XER,	"XER",		(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XER),
	      "FXU Exception Register"									},

  {    4, SPR_RTCU,	"RTCU",		(CPU_X											), FIELDS(sf_RTCU),
	      "Real-Time Clock Upper Register"							},
  {    5, SPR_RTCL,	"RTCL",		(CPU_X											), FIELDS(sf_RTCL),
	      "Real-Time Clock Lower Register"							},
  {    8, SPR_LR,	"LR",		(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_LR),
	     "Link Register"											},     
  {    9, SPR_CTR,	"CTR",		(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_CTR),
	     "Counter Register"											},    
  {   18, SPR_DSISR,"DSISR",	(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_DSISR),
	     "Data Storage Interrupt Status Register"					},  
  {   19, SPR_DAR,	"DAR",		(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_EA),
	     "Data Address Register"									},    
  {   22, SPR_DEC,	"DEC",		(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_DEC),
	     "Decrementer"									},

  {   25, SPR_SDR1,	"SDR1",		(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|	   CPU_V), FIELDS(sf_SDR1),
	     "Storage Description Register 1"							},  
  {   26, SPR_SRR0,	"SRR0",		(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SRR0),
	     "Status Save/Restore Register 0"							},  
  {   27, SPR_SRR1,	"SRR1",		(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G					), FIELDS(sf_SRR1),
	     "Status Save/Restore Register 1"							},  
  {   27, SPR_SRR1,	"SRR1",		(							   CPU_M|CPU_N|CPU_V), FIELDS(sf_SRR1_max),
	     "Status Save/Restore Register 1"							},  
  {  256, SPR_VRSAVE,"VRSAVE",	(							   CPU_M|CPU_N|CPU_V), FIELDS(sf_VRSAVE),
	     "VRsave Register"											}, 
  {  272, SPR_SPRG0,"SPRG0",	(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SPRG),
	     "General Purpose SPR 0"									},
  {  273, SPR_SPRG1,"SPRG1",	(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SPRG),
	     "General Purpose SPR 1"									},
  {  274, SPR_SPRG2,"SPRG2",	(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SPRG),
	     "General Purpose SPR 2"									},
  {  275, SPR_SPRG3,"SPRG3",	(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SPRG),
	     "General Purpose SPR 3"									},
  {  280, SPR_ASR,	"ASR",		(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_ASR),
	 "Address Space Register"									},
  {  282, SPR_EAR,	"EAR",		(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_EAR),
	     "External Access Register"									},
  {  284, SPR_TBL,	"TBL",		(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_TBL),
	     "Time Base Lower Register"									},
  {  285, SPR_TBU,	"TBU",		(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_TBU),
	     "Time Base Upper Register"									},
  {  287, SPR_PVR,	"PVR",		(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N      ), FIELDS(sf_PVR),
	     "Processor Version Register"								},
  {  287, SPR_PVR,	"PVR",		(										   CPU_V), FIELDS(sf_PVR_vger),
	     "Processor Version Register"								},

  {  528, SPR_IBAT0U,"IBAT0U",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATU),
	     "Instruction Block Addr. Translation Reg. 0 Upper"			},
  {  528, SPR_BAT0U,"BAT0U",	(CPU_X											), FIELDS(sf_BATU),
	     "Block Addr. Translation Reg. 0 Upper"						},
  {  529, SPR_IBAT0L,"IBAT0L",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATL),
	     "Instruction Block Addr. Translation Reg. 0 Lower"			},
  {  529, SPR_BAT0L,"BAT0L",	(CPU_X											), FIELDS(sf_BATL),
	     "Block Addr. Translation Reg. 0 Lower"						},

  {  530, SPR_IBAT1U,"IBAT1U",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATU),
	     "Instruction Block Addr. Translation Reg. 1 Upper"			},
  {  530, SPR_BAT1U,"BAT1U",	(CPU_X											), FIELDS(sf_BATU),
	     "Block Addr. Translation Reg. 1 Upper"						},
  {  531, SPR_IBAT1L,"IBAT1L",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATU),
	     "Instruction Block Addr. Translation Reg. 1 Lower"			},
  {  531, SPR_BAT1L,"BAT1L",	(CPU_X											), FIELDS(sf_BATU),
	     "Block Addr. Translation Reg. 1 Lower"						},

  {  532, SPR_IBAT2U,"IBAT2U",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATU),
	     "Instruction Block Addr. Translation Reg. 2 Upper"			},
  {  532, SPR_BAT2U,"BAT2U",	(CPU_X											), FIELDS(sf_BATU),
	     "Block Addr. Translation Reg. 2 Upper"						},
  {  533, SPR_IBAT2L,"IBAT2L",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATU),
	     "Instruction Block Addr. Translation Reg. 2 Lower"			},
  {  533, SPR_BAT2L,"BAT2L",	(CPU_X											), FIELDS(sf_BATU),
	     "Block Addr. Translation Reg. 2 Lower"						},

  {  534, SPR_IBAT3U,"IBAT3U",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATU),
	     "Instruction Block Addr. Translation Reg. 3 Upper"			},
  {  534, SPR_BAT3U,"BAT3U",	(CPU_X											), FIELDS(sf_BATU),
	     "Block Addr. Translation Reg. 3 Upper"						},
  {  535, SPR_IBAT3L,"IBAT3L",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATU),
	     "Instruction Block Addr. Translation Reg. 3 Lower"			},
  {  535, SPR_BAT3L,"BAT3L",	(CPU_X											), FIELDS(sf_BATU),
	     "Block Addr. Translation Reg. 3 Lower"						},

  {  536, SPR_DBAT0U,"DBAT0U",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATU),
	     "Data Block Addr. Translation Reg. 0 Upper"				},
  {  537, SPR_DBAT0L,"DBAT0L",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATL),
	     "Data Block Addr. Translation Reg. 0 Lower"				},
  {  538, SPR_DBAT1U,"DBAT1U",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATU),
	     "Data Block Addr. Translation Reg. 1 Upper"				},
  {  539, SPR_DBAT1L,"DBAT1L",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATL),
	     "Data Block Addr. Translation Reg. 1 Lower"				},
  {  540, SPR_DBAT2U,"DBAT2U",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATU),
	     "Data Block Addr. Translation Reg. 2 Upper"				},
  {  541, SPR_DBAT2L,"DBAT2L",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATL),
	     "Data Block Addr. Translation Reg. 2 Lower"				},
  {  542, SPR_DBAT3U,"DBAT3U",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATU),
	     "Data Block Addr. Translation Reg. 3 Upper"				},
  {  543, SPR_DBAT3L,"DBAT3L",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_XBATL),
	     "Data Block Addr. Translation Reg. 3 Lower"				},

  {  560, SPR_IBAT4U,"IBAT4U",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATU),
	     "Instruction Block Addr. Translation Reg. 4 Upper"			},
  {  561, SPR_IBAT4L,"IBAT4L",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATL),
	     "Instruction Block Addr. Translation Reg. 4 Lower"			},
  {  562, SPR_IBAT5U,"IBAT5U",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATU),
	     "Instruction Block Addr. Translation Reg. 5 Upper"			},
  {  563, SPR_IBAT5L,"IBAT5L",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATL),
	     "Instruction Block Addr. Translation Reg. 5 Lower"			},
  {  564, SPR_IBAT6U,"IBAT6U",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATU),
	     "Instruction Block Addr. Translation Reg. 6 Upper"			},
  {  565, SPR_IBAT6L,"IBAT6L",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATL),
	     "Instruction Block Addr. Translation Reg. 6 Lower"			},
  {  566, SPR_IBAT7U,"IBAT7U",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATU),
	     "Instruction Block Addr. Translation Reg. 7 Upper"			},
  {  567, SPR_IBAT7L,"IBAT7L",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATL),
	     "Instruction Block Addr. Translation Reg. 7 Lower"			},
  {  568, SPR_DBAT4U,"DBAT4U",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATU),
	     "Data Block Addr. Translation Reg. 4 Upper"			},
  {  569, SPR_DBAT4L,"DBAT4L",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATL),
	     "Data Block Addr. Translation Reg. 4 Lower"			},
  {  570, SPR_DBAT5U,"DBAT5U",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATU),
	     "Data Block Addr. Translation Reg. 5 Upper"			},
  {  571, SPR_DBAT5L,"DBAT5L",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATL),
	     "Data Block Addr. Translation Reg. 5 Lower"			},
  {  572, SPR_DBAT6U,"DBAT6U",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATU),
	     "Data Block Addr. Translation Reg. 6 Upper"			},
  {  573, SPR_DBAT6L,"DBAT6L",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATL),
	     "Data Block Addr. Translation Reg. 6 Lower"			},
  {  574, SPR_DBAT7U,"DBAT7U",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATU),
	     "Data Block Addr. Translation Reg. 7 Upper"			},
  {  575, SPR_DBAT7L,"DBAT7L",  (                        CPU_G|            CPU_V), FIELDS(sf_XBATL),
	     "Data Block Addr. Translation Reg. 7 Lower"			},

  {  928, SPR_UMMCR2,"UMMCR2",	(                        CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_MMCR2),
	     "Performance Monitor Control Register"						},
  {  935, SPR_UBAMR, "UBAMR",	(                        CPU_G|CPU_M|CPU_N|CPU_V ), FIELDS(sf_BAMR),
	     "User Breakout Address Mask Register"						},
  {  936, SPR_UMMCR0,"UMMCR0",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_MMCR0),
	     "User Monitor Control 0"									},
  {  937, SPR_UPMC1, "UPMC1",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_PMCX),
	     "User Performance Monitor 1"								},
  {  938, SPR_UPMC2, "UPMC2",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_PMCX),
	     "User Performance Monitor 2"								},
  {  939, SPR_USIA,  "USIA",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_EA),
	     "User Sampled Instruction Address"							},
  {  940, SPR_UMMCR1,"UMMCR1",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_MMCR1),
	     "User Monitor Control 1"									},
  {  941, SPR_UPMC3, "UPMC3",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_PMCX),
	     "Performance Monitor 3"									},
  {  942, SPR_UPMC4, "UPMC4",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_PMCX),
	     "Performance Monitor 4"									},
  {  944, SPR_MMCR2, "MMCR2",	(                              CPU_M|      CPU_V), FIELDS(sf_MMCR2),
	     "Performance Monitor Control Register 2"					},
  {  951, SPR_BAMR,  "BAMR",	(                        CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_BAMR),
	     "Breakout Address Mask Register"							},
  {  952, SPR_MMCR0, "MMCR0",	(            CPU_S								), FIELDS(sf_MMCR0_604),
	     "Monitor Mode Control Register 0"							}, 
  {  952, SPR_MMCR0, "MMCR0",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_MMCR0),
	     "Monitor Mode Control Register 0"							}, 
  {  953, SPR_PMC1,	 "PMC1",	(            CPU_S								), FIELDS(sf_PMCX),
	     "Performance Monitor Counter 1" 							},  
  {  953, SPR_PMC1,	 "PMC1",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_PMCX),
	     "Performance Monitor Counter 1" 							},  
  {  954, SPR_PMC2,  "PMC2",	(            CPU_S								), FIELDS(sf_PMCX),
	     "Performance Monitor Counter 2" 							},  
  {  954, SPR_PMC2,  "PMC2",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_PMCX),
	     "Performance Monitor Counter 2" 							},  
  {  955, SPR_SIA,   "SIA",		(            CPU_S|            CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_EA),
	     "Sampled Instruction Address"								},
  {  956, SPR_MMCR1, "MMCR1",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_MMCR1),
	     "Monitor Mode Control Register 1"							}, 
  {  957, SPR_PMC3,  "PMC3",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_PMCX),
	     "Performance Monitor Counter 3" 							},  
  {  958, SPR_PMC4,  "PMC4",	(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_PMCX),
	     "Performance Monitor Counter 4" 							},  
  {  959, SPR_SDA,   "SDA",		(                  CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_EA),
	     "Sampled Data Address"										},   
  {  976, SPR_DMISS, "DMISS",	(            CPU_S								), FIELDS(sf_EA),
	     "D-TLB Miss Address"										},   
  {  977, SPR_DCMP,  "DCMP",	(            CPU_S								), FIELDS(sf_DCMP),
	     "D-TLB Miss Compare Value"									},  
  {  978, SPR_HASH1, "HASH1",	(            CPU_S								), FIELDS(sf_HASH),
	     "1st PTEG Hashed Address"									},  
  {  979, SPR_HASH2, "HASH2",	(            CPU_S								), FIELDS(sf_HASH),
	     "2nd PTEG Hashed Address"									},  
  {  980, SPR_IMISS, "IMISS",	(            CPU_S								), FIELDS(sf_EA),
	     "I-TLB Miss Address"										},   
  {  981, SPR_ICMP,  "ICMP",	(            CPU_S								), FIELDS(sf_DCMP),
	     "I-TLB Miss Compare Value"									},  
  {  982, SPR_RPA,   "RPA",		(            CPU_S								), FIELDS(sf_RPA),
	     "Real Page Address"										},   

  {  983, SPR_L3PM,  "L3DM",    (                                          CPU_V), FIELDS(sf_L3DM),
	     "L3 Direct Map Control Register"							},
  { 1008, SPR_HID0,  "HID0",	(      CPU_W									), FIELDS(sf_HID0),
	     "HW Implementation Dependent Reg. 0"						},   
  { 1008, SPR_HID0,  "HID0",	(            CPU_S								), FIELDS(sf_HID0_604),
	     "HW Implementation Dependent Reg. 0"						},   
  { 1008, SPR_HID0,  "HID0",	(                  CPU_A|CPU_G					), FIELDS(sf_HID0_7xx),
	     "HW Implementation Dependent Reg. 0"						},   
  { 1008, SPR_HID0,  "HID0",	(                              CPU_M|CPU_N      ), FIELDS(sf_HID0_max),
	     "HW Implementation Dependent Reg. 0"						},   
  { 1008, SPR_HID0,  "HID0",	(										   CPU_V), FIELDS(sf_HID0_vger),
	     "HW Implementation Dependent Reg. 0"						},   
  { 1009, SPR_HID1,  "HID1",	(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N      ), FIELDS(sf_HID1),
	     "HW Implementation Dependent Reg. 1"						},   
  { 1009, SPR_HID1,  "HID1",	(                                          CPU_V), FIELDS(sf_HID1_vger),
	     "HW Implementation Dependent Reg. 1"						},   
  { 1010, SPR_IABR,  "IABR",	(      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_IABR),
	     "Instruction Address Breakpoint Reg."						},    
  { 1011, SPR_HID2,  "HID2",    (                                          CPU_V), FIELDS(sf_HID2),
	     "HW Implementation Dependent Reg. 2"						},  

  { 1013, SPR_DABR,  "DABR",	(            CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_DABR),
	     "Data Address Breakpoint Reg."								},    
  { 1014, SPR_MSSCR0,"MSSCR0",	(                              CPU_M|CPU_N      ), FIELDS(sf_MSSCR0),
	      "Memory Subsystem Control Register 0"						},   
  { 1014, SPR_MSSCR0,"MSSCR0",	(                                          CPU_V), FIELDS(sf_MSSCR0_vger),
	      "Memory Subsystem Control Register 0"						},   
  { 1015, SPR_MSSCR1,"MSSCR1",	(                              CPU_M|CPU_N      ), FIELDS(sf_MSSCR1),
	      "Memory Subsystem Control Register 1"						},   
  { 1015, SPR_MSSSR0,"MSSSR0",	(                                          CPU_V), FIELDS(sf_MSSSR0),
	      "Memory Subsystem Status Register 0"						},   
  { 1016, SPR_L2DM,  "L2DM",    (            CPU_G|      CPU_N					), FIELDS(sf_L2DM),
	     "L2 Direct Map Control Register"							},
  { 1017, SPR_L2CR,  "L2CR",	(      CPU_A|CPU_G								), FIELDS(sf_L2CR),
	     "L2 Control"												},     
  { 1017, SPR_L2CR,  "L2CR",	(                  CPU_M|CPU_N					), FIELDS(sf_L2CR_max),
	     "L2 Control"												},     
  { 1017, SPR_L2CR,  "L2CR",	(                                          CPU_V), FIELDS(sf_L2CR_vger),
	     "L2 Control"												},     
  { 1018, SPR_L3CR,  "L3CR",	(                                          CPU_V), FIELDS(sf_L3CR),
	     "L3 Control"												},     
  { 1019, SPR_ICTC,  "ICTC",	(      CPU_A|CPU_G|CPU_M|CPU_N|            CPU_V), FIELDS(sf_ICTC),
	     "Instruction Cache Throttle Control"						},   
  { 1020, SPR_THRM1, "THRM1",	(      CPU_A|CPU_G|CPU_M|                  CPU_V), FIELDS(sf_THRM1),
	     "Thermal Assist Unit 1"									},   
  { 1021, SPR_THRM2, "THRM2",	(      CPU_A|CPU_G|CPU_M|                  CPU_V), FIELDS(sf_THRM1),
	     "Thermal Assist Unit 2"									},   
  { 1022, SPR_THRM3, "THRM3",	(      CPU_A|CPU_G|CPU_M|                  CPU_V), FIELDS(sf_THRM3),
	     "Thermal Assist Unit 3"									},   
  { 1023, SPR_PIR,   "PID",	    (CPU_X											), FIELDS(sf_PID),
	     "Processor ID Register"									},    
  { 1023, SPR_PIR,   "PIR",	    (      CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_PIR),
	     "Processor ID Register"									}, 
  { 1025, SPR_CR,    "CR",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_CR),
	     "Condition Register"										},
  { 1026, SPR_FPSCR,"FPSCR",	(CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_FPSCR),
	     "Floating-Point Status and Control Register"				},
  { 1027, SPR_MSR,	"MSR",		(CPU_X|CPU_W|      CPU_A|CPU_G					), FIELDS(sf_MSR),
	     "Machine State Register"									},
  { 1027, SPR_MSR,	"MSR",		(            CPU_S							    ), FIELDS(sf_MSR_604),
	     "Machine State Register"									},
  { 1027, SPR_MSR,	"MSR",		(                              CPU_M|CPU_N|CPU_V), FIELDS(sf_MSR_max),
	     "Machine State Register"									},
  { 1028, SPR_VSCR, "VSCR",	    (						 CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_VSCR),
	     "Vector Status and Control Register"						},
  { 9000, SPR_SR0,  "SR0",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 0"										},
  { 9001, SPR_SR1,  "SR1",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 1"										},
  { 9002, SPR_SR2,  "SR2",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 2"										},
  { 9003, SPR_SR3,  "SR3",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 3"										},
  { 9004, SPR_SR4,  "SR4",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 4"										},
  { 9005, SPR_SR5,  "SR5",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 5"										},
  { 9006, SPR_SR6,  "SR6",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 6"										},
  { 9007, SPR_SR7,  "SR7",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 7"										},
  { 9008, SPR_SR8,  "SR8",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 8"										},
  { 9009, SPR_SR9,  "SR9",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 9"										},
  { 9010, SPR_SR10, "SR10",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 10"										},
  { 9011, SPR_SR11, "SR11",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 11"										},
  { 9012, SPR_SR12, "SR12",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 12"										},
  { 9013, SPR_SR13, "SR13",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 13"										},
  { 9014, SPR_SR14, "SR14",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 14"										},
  { 9015, SPR_SR15, "SR15",	    (CPU_X|CPU_W|CPU_S|CPU_A|CPU_G|CPU_M|CPU_N|CPU_V), FIELDS(sf_SR),
	     "Segment Register 15"										}
};

#define SPR_TAB_SIZE	(sizeof(spr_tab)/sizeof(REGFIELD_TABLE))


//---------------------------------------------------------------------------
// print_binary -- print <n> binary bits, from the <n>..LSB of 'value'.
//---------------------------------------------------------------------------
void print_binary( ULONG value, int n )
{
	ULONG mask;
							
	mask = 1 << (n-1);
	while (mask) {
		printf("%c", (value & mask) ? '1' : '0' );
		mask >>= 1;
	}
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int spr_get_value( PPCSIM *ppcsim, char **args, int reg )
{
	ULONG newVal;

	if (shell_getarg_or_prompt( ppcsim, args, "  New value ? ", &newVal ) == 1)
		SPR_set( reg, newVal );
	
	return( 0 );
}


//---------------------------------------------------------------------------
//	spr_get_value_bin -- get a binary field value.
//---------------------------------------------------------------------------
int spr_get_value_bin( PPCSIM *ppcsim, char **args, int sreg, int sf, int ef )
{
	ULONG   binVal;
	ULONG	value, fv, mask;
	short		  i, b;
	char		  nullprompt[24];
	char		  sz[2];

// Make a prompt.

	strcpy(nullprompt, "  New field value [");
	sz[0] = '0' + (ef - sf + 1);
	sz[1] = 0;
	strcat(nullprompt, sz );
	strcat(nullprompt, "bit] %%");

	if (shell_getarg_or_prompt( ppcsim, args, nullprompt, &binVal ) != 1)
		return( 0 );

// The user entered binary, but we receive it as hex/BCD.
// I.e. 0x10100 => %10100 => 20

	fv = 0;
	for (i = 0; i <= 7; i++) {
		b = (binVal >> (i * 4)) & 0x0f;
		if (b == 0)
			;
		else if (b == 1)
			fv = fv | (1 << i);
		else 
			return( 1 );
	}


// Position the new field accordingly.

	fv <<= (31 - ef);
	mask = 0;
	for (i = 0; i <= 31; i++)	// I'm sure there's a better way to do this.
		if (sf <= i  &&  i <= ef)
			mask |= (1 << (31-i));
	mask = ~mask;

	value = SPR_get( sreg );
	value = (value & mask) | fv;
	SPR_set( sreg, value );

	return( 0 );
}


//---------------------------------------------------------------------------
// DINK's built-in printf doesn't support variable width formatting (width
//		  as a vararg), so instead we pick from an array of constant-sized
//		  fields.
//---------------------------------------------------------------------------
static char *xfmts[] = {
  "%X",
  "0x%01X",
  "0x%02X",
  "0x%03X",
  "0x%04X",
  "0x%05X",
  "0x%06X",
  "0x%07X",
  "0x%08X",
  "0x%09X"
};


//---------------------------------------------------------------------------
// spr_reg_decode -- print the value of a register in a variety of ways:
//
//	1) Normal; name and value with a expansion of each field.
//	2) Quiet:  just the name and value.
//  3) Fields: name, value and fields in an array. 
//---------------------------------------------------------------------------
void spr_reg_decode( REGFIELD_TABLE *st, ULONG value, int sfield, int efield,
					 char dopt, short for_nb )
{
	REG_FIELDS *sf;
	ULONG		f;
	char	   *s;
	short 		i, c, no, x, o, bin_width;
	short		nmax, fmax, fw, ncols;

	bin_width = efield - sfield;
	if (bin_width < 0)
		bin_width = -bin_width;
	bin_width += 1;

// If verbose, print the long description name first.

	if (dopt & RD_OPT_VERBOSE) {
		printf("%s", st->desc);
		for (i = 76-strlen(st->desc)-9; i > 0; i--)
			printf(" ");
		if (for_nb)
			printf("     0x%02X\n", st->spr_no);
		else
			printf("SPR #%04d\n", st->spr_no);
		printf(
"----------------------------------------------------------------------------\n"
			  );
	}


// Next, print the name and the current value.

	if ((for_nb == 0  &&  sfield ==  0  &&  efield == 31)
	||  (for_nb == 1  &&  dopt != RD_OPT_FIELDS)) {
//		printf("for_nb: %d sf %d ef %d\n", for_nb, sfield, efield);
		i = (for_nb) ? (sfield-efield+1)/4 : 8;
		printf("%s : ", st->name);
		printf(xfmts[i], value);
		printf("\n");
	}


// If field expansion is set, show the values broken up into fields.

	if (dopt == RD_OPT_FIELDS) {
		sf = st->rfp;

// Find the maximum length of the field names and values.  Values
// are printed in binary for <=4 bits, and hex for >4.

		nmax = fmax = 0;
		for (i = 0; i < st->nfields; i++, sf++) {
			s = sf->name;
			if (s == NULL)					// No name.
				continue;
			if (*s == '=')					// Skip over '=' tag.
				s++;
			if ((c = strlen(s)) > nmax)
				nmax = c;
			x = (sf->size <= 4)
			  ?  sf->size 
			  : (sf->size/4) + (sf->size%4 ? 1 : 0) + 2;
			if ((sf->name != NULL)  &&  (x > fmax))
				fmax = x;
		}
		fw    = nmax + 1 + fmax;
		ncols = (80 - 2) / (fw + 4);

//		printf("fw %d  nc %d\n", fw, ncols);

// Now that the data is sized, print the fields.

		sf = st->rfp;
		c  = -1;
		for (no = 0; no < st->nfields; no++, sf++) {
			if (c < 0) {
				printf("  ");
				c++;
			}
			if ((for_nb == 0  &&  ((sf->msb < sfield  ||  sf->msb > efield)))
			||  (for_nb == 1  &&  ((sf->msb > sfield  ||  sf->msb < efield))))
				continue;

			if (sf->name == NULL)					// Ignore reserved fields
				continue;

			if (for_nb)
				f =  (value >> (sf->msb-sf->size+1));	// Shift right
			else
				f =  (value >> (32-sf->msb-sf->size));	// Shift right
			f &= ((1 << sf->size) - 1);					// Mask;

			s = sf->name;
			if (*s == '=')					// Skip over '=' tag.
				s++;
			printf("%s=", s);
			o = strlen(s)+1;

// If the name starts with '=', it is a partial address so we don't
// shift it over (always 32-bits too).  See L2DM, ASR, etc.

			if (*sf->name == '=') {
				f = value & (0xFFFFFFFF << (32-sf->msb-sf->size));
				printf(xfmts[8], f );
			}
			else if (sf->size <= 4) {
				print_binary( f, sf->size );
				o += sf->size;
			}
			else {
				x = (sf->size/4) + (sf->size%4 ? 1 : 0);
				printf(xfmts[x], f );
				o += (x + 2);
			}

			if (++c >= ncols) {
				printf("\n");
				c = -1;
			}
			else
				while (o++ < (fw+4))
					printf(" ");
		}
		if (c > 0)
			printf("\n");
	}

// Alternately, print out the bar-graph display (for want of a better
// description).

	else if (dopt == RD_OPT_VERBOSE) {
		print_binary( value, bin_width );
		printf("\n");

		sf = st->rfp;
		for (c = no = 0; no < st->nfields; no++, sf++) {
			fw = (for_nb) ? sfield-sf->msb : sf->msb;
			for (i = 0; i < fw; i++)
				printf(" ");
			for (i = 0; i < sf->size; i++)
				printf("+");
			fw = (for_nb) ? sf->msb-sf->size+1 : 32-sf->size-sf->msb;
			for (i = fw; i > 0; i--)
#ifdef NO_MARKER_BARS
				printf( "." );
#else
				printf( (i % 8 == 0) ? "|" : ".");
#endif

			if ((s = sf->name) == NULL)
//				printf(" : -------- : (reserved)\n");
				printf(" : -------- :\n");
			else {
				if (*s == '=')					// Skip over '=' tag.
					s++;
				printf(" : %8s : %s\n", s, sf->desc);
			}
		}
	}
}


//------------------------------------------------------------------------
//	SPR_RO_list -- registers in this list are read-only.
//------------------------------------------------------------------------
static short SPR_RO_list[] = {
	SPR_HASH1,	SPR_HASH2,
	SPR_TBL,	SPR_TBU,
	SPR_UBAMR,
	SPR_UMMCR1,	SPR_UMMCR2,
	SPR_UPMC1,	SPR_UPMC2,	SPR_UPMC3,	 SPR_UPMC4,
	SPR_USIA
};

#define SPR_RO_SIZE		(sizeof(SPR_RO_list)/sizeof(short))


//------------------------------------------------------------------------
//	SPR_rw -- read or write an SPR.
//------------------------------------------------------------------------
int SPR_rw( PPCSIM *ppcsim, char **args, int reg, int r_w, int sf, int ef, char dopt )
{
	int		status;
	ULONG	value;
	short	cpumask, i, ro, matched_spr;


// Look for the matching SPR data, based on SPR number and the processor
// type, since registers vary based on CPU type.
//
	cpumask 	= ppcsim->CPU_class;
	matched_spr = 0;
	for (i = 0; i < SPR_TAB_SIZE; i++)
		if (spr_tab[i].spr_no == reg) {
			matched_spr++;
			if (spr_tab[i].proc_list & cpumask)
				break;
		}

	if (i >= SPR_TAB_SIZE)
		return( (matched_spr) ? 1 : 2 );

	value = SPR_get( spr_tab[i].spr_loc );


// Unless modifying on the command line, print the register in normal,
// quiet or field-expanded mode.

	if (**args != '=')
		spr_reg_decode( &spr_tab[i], value, sf, ef, dopt, 0 );


// If modify requested, and it is not in the list of read-only
// registers, modify it.

	if (r_w == 1)		// read
		return( 0 );

	for (ro = 0; ro < SPR_RO_SIZE; ro++)
		if (reg == SPR_RO_list[ro]) {
			printf("  SPR is R/O.\n");
			return( 1 );
		}

// OK, change the register.  If not in field mode, change all the
// bits.
//
	if (sf == 0  &&  ef == 31)
		status = spr_get_value( ppcsim, args, spr_tab[i].spr_loc );
	else
		status = spr_get_value_bin( ppcsim, args, spr_tab[i].spr_loc, sf, ef );

	return( status );
}


//------------------------------------------------------------------------
//	SPR_search -- extract an SPR by name or by number.  Return SPR to
//				  caller and 0, or -1 on error.
//------------------------------------------------------------------------
int SPR_search( char **string_s, int *reg )
{
	int   i, l;
	char *s;

	s = *string_s;

	for (i = 0; i < SPR_TAB_SIZE; i++) {
		l = strlen( spr_tab[i].name );
		if (strncasecmp( spr_tab[i].name, s, l ) == 0) {
			*string_s = s + l;
			*reg      = spr_tab[i].spr_no;
			return( 0 );
		}
	}

// No match, is it 's###'?

	if (*s == 's'  ||  *s == 'S') {
		s++;
		*reg = (int) strtoul( s, string_s, 10 );
		if (s != *string_s)
			return( 0 );
	}

	return( -1 );
}


//------------------------------------------------------------------------
//	spr_pull -- same as search_spr, different calling syntax.
//------------------------------------------------------------------------
int spr_pull( char *sname )
{
	int   reg;
	char *s;

	s = sname;
	if (SPR_search( &s, &reg ) == 0)
		return( reg );
	return( -1 );
}


//------------------------------------------------------------------------
//	SPR_get_fields -- look through the SPR definition for a matching
//					  field name.
//------------------------------------------------------------------------
int SPR_get_fields( PPCSIM *ppcsim, int spr_no, char **pfield, int *sfield, int *efield )
{
	REGFIELD_TABLE *st;
	REG_FIELDS     *sf;
	char           *s, *field;
	short		    cpumask;
	int             i, l;
	
	field = *pfield;

//	printf("get_spr_fields: on %d for `%s'\n", spr_no, field);

	cpumask 	= ppcsim->CPU_class;
	for (i = 0; i < SPR_TAB_SIZE; i++) {
		if (spr_tab[i].spr_no >= 9000)
			break;
		if ((spr_tab[i].spr_no == spr_no)
		&&  (spr_tab[i].proc_list & cpumask))
			break;
	}
	if (i >= SPR_TAB_SIZE)
		return( 1 );


// spr_tab[i] is the matching SPR.

	st = &spr_tab[i];
	sf = st->rfp;

	for (i = 0; i < st->nfields; i++, sf++) {
		s = sf->name;
		if (s == NULL)
			continue;
		if (*s == '=')					// Skip over '=' tag.
			s++;
		l = strlen( s );
		if (strncasecmp( field, s, l ) == 0) {
			*sfield = sf->msb;
			*efield = sf->msb + sf->size - 1;
			*pfield = field + l;
			return(0);
		}
	}

	return( 1 );
}


//---------------------------------------------------------------------------
//	SPR_list -- list the name, number and description of all SPR's valid
//				for the current processor.
//---------------------------------------------------------------------------
int SPR_list( PPCSIM *ppcsim )
{
	int	 i, cpumask;
	int	 lim, lines, opt_segs;

	printf("  SPR       Number  Value     Register Name\n"
		   "  ===       ======  ========  =============\n");

	lines		= 3;
	lim			= shell_more_lines( 23 );
	cpumask		= ppcsim->CPU_class;
	opt_segs	= 0;

	for (i = 0; i < SPR_TAB_SIZE; i++) {
		if (opt_segs == 0  &&  spr_tab[i].spr_no >= 9000)
			break;
		if (spr_tab[i].proc_list & cpumask) {
			printf("  %-10s %4d   %08X  %s\n",  
				spr_tab[i].name, spr_tab[i].spr_no, 
				SPR_get( spr_tab[i].spr_no ), spr_tab[i].desc );

			if (lines++ >= lim) {
				if (!shell_more())
					break;
				lines = 0;
			}
		}
	}

	return( 0 );
}


//---------------------------------------------------------------------------
//	reg_spr_printname	-- print the name of an SPR, given the SPR #.
//---------------------------------------------------------------------------
int reg_spr_printname( ULONG spr_no )
{
	int i;

	for (i = 0; i < SPR_TAB_SIZE; i++)
		if (spr_tab[i].spr_no == spr_no) {
			printf("%s", spr_tab[i].name);
			break;
		}

	return( 0 );
}


//================================================================================
//								BATS
//================================================================================

static char *BAT_size_text[] = {
	"128K",
	"256K",
	"512K",
	"  1M",
	"  2M",
	"  4M",
	"  8M",
	" 16M",
	" 32M",
	" 64M",
	"128M",
	"256M",
	"----"
};


static ULONG BAT_sizes[] = {
	0x00020000,			// 128K
	0x00040000,			// 256K
	0x00080000,			// 512K
	0x00100000,			//   1M
	0x00200000,			//   2M
	0x00400000,			//   4M
	0x00800000,			//   8M
	0x01000000,			//  16M
	0x02000000,			//  32M
	0x04000000,			//  64M
	0x08000000,			// 128M
	0x10000000,			// 256M
	0					// illegal
};
	
static char *pp_bits[4]  = { "NONE",  "RO  ",  "R/W ",  "RO  "  };
static char *val_bits[4] = { "[-,-]", "[-,U]", "[S,-]", "[S,P]" };


//---------------------------------------------------------------------------
//	decode_bat -- 
//---------------------------------------------------------------------------
static void decode_bat( int no, ULONG uv, ULONG lv )
{
	ULONG start, size;
	ULONG rep;
	int	  i;

	start = uv & 0xFFFC0000;
	size  = (uv & 0x1FFF) >> 2;
	rep   = lv & 0xFFFC0000;
	for (i = 0; size; size >>= 1)			// Count 1's
		if (size)
			i++;
	if (i >= 11)
		i = 11;

//	printf("ones = %d\n", i);

	if ((uv & 0x03) == 0)
		printf("   %1d    --------..-------- ----  --------..--------  ----"
              "  ----  [off]\n", no);

	else	
		printf("   %d    %08X..%08X %s  %08X..%08X  %d%d%d%d  %s  %s\n",
				no, start, start-1+BAT_sizes[i], BAT_size_text[i], 
				rep, rep-1+BAT_sizes[i],
				(lv >> 6) & 1,
				(lv >> 5) & 1,
				(lv >> 4) & 1,
				(lv >> 3) & 1,
				pp_bits[ lv & 0x03 ],
				val_bits[ uv & 0x03 ] );
}


// These arrays map sequential indices to IBAT/DBAT storage locations.
//
static short ibatu_map[8] = { SPR_IBAT0U, SPR_IBAT1U, SPR_IBAT2U, SPR_IBAT3U,
							  SPR_IBAT4U, SPR_IBAT5U, SPR_IBAT6U, SPR_IBAT7U };
static short ibatl_map[8] = { SPR_IBAT0L, SPR_IBAT1L, SPR_IBAT2L, SPR_IBAT3L,
							  SPR_IBAT4L, SPR_IBAT5L, SPR_IBAT6L, SPR_IBAT7L };
static short dbatu_map[8] = { SPR_DBAT0U, SPR_DBAT1U, SPR_DBAT2U, SPR_DBAT3U,
							  SPR_DBAT4U, SPR_DBAT5U, SPR_DBAT6U, SPR_DBAT7U };
static short dbatl_map[8] = { SPR_DBAT0L, SPR_DBAT1L, SPR_DBAT2L, SPR_DBAT3L,
							  SPR_DBAT4L, SPR_DBAT5L, SPR_DBAT6L, SPR_DBAT7L };


static char *BAT_header = 
  "  %cBAT  BLOCK ADDRESS      SIZE  REPLACEMENT ADDR    WIMG  PROT  VALID\n"
  "  ====  ================== ====  ==================  ====  ====  =====\n";
static char *BAT_bin = "  %cBAT%1dU=%08lX   %cBAT%1dL=%08lX";


//---------------------------------------------------------------------------
//	SPR_bats_rd -- display and decode all BATs.
//---------------------------------------------------------------------------
int SPR_bats_rd( PPCSIM *ppcsim, char dopt )
{
	ULONG	uv, lv;
	int		i, no_bats;


// Determine the number of BATs.

	no_bats = (ppcsim->process_type == CPU_MPC755) ? 8 : 4;


// Display all the IBATs.  There are 4 or 8, depending on the CPU.
//
	if (dopt == RD_OPT_VERBOSE) {
		for (i = 0; i < no_bats; i++) {
			uv = SPR_get( ibatu_map[i] );
			lv = SPR_get( ibatl_map[i] );
			printf(BAT_bin, 'I', i, uv, 'I', i, lv );
			printf("   ");

			uv = SPR_get( dbatu_map[i] );
			lv = SPR_get( dbatl_map[i] );
			printf(BAT_bin, 'D', i, uv, 'D', i, lv );

			printf("\n");
		}
		printf("\n");
	}


// Decode all the IBATs.  There are 4 or 8, depending on the CPU.
//
	printf( BAT_header, 'I' );
	for (i = 0; i < no_bats; i++) {
		uv = SPR_get( ibatu_map[i] );
		lv = SPR_get( ibatl_map[i] );
		decode_bat( i, uv, lv );
	}
		
	printf( "\n" );
	printf( BAT_header, 'D' );
	for (i = 0; i < no_bats; i++) {
		uv = SPR_get( dbatu_map[i] );
		lv = SPR_get( dbatl_map[i] );
		decode_bat( i, uv, lv );
	}

	return( 0 );
}
