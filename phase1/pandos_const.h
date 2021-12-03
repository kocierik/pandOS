#ifndef PANDOS_CONST_H_INCLUDED
#define PANDOS_CONST_H_INCLUDED

/****************************************************************************
 *
 * This header file contains utility constants & macro definitions.
 *
 ****************************************************************************/

#include <umps3/umps/const.h>

/* Hardware & software constants */
#define PAGESIZE 4096 /* page size in bytes	*/
#define WORDLEN  4    /* word size in bytes	*/


/* timer, timescale, TOD-LO and other bus regs */
#define RAMBASEADDR   0x10000000
#define RAMBASESIZE   0x10000004
#define TODLOADDR     0x1000001C
#define INTERVALTMR   0x10000020
#define TIMESCALEADDR 0x10000024

/* Memory related constants */
#define KSEG0        0x00000000
#define KSEG1        0x20000000
#define KSEG2        0x40000000
#define KUSEG        0x80000000
#define RAMSTART     0x20000000
#define BIOSDATAPAGE 0x0FFFF000
#define PASSUPVECTOR 0x0FFFF900

/* Exceptions related constants */
#define PGFAULTEXCEPT 0
#define GENERALEXCEPT 1

/* Mikeyg Added constants */

#define MAXPROC 20

#define CREATEPROCESS 1
#define TERMPROCESS   2
#define PASSEREN      3
#define VERHOGEN      4
#define IOWAIT        5
#define GETTIME       6
#define CLOCKWAIT     7
#define GETSUPPORTPTR 8
#define TERMINATE     9
#define GET_TOD       10
#define WRITEPRINTER  11
#define WRITETERMINAL 12
#define READTERMINAL  13
#define DISK_GET      14
#define DISK_PUT      15
#define FLASH_READ    16
#define FLASH_WRITE   17
#define DELAY         18
#define PSEMVIRT      19
#define VSEMVIRT      20


/* Status register constants */
#define ALLOFF      0x00000000
#define USERPON     0x00000008
#define IEPON       0x00000004
#define IECON       0x00000001
#define IMON        0x0000FF00
#define TEBITON     0x08000000
#define DISABLEINTS 0xFFFFFFFE

/* Cause register constants */
#define GETEXECCODE    0x0000007C
#define CLEAREXECCODE  0xFFFFFF00
#define LOCALTIMERINT  0x00000200
#define TIMERINTERRUPT 0x00000400
#define DISKINTERRUPT  0x00000800
#define FLASHINTERRUPT 0x00001000
#define PRINTINTERRUPT 0x00004000
#define TERMINTERRUPT  0x00008000
#define IOINTERRUPTS   0
#define TLBINVLDL      2
#define TLBINVLDS      3
#define SYSEXCEPTION   8
#define BREAKEXCEPTION 9
#define PRIVINSTR      10
#define CAUSESHIFT     2


/* EntryLO register (NDVG) constants */
#define DIRTYON  0x00000400
#define VALIDON  0x00000200
#define GLOBALON 0x00000100


/* EntryHI register constants */
#define GETPAGENO     0x3FFFF000
#define GETSHAREFLAG  0xC0000000
#define VPNSHIFT      12
#define ASIDSHIFT     6
#define SHAREDSEGFLAG 30


/* Index register constants */
#define PRESENTFLAG 0x80000000


/* Device register constants */
#define DEV0ON 0x00000001
#define DEV1ON 0x00000002
#define DEV2ON 0x00000004
#define DEV3ON 0x00000008
#define DEV4ON 0x00000010
#define DEV5ON 0x00000020
#define DEV6ON 0x00000040
#define DEV7ON 0x00000080


#define OKCHARTRANS  5
#define TRANSMITCHAR 2

#define SEEKTOCYL  2
#define DISKREAD   3
#define DISKWRITE  4
#define FLASHREAD  2
#define FLASHWRITE 3
#define BACKREAD   1
#define BACKWRITE  2


/* Memory Constants */
#define UPROCSTARTADDR 0x800000B0
#define USERSTACKTOP   0xC0000000
#define KERNELSTACK    0x20001000


#define SHARED  0x3
#define PRIVATE 0x2


/* Utility constants */
#define ON         1
#define OFF        0
#define OK         0
#define NOPROC     -1
#define BYTELENGTH 8

#define PSECOND    100000
#define TIMESLICE  5000 /* length of proc's time slice	*/
#define NEVER      0x7FFFFFFF
#define SECOND     1000000
#define STATESIZE  0x8C
#define DEVICECNT  (DEVINTNUM * DEVPERINT)
#define MAXSTRLENG 128

#define DELAYASID    (UPROCMAX + 1)
#define KUSEG3SECTNO 0

#define VMDISK        0
#define MAXPAGES      32
#define USERPGTBLSIZE MAXPAGES
#define OSFRAMES      32

#define FLASHPOOLSTART (RAMSTART + (OSFRAMES * PAGESIZE))
#define DISKPOOLSTART  (FLASHPOOLSTART + (DEVPERINT * PAGESIZE))
#define FRAMEPOOLSTART (DISKPOOLSTART + (DEVPERINT * PAGESIZE))

#define RAMTOP(T) ((T) = ((*((int *)RAMBASEADDR)) + (*((int *)RAMBASESIZE))))


#define DISKBACK     1
#define FLASHBACK    0
#define BACKINGSTORE FLASHBACK

#define UPROCMAX 8
#define POOLSIZE (UPROCMAX * 2)
/* End of Mikeyg constants */


#endif
