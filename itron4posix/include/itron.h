/*
 *	uITRON ver.4.0 for POSIX
 *	itron.h
 *	
 */

#ifndef _ITRON_H_
#define _ITRON_H_
#include "depend.h"

#if __GNUC__
#include <stdint.h>
#endif

/****************************************************************************/
/*							itron data type									*/
/****************************************************************************/
/*------------------------- standard profile -------------------------------*/
typedef signed char		B;				/* signed 8 bit integer				*/
typedef signed short	H;				/* signed 16 bit integer			*/
#if __GNUC__
typedef int32_t			W;
typedef uint32_t		UW;
typedef int32_t			VW;
#else
typedef signed long		W;				/* signed 32 bit integer			*/
typedef unsigned long	UW;				/* unsigned 32 bit integer			*/
typedef long			VW;				/* variable data type (32 bit)		*/
#endif
typedef unsigned char	UB;				/* unsigned 8 bit integer			*/
typedef unsigned short	UH;				/* unsigned 16 bit integer			*/
typedef char			VB;				/* variable data type (8 bit)		*/
typedef short			VH;				/* variable data type (16 bit)		*/
typedef void			*VP;			/* pointer to variable data type	*/
typedef int				INT;			/* signed integer (CPU dependent)	*/
typedef unsigned int	UINT;			/* unsigned integer (CPU dependent) */
typedef INT				BOOL;			/* Bool value						*/
typedef INT				FN;				/* function code					*/
typedef INT				VP_INT;			/* pointer or integer				*/
typedef W				ER;				/* error code						*/
typedef H				ID;				/* object ID (xxxid)				*/
typedef UW				ATR;			/* attribute						*/
typedef UW				STAT;			/* object status					*/
typedef UH				MODE;			/* service call mode				*/
typedef H				PRI;			/* task priority					*/
typedef UW				SIZE;			/* memory area size					*/
typedef W				TMO;			/* time out							*/
typedef BOOL			ER_BOOL;		/* error code or bool value			*/
typedef ER				ER_ID;			/* error code or ID number			*/
typedef UINT			ER_UINT;		/* error code or unsigned int		*/
typedef struct {						/* system time						*/
	W	utime;							/* current date/time (upper)		*/
	UW	ltime;							/* current date/time (lower)		*/
} SYSTIM;
typedef SYSTIM			RELTIM;			/* relocation time					*/

typedef H				HNO;			/* handler number					*/
typedef H				TPRI;			/* task priority					*/
typedef H				DVN;			/* I/O device number				*/

typedef void			(*FP)(VP_INT);	/* program start address			*/
typedef void			TASK;			/* task								*/
typedef TASK			(*TASKP)();		/* pointer to task					*/


/****************************************************************************/
/*							general define									*/
/****************************************************************************/
#define TRUE			1				/* true								*/
#define FALSE			0				/* false							*/

/****************************************************************************/
/*							main error code									*/
/****************************************************************************/
#define E_OK		(ER)0				/* normal end						*/
#define E_SYS		(ER)(-5)			/* system error						*/
#define E_NOSPT		(ER)(-9)			/* unsupported function				*/
#define E_RSFN		(ER)(-10)			/* reserved function				*/
#define E_RSATR		(ER)(-11)			/* reserved attribute				*/
#define E_PAR		(ER)(-17)			/* parameter error					*/
#define E_ID		(ER)(-18)			/* incorrect ID number				*/
#define E_CTX		(ER)(-25)			/* context error					*/
#define E_MACV		(ER)(-26)			/* memory access					*/
#define E_OACV		(ER)(-27)			/* object access					*/
#define E_ILUSE		(ER)(-28)			/* incorrect service call use		*/
#define E_NOMEM		(ER)(-33)			/* memmory empty					*/
#define E_NOID		(ER)(-34)			/* ID empty							*/
#define E_OBJ		(ER)(-41)			/* object status error				*/
#define E_NOEXS		(ER)(-42)			/* object no exsist					*/
#define E_QOVR		(ER)(-43)			/* queue/nest over flaw				*/
#define E_RLWAI		(ER)(-49)			/* force release wait				*/
#define E_TMOUT		(ER)(-50)			/* porling fail/time out			*/
#define E_DLT		(ER)(-51)			/* delete waiting object			*/
#define E_CLS		(ER)(-52)			/* waiting object status change		*/
#define E_WBLK		(ER)(-57)			/* non blocking						*/
#define E_BOVR		(ER)(-58)			/* buffer over flaw					*/

#define MERCD(ercd) (ER)(ercd & 0x0000000f)
#define SERCD(ercd) (ER)(ercd >> 8)

/****************************************************************************/
/*							miserronus define								*/
/****************************************************************************/
/*-------------------------------- object ----------------------------------*/
#define TA_NULL		(ATR)0				/* no object						*/

/*--------------------------------- tmout ----------------------------------*/
#define TMO_POL			(TMO)0			/* polling							*/
#define TMO_FEVR		(TMO)(-1)		/* forever wait						*/
#define TMO_NBLK		(TMO)(-2)		/* non blocking						*/

#define SYSTIC (TIC_NUME/TIC_DENO)

#include "kernel.h"
#endif
