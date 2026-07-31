// vim: ts=4 tabstop=4 softtabstop=4 shiftwidth=4 columns=120 lines=48 nobackup


#include "ppcsim.h"
#include "asm_dsm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>


//--------------------------------------------------------------------------------
//
asm_dasm_element ops_and_mnemonics[] = {
	{ "bdnz" , 0x42000000	 , (smBC+4)	},
	{ "bdnzt" , 0x41000000	 , (smBC+2)	},
	{ "bdnzf" , 0x40000000	 , (smBC+2)	},
	{ "bdz" , 0x42400000		 , (smBC+4)	},
	{ "bdzt" , 0x41400000	 , (smBC+2)	},
	{ "bdzf" , 0x40400000	 , (smBC+2)	},
	{ "blt" , 0x41800000		 , smBC	},
	{ "ble" , 0x40800000		 , (smBC+1)	},
	{ "beq" , 0x41800000		 , (smBC+2)	},
	{ "bge" , 0x40800000		 , smBC	},
	{ "bgt" , 0x41800000		 , (smBC+1)	},
	{ "bnl" , 0x40800000		 , smBC	},
	{ "bne" , 0x40800000		 , (smBC+2)	},
	{ "bng" , 0x40800000		 , (smBC+1)	},
	{ "bso" , 0x41800000		 , (smBC+3)	},
	{ "bns" , 0x40800000		 , (smBC+3)	},
	{ "bun" , 0x41800000		 , (smBC+3)	},
	{ "bnu" , 0x40800000		 , (smBC+3)	},
	{ "bta" , 0x41800002 , (smBCA+4)	},
	{ "bfa" , 0x40800002 , (smBCA+4)	},
	{ "bdnza" , 0x42000002 , (smBCA+4)	},
	{ "bdnzta" , 0x41000002 , (smBCA+2)	},
	{ "bdnzfa" , 0x40000002 , (smBCA+2)	},
	{ "bdza" , 0x42400002 , (smBCA+4)	},
	{ "bdzta" , 0x41400002 , (smBCA+2)	},
	{ "bdzfa" , 0x40400002 , (smBCA+2)	},
	{ "blta" , 0x41800002 , smBCA	},
	{ "blea" , 0x40800002 , (smBCA+1)	},
	{ "beqa" , 0x41800002 , (smBCA+2)	},
	{ "bgea" , 0x40800002	 , smBCA	},
	{ "bgta" , 0x41800002 , (smBCA+1)	},
	{ "bnla" , 0x40800002	 , smBCA	},
	{ "bnea" , 0x40800002 , (smBCA+2)	},
	{ "bnga" , 0x40800002 , (smBCA+1)	},
	{ "bsoa" , 0x41800002 , (smBCA+3)	},
	{ "bnsa" , 0x40800002 , (smBCA+3)	},
	{ "buna" , 0x41800002 , (smBCA+3)	},
	{ "bnua" , 0x40800002 , (smBCA+3)	},
	{ "btl" , 0x41800001 , (smBCL+4)	},
	{ "bfl" , 0x40800001 , (smBCL+4)	},
	{ "bdnzl" , 0x42000001 , (smBCL+4)	},
	{ "bdnztl" , 0x41000001 , (smBCL+2)	},
	{ "bdnzfl" , 0x40000001 , (smBCL+2)	},
	{ "bdzl" , 0x42400001 , (smBCL+4)	},
	{ "bdztl" , 0x41400001 , (smBCL+2)	},
	{ "bdzfl" , 0x40400001 , (smBCL+2)	},
	{ "bltl" , 0x41800001	 , smBCL	},
	{ "blel" , 0x40800001 , (smBCL+1)	},
	{ "beql" , 0x41800001 , (smBCL+2)	},
	{ "bgel" , 0x40800001	 , smBCL	},
	{ "bgtl" , 0x41800001 , (smBCL+1)	},
	{ "bnll" , 0x40800001	 , smBCL	},
	{ "bnel" , 0x40800001 , (smBCL+2)	},
	{ "bngl" , 0x40800001 , (smBCL+1)	},
	{ "bsol" , 0x41800001 , (smBCL+3)	},
	{ "bnsl" , 0x40800001 , (smBCL+3)	},
	{ "bunl" , 0x41800001 , (smBCL+3)	},
	{ "bnul" , 0x40800001 , (smBCL+3)	},
	{ "btla" , 0x41800003 , (smBCLA+4)	},
	{ "bfla" , 0x40800003 , (smBCLA+4)	},
	{ "bdnzla" , 0x42000003 , (smBCLA+4)	},
	{ "bdnztla" , 0x41000003 , (smBCLA+2)	},
	{ "bdnzfla" , 0x40000003 , (smBCLA+2)	},
	{ "bdzla" , 0x42400003 , (smBCLA+4)	},
	{ "bdztla" , 0x41400003 , (smBCLA+2)	},
	{ "bdzfla" , 0x40400003 , (smBCLA+2)	},
	{ "bltla" , 0x41800003 , smBCLA	},
	{ "blela" , 0x40800003 , (smBCLA+1)	},
	{ "beqla" , 0x41800003 , (smBCLA+2)	},
	{ "bgela" , 0x40800003  , smBCLA	},
	{ "bgtla" , 0x41800003  , (smBCLA+1)	},
	{ "bnlla" , 0x40800003 , smBCLA	},
	{ "bnela" , 0x40800003 , (smBCLA+2)	},
	{ "bngla" , 0x40800003 , (smBCLA+1)	},
	{ "bsola" , 0x41800003 , (smBCLA+3)	},
	{ "bnsla" , 0x40800003 , (smBCLA+3)	},
	{ "bunla" , 0x41800003 , (smBCLA+3)	},
	{ "bnula" , 0x40800003 , (smBCLA+3)	},
	{ "btlr" , 0x4D800020	 , (smBCLR+4)	},
	{ "bflr" , 0x4C800020	 , (smBCLR+4)	},
	{ "bdnzlr" , 0x4E000020	 , (smBCLR+4)	},
	{ "bdnztlr" , 0x4D000020	 , (smBCLR+2)	},
	{ "bdnzflr" , 0x4C000020	 , (smBCLR+2)	},
	{ "bdzlr" , 0x4E400020	 , (smBCLR+4)	},
	{ "bdztlr" , 0x4D400020	 , (smBCLR+2)	},
	{ "bdzflr" , 0x4C400020	 , (smBCLR+2)	},
	{ "bltlr" , 0x4D800020	 , smBCLR	},
	{ "blelr" , 0x4C800020	 , (smBCLR+1)	},
	{ "beqlr" , 0x4D800020	 , (smBCLR+2)	},
	{ "bgelr" , 0x4C800020	 , smBCLR	},
	{ "bgtlr" , 0x4D800020	 , (smBCLR+1)	},
	{ "bnllr" , 0x4C800020	 , smBCLR	},
	{ "bnelr" , 0x4C800020	 , (smBCLR+2)	},
	{ "bnglr" , 0x4C800020	 , (smBCLR+1)	},
	{ "bsolr" , 0x4D800020	 , (smBCLR+3)	},
	{ "bnslr" , 0x4C800020	 , (smBCLR+3)	},
	{ "bunlr" , 0x4D800020	 , (smBCLR+3)	},
	{ "bnulr" , 0x4C800020	 , (smBCLR+3)	},
	{ "btlrl" , 0x4D800021	 , (smBCLRL+4)	},
	{ "bflrl" , 0x4C800021	 , (smBCLRL+4)	},
	{ "bdnzlrl" , 0x4E000021 , (smBCLRL+4)	},
	{ "bdnztlrl" , 0x4D000021 , (smBCLRL+2)	},
	{ "bdnzflrl" , 0x4C000021 , (smBCLRL+2)	},
	{ "bdzlrl" , 0x4E400021	 , (smBCLRL+4)	},
	{ "bdztlrl" , 0x4D400021 , (smBCLRL+2)	},
	{ "bdzflrl" , 0x4C400021 , (smBCLRL+2)	},
	{ "bltlrl" , 0x4D800021	 , smBCLRL	},
	{ "blelrl" , 0x4C800021	  , (smBCLRL+1)	},
	{ "beqlrl" , 0x4D800021	  , (smBCLRL+2)	},
	{ "bgelrl" , 0x4C800021	 , smBCLRL	},
	{ "bgtlrl" , 0x4D800021	 , (smBCLRL+1)	},
	{ "bnllrl" , 0x4C800021	 , smBCLRL	},
	{ "bnelrl" , 0x4C800021	 , (smBCLRL+2)	},
	{ "bnglrl" , 0x4C800021	 , (smBCLRL+1)	},
	{ "bsolrl" , 0x4D800021	 , (smBCLRL+3)	},
	{ "bnslrl" , 0x4C800021	 , (smBCLRL+3)	},
	{ "bunlrl" , 0x4D800021	 , (smBCLRL+3)	},
	{ "bnulrl" , 0x4C800021	 , (smBCLRL+3)	},
	{ "btctr" , 0x4D800420	 , (smBCCTR+4)	},
	{ "bfctr" , 0x4C800420	 , (smBCCTR+4)	},
	{ "bltctr" , 0x4D800420		 , smBCCTR	},
	{ "blectr" , 0x4C800420	 , (smBCCTR+1)	},
	{ "beqctr" , 0x4D800420	 , (smBCCTR+2)	},
	{ "bgectr" , 0x4C800420	 , smBCCTR	},
	{ "bgtctr" , 0x4D800420	 , (smBCCTR+1)	},
	{ "bnlctr" , 0x4C800420	 , smBCCTR	},
	{ "bnectr" , 0x4C800420	 , (smBCCTR+2)	},
	{ "bngctr" , 0x4C800420	 , (smBCCTR+1)	},
	{ "bsoctr" , 0x4D800420	 , (smBCCTR+3)	},
	{ "bnsctr" , 0x4C800420	 , (smBCCTR+3)	},
	{ "bunctr" , 0x4D800420	 , (smBCCTR+3)	},
	{ "bnuctr" , 0x4C800420	 , (smBCCTR+3)	},
	{ "btctrl" , 0x4D800421 , (smBCCTRL+4)	},
	{ "bfctrl" , 0x4C800421 , (smBCCTRL+4)	},
	{ "bltctrl" , 0x4D800421	 , smBCCTRL	},
	{ "blectrl" , 0x4C800421 , (smBCCTRL+1)	},
	{ "beqctrl" , 0x4D800421 , (smBCCTRL+2)	},
	{ "bgectrl" , 0x4C800421	 , smBCCTRL	},
	{ "bgtctrl" , 0x4D800421 , (smBCCTRL+1)	},
	{ "bnlctrl" , 0x4C800421	 , smBCCTRL	},
	{ "bnectrl" , 0x4C800421 , (smBCCTRL+2)	},
	{ "bngctrl" , 0x4C800421 , (smBCCTRL+1)	},
	{ "bsoctrl" , 0x4D800421 , (smBCCTRL+3)	},
	{ "bnsctrl" , 0x4C800421 , (smBCCTRL+3)	},
	{ "bunctrl" , 0x4D800421 , (smBCCTRL+3)	},
	{ "bnuctrl" , 0x4C800421	 , (smBCCTRL+3)	},
	{ "bt" , 0x41800000                   	 , (smBC+4)	},
	{ "bf" , 0x40800000             , (smBC+4)	},
	{ "blr" , 0x4E800020	 , (smBCLR-1)	},
	{ "bctr" , 0x4E800420      	 , (smBCCTR-1)	},
	{ "blrl" , 0x4E800021      	 , (smBCLRL-1)	},
	{ "bctrl" , 0x4E800421           , (smBCCTRL-1)	},
	{ "vmr" , 0x10000484 , smVX	},
	{ "vnot" , 0x10000504 , smVX	},
	{ "mfasr" , 0x7c1842a6	 , smXFX	},
	{ "mfear" , 0x7c1a42a6 , smXFX	},
	{ "mtctr" , 0x7c0903a6 , smXFX	},
	{ "mfdar" , 0x7c1302a6 , smXFX	},
	{ "mfdec" , 0x7c1602a6 , smXFX	},
	{ "mfdsisr" , 0x7c1202a6	 , smXFX	},
	{ "mflr" , 0x7c0802a6		 , smXFX	},
	{ "mfpvr" , 0x7c1f42a6	 , smXFX	},
	{ "mfsdr1" , 0x7c1902a6		 , smXFX	},
	{ "mfsprg" , 0x7c1042a6 , smXFX	},
	{ "mfsprg" , 0x7c1142a6  , smXFX	},
	{ "mfsprg" , 0x7c1242a6   , smXFX	},
	{ "mfsprg" , 0x7c1342a6  , smXFX	},
	{ "mfsrr0" , 0x7c1a02a6 , smXFX	},
	{ "mfsrr1" , 0x7c1b02a6 , smXFX	},
	{ "mftbu" , 0x7c0d42e6  , smXFX	},
	{ "mfxer" , 0x7c0102a6 , smXFX	},
	{ "mr" , 0x7c000378	 , smX	},
	{ "mtasr" , 0x7c1843a6	 , smXFX	},
	{ "mfctr" , 0x7c0902a6		 , smXFX	},
	{ "mtdar" , 0x7c1303a6 , smXFX	},
	{ "mtdbatu" , 0x7c1883a6	 , smXFX	},
	{ "mtdbatu" , 0x7c1a83a6	 , smXFX	},
	{ "mtdbatu" , 0x7c1c83a6 	 , smXFX	},
	{ "mtdbatu" , 0x7c1e83a6 	 , smXFX	},
	{ "mtdbatl" , 0x7c1983a6	 , smXFX	},
	{ "mtdbatl" , 0x7c1b83a6 , smXFX	},
	{ "mtdbatl" , 0x7c1d83a6 	 , smXFX	},
	{ "mtdbatl" , 0x7c1f83a6 	 , smXFX	},
	{ "mfdbatu" , 0x7c1882a6         	 , smXFX	},
	{ "mfdbatu" , 0x7c1a82a6         	 , smXFX	},
	{ "mfdbatu" , 0x7c1c82a6 , smXFX	},
	{ "mfdbatu" , 0x7c1e82a6        	 , smXFX	},
	{ "mfdbatl" , 0x7c1982a6  , smXFX	},
	{ "mfdbatl" , 0x7c1b82a6  , smXFX	},
	{ "mfdbatl" , 0x7c1d82a6  , smXFX	},
	{ "mfdbatl" , 0x7c1f82a6	 , smXFX	},
	{ "mtdec" , 0x7c1603a6 , smXFX	},
	{ "mtdsisr" , 0x7c1203a6	 , smXFX	},
	{ "mtear" , 0x7c1a43a6 , smXFX	},
	{ "mtibatu" , 0x7c1083a6 , smXFX	},
	{ "mtibatu" , 0x7c1283a6  , smXFX	},
	{ "mtibatu" , 0x7c1483a6 	 , smXFX	},
	{ "mtibatu" , 0x7c1683a6 , smXFX	},
	{ "mtibatl" , 0x7c1183a6 , smXFX	},
	{ "mtibatl" , 0x7c1383a6 	 , smXFX	},
	{ "mtibatl" , 0x7c1583a6  , smXFX	},
	{ "mtibatl" , 0x7c1783a6 	 , smXFX	},
	{ "mfibatu" , 0x7c1082a6 , smXFX	},
	{ "mfibatu" , 0x7c1282a6         	 , smXFX	},
	{ "mfibatu" , 0x7c1482a6  , smXFX	},
	{ "mfibatu" , 0x7c1682a6  , smXFX	},
	{ "mfibatl" , 0x7c1182a6  , smXFX	},
	{ "mfibatl" , 0x7c1382a6  , smXFX	},
	{ "mfibatl" , 0x7c1582a6 , smXFX	},
	{ "mfibatl" , 0x7c1782a6 	 , smXFX	},
	{ "mtlr" , 0x7c0803a6 , smXFX	},
	{ "mtsdr1" , 0x7c1903a6 , smXFX	},
	{ "mtsprg" , 0x7c1043a6 , smXFX	},
	{ "mtsprg" , 0x7c1143a6   , smXFX	},
	{ "mtsprg" , 0x7c1243a6  	 , smXFX	},
	{ "mtsprg" , 0x7c1343a6  , smXFX	},
	{ "mtsrr0" , 0x7c1a03a6 , smXFX	},
	{ "mtsrr1" , 0x7c1b03a6 , smXFX	},
	{ "mttbl" , 0x7c1c43a6 , smXFX	},
	{ "mttbu" , 0x7c1d43a6 , smXFX	},
	{ "mtxer" , 0x7c0103a6 , smXFX	},
	{ "sub" , 0x7c000050    , smXO	},
	{ "subo" , 0x7c000450  , smXO	},
	{ "sub." , 0x7c000051       , smXO	},
	{ "subo." , 0x7c000451      , smXO	},
	{ "subc" , 0x7c000010  , smXO	},
	{ "subc." , 0x7c000011 , smXO	},
	{ "subco" , 0x7c000410 , smXO	},
	{ "subco." , 0x7c000411           , smXO	},
	{ "not" , 0x7c0000f8              	 , smX	},
	{ "trap" , 0x7fe00008	 , smX	},
	{ "tweqi" , 0x0c800000			 , smD	},
	{ "tweq" , 0x7c800008	 , smX	},
	{ "twge" , 0x7d800008		 , smX	},
	{ "twgei" , 0x0d800000		 , smD	},
	{ "twgt" , 0x7d000008	 , smX	},
	{ "twgti" , 0x0d000000		 , smD	},
	{ "twle" , 0x7e400008		 , smX	},
	{ "twlei" , 0x0e800000			 , smD	},
	{ "twlge" , 0x7ca00008		 , smX	},
	{ "twlgei" , 0x0ca00000		 , smD	},
	{ "twlgt" , 0x7c200008		 , smX	},
	{ "twlgti" , 0x0c200000		 , smD	},
	{ "twlle" , 0x7cc00008		 , smX	},
	{ "twllei" , 0x0cc00000		 , smD	},
	{ "twllt" , 0x7c400008		 , smX	},
	{ "twllti" , 0x0c400000		 , smD	},
	{ "twng" , 0x7e800008	 , smX	},
	{ "twlng" , 0x7cc00008	 , smX	},
	{ "twlngi" , 0x0cc00000		 , smD	},
	{ "twlnl" , 0x7ca00008	 , smX	},
	{ "twlnli" , 0x0ca00000		 , smD	},
	{ "twlt" , 0x7e000008 , smX	},
	{ "twlti" , 0x0e000000	 , smD	},
	{ "twne" , 0x7f000008		 , smX	},
	{ "twnei" , 0x0f000000			 , smD	},
	{ "twngi" , 0x0e800000		 , smD	},
	{ "twnl" , 0x7d800008			 , smX	},
	{ "twnli" , 0x0d800000		 , smD	},
	{ "rotlwi" , 0x5400003e , smM	},
	{ "rotrwi" , 0x5400003e           	 , smM	},
	{ "clrrwi" , 0x54000000           	 , smM	},
	{ "slwi" , 0x54000000               	 , smM	},
	{ "clrlwi" , 0x5400003e          , smM	},
	{ "srwi" , 0x5400003e            , smM	},
	{ "clrlslwi" , 0x54000000   , smM	},
	{ "rotlw" , 0x5c00003e         	 , smM	},
	{ "extlwi" , 0x54000000     , smM	},
	{ "extrwi" , 0x5400003e           	 , smM	},
	{ "insrwi" , 0x50000000            , smM	},
	{ "inslwi" , 0x50000000        	 , smM	},
	{ "rotlwi." , 0x5400003f          , smM	},
	{ "rotrwi." , 0x5400003f           	 , smM	},
	{ "clrrwi." , 0x54000001           , smM	},
	{ "slwi." , 0x54000001                    	 , smM	},
	{ "srwi." , 0x5400003f               , smM	},
	{ "clrlwi." , 0x5400003f            , smM	},
	{ "clrlslwi." , 0x54000001   , smM	},
	{ "rotlw." , 0x5c00003f , smM	},
	{ "extlwi." , 0x54000001             , smM	},
	{ "extrwi." , 0x5400003f             , smM	},
	{ "insrwi." , 0x50000001            	 , smM	},
	{ "inslwi." , 0x50000001       , smM	},
	{ "crset" , 0x4c000242 , smXL	},
	{ "crclr" , 0x4c000182 , smXL	},
	{ "crmove" , 0x4c000382 , smXL	},
	{ "crnot" , 0x4c000042 , smXL	},
	{ "lis" , 0x3c000000               , smD	},

	// Ray wanted to see addi before la , li and subi so Matt
	// moved addi here ,  ,  above la in this list!


	{ "addi" , 0x38000000 , D	},
	{ "li" , 0x38000000          , smD	},
	{ "la" , 0x38000000               	 , smD	},
	{ "subi" , 0x38000000 , smD	},
	{ "nop" , 0x60000000         , smD	},
	{ "subic" , 0x30000000                    	 , smD	},
	{ "subic." , 0x34000000                     	 , smD	},
	{ "subis" , 0x3c000000               , smD	},
	{ "cmplw" , 0x7c000040               , smX	},
	{ "cmplwi" , 0x28000000                    		 , smD	},
	{ "cmpw" , 0x7c000000                       	 , smX	},
	{ "cmpwi" , 0x2c000000             , smD	},

	// END OF SIMPLIFIED MNEMONICS! MATT 

	{ "abs" , 0x7c0002d0 , XO	},
	{ "abs." , 0x7c0002d1 , XO	},
	{ "abso" , 0x7c0006d0 , XO	},
	{ "abso." , 0x7c0006d1 , XO	},
	{ "add" , 0x7c000214 , XO	},
	{ "add." , 0x7c000215 , XO	},
	{ "addo" , 0x7c000614 , XO	},
	{ "addo." , 0x7c000615 , XO	},
	{ "addc" , 0x7c000014 , XO	},
	{ "addc." , 0x7c000015 , XO	},
	{ "addco" , 0x7c000414 , XO	},
	{ "addco." , 0x7c000415 , XO	},
	{ "adde" , 0x7c000114 , XO	},
	{ "adde." , 0x7c000115 , XO	},
	{ "addeo" , 0x7c000514 , XO	},
	{ "addeo." , 0x7c000515 , XO	},

	// Ray wanted to see "addi" instead of "la" so
	// Matt moved addi above la in this list!

	// { "addi" , 0x38000000 , D	},
	{ "addic" , 0x30000000 , D	},
	{ "addic." , 0x34000000 , D	},
	{ "addis" , 0x3c000000 , D	},
	{ "addme" , 0x7c0001d4 , XO	},
	{ "addme." , 0x7c0001d5 , XO	},
	{ "addmeo" , 0x7c0005d4 , XO	},
	{ "addmeo." , 0x7c0005d5 , XO	},
	{ "addze" , 0x7c000194 , XO	},
	{ "addze." , 0x7c000195 , XO	},
	{ "addzeo" , 0x7c000594 , XO	},
	{ "addzeo." , 0x7c000595 , XO	},
	{ "and" , 0x7c000038 , X	},
	{ "and." , 0x7c000039 , X	},
	{ "andc" , 0x7c000078 , X	},
	{ "andc." , 0x7c000079 , X	},
	{ "andi." , 0x70000000 , D	},
	{ "andis." , 0x74000000 , D	},
	{ "b" , 0x48000000 , I	},
	{ "ba" , 0x48000002 , I	},
	{ "bl" , 0x48000001 , I	},
	{ "bla" , 0x48000003 , I	},
	{ "bc" , 0x40000000 , B	},
	{ "bca" , 0x40000002 , B	},
	{ "bcl" , 0x40000001 , B	},
	{ "bcla" , 0x40000003 , B	},
	{ "bcctr" , 0x4c000420 , XL	},
	{ "bcctrl" , 0x4c000421 , XL	},
	{ "bcl" , 0x40000001 , B	},
	{ "bcla" , 0x40000003 , B	},
	{ "bclr" , 0x4c000020 , XL	},
	{ "bclrl" , 0x4c000021 , XL	},
	{ "cmp" , 0x7c000000 , X	},
	{ "cmpi" , 0x2c000000 , D	},
	{ "cmpl" , 0x7c000040 , X	},
	{ "cmpli" , 0x28000000 , D	},
	{ "cntlzd" , 0x7c000074 , X	},
	{ "cntlzd." , 0x7c000075 , X	},
	{ "cntlzw" , 0x7c000034 , X	},
	{ "cntlzw." , 0x7c000035 , X	},
	{ "crand" , 0x4c000202 , XL	},
	{ "crandc" , 0x4c000102 , XL	},
	{ "creqv" , 0x4c000242 , XL	},
	{ "crnand" , 0x4c0001c2 , XL	},
	{ "crnor" , 0x4c000042 , XL	},
	{ "cror" , 0x4c000382 , XL	},
	{ "crorc" , 0x4c000342 , XL	},
	{ "crxor" , 0x4c000182 , XL	},
	{ "dcbf" , 0x7c0000ac , X	},
	{ "dcbi" , 0x7c0003ac , X	},
	{ "dcbst" , 0x7c00006c , X	},
	{ "dcbt" , 0x7c00022c , X	},
	{ "dcbtst" , 0x7c0001ec , X	},
	{ "dcbz" , 0x7c0007ec , X	},
	{ "div" , 0x7c000296 , XO	},
	{ "div." , 0x7c000297 , XO	},
	{ "divd" , 0x7c0003d2 , XO	},
	{ "divd." , 0x7c0003d3 , XO	},
	{ "divdo" , 0x7c0007d2 , XO	},
	{ "divdo." , 0x7c0007d3 , XO	},
	{ "divdu" , 0x7c000392 , XO	},
	{ "divdu." , 0x7c000393 , XO	},
	{ "divduo" , 0x7c000792 , XO	},
	{ "divduo." , 0x7c000793 , XO	},
	{ "divo" , 0x7c000696 , XO	},
	{ "divo." , 0x7c000697 , XO	},
	{ "divs" , 0x7c0002d6 , XO	},
	{ "divs." , 0x7c0002d7 , XO	},
	{ "divso" , 0x7c0006d6 , XO	},
	{ "divso." , 0x7c0006d7 , XO	},
	{ "divw" , 0x7c0003d6 , XO	},
	{ "divw." , 0x7c0003d7 , XO	},
	{ "divwo" , 0x7c0007d6 , XO	},
	{ "divwo." , 0x7c0007d7 , XO	},
	{ "divwu" , 0x7c000396 , XO	},
	{ "divwu." , 0x7c000397 , XO	},
	{ "divwuo" , 0x7c000796 , XO	},
	{ "divwuo." , 0x7c000797 , XO	},
	{ "doz" , 0x7c000210 , XO	},
	{ "doz." , 0x7c000211 , XO	},
	{ "dozi" , 0x24000000 , D	},
	{ "dozo" , 0x7c000610 , XO	},
	{ "dozo." , 0x7c000611 , XO	},
	{ "dss" , 0x7c00066C , X	},
	{ "dssall" , 0x7e00066C , X	},
	{ "dst" , 0x7c0002AC , X	},
	{ "dstt" , 0x7e0002ac , X	},
	{ "dstst" , 0x7c0002ec , X	},
	{ "dststt" , 0x7e0002ec , X	},
	{ "eciwx" , 0x7c00026c , X	},
	{ "ecowx" , 0x7c00036c , X	},
	{ "eieio" , 0x7c0006ac , X	},
	{ "eqv" , 0x7c000238 , X	},
	{ "eqv." , 0x7c000239 , X	},
	{ "extsb" , 0x7c000774 , X	},
	{ "extsb." , 0x7c000775 , X	},
	{ "extsh" , 0x7c000734 , X	},
	{ "extsh." , 0x7c000735 , X	},
	{ "extsw" , 0x7c0007b4 , X	},
	{ "extsw." , 0x7c0007b5 , X	},
	{ "fabs" , 0xfc000210 , X	},
	{ "fabs." , 0xfc000211 , X	},
	{ "fadd" , 0xfc00002a , A	},
	{ "fadd." , 0xfc00002b , A	},
	{ "fadds" , 0xec00002a , A	},
	{ "fadds." , 0xec00002b , A	},
	{ "fcfid" , 0xfc00069c , X	},
	{ "fcfid." , 0xfc00069d , X	},
	{ "fcmpo" , 0xfc000040 , X	},
	{ "fcmpu" , 0xfc000000 , X	},
	{ "fctid" , 0xfc00065c , X	},
	{ "fctid." , 0xfc00065d , X	},
	{ "fctidz" , 0xfc00065e , X	},
	{ "fctidz." , 0xfc00065f , X	},
	{ "fctiw" , 0xfc00001c , X	},
	{ "fctiw." , 0xfc00001d , X	},
	{ "fctiwz" , 0xfc00001e , X	},
	{ "fctiwz." , 0xfc00001f , X	},
	{ "fdiv" , 0xfc000024 , A	},
	{ "fdiv." , 0xfc000025 , A	},
	{ "fdivs" , 0xec000024 , A	},
	{ "fdivs." , 0xec000025 , A	},
	{ "fmadd" , 0xfc00003a , A	},
	{ "fmadd." , 0xfc00003b , A	},
	{ "fmadds" , 0xec00003a , A	},
	{ "fmadds." , 0xec00003b , A	},
	{ "fmr" , 0xfc000090 , X	},
	{ "fmr." , 0xfc000091 , X	},
	{ "fmsub" , 0xfc000038 , A	},
	{ "fmsub." , 0xfc000039 , A	},
	{ "fmsubs" , 0xec000038 , A	},
	{ "fmsubs." , 0xec000039 , A	},
	{ "fmul" , 0xfc000032 , A	},
	{ "fmul." , 0xfc000033 , A	},
	{ "fmuls" , 0xec000032 , A	},
	{ "fmuls." , 0xec000033 , A	},
	{ "fnabs" , 0xfc000110 , X	},
	{ "fnabs." , 0xfc000111 , X	},
	{ "fneg" , 0xfc000050 , X	},
	{ "fneg." , 0xfc000051 , X	},
	{ "fnmadd" , 0xfc00003e , A	},
	{ "fnmadd." , 0xfc00003f , A	},
	{ "fnmadds" , 0xec00003e , A	},
	{ "fnmadds." , 0xec00003f , A	},
	{ "fnmsub" , 0xfc00003c , A	},
	{ "fnmsub." , 0xfc00003d , A	},
	{ "fnmsubs" , 0xec00003c , A	},
	{ "fnmsubs." , 0xec00003d , A	},
	{ "fres" , 0xec000030 , A	},
	{ "fres." , 0xec000031 , A	},
	{ "frsp" , 0xfc000018 , X	},
	{ "frsp." , 0xfc000019 , X	},
	{ "frsqrte" , 0xfc000034 , A	},
	{ "frsqrte." , 0xfc000035 , A	},
	{ "fsel" , 0xfc00002e , A	},
	{ "fsel." , 0xfc00002f , A	},
	{ "fsqrt" , 0xfc00002c , A	},
	{ "fsqrt." , 0xfc00002d , A	},
	{ "fsqrts" , 0xec00002c , A	},
	{ "fsqrts." , 0xec00002d , A	},
	{ "fsub" , 0xfc000028 , A	},
	{ "fsub." , 0xfc000029 , A	},
	{ "fsubs" , 0xec000028 , A	},
	{ "fsubs." , 0xec000029 , A	},
	{ "icbi" , 0x7c0007ac , X	},
	{ "isync" , 0x4c00012c , XL	},
	{ "lbz" , 0x88000000 , D	},
	{ "lbzu" , 0x8c000000 , D	},
	{ "lbzux" , 0x7c0000ee , X	},
	{ "lbzx" , 0x7c0000ae , X	},
	{ "ld" , 0xe8000000 , DS	},
	{ "ldarx" , 0x7c0000a8 , X	},
	{ "ldu" , 0xe8000001 , DS	},
	{ "ldux" , 0x7c00006a , X	},
	{ "ldx" , 0x7c00002a , X	},
	{ "lfd" , 0xc8000000 , D	},
	{ "lfdu" , 0xcc000000 , D	},
	{ "lfdux" , 0x7c0004ee , X	},
	{ "lfdx" , 0x7c0004ae , X	},
	{ "lfs" , 0xc0000000 , D	},
	{ "lfsu" , 0xc4000000 , D	},
	{ "lfsux" , 0x7c00046e , X	},
	{ "lfsx" , 0x7c00042e , X	},
	{ "lha" , 0xa8000000 , D	},
	{ "lhau" , 0xac000000 , D	},
	{ "lhaux" , 0x7c0002ee , X	},
	{ "lhax" , 0x7c0002ae , X	},
	{ "lhbrx" , 0x7c00062c , X	},
	{ "lhz" , 0xa0000000 , D	},
	{ "lhzu" , 0xa4000000 , D	},
	{ "lhzux" , 0x7c00026e , X	},
	{ "lhzx" , 0x7c00022e , X	},
	{ "lmw" , 0xb8000000 , D	},
	{ "lscbx" , 0x7c00022a , X	},
	{ "lscbx." , 0x7c00022b , X	},
	{ "lswi" , 0x7c0004aa , X	},
	{ "lswx" , 0x7c00042a , X	},
	{ "lvebx" , 0x7c00000e , X	},
	{ "lvehx" , 0x7c00004e , X	},
	{ "lvewx" , 0x7c00008e , X	},
	{ "lvsl" , 0x7c00000c , X	},
	{ "lvsr" , 0x7c00004c , X	},
	{ "lvx" , 0x7c0000ce , X	},
	{ "lvxl" , 0x7c0002ce , X	},
	{ "lwa" , 0xe8000002 , DS	},
	{ "lwarx" , 0x7c000028 , X	},
	{ "lwaux" , 0x7c0002ea , X	},
	{ "lwax" , 0x7c0002aa , X	},
	{ "lwbrx" , 0x7c00042c , X	},
	{ "lwz" , 0x80000000 , D	},
	{ "lwzu" , 0x84000000 , D	},
	{ "lwzux" , 0x7c00006e , X	},
	{ "lwzx" , 0x7c00002e , X	},
	{ "maskg" , 0x7c00003a , X	},
	{ "maskg." , 0x7c00003b , X	},
	{ "maskir" , 0x7c00043a , X	},
	{ "maskir." , 0x7c00043b , X	},
	{ "mcrf" , 0x4c000000 , XL	},
	{ "mcrfs" , 0xfc000080 , X	},
	{ "mcrxr" , 0x7c000400 , X	},
	{ "mfcr" , 0x7c000026 , X	},
	{ "mffs" , 0xfc00048e , X	},
	{ "mffs." , 0xfc00048f , X	},
	{ "mfmsr" , 0x7c0000a6 , X	},
	{ "mfspr" , 0x7c0002a6 , XFX	},
	{ "mfsr" , 0x7c0004a6 , X	},
	{ "mfsrin" , 0x7c000526 , X	},
	{ "mftb" , 0x7c0002e6 , XFX	},
	{ "mfvscr" , 0x10000604 , VX	},
	{ "mtcrf" , 0x7c000120 , XFX	},
	{ "mtfsb0" , 0xfc00008c , X	},
	{ "mtfsb0." , 0xfc00008d , X	},
	{ "mtfsb1" , 0xfc00004c , X	},
	{ "mtfsb1." , 0xfc00004d , X	},
	{ "mtfsf" , 0xfc00058e , XFL	},
	{ "mtfsf." , 0xfc00058f , XFL	},
	{ "mtfsfi" , 0xfc00010c , X	},
	{ "mtfsfi." , 0xfc00010d , X	},
	{ "mtmsr" , 0x7c000124 , X	},
	{ "mtspr" , 0x7c0003a6 , XFX	},
	{ "mtsr" , 0x7c0001a4 , X	},
	{ "mtsrin" , 0x7c0001e4 , X	},
	{ "mtvscr" , 0x10000644 , VX	},
	{ "mul" , 0x7c0000d6 , XO	},
	{ "mul." , 0x7c0000d7 , XO	},
	{ "mulhd" , 0x7c000092 , XO	},
	{ "mulhd." , 0x7c000093 , XO	},
	{ "mulhdu" , 0x7c000012 , XO	},
	{ "mulhdu." , 0x7c000013 , XO	},
	{ "mulhw" , 0x7c000096 , XO	},
	{ "mulhw." , 0x7c000097 , XO	},
	{ "mulhwu" , 0x7c000016 , XO	},
	{ "mulhwu." , 0x7c000017 , XO	},
	{ "mulld" , 0x7c0001d2 , XO	},
	{ "mulld." , 0x7c0001d3 , XO	},
	{ "mulldo" , 0x7c0005d2 , XO	},
	{ "mulldo." , 0x7c0005d3 , XO	},
	{ "mulli" , 0x1c000000 , D	},
	{ "mullw" , 0x7c0001d6 , XO	},
	{ "mullw." , 0x7c0001d7 , XO	},
	{ "mullwo" , 0x7c0005d6 , XO	},
	{ "mullwo." , 0x7c0005d7 , XO	},
	{ "mulo" , 0x7c0004d6 , XO	},
	{ "mulo." , 0x7c0004d7 , XO	},
	{ "nabs" , 0x7c0003d0 , XO	},
	{ "nand" , 0x7c0003b8 , X	},
	{ "nand." , 0x7c0003b9 , X	},
	{ "neg" , 0x7c0000d0 , XO	},
	{ "neg." , 0x7c0000d1 , XO	},
	{ "nego" , 0x7c0004d0 , XO	},
	{ "nego." , 0x7c0004d1 , XO	},
	{ "nor" , 0x7c0000f8 , X	},
	{ "nor." , 0x7c0000f9 , X	},
	{ "or" , 0x7c000378 , X	},
	{ "or." , 0x7c000379 , X	},
	{ "orc" , 0x7c000338 , X	},
	{ "orc." , 0x7c000339 , X	},
	{ "ori" , 0x60000000 , D	},
	{ "oris" , 0x64000000 , D	},
	{ "rfi" , 0x4c000064 , XL	},
	{ "rldcl" , 0x78000010 , MDS	},
	{ "rldcl." , 0x78000011 , MDS	},
	{ "rldcr" , 0x78000012 , MDS	},
	{ "rldcr." , 0x78000013 , MDS	},
	{ "rldic" , 0x78000008 , MD	},
	{ "rldic." , 0x78000009 , MD	},
	{ "rldicl" , 0x78000000 , MD	},
	{ "rldicl." , 0x78000001 , MD	},
	{ "rldicr" , 0x78000004 , MD	},
	{ "rldicr." , 0x78000005 , MD	},
	{ "rldimi" , 0x7800000c , MD	},
	{ "rldimi." , 0x7800000d , MD	},
	{ "rlmi" , 0x58000000 , M	},
	{ "rlmi." , 0x58000001 , M	},
	{ "rlwimi" , 0x50000000 , M	},
	{ "rlwimi." , 0x50000001 , M	},
	{ "rlwinm" , 0x54000000 , M	},
	{ "rlwinm." , 0x54000001 , M	},
	{ "rlwnm" , 0x5c000000 , M	},
	{ "rlwnm." , 0x5c000001 , M	},
	{ "rrib" , 0x7c000432 , X	},
	{ "rrib." , 0x7c000433 , X	},
	{ "sc" , 0x44000002 , SC	},
	{ "slbia" , 0x7c0003e4 , X	},
	{ "slbie" , 0x7c000364 , X	},
	{ "slbiex" , 0x7c0003a4 , X	},
	{ "sld" , 0x7c000036 , X	},
	{ "sld." , 0x7c000037 , X	},
	{ "sle" , 0x7c000132 , X	},
	{ "sle." , 0x7c000133 , X	},
	{ "sleq" , 0x7c0001b2 , X	},
	{ "sleq." , 0x7c0001b3 , X	},
	{ "sliq" , 0x7c000170 , X	},
	{ "sliq." , 0x7c000171 , X	},
	{ "slliq" , 0x7c0001f0 , X	},
	{ "slliq." , 0x7c0001f1 , X	},
	{ "sllq" , 0x7c0001b0 , X	},
	{ "sllq." , 0x7c0001b1 , X	},
	{ "slq" , 0x7c000130 , X	},
	{ "slq." , 0x7c000131 , X	},
	{ "slw" , 0x7c000030 , X	},
	{ "slw." , 0x7c000031 , X	},
	{ "srad" , 0x7c000634 , X	},
	{ "srad." , 0x7c000635 , X	},
	{ "sradi" , 0x7c000674 , XS	},
	{ "sradi." , 0x7c000675 , XS	},
	{ "sraiq" , 0x7c000770 , X	},
	{ "sraiq." , 0x7c000771 , X	},
	{ "sraq" , 0x7c000730 , X	},
	{ "sraq." , 0x7c000731 , X	},
	{ "sraw" , 0x7c000630 , X	},
	{ "sraw." , 0x7c000631 , X	},
	{ "srawi" , 0x7c000670 , X	},
	{ "srawi." , 0x7c000671 , X	},
	{ "srd" , 0x7c000436 , X	},
	{ "srd." , 0x7c000437 , X	},
	{ "sre" , 0x7c000532 , X	},
	{ "sre." , 0x7c000533 , X	},
	{ "srea" , 0x7c000732 , X	},
	{ "srea." , 0x7c000733 , X	},
	{ "sreq" , 0x7c0005b2 , X	},
	{ "sreq." , 0x7c0005b3 , X	},
	{ "sriq" , 0x7c000570 , X	},
	{ "sriq." , 0x7c000571 , X	},
	{ "srliq" , 0x7c0005f0 , X	},
	{ "srliq." , 0x7c0005f1 , X	},
	{ "srlq" , 0x7c0005b0 , X	},
	{ "srlq." , 0x7c0005b1 , X	},
	{ "srq" , 0x7c000530 , X	},
	{ "srq." , 0x7c000531 , X	},
	{ "srw" , 0x7c000430 , X	},
	{ "srw." , 0x7c000431 , X	},
	{ "stb" , 0x98000000 , D	},
	{ "stbu" , 0x9c000000 , D	},
	{ "stbux" , 0x7c0001ee , X	},
	{ "stbx" , 0x7c0001ae , X	},
	{ "std" , 0xf8000000 , DS	},
	{ "stdcx." , 0x7c0001ad , X	},
	{ "stdu" , 0xf8000001 , DS	},
	{ "stdux" , 0x7c00016a , X	},
	{ "stdx" , 0x7c00012a , X	},
	{ "stfd" , 0xd8000000 , D	},
	{ "stfdu" , 0xdc000000 , D	},
	{ "stfdux" , 0x7c0005ee , X	},
	{ "stfdx" , 0x7c0005ae , X	},
	{ "stfiwx" , 0x7c0007ae , X	},
	{ "stfs" , 0xd0000000 , D	},
	{ "stfsu" , 0xd4000000 , D	},
	{ "stfsux" , 0x7c00056e , X	},
	{ "stfsx" , 0x7c00052e , X	},
	{ "sth" , 0xb0000000 , D	},
	{ "sthbrx" , 0x7c00072c , X	},
	{ "sthu" , 0xb4000000 , D	},
	{ "sthux" , 0x7c00036e , X	},
	{ "sthx" , 0x7c00032e , X	},
	{ "stmw" , 0xbc000000 , D	},
	{ "stswi" , 0x7c0005aa , X	},
	{ "stswx" , 0x7c00052a , X	},
	{ "stvebx" , 0x7c00010e , X	},
	{ "stvehx" , 0x7c00014e , X	},
	{ "stvewx" , 0x7c00018e , X	},
	{ "stvx" , 0x7c0001ce , X	},
	{ "stvxl" , 0x7c0003ce , X	},
	{ "stw" , 0x90000000 , D	},
	{ "stwbrx" , 0x7c00052c , X	},
	{ "stwcx." , 0x7c00012d , X	},
	{ "stwu" , 0x94000000 , D	},
	{ "stwux" , 0x7c00016e , X	},
	{ "stwx" , 0x7c00012e , X	},
	{ "subf" , 0x7c000050 , XO	},
	{ "subf." , 0x7c000051 , XO	},
	{ "subfc" , 0x7c000010 , XO	},
	{ "subfc." , 0x7c000011 , XO	},
	{ "subfco" , 0x7c000410 , XO	},
	{ "subfco." , 0x7c000411 , XO	},
	{ "subfe" , 0x7c000110 , XO	},
	{ "subfe." , 0x7c000111 , XO	},
	{ "subfeo" , 0x7c000510 , XO	},
	{ "subfeo." , 0x7c000511 , XO	},
	{ "subfic" , 0x20000000 , D	},
	{ "subfme" , 0x7c0001d0 , XO	},
	{ "subfme." , 0x7c0001d1 , XO	},
	{ "subfmeo" , 0x7c0005d0 , XO	},
	{ "subfmeo." , 0x7c0005d1 , XO	},
	{ "subfo" , 0x7c000450 , XO	},
	{ "subfo." , 0x7c000451 , XO	},
	{ "subfze" , 0x7c000190 , XO	},
	{ "subfze." , 0x7c000191 , XO	},
	{ "subfzeo" , 0x7c000590 , XO	},
	{ "subfzeo." , 0x7c000591 , XO	},
	{ "sync" , 0x7c0004ac , X	},
	{ "td" , 0x7c000088 , X	},
	{ "tdi" , 0x8000000 , D	},
	{ "tlbia" , 0x7c0002e4 , X	},
	{ "tlbie" , 0x7c000264 , X	},
	{ "tlbiex" , 0x7c0002a4 , X	},
	{ "tlbsync" , 0x7c00046c , X	},
	{ "tw" , 0x7c000008 , X	},
	{ "twi" , 0x0c000000 , D	},

	{ "vaddcuw" , 0x10000180 , VX	},
	{ "vaddfp" , 0x1000000A , VX	},
	{ "vaddsbs" , 0x10000300 , VX	},
	{ "vaddshs" , 0x10000340 , VX	},
	{ "vaddsws" , 0x10000380 , VX	},
	{ "vaddubm" , 0x10000000 , VX	},
	{ "vaddubs" , 0x10000200 , VX	},
	{ "vadduhm" , 0x10000040 , VX	},
	{ "vadduhs" , 0x10000240 , VX	},
	{ "vadduwm" , 0x10000080 , VX	},
	{ "vadduws" , 0x10000280 , VX	},
	{ "vand" , 0x10000404 , VX	},
	{ "vandc" , 0x10000444 , VX	},
	{ "vavgsb" , 0x10000502 , VX	},
	{ "vavgsh" , 0x10000542 , VX	},
	{ "vavgsw" , 0x10000582 , VX	},
	{ "vavgub" , 0x10000402 , VX	},
	{ "vavguh" , 0x10000442 , VX	},
	{ "vavguw" , 0x10000482 , VX	},
	{ "vcfsx" , 0x1000034a , VX	},
	{ "vcfux" , 0x1000030A , VX	},
	{ "vcmpbfp" , 0x100003C6 , VXR	},
	{ "vcmpbfp." , 0x100007C6 , VXR	},
	{ "vcmpeqfp" , 0x100000C6 , VXR	},
	{ "vcmpeqfp." , 0x100004C6 , VXR	},
	{ "vcmpequb" , 0x10000006 , VXR	},
	{ "vcmpequb." , 0x10000406 , VXR	},
	{ "vcmpequh" , 0x10000046 , VXR	},
	{ "vcmpequh." , 0x10000446 , VXR	},
	{ "vcmpequw" , 0x10000086 , VXR	},
	{ "vcmpequw." , 0x10000486 , VXR	},
	{ "vcmpgefp" , 0x100001C6 , VXR	},
	{ "vcmpgefp." , 0x100005C6 , VXR	},
	{ "vcmpgtfp" , 0x100002C6 , VXR	},
	{ "vcmpgtfp." , 0x100006C6 , VXR	},
	{ "vcmpgtsb" , 0x10000306 , VXR	},
	{ "vcmpgtsb." , 0x10000706 , VXR	},
	{ "vcmpgtsh" , 0x10000346 , VXR	},
	{ "vcmpgtsh." , 0x10000746 , VXR	},
	{ "vcmpgtsw" , 0x10000386 , VXR	},
	{ "vcmpgtsw." , 0x10000786 , VXR	},
	{ "vcmpgtub" , 0x10000206 , VXR	},
	{ "vcmpgtub." , 0x10000606 , VXR	},
	{ "vcmpgtuh" , 0x10000246 , VXR	},
	{ "vcmpgtuh." , 0x10000646 , VXR	},
	{ "vcmpgtuw" , 0x10000286 , VXR	},
	{ "vcmpgtuw." , 0x10000686 , VXR	},
	{ "vctsxs" , 0x100003CA , VX	},
	{ "vctuxs" , 0x1000038A , VX	},
	{ "vexptefp" , 0x1000018a , VX	},
	{ "vlogefp" , 0x100001CA , VX	},
	{ "vmaddfp" , 0x1000002E , VA	},
	{ "vmaxfp" , 0x1000040A , VX	},
	{ "vmaxsb" , 0x10000102 , VX	},
	{ "vmaxsh" , 0x10000142 , VX	},
	{ "vmaxsw" , 0x10000182 , VX	},
	{ "vmaxub" , 0x10000002 , VX	},
	{ "vmaxuh" , 0x10000042 , VX	},
	{ "vmaxuw" , 0x10000082 , VX	},
	{ "vmhaddshs" , 0x10000020 , VA	},
	{ "vmhraddshs" , 0x10000021 , VA	},
	{ "vminfp" , 0x1000044A , VX	},
	{ "vminsb" , 0x10000302 , VX	},
	{ "vminsh" , 0x10000342 , VX	},
	{ "vminsw" , 0x10000382 , VX	},
	{ "vminub" , 0x10000202 , VX	},
	{ "vminuh" , 0x10000242 , VX	},
	{ "vminuw" , 0x10000282 , VX	},
	{ "vmladduhm" , 0x10000022 , VA	},
	{ "vmr" , 0x10000484 , smVX	},
	{ "vmrghb" , 0x1000000C , VX	},
	{ "vmrghh" , 0x1000004C , VX	},
	{ "vmrghw" , 0x1000008C , VX	},
	{ "vmrglb" , 0x1000010C , VX	},
	{ "vmrglh" , 0x1000014C , VX	},
	{ "vmrglw" , 0x1000018C , VX	},
	{ "vmsummbm" , 0x10000025 , VA	},
	{ "vmsumshm" , 0x10000028 , VA	},
	{ "vmsumshs" , 0x10000029 , VA	},
	{ "vmsumubm" , 0x10000024 , VA	},
	{ "vmsumuhm" , 0x10000026 , VA	},
	{ "vmsumuhs" , 0x10000027 , VA	},
	{ "vmulesb" , 0x10000308 , VX	},
	{ "vmulesh" , 0x10000348 , VX	},
	{ "vmuleub" , 0x10000208 , VX	},
	{ "vmuleuh" , 0x10000248 , VX	},
	{ "vmulosb" , 0x10000108 , VX	},
	{ "vmulosh" , 0x10000148 , VX	},
	{ "vmuloub" , 0x10000008 , VX	},
	{ "vmulouh" , 0x10000048 , VX	},
	{ "vnmsubfp" , 0x1000002F , VA	},
	{ "vnor" , 0x10000504 , VX	},
	{ "vnot" , 0x10000504 , smVX	},
	{ "vor" , 0x10000484 , VX	},
	{ "vperm" , 0x1000002B , VA	},
	{ "vpkpx" , 0x1000030E , VX	},
	{ "vpkshss" , 0x1000018E , VX	},
	{ "vpkshus" , 0x1000010E , VX	},
	{ "vpkswss" , 0x100001CE , VX	},
	{ "vpkswus" , 0x1000014E , VX	},
	{ "vpkuhum" , 0x1000000E , VX	},
	{ "vpkuhus" , 0x1000008E , VX	},
	{ "vpkuwum" , 0x1000004E , VX	},
	{ "vpkuwus" , 0x100000CE , VX	},
	{ "vrefp" , 0x1000010A , VX	},
	{ "vrfim" , 0x100002CA , VX	},
	{ "vrfin" , 0x1000020A , VX	},
	{ "vrfip" , 0x1000028A , VX	},
	{ "vrfiz" , 0x1000024A , VX	},
	{ "vrlb" , 0x10000004 , VX	},
	{ "vrlh" , 0x10000044 , VX	},
	{ "vrlw" , 0x10000084 , VX	},
	{ "vrsqrtefp" , 0x1000014A , VX	},
	{ "vsel" , 0x1000002A , VA	},
	{ "vsl" , 0x100001C4 , VX	},
	{ "vslb" , 0x10000104 , VX	},
	{ "vsldoi" , 0x1000002C , VA	},
	{ "vslh" , 0x10000144 , VX	},
	{ "vslo" , 0x1000040C , VX	},
	{ "vslw" , 0x10000184 , VX	},
	{ "vspltb" , 0x1000020C , VX	},
	{ "vsplth" , 0x1000024C , VX	},
	{ "vspltisb" , 0x1000030C , VX	},
	{ "vspltish" , 0x1000034C , VX	},
	{ "vspltisw" , 0x1000038C , VX	},
	{ "vspltw" , 0x1000028C , VX	},
	{ "vsr" , 0x100002C4 , VX	},
	{ "vsrab" , 0x10000304 , VX	},
	{ "vsrah" , 0x10000344 , VX	},
	{ "vsraw" , 0x10000384 , VX	},
	{ "vsrb" , 0x10000204 , VX	},
	{ "vsrh" , 0x10000244 , VX	},
	{ "vsro" , 0x1000044C , VX	},
	{ "vsrw" , 0x10000284 , VX	},
	{ "vsubcuw" , 0x10000580 , VX	},
	{ "vsubfp" , 0x1000004A , VX	},
	{ "vsubsbs" , 0x10000700 , VX	},
	{ "vsubshs" , 0x10000740 , VX	},
	{ "vsubsws" , 0x10000780 , VX	},
	{ "vsububm" , 0x10000400 , VX	},
	{ "vsububs" , 0x10000600 , VX	},
	{ "vsubuhm" , 0x10000440 , VX	},
	{ "vsubuhs" , 0x10000640 , VX	},
	{ "vsubuwm" , 0x10000480 , VX	},
	{ "vsubuws" , 0x10000680 , VX	},
	{ "vsum2sws" , 0x10000688 , VX	},
	{ "vsum4sbs" , 0x10000708 , VX	},
	{ "vsum4shs" , 0x10000648 , VX	},
	{ "vsum4ubs" , 0x10000608 , VX	},
	{ "vsumsws" , 0x10000788 , VX	},
	{ "vupkhpx" , 0x1000034E , VX	},
	{ "vupkhsb" , 0x1000020E , VX	},
	{ "vupkhsh" , 0x1000024E , VX	},
	{ "vupklpx" , 0x100003CE , VX	},
	{ "vupklsb" , 0x1000028E , VX	},
	{ "vupklsh" , 0x100002CE , VX	},
	{ "vxor" , 0x100004C4 , VX	},
	{ "xor" , 0x7c000278 , X	},
	{ "xor." , 0x7c000279 , X	},
	{ "xori" , 0x68000000 , D	},
	{ "xoris" , 0x6c000000 , D	}
};

int number_of_mnemonics;


int test_branch_label( PPCSIM *ppcsim, ULONG opcode, ULONG addr, ULONG tmp, char *label );

/*

asm_dasm_element ops_and_mnemonics[];
*/


/* command operand field of 32-bit instruction */
/*
static unsigned int BA ( unsigned int inst )
{
    return		 ((inst >> 16) & 0x1F);
}

static unsigned int BB (unsigned long inst )
{
		 return ((inst >> 11) & 0x1F);
}

static unsigned int BD (unsigned long inst )
{
		 return (inst & 0x0000FFFC);
}
	 
static unsigned int BF  ( unsigned long inst )
{
		 return ((inst >> 23) & 0x7);
}
	 
static unsigned int BFA (unsigned long inst )
{
		 return ((inst >> 18) & 0x7);
}
	 
static unsigned int BI  (unsigned long inst )
	 {
		 return ((inst >> 16) & 0x1F);
	 }
	 
static unsigned int BT  (unsigned long inst )
	 {
		 return ((inst >> 21) & 0x1F);
	 }
	 
static unsigned int D  (unsigned long inst )
	 {
		 return (inst & 0x0000FFFF);
	 }
	 
static unsigned int FLM ( unsigned long inst )
	 {
		 return ((inst >> 17) & 0x0F);
	 }
	 
static unsigned int FRC ( unsigned long inst )
	 {
		 return ((inst >>  6) & 0x1F);
	 }
	 
static unsigned int FRS ( unsigned long inst )
	 {
		 return ((inst >> 21) & 0x1F);
	 }
	 
static unsigned int FXM ( unsigned long inst )
	 {
		 return ((inst >> 12) & 0x0FF);
	 }
	 
static unsigned int LI  ( unsigned long inst )
	 {
		 return (inst & 0x03FFFFFC);
	 }
	 
static unsigned int MB  ( unsigned long inst )
	 {
		 return ((inst >>  6) & 0x1F);
	 }
	 
static unsigned int ME  ( unsigned long inst )
	 {
		 return ((inst >>  1) & 0x1F);
	 }
*/	 
static unsigned int RA  ( unsigned long inst )
	 {
		 return ((inst >> 16) & 0x1F);
	 }
	 
static unsigned int RB  ( unsigned long inst )
	 {
		 return ((inst >> 11) & 0x1F);
	 }
/*	 
static unsigned int RS  ( unsigned long inst )
	 {
		 return ((inst >> 21) & 0x1F);
	 }
	 
static unsigned int RT  ( unsigned long inst )
	 {
		 return ((inst >> 21) & 0x1F);
	 }
	 
static unsigned int SH  ( unsigned long inst )
	 {
		 return ((inst >> 11) & 0x1F);
	 }
	 
static unsigned int  SI  ( unsigned long inst )
	 {
		 return (inst & 0x0000FFFF);
	 }
	 
static unsigned  int TO  ( unsigned long inst )
	 {
		 return ((inst >> 21) & 0x1F);
	 }
	 
static unsigned int  U  ( unsigned long inst )
	 {
		 return ((inst >> 12) & 0x0F);
	 }
	 
static unsigned int UI  ( unsigned long inst )
	 {
		 return (inst & 0x0000FFFF);
	 }
	 
static unsigned int SR  ( unsigned long inst )
	 {
		 return ((inst >> 16) & 0x0000000F);
	 }
	 
static unsigned int SPR ( unsigned long inst )
	 {
		 return ((inst >> 11) & 0x3FF);
	 }
	 
static unsigned int BO  ( unsigned long inst )
	 {
		 return ((inst >> 21) & 0x1F);
	 }
*/
	 
/* new VMX fields */	 
static unsigned int VT6_10 ( unsigned long inst )
	 {
		 return ((inst >> 21) & 0x1F);
	 }
/* unreferenced function 
  
static unsigned int VS6_10 ( unsigned long inst )
	 {
		 return ((inst >> 21) & 0x1F);
	 }
*/	 
static unsigned int VA11_15 ( unsigned long inst )
	 {
		 return ((inst >> 16) & 0x1F);
	 }
	 
static unsigned int IMM11_15 ( unsigned long inst )
	 {
		 return ((inst >> 16) & 0x1F);
	 }
	 
static unsigned int VB16_20 ( unsigned long inst )
	 {
		 return ((inst >> 11) & 0x1F);
	 }
	 
static unsigned int VC21_25 ( unsigned long inst )
	 {
		 return ((inst >> 6) & 0x1F);
	 }
	 
static unsigned int SH22_25 ( unsigned long inst )
	 {
		 return ((inst >> 6) & 0x0F);
	 }
	 
static unsigned int IMM9_10 ( unsigned long inst )
	 {
		 return ((inst >> 21) & 0x03);
	 }

ALTVEC_DISASS_STRUC altvec_tbl[] =
{
 /* mask, opcode, num_opa, fmt, opa_func_1, opa_func_2, opa_func_3, opa_func_4 */
 { 0xFC00003F, 0x1000002B, 4, "vperm     V%d,V%d,V%d,V%d",{ VT6_10,VA11_15,VB16_20,VC21_25 }},
 { 0xFC0007FF, 0x1000000E, 3, "vpkuhum   V%d,V%d,V%d",{ VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000004E, 3, "vpkuwum   V%d,V%d,V%d",{ VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000008E, 3, "vpkuhus   V%d,V%d,V%d",{ VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100000CE, 3, "vpkuwus   V%d,V%d,V%d",{ VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000010E, 3, "vpkshus   V%d,V%d,V%d",{ VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000014E, 3, "vpkswus   V%d,V%d,V%d",{ VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000018E, 3, "vpkshss   V%d,V%d,V%d",{ VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100001CE, 3, "vpkswss   V%d,V%d,V%d",{ VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000020E, 2, "vupkhsb   V%d,V%d",{ VT6_10,VB16_20, 0,0 }},
 { 0xFC0007FF, 0x1000024E, 2, "vupkhsh   V%d,V%d",{ VT6_10,VB16_20, 0,0 }},
 { 0xFC0007FF, 0x1000028E, 2, "vupklsb   V%d,V%d",{ VT6_10,VB16_20, 0,0 }},
 { 0xFC0007FF, 0x100002CE, 2, "vupklsh   V%d,V%d",{ VT6_10,VB16_20, 0,0 }},
 { 0xFC0007FF, 0x1000030E, 3, "vpkpx     V%d,V%d,V%d",{ VT6_10,VA11_15,VB16_20,0 }},
 { 0xFC0007FF, 0x1000034E, 2, "vupkhpx   V%d,V%d",{ VT6_10,VB16_20, 0, 0 }},
 { 0xFC0007FF, 0x100003CE, 2, "vupklpx   V%d,V%d",{ VT6_10,VB16_20, 0, 0 }},
 { 0xFC0007FF, 0x1000000C, 3, "vmrghb    V%d,V%d,V%d", { VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000004C, 3, "vmrghh    V%d,V%d,V%d", { VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000008C, 3, "vmrghw    V%d,V%d,V%d", { VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000010C, 3, "vmrglb    V%d,V%d,V%d", { VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000014C, 3, "vmrglh    V%d,V%d,V%d", { VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000018C, 3, "vmrglw    V%d,V%d,V%d", { VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000020C, 3, "vspltb    V%d,V%d,0x%x", { VT6_10,VB16_20,IMM11_15, 0}},
 { 0xFC0007FF, 0x1000024C, 3, "vsplth    V%d,V%d,0x%x", { VT6_10,VB16_20,IMM11_15, 0 }},
 { 0xFC0007FF, 0x1000028C, 3, "vspltw    V%d,V%d,0x%x", { VT6_10,VB16_20,IMM11_15, 0 }},
 { 0xFC0007FF, 0x1000030C, 2, "vspltisb  V%d,0x%x",{VT6_10,IMM11_15, 0, 0 }},
 { 0xFC0007FF, 0x1000034C, 2, "vspltish  V%d,0x%x",{VT6_10,IMM11_15, 0, 0 }},
 { 0xFC0007FF, 0x1000038C, 2, "vspltisw  V%d,0x%x",{VT6_10,IMM11_15, 0, 0 }},
 { 0xFC00003F, 0x1000002C, 4, "vsldoi    V%d,V%d,V%d,0x%x",{VT6_10,VA11_15,VB16_20,SH22_25}},
 { 0xFC0007FF, 0x1000040C, 3, "vslo      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0}},
 { 0xFC0007FF, 0x1000044C, 3, "vsro      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0}},
 { 0xFC00003F, 0x10000022, 4, "vmladduhm V%d,V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20,VC21_25 }},
 { 0xFC00003F, 0x10000024, 4, "vmsumubm  V%d,V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20,VC21_25 }},
 { 0xFC00003F, 0x10000025, 4, "vmsummbm  V%d,V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20,VC21_25 }},
 { 0xFC00003F, 0x10000026, 4, "vmsumuhm  V%d,V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20,VC21_25 }},
 { 0xFC00003F, 0x10000027, 4, "vmsumuhs  V%d,V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20,VC21_25 }},
 { 0xFC00003F, 0x10000028, 4, "vmsumshm  V%d,V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20,VC21_25 }},
 { 0xFC00003F, 0x10000029, 4, "vmsumshs  V%d,V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20,VC21_25 }},
 { 0xFC00003F, 0x10000020, 4, "vmhaddshs V%d,V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20,VC21_25 }},
 { 0xFC00003F, 0x10000021, 4, "vmhraddshs V%d,V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20,VC21_25 }},
 { 0xFC0007FF, 0x10000788, 3, "vsumsws   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000688, 3, "vsum2sws  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000708, 3, "vsum4sbs  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000608, 3, "vsum4ubs  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000648, 3, "vsum4shs  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000008, 3, "vmuloub   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000108, 3, "vmulosb   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000048, 3, "vmulouh   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000148, 3, "vmulosh   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000208, 3, "vmuleub   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000308, 3, "vmulesb   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000248, 3, "vmuleuh   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000348, 3, "vmulesh   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000000, 3, "vaddubm   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000200, 3, "vaddubs   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000300, 3, "vaddsbs   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000040, 3, "vadduhm   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000240, 3, "vadduhs   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000340, 3, "vaddshs   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000080, 3, "vadduwm   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000280, 3, "vadduws   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000380, 3, "vaddsws   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000400, 3, "vsububm   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000600, 3, "vsububs   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000700, 3, "vsubsbs   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000440, 3, "vsubuhm   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000640, 3, "vsubuhs   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000740, 3, "vsubshs   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000480, 3, "vsubuwm   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000680, 3, "vsubuws   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000780, 3, "vsubsws   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000180, 3, "vaddcuw   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000580, 3, "vsubcuw   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000402, 3, "vavgub    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000502, 3, "vavgsb    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }}, 
 { 0xFC0007FF, 0x10000442, 3, "vavguh    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000542, 3, "vavgsh    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000482, 3, "vavguw    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000582, 3, "vavgsw    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000002, 3, "vmaxub    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000102, 3, "vmaxsb    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000042, 3, "vmaxuh    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }}, 
 { 0xFC0007FF, 0x10000142, 3, "vmaxsh    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }}, 
 { 0xFC0007FF, 0x10000082, 3, "vmaxuw    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }}, 
 { 0xFC0007FF, 0x10000182, 3, "vmaxsw    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000202, 3, "vminub    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000302, 3, "vminsb    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000242, 3, "vminuh    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000342, 3, "vminsh    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000282, 3, "vminuw    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000382, 3, "vminsw    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000006, 3, "vcmpequb  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000046, 3, "vcmpequh  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000086, 3, "vcmpequw  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000206, 3, "vcmpgtub  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000306, 3, "vcmpgtsb  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000246, 3, "vcmpgtuh  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000346, 3, "vcmpgtsh  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000286, 3, "vcmpgtuw  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000386, 3, "vcmpgtsw  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000406, 3, "vcmpequb. V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000446, 3, "vcmpequh. V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000486, 3, "vcmpequw. V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000606, 3, "vcmpgtub. V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000706, 3, "vcmpgtsb. V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000646, 3, "vcmpgtuh. V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000746, 3, "vcmpgtsh. V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000686, 3, "vcmpgtuw. V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000786, 3, "vcmpgtsw. V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000004, 3, "vrlb      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000044, 3, "vrlh      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000084, 3, "vrlw      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000104, 3, "vslb      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000144, 3, "vslh      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000184, 3, "vslw      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100001C4, 3, "vsl       V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000204, 3, "vsrb      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000304, 3, "vsrab     V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000244, 3, "vsrh      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000344, 3, "vsrah     V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000284, 3, "vsrw      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000384, 3, "vsraw     V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100002C4, 3, "vsr       V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000404, 3, "vand      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000444, 3, "vandc     V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000484, 3, "vor       V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100004C4, 3, "vxor      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x10000504, 3, "vnor      V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC00003F, 0x1000002A, 4, "vsel      V%d,V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20,VC21_25}},
 { 0xFC0007FF, 0x10000604, 1, "mfvscr    V%d",{VT6_10, 0, 0, 0 }},
 { 0xFC0007FF, 0x10000644, 1, "mtvscr    V%d",{VB16_20, 0, 0, 0 }},
 { 0xFC0007FF, 0x1000040A, 3, "vmaxfp    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000044A, 3, "vminfp    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100000C6, 3, "vcmpeqfp  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100002C6, 3, "vcmpgtfp  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100001C6, 3, "vcmpgefp  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100003C6, 3, "vcmpbfp   V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100004C6, 3, "vcmpeqfp. V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100006C6, 3, "vcmpgtfp. V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100005C6, 3, "vcmpgefp. V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x100007C6, 3, "vcmpbfp.  V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC00003F, 0x1000002E, 4, "vmaddfp   V%d,V%d,V%d,V%d",{VT6_10,VA11_15,VC21_25,VB16_20 }},
 { 0xFC00003F, 0x1000002F, 4, "vnmsubfp  V%d,V%d,V%d,V%d",{VT6_10,VA11_15,VC21_25,VB16_20 }},
 { 0xFC0007FF, 0x1000000A, 3, "vaddfp    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000004A, 3, "vsubfp    V%d,V%d,V%d",{VT6_10,VA11_15,VB16_20, 0 }},
 { 0xFC0007FF, 0x1000010A, 2, "vrefp     V%d,V%d",{VT6_10,VB16_20, 0, 0 }},
 { 0xFC0007FF, 0x1000014A, 2, "vrsqrefp  V%d,V%d",{VT6_10,VB16_20, 0, 0 }},
 { 0xFC0007FF, 0x1000018A, 2, "vexptefp  V%d,V%d",{VT6_10,VB16_20, 0, 0 }},
 { 0xFC0007FF, 0x100001CA, 2, "vlogefp   V%d,V%d",{VT6_10,VB16_20, 0, 0 }},
 { 0xFC0007FF, 0x1000020A, 2, "vrfin     V%d,V%d",{VT6_10,VB16_20, 0, 0 }},
 { 0xFC0007FF, 0x1000024A, 2, "vrfiz     V%d,V%d",{VT6_10,VB16_20, 0, 0 }},
 { 0xFC0007FF, 0x1000028A, 2, "vrfip     V%d,V%d",{VT6_10,VB16_20, 0, 0 }},
 { 0xFC0007FF, 0x100002CA, 2, "vrfim     V%d,V%d",{VT6_10,VB16_20, 0, 0 }},
 { 0xFC0007FF, 0x1000030A, 3, "vcfux     V%d,V%d,0x%x",{VT6_10,VB16_20,IMM11_15, 0 }},
 { 0xFC0007FF, 0x1000034A, 3, "vcfsx     V%d,V%d,0x%x",{VT6_10,VB16_20,IMM11_15, 0 }},
 { 0xFC0007FF, 0x1000038A, 3, "vctuxs    V%d,V%d,0x%x",{VT6_10,VB16_20,IMM11_15, 0 }},
 { 0xFC0007FF, 0x100003CA, 3, "vctsxs    V%d,V%d,0x%x",{VT6_10,VB16_20,IMM11_15, 0 }},
 { 0xFC0007FE, 0x7C00000E, 3, "lvebx     V%d,R%d,R%d",{VT6_10,RA,RB, 0 }},
 { 0xFC0007FE, 0x7C00004E, 3, "lvehx     V%d,R%d,R%d",{VT6_10,RA,RB, 0 }},
 { 0xFC0007FE, 0x7C00008E, 3, "lvewx     V%d,R%d,R%d",{VT6_10,RA,RB, 0 }},
 { 0xFC0007FE, 0x7C0000CE, 3, "lvx       V%d,R%d,R%d",{VT6_10,RA,RB, 0 }},
 { 0xFC0007FE, 0x7C0002CE, 3, "lvxl      V%d,R%d,R%d",{VT6_10,RA,RB, 0 }},
 { 0xFC0007FE, 0x7C00010E, 3, "stvebx    V%d,R%d,R%d",{VT6_10,RA,RB, 0 }},
 { 0xFC0007FE, 0x7C00014E, 3, "stvehx    V%d,R%d,R%d",{VT6_10,RA,RB, 0 }},
 { 0xFC0007FE, 0x7C00018E, 3, "stvewx    V%d,R%d,R%d",{VT6_10,RA,RB, 0 }},
 { 0xFC0007FE, 0x7C0001CE, 3, "stvx      V%d,R%d,R%d",{VT6_10,RA,RB, 0 }},
 { 0xFC0007FE, 0x7C0003CE, 3, "stvxl     V%d,R%d,R%d",{VT6_10,RA,RB, 0 }},
 { 0xFC0007FE, 0x7C00000C, 3, "lvsl      V%d,R%d,R%d",{VT6_10,RA,RB, 0 }},
 { 0xFC0007FE, 0x7C00004C, 3, "lvsr      V%d,R%d,R%d",{VT6_10,RA,RB, 0 }},
 { 0xFE0007FE, 0x7C0002AC, 3, "dst       R%d,R%d,%d",{RA,RB,IMM9_10, 0 }}, 
 { 0xFE0007FE, 0x7E0002AC, 3, "dstt      R%d,R%d,%d",{RA,RB,IMM9_10, 0 }}, 
 { 0xFE0007FE, 0x7C0002EC, 3, "dstst     R%d,R%d,%d",{RA,RB,IMM9_10, 0 }}, 
 { 0xFE0007FE, 0x7E0002EC, 3, "dststt    R%d,R%d,%d",{RA,RB,IMM9_10, 0 }},
 { 0xFE0007FE, 0x7C00066C, 1, "dss       %d",        {IMM9_10, 0, 0, 0}},
 { 0xFE0007FE, 0x7E00066C, 0, "dssall",              { 0, 0, 0, 0}},
	
 { 0,		   0,		   0, "",		              {0, 0, 0, 0 }},
};


//--------------------------------------------------------------------------------
// is_altivec -- return the index of altvec_tbl if the given 32-bit 
//				 instruction is altvec instruction. Otherwise return -1.
//--------------------------------------------------------------------------------
static int is_altivec( ULONG opcode )
{
	int i;
	int j = -1;
	
	for (i = 0; altvec_tbl[i].mask != 0; i++) {
        if ((opcode & altvec_tbl[i].mask) == altvec_tbl[i].opcode) {
		   j = i;
           break;
		}
	}

	return( j );
}


//--------------------------------------------------------------------------------
// disassemble_altvec -- disassemble an altvec instruction based on its
//						 instruction format in the altvec_tbl
//--------------------------------------------------------------------------------
static void disassemble_altvec( ULONG opcode, int altvec_idx )
{
	int i;
	unsigned int opa[4];

	for (i = 0; i < altvec_tbl[altvec_idx].num_opa; i++) {
		if (altvec_tbl[altvec_idx].opa_func[i] != 0)
			opa[i] = (*(altvec_tbl[altvec_idx].opa_func[i]))( opcode );
		else
			opa[i] = 0;
	}

	switch (altvec_tbl[altvec_idx].num_opa) {
	case 0:
		printf( (unsigned char *) altvec_tbl[altvec_idx].fmt );
        break;
	case 1:
		printf( altvec_tbl[altvec_idx].fmt, opa[0] );
	    break;
	case 2:
		printf( altvec_tbl[altvec_idx].fmt, opa[0],opa[1] );
		break;
	case 3:
		printf( altvec_tbl[altvec_idx].fmt, opa[0], opa[1], opa[2] );
		break;
	default:
		printf( altvec_tbl[altvec_idx].fmt, opa[0],opa[1],opa[2],opa[3] );
		break;
	}
}
	

//--------------------------------------------------------------------------------

/* --------------------------------------------------------------------- */
/*

function:	disassemble
purpose:	This function will display a disassembled version of
		the memory space requested.
inputs:		Two parameters are sent in: start_addr and end_addr.
		The caller is telling us to display data at an adddress in
		memory and to format what we find there as a PowerPC mneumonic.
		start_addr:	This is the starting address in memory
				where we will begin to disassemble.
		end_addr:	This is the ending address in memory
				where we will stop disassembling.
outputs:	None.
return:		A virtual pluthera of status codes.  Just kidding.  I'll
		put the possible status codes up here when I've finished
		coding and decide what codes are appropriate.
mod history:	6/1/93	MH	Modified disassembler to recognize little-
				endian based opcodes.  This is envoked by
				a "define" in asm_dsm.h.
		2/2/93	MH

*/
//--------------------------------------------------------------------------------


// Disassembler assistants...

#ifdef DASM_LIKE_GCC
static char dasm_R_label = 'r';
static char dasm_F_label = 'f';
#else // DASM_LIKE_MOT
static char dasm_R_label = 'R';
static char dasm_F_label = 'F';
#endif

void pGPR (int n) { printf( "%c%d", dasm_R_label, n ); }
void pGPRc(int n) { printf( "%c%d,", dasm_R_label, n ); }
void pGPRi(int n) { printf( "(%c%d)", dasm_R_label, n ); }
void pFPR (int n) { printf( "%c%d",  dasm_F_label, n ); }
void pFPRc(int n) { printf( "%c%d,", dasm_F_label, n ); }
void pCRc (int n) { if (n) printf( "cr%d,",  n ); }
void p02X (int n) { printf( "0x%02X", n ); }
void p02Xc(int n) { printf( "0x%02X,", n ); }
void pDR  (int n, int d)
{

	if (-16 < d  &&  d < 16)
		printf( "%d", d );
	else
		printf( "0x%X", d );
	pGPRi(n);
}

//---------------------------------------------------------------------------
//
//	function:	disassemble_an_opcode
//	purpose:	This function will disassemble an opcode that's sent in.
//			It will print out the disassembled version of the opcode
//			BUT will NOT print a carrage return at the end of line.
//			It's up to the caller to do that.
//	inputs:		current add:	This is the current address of the PC 
//			opcode:		This is the 32-bit opcode that we will
//					disassemble.
//	outputs:	None:
//	return:		None.
//	mod history:	2/2/93	MH
//			10/1/95 MM added simplified mnemonics
//---------------------------------------------------------------------------
int disassemble_an_opcode( PPCSIM *ppcsim, ULONG opcode, ULONG current_add )
{
	int		index, count, status;
	ULONG	tmp_long;
	short	rA, rB, rS;
	char   *s, mark;
	char 		new_label[20];
	char		mnemonic[80];
	asm_dasm_element *opi;


// Print a branch label, if any.
//
	new_label[0] = 0;
	if ((s = symbol_get_byaddr( ppcsim, current_add )) != NULL) {
		strcpy(new_label, s);
		strcat(new_label, ":");
	}

// Print label/address and opcode.  Optionally mark current location with '>'.
//
	mark = ' ';
	printf("%-16s %1c%08X  %08X   ", new_label, 
				(current_add == ppcsim->CIA) ? mark : ' ',
				 current_add, opcode);
	fflush(stdout);


// If this is an AltiVec opcode, use the AltiVec disassembler;
// otherwise, continue on with the standard one.

	if (((opcode >> 26) & 0x3F) == 4
	||  ((opcode >> 26) & 0x3F) == 31) {
		if ((index = is_altivec( opcode ) ) >= 0 ) {
			disassemble_altvec( opcode, index );
			return( 0 );
		}
	}


// Extract information matching the given opcode.  If no match, print as data.
//
	index  = 0;
	if ((status = search_ops_and_mnemonics( opcode, mnemonic, &index )) != 0) {
		printf( ".WORD     0x%08X", opcode );
		fflush( stdout );
		return( 0 );
	}


// Precalculate interesting fields that may or may not be used later.
//
	opi = &ops_and_mnemonics[index];
	rA  = EXTRACT_RA( opcode );			// aka: FRA BI
	rB  = EXTRACT_RB( opcode );			// aka: NB
	rS  = EXTRACT_RS( opcode );			// aka: RS RT FRT BO TO



			/* If I get here, I have a match...now I
			   need to determine if I have a simplified
			   branch mnemonic or not...if not then skip
			   the next chunk...else, I can't just take
			   the first simplified mnemonic I get...I
			   need to break it up a little bit and get
			   a better simplified mnemonic.
			*/
	if (opi->form >= smBC &&
		opi->form <= (smBCCTRL+4) )
	{
			/* I've got a simplified branch mnemonic!!
			   Now, look at bits 11-15 and make a call 
			   to search the mnemonic list again but 
			   this time search for a match on the NEW FORM
			   AND the bits HEX opcode!! 
			   Call a new function (do_simplified_branch) to
			   do the work!!
			*/
		do_simplified_branch( ppcsim, opcode, index, current_add); 
		fflush( stdout );
		return( 0 );
	}


// Print the mnemonic.

	printf( "%-10s", opi->mnemonic );
		

// An "I" form instruction.
// A "B" form instructions

	if (opi->form == I
	||  opi->form == I) {
		if (opi->form == I)
			tmp_long = EXTRACT_LI( opcode );
		else {
			p02Xc( rS );						// BO
			p02Xc( rA );						// BI
			tmp_long = EXTRACT_BD( opcode );
		}

		tmp_long = test_branch_label( ppcsim, opcode, current_add, tmp_long, 
									  &new_label[0] );
		if (tmp_long == 0) {
			printf( "%s", new_label );
			/*
			if ((s = symbol_get_byaddr( ppcsim, current_add )) != NULL) {
			if ((tmp_long = symbol_get_byval( ppcsim, new_label )) != NULL) {
				strcpy(new_label, s);
				strcat(new_label, ":");
			}

			for (count = 0;count < MAXBRANCHLABEL; count++) {
				if (!strcmp(label_table[count].name,new_label)) {
					printf( "\t<0x%02lX>",label_table[count].address );
					break;
				}
			}
			*/
		}

		// Just a value.
		else
			 printf( "0x%02X",tmp_long);
	}

// SC form instructions have no arguments

	else if (opi->form == SC)
		;

// A "D" form instruction

	else if (opi->form == D || opi->form == smD) {
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
	
		tmp_long = EXTRACT_PRIMARY( opcode );
		if (( tmp_long <= 29 ) &&
			( tmp_long >= 2  )) {
		
			/* We've got all the 1 and 1a catagories
			   in here.  Now we need to seperate those
			   two catagories.
			*/
			/* if it's NOP instruction do nothing! */
			if ( tmp_long == 24 &&
							opi->form == smD &&
			strncmp(&opi->mnemonic[0],
			"nop",3) == 0 
			   )
			{
			} 
			else if (( tmp_long == 11 ) ||
					 ( tmp_long == 10 )) {		// type D/smD -- 1a insts.
				tmp_long = EXTRACT_BF( opcode );
				if (opi->form == smD)
					pCRc( tmp_long );
				else 
					printf( "%01d,",(int)(tmp_long));

				/* MATT simplified mnemonic!... */
				/* cmplwi/cmpwi */
				if (opi->form != smD) {
					tmp_long = EXTRACT_L( opcode );
					printf("%01d,",(int)(tmp_long));
				}
				pGPRc( rA );
			}
			else if (( tmp_long == 2 ) ||
					 ( tmp_long == 3 )) {		// type D/smD -- 1b insts.
				if (opi->form != smD)
					p02Xc( rS );				// TO
				pGPRc( rA );
			}
			else if (( tmp_long == 28 ) ||
					 ( tmp_long == 29 ) ||
					 ( tmp_long == 24 ) || 
					 ( tmp_long == 25 ) ||
					 ( tmp_long == 26 ) ||
					 ( tmp_long == 27 )) {		 // type D/smD -- 1c insts.
				pGPRc( rA );
				pGPRc( rS );
			}
			else {								// type D/smD -- 1. insts.
				pGPRc( rS );
				if (opi->form != smD || opi->mnemonic[0] != 'l')
					pGPRc( rA );
			}

// Now take care of the last operand of types 1, 1a, and 1b instructions.
			
			tmp_long = EXTRACT_SI( opcode );

			// if it's NOP instruction do nothing!

			if (strncmp(&opi->mnemonic[0], "nop",3)==0)
				;
			else if(opi->form == smD && 
					EXTRACT_PRIMARY(opcode) >= 12 &&
					EXTRACT_PRIMARY(opcode) <= 15 &&
					opi->mnemonic[0] != 'l') {
				printf( "0x%X",( (-tmp_long)&0x00ffff) );
			} 
			else {
				printf( "0x%X", (tmp_long&0x00ffff) );
				if(opi->form==smD &&
				strncmp(&
				opi->mnemonic[0],
				"la",2)==0
				  ) {
					pGPRi( rA );
				}
			}
		}

// type D/smD -- form 2/2a instructions.

		else {
			if (( tmp_long == 50 ) ||
				( tmp_long == 51 ) ||
				( tmp_long == 48 ) ||
				( tmp_long == 49 ) ||
				( tmp_long == 54 ) ||
				( tmp_long == 55 ) ||
				( tmp_long == 52 ) ||
				( tmp_long == 53 ))  					// type 2a
				pFPRc( rS );
			else  										// type 2
				pGPRc( rS );
			tmp_long = EXTRACT_D( opcode );
			pDR( rA, tmp_long );
		}
	}

//	"DS" form instruction.

	else if (opi->form == DS ) {
		pGPRc( rS );
		tmp_long = EXTRACT_DS( opcode );
		pDR( rA, tmp_long );
	}

// An "X" or "smX" form instruction

	else if (opi->form == X || opi->form == smX ) {

		/*
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

		tmp_long = EXTRACT_PRIMARY( opcode );

		if (tmp_long == 31 ) {
			tmp_long = EXTRACT_SECONDARY_X( opcode );

			if (( tmp_long == 0  ) ||
				( tmp_long == 32 )) {					// case 2 -- cmpw

				pCRc ( EXTRACT_BF( opcode ) );
					
				/* MATT simplified mnemonic!...  cmpw/cmplw */
				if (opi->form!=smX) {
					tmp_long = EXTRACT_L( opcode );
					printf( "%01d, ", 
							(int)( tmp_long ) );
					}

				pGPRc( rA );
				pGPR ( EXTRACT_RB( opcode ) );
			}
			else if (( tmp_long == 58 ) ||
					 ( tmp_long == 26 ) ||
					 ( tmp_long == 954 ) ||
					 ( tmp_long == 922 ) ||
					 ( tmp_long == 986 )) {			// case 3 -- extsh, etc.
				pGPRc( rA );
				pGPR ( rS );
			}
			else if (( tmp_long == 86  ) ||
					 ( tmp_long == 470  ) ||
					 ( tmp_long == 54   ) ||
					 ( tmp_long == 278  ) ||
					 ( tmp_long == 246  ) ||
					 ( tmp_long == 1014 ) ||
					 ( tmp_long == 982 )) {			// case 4 -- dcbst, etc.
				pGPRc( rA );
				pGPR ( rB );
			}
			else if (( tmp_long == 854 ) ||
					 ( tmp_long == 498 ) ||
					 ( tmp_long == 598 ) ||
					 ( tmp_long == 370 ) ||
					 ( tmp_long == 566 )) {			// case 5 -- no arguments
					;
			}
			else if (( tmp_long == 599 ) ||
					 ( tmp_long == 631 ) ||
					 ( tmp_long == 567 ) ||
					 ( tmp_long == 535 ) ||
					 ( tmp_long == 759 ) ||
					 ( tmp_long == 727 ) ||
					 ( tmp_long == 983 ) ||
					 ( tmp_long == 695 ) ||
					 ( tmp_long == 663 )) {			// case 8
				pFPRc( rS );
				pGPRc( rA );
				pGPR ( rB );
			}
			else if (( tmp_long == 597 ) ||
					 ( tmp_long == 725 )) {			// case 9
				pGPRc( rS );
				pGPRc( rA );
				p02X ( rB );
			}
			else if (( tmp_long == 512 ) ) {		// case 11
				tmp_long = EXTRACT_BF( opcode );
				printf( "0x%01x", tmp_long );
			}
			else if (( tmp_long == 19 ) ||
					 ( tmp_long == 83 ) ||
					 ( tmp_long == 146 )) {			// case 12
				pGPR ( rS );
			}
			else if(( tmp_long == 210 )) {			// case 14 -- mtsr SR, rx
				p02Xc( rA );						// BI
				pGPR ( rS );
			}
			else if(( tmp_long == 595 )) {			// case 14 -- mfsr SR, rx
				pGPRc( rS );
				p02X ( rA );						// BI
			}
			else if (( tmp_long == 659 ) ||
					 ( tmp_long == 242 )) {			// case 15
				pGPRc( rS );
				pGPR ( rB );
			}
			else if (( tmp_long == 434 ) ||
					 ( tmp_long == 466 ) ||
					 ( tmp_long == 306 ) ||
					 ( tmp_long == 338 )) {			// case 18
				pGPR ( rB );
			}
			else if (( tmp_long == 184 ) ||
					 ( tmp_long == 248 ) ||
					 ( tmp_long == 952 ) ||
					 ( tmp_long == 824 ) ||
					 ( tmp_long == 696 ) ||
					 ( tmp_long == 760 )) {			// case 19
				pGPRc( rA );
				pGPRc( rS );
				p02X ( EXTRACT_SH32( opcode ) );
			}
			else if (( tmp_long == 68 ) ||
					 ( tmp_long == 4  )) {			// case 20
				tmp_long = EXTRACT_TO( opcode );

				/* MATT simplifid mnemonic!...*/
				/* All trap mnemonics! */
				if(opi->form != smX)
				{
					printf( "0x%02x, ", tmp_long );
				}
			/* MATT special trap mnemonic!...*/
			/* trap mnemonic gets nothing printed! */
				if ( tmp_long == 31 && opi->form == smX )
					;
				else {
					pGPRc( rA );
					pGPR ( rB );
				}
			}
			else {
				
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
				tmp_long = EXTRACT_SECONDARY_X(opcode);
				if (( tmp_long == 119 ) ||
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
					( tmp_long == 487 )) {
					pGPRc( rS );
					pGPRc( rA );
					pGPR ( rB );
				}
				else {
			/* We're back to the few X form instructions
			   that fit into this little catagory who
			   have their arguments reversed.  In other
			   words, their arguments appear in the
			   assembly as: RA,RS,RB and are encoded
			   as RS,RA,RB.
			*/
					pGPRc( rA );
		
				/* MATT simplified mnemonics!...*/
				/* not/mr */
					if(opi->form != smX) {
						pGPRc( rS );
						pGPR ( rB );
					}
					else
						pGPR ( rS );
				}
			}
		}

// primary opcode is 63.

		else if (tmp_long == 63 ) {
			tmp_long = EXTRACT_SECONDARY_X( opcode );
			
			if (( tmp_long == 264 ) ||
				( tmp_long == 846 ) ||
				( tmp_long == 814 ) ||
				( tmp_long == 815 ) ||
				( tmp_long == 14  ) ||
				( tmp_long == 15  ) ||
				( tmp_long == 72  ) ||
				( tmp_long == 136 ) ||
				( tmp_long == 40  ) ||
				( tmp_long == 12  )
			  ) { 											// case 6
				pFPRc( rS );
				pFPR ( rB );
			}
			else if (( tmp_long == 32 ) ||
					 ( tmp_long == 0  )) {					// case 7
				p02Xc( EXTRACT_BF( opcode ) );
				pFPRc( rA );
				pFPR ( rB );
			}
			else if (( tmp_long == 64 )) {					// case 10
				tmp_long = EXTRACT_BF( opcode );
				printf( "0x%01x, ", tmp_long );
				tmp_long = EXTRACT_BFA( opcode );
				printf( "0x%01x", tmp_long );
			}
			else if (( tmp_long == 583 )) {					// case 13
				pFPRc( rS );
			}
			else if (( tmp_long == 70 ) ||
					 ( tmp_long == 38 )) {					// case 16
				tmp_long = EXTRACT_BT( opcode );
				printf( "0x%02x", tmp_long );
			}
			else if (( tmp_long == 134 )) {					// case 17
				tmp_long = EXTRACT_BF( opcode );
				printf( "0x%02x,", tmp_long );
				tmp_long = EXTRACT_U( opcode );
				printf( "0x%02x", tmp_long );
			}
		}
	}

// an "XL" form instruction 

	else if (opi->form == XL || opi->form == smXL ) {
		tmp_long = EXTRACT_SECONDARY_XL( opcode );
		if (( tmp_long == 528 ) ||
			( tmp_long == 16  ) ) {
			p02Xc( rS );						// BO
			p02X ( rA );						// BI
		}
		else if (( tmp_long == 150 ) ||
				 ( tmp_long == 50  ) ) {		// no fields -- e.g. isync
			;
		}
		else if (( tmp_long == 0 ) ) {
			/* Take care of cases where we have two
			   bit fields from the CR as arguments.  An
			   example might be the mcrf instr.
			*/
			tmp_long = EXTRACT_BF( opcode );
			printf( "0x%01X,", tmp_long );
		
			tmp_long = EXTRACT_BFA( opcode );
			printf( "0x%01X", tmp_long );
		}
		else {
		
			/* If we get here, then we are in the general
			   case where we need to get 3 bit-fields.
			   Or, if have smX, we only need one or
			   2 of those fields because they duplicate!!
			*/
			p02X( EXTRACT_BT( opcode ) );
			
			if((EXTRACT_SECONDARY_XL(opcode)==449 ||
				EXTRACT_SECONDARY_XL(opcode)==33) &&
				opi->form == smXL
			   )
			{
				printf( "," );
				p02X( EXTRACT_BA( opcode ) );
			}
			else if(opi->form!=smXL)
			{
				printf( "," );
				p02Xc( EXTRACT_BA( opcode ) );
				p02X ( EXTRACT_BB( opcode ) );
			}
		}
	}

// an "XFX" form instruction.

	else if ( opi->form == XFX
		 ||   opi->form == smXFX) {

		tmp_long = EXTRACT_SECONDARY_XFX( opcode );
		if (( tmp_long == 339 ) ||
			( tmp_long == 371 )) { 		

// We have a mfspr instr, which has operands in the form of: RT,SPR

			pGPR ( rS );
			
			/* if NOT a sim. mnemonic print out 
			   the SPR...else skip it! MATT 
			*/	
		
			if (opi->form != smXFX ||
			strncmp(&opi->mnemonic[0],
			"mfsprg",6)==0 ||
			strncmp(&opi->mnemonic[0],
			"mfibatu",7)==0 ||
			strncmp(&opi->mnemonic[0],
			"mfibatl",7)==0 ||
			strncmp(&opi->mnemonic[0],
			"mfdbatu",7)==0 ||
			strncmp(&opi->mnemonic[0],
			"mfdbatl",7)==0
			   )
			{ 
				printf( "," );	
				tmp_long = EXTRACT_SPR( opcode );

				/* There's some wierd ones in the sim.
				   mnemonics...mfsprg,mfibatu,mfibatl
				   mfdbatu,mfdbatl...need the #
				   of the prg register printed out. 
				*/
				if (strncmp(&
				opi->mnemonic[0],
				"mfsprg",6)==0
				   )
				{
					tmp_long = tmp_long - 272;
				}
				if (strncmp(&
				opi->mnemonic[0],
				"mfibatu",7)==0
				   )	
				{
					tmp_long = (tmp_long - 528)/2;
				}
				if (strncmp(&
				opi->mnemonic[0],
				"mfibatl",7)==0
				   ) 
				{
					tmp_long = (tmp_long - 529)/2;
				}
				if (strncmp(&
				opi->mnemonic[0],
				"mfdbatu",7)==0
				   )
				{
					tmp_long = (tmp_long - 536)/2;
				}
				if (strncmp(&
				opi->mnemonic[0],
				"mfdbatl",7)==0
				   )	
				{
					tmp_long = (tmp_long - 537)/2;
				}
				printf("%d", tmp_long);
			}
			else
				reg_spr_printname(tmp_long);
		}

// mtcrf FMX,RS

		else if (tmp_long == 144 ) {
			p02Xc( EXTRACT_FXM( opcode ) );
			pGPR ( rS );
		}

// mtspr SPR,RS

		else {
			if (opi->form != smXFX 
			||  strncmp(&opi->mnemonic[0], "mtsprg",6)==0 
			||  strncmp(&opi->mnemonic[0], "mtibatu",7)==0 
			||  strncmp(&opi->mnemonic[0], "mtibatl",7)==0 
			||  strncmp(&opi->mnemonic[0], "mtdbatu",7)==0 
			||  strncmp(&opi->mnemonic[0], "mtdbatl",7)==0) {
				tmp_long = EXTRACT_SPR( opcode );
				if (strncmp(& opi->mnemonic[0], "mtsprg",6)==0) {
					tmp_long = tmp_long - 272;
					printf("%d",tmp_long);
				}
				else if (strncmp(&opi->mnemonic[0], "mtibatu",7)==0) {
					tmp_long = (tmp_long - 528)/2;
					printf("%d",tmp_long);
				}
				else if (strncmp(&opi->mnemonic[0], "mtibatl",7)==0) {
					tmp_long = (tmp_long - 529)/2;
					printf("%d",tmp_long);
				}
				else if (strncmp(&opi->mnemonic[0], "mtdbatu",7)==0) {
					tmp_long = (tmp_long - 536)/2;
					printf("%d",tmp_long);
				}
				else if (strncmp(&opi->mnemonic[0], "mtdbatl",7)==0) {
					tmp_long = (tmp_long - 537)/2;
					printf("%d",tmp_long);
				}
				else {
					reg_spr_printname(tmp_long);
				}
				printf( "," );
			}
			pGPR ( rS );
		}
	}

// "XFL" form instruction.

	else if (opi->form == XFL ) {
		p02Xc( EXTRACT_FLM( opcode ) );
		pFPR ( rB );
	}

// "XS" form instruction.

	else if (opi->form == XS ) {
		pGPRc( rA );
		pGPRc( rS );
		p02X ( EXTRACT_SH64( opcode ) );
	}

// An "XO" or "smXO" form instruction.

	else if (opi->form == XO || opi->form == smXO) {
		tmp_long = EXTRACT_SECONDARY_XO( opcode );
		pGPRc( rS );

		if (( tmp_long == 234 ) ||
			( tmp_long == 232 ) ||
			( tmp_long == 202 ) ||
			( tmp_long == 200 ) ||
			( tmp_long == 104 ) ||
			( tmp_long == 360 ) ||
			( tmp_long == 488 )) {
			pGPR ( rA );
		}
		else {
			/* We have some instructions that have
			   3 arguments.
			*/
		
			/* simplified mnemonic!! sub,subc MATT */
			if (opi->form == XO) {	
				pGPRc( rA );
				pGPR ( rB );
			}
			if (opi->form == smXO) { 
				pGPRc( rB );
				pGPR ( rA );
			}
		}
	}

// An "A" form instruction.

	else if (opi->form == A ) {
		pFPR( rS );
		
		tmp_long = EXTRACT_SECONDARY_A( opcode );
		if (( tmp_long == 18 ) ||
			( tmp_long == 20 ) ||
			( tmp_long == 21 ) ||
			( tmp_long == 23 ) ||
			( tmp_long == 25 ) ||
			( tmp_long == 28 ) ||
			( tmp_long == 29 ) ||
			( tmp_long == 30 ) ||
			( tmp_long == 31 )) {
			printf(",");pFPR ( rA );
		}
		else if (( tmp_long == 28 ) ||
				 ( tmp_long == 29 ) ||
				 ( tmp_long == 30 ) ||
				 ( tmp_long == 31 )) {
			printf(",");pFPR ( EXTRACT_FRC( opcode ) );
		}

		tmp_long = EXTRACT_SECONDARY_A( opcode );
		if (( tmp_long == 18 ) ||
			( tmp_long == 20 ) ||
			( tmp_long == 21 ) ||
			( tmp_long == 22 ) ||
			( tmp_long == 23 ) ||
			( tmp_long == 24 ) ||
			( tmp_long == 26 )) {
			printf(",");pFPR ( rB );
		}
		else if (( tmp_long == 28 ) ||
				 ( tmp_long == 29 ) ||
				 ( tmp_long == 30 ) ||
				 ( tmp_long == 31 )) {
			printf(",");pFPR ( EXTRACT_FRC( opcode ) );
		}
		
		tmp_long = EXTRACT_SECONDARY_A( opcode );
		if (( tmp_long == 23 ) ||
			( tmp_long == 25 )) {
			printf(",");pFPR ( EXTRACT_FRC( opcode ) );
		}
		else if (( tmp_long == 28 ) ||
				 ( tmp_long == 29 ) ||
				 ( tmp_long == 30 ) ||
				 ( tmp_long == 31 )) {
			pFPR( rB );
		}
	}

// We've got a "M" form instruction.

	else if (opi->form == M ) {
		pGPRc( rA );
		pGPRc( rS );
		
		tmp_long = EXTRACT_PRIMARY( opcode );
		if (tmp_long == 23) {
			tmp_long = EXTRACT_RB( opcode );
			printf( "r%02d,", (int)( tmp_long ) );
		}
		else {
			tmp_long = EXTRACT_SH32( opcode );
			printf( "0x%02X,", tmp_long ); 
		}
		tmp_long = EXTRACT_MBE( opcode );
		printf( "0x%02X,", ( ( tmp_long & 0xFFFFFFE0 ) >> 5 ));
		
		printf( "0x%02X", ( tmp_long & 0x1F ) );

	}

	else if (opi->form == smM) {
			/* We've got a smM form instruction.
			   I only needed to do rotlwi,slwi,srwi,
			   clrlwi,clrrwi,clrlslwi because they can
			   be used to represent the rest!!
			*/

		/* All mnemonics have the RA and RS!! */

		pGPRc( rA );
		pGPRc( rS );

		if(EXTRACT_PRIMARY(opcode) == 23) {
			pGPR ( rB );
		}
		else if(EXTRACT_PRIMARY(opcode) == 20)
		{
			tmp_long = EXTRACT_MB_smM(opcode);
			tmp_long = 1-tmp_long+EXTRACT_ME_smM(opcode);
			printf( "%02d,",(int)(tmp_long));
			tmp_long = EXTRACT_MB_smM(opcode);
			printf( "%02d",(int)(tmp_long));
		}
		if(strncmp(&opi->mnemonic[0],
		"clrlslwi",8)==0
		  )
		{
			tmp_long = EXTRACT_MB_smM(opcode);
			printf("%02d,",(EXTRACT_SH32(opcode)+tmp_long));
		}

		if(strncmp(&opi->mnemonic[0],
		"rotlwi",6)==0 ||
		strncmp(&opi->mnemonic[0],
		"slwi",4)==0 ||
		strncmp(&opi->mnemonic[0],
		"clrlslwi",8)==0
		  )
		{	
			tmp_long = EXTRACT_SH32( opcode );
			printf( "%02d", (int)( tmp_long ) );
		}
		else if(strncmp(&opi->mnemonic[0],
			"srwi",4)==0
			   )
		{
			tmp_long = EXTRACT_SH32( opcode );
			printf( "%02d",(int)( 32 - tmp_long ));
		}
		else if(strncmp(&opi->mnemonic[0],
		"clrlwi",6)==0
				   )
		{
			tmp_long = EXTRACT_MB_smM(opcode);
			printf( "%02d",(int)(tmp_long));
		}
		else if(strncmp(&opi->mnemonic[0],
		"clrrwi",6)==0
			   )
		{
			tmp_long = EXTRACT_ME_smM(opcode);
			printf( "%02d",(int)(31 - tmp_long ));
		}
	}

// "MD" form instruction.

	else if (opi->form == MD ) {
		pGPRc( rA );
		pGPRc( rS );
		p02Xc( EXTRACT_SH64( opcode ) );
		p02X ( EXTRACT_MB( opcode ) );
	}

// "MDS" form instruction.

	else if (opi->form == MDS) {
		pGPRc( rA );
		pGPRc( rS );
		pGPRc( rB );
		p02X ( EXTRACT_MB( opcode ) );
	}

	fflush(stdout);
	return( 0 );
}


//--------------------------------------------------------------------------------
// do_simplified_branch -- handle all simplified branch disassemble steps
//
// <index>	This is used if a 0x4200000 or 0x4240000 
//				opcode is given...in which case, no more
//				searching needs to be done.
//--------------------------------------------------------------------------------
int do_simplified_branch( PPCSIM *ppcsim, ULONG opcode, int index, ULONG current_add )
{
	ULONG	tmp_long;
	int		status;
	char 	new_label[80];

// First thing is to get bits 0:15....and then test to see if they are any of the special cases
// that don't need any work done...no operands!
//
	if (EXTRACT_SPECIAL(opcode) == EXTRACT_SPECIAL(0x42400000) ||
	    EXTRACT_SPECIAL(opcode) == EXTRACT_SPECIAL(0x42000000) ||
	    EXTRACT_SPECIAL(opcode) == EXTRACT_SPECIAL(0x4D800000) ||
	    EXTRACT_SPECIAL(opcode) == EXTRACT_SPECIAL(0x4C800000) ||
	    EXTRACT_SPECIAL(opcode) == EXTRACT_SPECIAL(0x4E000000) ||
	    EXTRACT_SPECIAL(opcode) == EXTRACT_SPECIAL(0x4E400000)) {
		;

	} else {
		tmp_long = EXTRACT_BI( opcode );
		tmp_long = tmp_long - (( tmp_long/4 ) * 4 );
			/* Now, tmp_long is 0-4 where 0-3 are
		 	   offsets into the CR? and 4 is NO	
			   OFFSET...now search the list of 
			   mnemonics and get a better match!
			*/ 
		if ((status = special_search_ops_and_mnemonics( opcode, &index, tmp_long )) != 0)	// No match
			return( 0 );
	}

	printf( "%-10s", ops_and_mnemonics[index].mnemonic );
	

// NOT blr,bctr,blrl,bctrl
//
	if (ops_and_mnemonics[index].form != (smBCLR-1)
	&&  ops_and_mnemonics[index].form != (smBCCTR-1)
	&&  ops_and_mnemonics[index].form != (smBCLRL-1)
	&&  ops_and_mnemonics[index].form != (smBCCTRL-1)) {

		if ((ops_and_mnemonics[index].form >= smBC &&
             ops_and_mnemonics[index].form <= (smBCA+4))
		||  (ops_and_mnemonics[index].form >= smBCL &&
             ops_and_mnemonics[index].form <= (smBCLA+4))) {

			// Get the BI field and convert to a CR offset
			//
			tmp_long = EXTRACT_BI( opcode );
			if (strncmp(&ops_and_mnemonics[index].mnemonic[0], "bt",2) == 0 
			||  strncmp(&ops_and_mnemonics[index].mnemonic[0], "bf",2) == 0) {
				printf( "%02d", (int)( tmp_long ) );
			}

			else if ((EXTRACT_SPECIAL(opcode) == EXTRACT_SPECIAL(0x42400000))
				 ||  (EXTRACT_SPECIAL(opcode) == EXTRACT_SPECIAL(0x42000000))) {
				tmp_long = EXTRACT_BD( opcode );
				tmp_long = test_branch_label( ppcsim, opcode, current_add, tmp_long, &new_label[0] );

				if (tmp_long == 0)	printf( "%s",new_label);
				else				printf( "0x%02x",tmp_long);
			}


// Conditional branches here. BGT, BLT
// 
			else {
				tmp_long = tmp_long/4;
				pCRc( tmp_long );
				tmp_long = EXTRACT_BD( opcode );

				tmp_long = test_branch_label( ppcsim, opcode, current_add, tmp_long, new_label );
				if (tmp_long == 0)	printf( "%s",new_label);
				else				printf( "0x%02x",tmp_long);
			}
		}

		else {
			if (strncmp(&ops_and_mnemonics[index].mnemonic[0], "bdnzlr",6) == 0
			||  strncmp(&ops_and_mnemonics[index].mnemonic[0], "bdzlr" ,5) == 0)
				;
			else {
				tmp_long = EXTRACT_BI( opcode );
				tmp_long = tmp_long/4;
				pCRc( tmp_long );
			}
		}
	}

	return( 0 );
}


/* --------------------------------------------------------------------- */
/*

function:	test_branch_label	
purpose:	This function will see if we have a branch label that 
		matches the address of the current instruction
inputs:		opcode: 32 bit opcode of current instruction
		current_add: the address of current execution
		tmp_long:  current address taken from opcode
		label: the lable if we found one to match
outputs: 	None.	
return:		None.
mod history:	03/14/96 MM	

	return 0 if label match; else return addr.
*/
int test_branch_label( PPCSIM *ppcsim, ULONG opcode, ULONG current_add, ULONG tmp_long, char *label )
{
	int		count = 0;
	ULONG	sv;
	char	*s;

/* printf("\n IN opcode = %x\n",opcode);
printf("\n IN current_add = %x\n",current_add);
printf("\n IN tmp_long = %x\n",tmp_long);  */
   
	if (EXTRACT_PRIMARY( opcode ) == 18 
	||  EXTRACT_PRIMARY( opcode ) == 16) {

		if (EXTRACT_PRIMARY( opcode ) == 16) {
			if ((tmp_long & 0x00008000) == 0x00008000) {
				tmp_long = tmp_long | 0xFFFF0000;
			} else {
				tmp_long = tmp_long | 0x00000000;
			}
	    }
	} else if (EXTRACT_PRIMARY( opcode ) == 18) {
		if ((tmp_long & 0x02000000) == 0x02000000)
			tmp_long = tmp_long | 0xFD000000;
	    else
			tmp_long = tmp_long | 0x00000000;
	}


	if ((EXTRACT_LK_AA( opcode ) == 0)  ||  (EXTRACT_LK_AA( opcode ) == 1)) {		// BL
		tmp_long = tmp_long & 0xFFFFFF;			// GM patch: seems to be extra bits leaking in.
		sv = (tmp_long & 0x800000) ? tmp_long | 0xFF000000 : tmp_long;
	    tmp_long = current_add + sv;
	} else {
	    tmp_long = tmp_long;
	}


	if ((s = symbol_get_byaddr( ppcsim, tmp_long )) != NULL) {
		sprintf( label, "%x <%s>", tmp_long, s);
		return( 0 );
	}
	else
		return( tmp_long );
}


//---------------------------------------------------------------------------
//	disassemble_one -- disassemble one line.
//---------------------------------------------------------------------------
int disassemble_one( PPCSIM *ppcsim, ULONG opcode, ULONG addr, int nl )
{
	int stat;

	stat = disassemble_an_opcode( ppcsim, opcode, addr );

	if (nl)
		printf("\n");
}


//---------------------------------------------------------------------------
//	disassemble -- disassemble a block of memory.
//---------------------------------------------------------------------------
int disassemble( PPCSIM *ppcsim, ULONG start, ULONG end )
{
	ULONG	opcode;
	short	lines, lim;
	int		status;


// Get the opcodes from 'start' forward.

	lines = 1;
	lim   = shell_more_lines(23);
	while (start <= end) {

// Get the opcode directly from memory.

		if ((status = MSS_read( ppcsim, start, &opcode, 4 )) != 0)
			return( status );

		disassemble_one( ppcsim, opcode, start, 1 );

// Find the next opcode.  Aren't RISC machines nice?

		start += 4;

		if (++lines >= lim) {
			if (!shell_more())
				return( 0 );
			lines = 1;
		}
	}
	
	return( 0 );
}


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
int DASM_init( PPCSIM *ppcsim )
{

	number_of_mnemonics = sizeof(ops_and_mnemonics)/sizeof(asm_dasm_element);

	return( 0 );
}


//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void DASM_deinit( PPCSIM *ppcsim )
{

}
