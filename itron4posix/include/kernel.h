#ifndef _KERNEL_H_
#define _KERNEL_H_
#include "itron.h"

/****************************************************************************/
/*						kernel common define								*/
/****************************************************************************/
/*-------------------- task/handler attribute (***atr) ---------------------*/
#define TA_HLNG			(ATR)0			/* high-level language program		*/
#define TA_ASM			(ATR)1			/* assembler program				*/

/*----------------------- object attribute (***atr) ------------------------*/
#define TA_TFIFO		(ATR)0			/* FIFO wait queue					*/
#define TA_TPRI			(ATR)1			/* priority based wait queue		*/
#define TA_MFIFO		(ATR)0			/* FIFO massage queue (on mailbox)	*/
#define TA_MPRI			(ATR)1			/* priority based massage queue		*/
#define TA_INHERIT		(ATR)2			/* mutex attribute inheritance		*/
#define TA_CEILING		(ATR)3			/* mutex attribute ceiling			*/

/*------------------------------ cycle -------------------------------------*/
#define TA_PHS			(ATR)2			/* Phase sync 						*/
#define TA_STA			(ATR)4			/* Cycle Start 						*/
#define TCYC_STA		(STAT)1			/* Cycle started					*/
#define TCYC_STP		(STAT)0			/* Cycle stopped					*/

/*------------------------------ alarm -------------------------------------*/
#define TALM_STA		(STAT)1			/* Alarm started					*/
#define TALM_STP		(STAT)0			/* Alarm stopped					*/

/*------------------------------ task id -----------------------------------*/
#define TSK_SELF		(ID)0			/* self task						*/
#define TSK_NONE		(ID)0			/* none task						*/

#define TA_WSGL			(ATR)0			/* Wait single task (on eventflag)	*/
#define TA_WMUL			(ATR)2			/* Wait multiple task (on eventflag)*/
#define TA_CLR			(ATR)4			/* clear all bit when wake up		*/

/*------------------------------ tskpri ------------------------------------*/
#define TPRI_INI		(PRI)0			/* initial priority					*/
#define TPRI_RUN		(PRI)0			/* RUN task's priority				*/

/****************************************************************************/
/*				application program interface code deifine					*/
/****************************************************************************/
#define TFN_NONE		0
#define TFN_CRE_TSK		(-0x05)
#define TFN_ACRE_TSK	(-0xc1)
#define TFN_DEL_TSK		(-0x06)
#define TFN_ACT_TSK		(-0x07)
#define TFN_IACT_TSK	(-0x71)
#define TFN_CAN_ACT		(-0x08)
#define TFN_STA_TSK		(-0x09)
#define TFN_EXT_TSK		(-0x0a)
#define TFN_EXD_TSK		(-0x0b)
#define TFN_TER_TSK		(-0x0c)
#define TFN_CHG_PRI		(-0x0d)
#define TFN_GET_PRI		(-0x0e)
#define TFN_REF_TSK		(-0x0f)
#define TFN_REF_TST		(-0x10)
#define TFN_SLP_TSK		(-0x11)
#define TFN_TSLP_TSK	(-0x12)
#define TFN_WUP_TSK		(-0x13)
#define TFN_IWUP_TSK	(-0x72)
#define TFN_CAN_WUP		(-0x14)
#define TFN_REL_WAI		(-0x15)
#define TFN_IREL_WAI	(-0x73)
#define TFN_SUS_TSK		(-0x16)
#define TFN_RSM_TSK		(-0x17)
#define TFN_FRSM_TSK	(-0x18)
#define TFN_DLY_TSK		(-0x19)
#define TFN_DEF_TEX		(-0x1b)
#define TFN_RAS_TEX		(-0x1c)
#define TFN_IRAS_TEX	(-0x74)
#define TFN_DIS_TEX		(-0x1d)
#define TFN_ENA_TEX		(-0x1e)
#define TFN_SNS_TEX		(-0x1f)
#define TFN_REF_TEX		(-0x20)
#define TFN_CRE_SEM		(-0x21)
#define TFN_ACRE_SEM	(-0xc2)
#define TFN_DEL_SEM		(-0x22)
#define TFN_SIG_SEM		(-0x23)
#define TFN_ISIG_SEM	(-0x75)
#define TFN_WAI_SEM		(-0x25)
#define TFN_POL_SEM		(-0x26)
#define TFN_TWAI_SEM	(-0x27)
#define TFN_REF_SEM		(-0x28)
#define TFN_CRE_FLG		(-0x29)
#define TFN_ACRE_FLG	(-0xc3)
#define TFN_DEL_FLG		(-0x2a)
#define TFN_SET_FLG		(-0x2b)
#define TFN_ISET_FLG	(-0x76)
#define TFN_CLR_FLG		(-0x2c)
#define TFN_WAI_FLG		(-0x2d)
#define TFN_POL_FLG		(-0x2e)
#define TFN_TWAI_FLG	(-0x2f)
#define TFN_REF_FLG		(-0x30)
#define TFN_CRE_DTQ		(-0x31)
#define TFN_ACRE_DTQ	(-0xc4)
#define TFN_DEL_DTQ		(-0x32)
#define TFN_SND_DTQ		(-0x35)
#define TFN_PSND_DTQ	(-0x36)
#define TFN_IPSND_DTQ	(-0x77)
#define TFN_TSND_DTQ	(-0x37)
#define TFN_FSND_DTQ	(-0x38)
#define TFN_IFSND_DTQ	(-0x78)
#define TFN_RCV_DTQ		(-0x39)
#define TFN_PRCV_DTQ	(-0x3a)
#define TFN_TRCV_DTQ	(-0x3b)
#define TFN_REF_DTQ		(-0x3c)
#define TFN_CRE_MBX		(-0x3d)
#define TFN_ACRE_MBX	(-0xc5)
#define TFN_DEL_MBX		(-0x3e)
#define TFN_SND_MBX		(-0x3f)
#define TFN_RCV_MBX		(-0x41)
#define TFN_PRCV_MBX	(-0x42)
#define TFN_TRCV_MBX	(-0x43)
#define TFN_REF_MBX		(-0x44)
#define TFN_CRE_MTX		(-0x81)
#define TFN_ACRE_MTX	(-0xc6)
#define TFN_DEL_MTX		(-0x82)
#define TFN_LOC_MTX		(-0x85)
#define TFN_PLOC_MTX	(-0x86)
#define TFN_TLOC_MTX	(-0x87)
#define TFN_UNL_MTX		(-0x83)
#define TFN_REF_MTX		(-0x88)
#define TFN_CRE_MBF		(-0x89)
#define TFN_ACRE_MBF	(-0xc7)
#define TFN_DEL_MBF		(-0x8a)
#define TFN_SND_MBF		(-0x8d)
#define TFN_PSND_MBF	(-0x8e)
#define TFN_TSND_MBF	(-0x8f)
#define TFN_RCV_MBF		(-0x91)
#define TFN_PRCV_MBF	(-0x92)
#define TFN_TRCV_MBF	(-0x93)
#define TFN_REF_MBF		(-0x94)
#define TFN_CRE_POR		(-0x95)
#define TFN_ACRE_POR	(-0xc8)
#define TFN_DEL_POR		(-0x96)
#define TFN_CAL_POR		(-0x97)
#define TFN_TCAL_POR	(-0x98)
#define TFN_ACP_POR		(-0x99)
#define TFN_PACP_POR	(-0x9a)
#define TFN_TACP_POR	(-0x9b)
#define TFN_FWD_POR		(-0x9c)
#define TFN_RPL_RDV		(-0x9d)
#define TFN_REF_POR		(-0x9e)
#define TFN_REF_RDV		(-0x9f)
#define TFN_CRE_MPF		(-0x45)
#define TFN_ACRE_MPF	(-0xc9)
#define TFN_DEL_MPF		(-0x46)
#define TFN_GET_MPF		(-0x49)
#define TFN_PGET_MPF	(-0x4a)
#define TFN_TGET_MPF	(-0x4b)
#define TFN_REL_MPF		(-0x47)
#define TFN_REF_MPF		(-0x4c)
#define TFN_SET_TIM		(-0x4d)
#define TFN_GET_TIM		(-0x4e)
#define TFN_ISIG_TIM	(-0x7d)
#define TFN_CRE_CYC		(-0x4f)
#define TFN_ACRE_CYC	(-0xcb)
#define TFN_DEL_CYC		(-0x50)
#define TFN_STA_CYC		(-0x51)
#define TFN_STP_CYC		(-0x52)
#define TFN_REF_CYC		(-0x53)
#define TFN_CRE_MPL		(-0xa1)
#define TFN_ACRE_MPL	(-0xca)
#define TFN_DEL_MPL		(-0xa2)
#define TFN_GET_MPL		(-0xa5)
#define TFN_PGET_MPL	(-0xa6)
#define TFN_TGET_MPL	(-0xa7)
#define TFN_REL_MPL		(-0xa3)
#define TFN_REF_MPL		(-0xa8)

/****************************************************************************/
/*				application program interface data define					*/
/****************************************************************************/
/* cre_tsk */
#define TA_ACT			(ATR)2			/* create with runnable status		*/

										/* tskstat                          */
#define TTS_RUN         0x00000001      /* RUN                              */
#define TTS_RDY         0x00000002      /* READY                            */
#define TTS_WAI         0x00000004      /* WAIT                             */
#define TTS_SUS         0x00000008      /* SUSPEND                          */
#define TTS_WAS         0x0000000c      /* WAIT-SUSPEND                     */
#define TTS_DMT         0x00000010      /* DORMANT                          */
#define TTS_FWU			0x00000020		/* force wake up					*/
#define TTS_DLT			0x00000040		/* force wake up (object deleted)	*/
#define TTS_SCL			0x00000100		/* scheduler lock (non dispatch)	*/

/****************************************************************************/
/*				application program interface structure deifine				*/
/****************************************************************************/
/*--------------------------- task management ------------------------------*/
/*----------- cre_tsk -----------*/
typedef struct	t_ctsk {				/* cre_tsk packet					*/
			ATR			tskatr;				/* task attribute				*/
			VP_INT		exinf;				/* extended information			*/
			FP			task;				/* task start address			*/
			PRI			itskpri;			/* initial priority				*/
			SIZE		stksz;				/* stack size					*/
			VP			stk;				/* stack start address			*/
} T_CTSK;

/*----------- ref_tsk -----------*/
typedef struct	t_rtsk {				/* ref_tsk packet					*/
			STAT		tskstat;			/* task status					*/
			PRI			tskpri;				/* current priority				*/
			PRI			tskbpri;			/* base priority				*/
			STAT		tskwait;			/* factor of WAIT				*/
			ID			wobjid;				/* wait object ID				*/
			TMO			lefttmo;			/* left time for wake up		*/
			UINT		actcnt;				/* activate req number			*/
			UINT		wupcnt;				/* wakeup req number			*/
			UINT		suscnt;				/* suspend req nest number		*/
} T_RTSK;

/*----------- ref_tst -----------*/
typedef struct t_rtst {					/* simple tsk stat					*/
			STAT		tskstat;			/* tsk status					*/
			STAT		tskwait;			/* factor of WAIT				*/
} T_RTST;

/*------------------------ task exception routine --------------------------*/
typedef UW TEXPTN;						/* exception reason pattern			*/
#define TBIT_TEXPTN (sizeof(TEXPTN) * 8)/* exception reason bit number		*/

typedef struct t_dtex {					/* task exception routine definition*/
			ATR			texatr;				/* attribute of tex				*/
			FP			texrtn;				/* start address of tex			*/
} T_DTEX;

typedef struct t_rtex {					/* task exception stateus definition*/
		STAT		texstat;			/* exception status					*/
		TEXPTN		pndptn;				/* reserve exception reason			*/
} T_RTEX;

/*------------------------------ semaphore ---------------------------------*/
/*----------- cre_sem -----------*/
typedef struct	t_csem {				/* cre_sem packet					*/
			ATR			sematr;				/* semaphore attribute			*/
			UINT		isemcnt;			/* initial semaphore count		*/
			UINT		maxsem;				/* max semaphore resource		*/
} T_CSEM;


/*----------- ref_sem -----------*/
typedef struct	t_rsem {				/* ref_sem packet					*/
			ID			wtskid;				/* waiting task ID				*/
			UINT		semcnt;				/* current semaphore count		*/
} T_RSEM;

/*------------------------------ eventflag ---------------------------------*/
typedef UW FLGPTN;						/* event flag bit pattern			*/
#define TBIT_FLGPTN (sizeof(FLGPTN) * 8)/* event flag bit number			*/

/*----------- cre_flg -----------*/
typedef struct	t_cflg {				/* cre_flg packet					*/
			ATR			flgatr;				/* eventflag attribute			*/
			FLGPTN		iflgptn;			/* initial eventflag pattern	*/
} T_CFLG;

/*----------- ref_flg -----------*/
typedef struct	t_rflg {				/* ref_flg packet					*/
			ID			wtskid;				/* waiting task ID				*/
			FLGPTN		flgptn;				/* eventflag pattern			*/
} T_RFLG;

/*----------- wfmode ------------*/
#define TWF_ANDW		(UINT)0			/* AND wait condition				*/
#define TWF_ORW			(UINT)2			/* OR wait condition				*/
#define TWF_CLR			(UINT)1			/* clear condition					*/

/*----------------------------- data queue ---------------------------------*/
/*----------- cre_dtq -----------*/
#define TSZ_DTQ 4 + 4
typedef struct t_cdtq {					/* cre_dtq packet					*/
            ATR			dtqatr;				/* data queue attribute			*/
            UINT		dtqcnt;				/* data queue capacity			*/
            VP			dtq;				/* data queue area start address*/
} T_CDTQ;

/*----------- ref_dtq -----------*/
typedef struct t_rdtq {					/* ref_dtq packet					*/
            ID			stskid;				/* send waiting task ID			*/
            ID			rtskid;				/* recv waiting task ID			*/
            UINT		sdtqcnt;			/* data number in queue			*/
} T_RDTQ;

/*------------------------------- mailbox ----------------------------------*/
/*---- T_MSG format(example) ----*/
typedef struct	t_msg  {				/*	 message packet (for kernel)	*/
			struct t_msg *nextmsg;		/*	   next message pointer			*/
} T_MSG;

typedef struct t_msg_pri {				/* message packet with priority		*/
            T_MSG		msghdr;				/* message header				*/
            PRI			msgpri;				/* message priority				*/
} T_MSG_PRI;

typedef struct t_msg_prihd {			/* message management				*/
	T_MSG *top;								/* top msg pointer				*/
	T_MSG *end;								/* end msg pointer				*/
} T_MSG_PRIHD;

/*----------- cre_mbx -----------*/
#define TSZ_MPRIHD(maxmpri) (SIZE)(sizeof(T_MSG_PRIHD) * maxmpri)
typedef struct	t_cmbx {				/* cre_mbx packet					*/
			ATR			mbxatr;				/* mailbox attribute			*/
            PRI			maxmpri;			/* message priority max			*/
            VP			mprihd;				/* message que header start adr */
} T_CMBX;

/*----------- ref_mbx -----------*/
typedef struct	t_rmbx {				/* ref_mbx packet					*/
			ID			wtskid;				/* waiting task ID				*/
			T_MSG		*pk_msg;			/* first receiving message		*/
} T_RMBX;

/*-------------------------------- mutex -----------------------------------*/
/*----------- cre_mtx -----------*/
typedef struct t_cmtx {					/* cre_mtx packet					*/
            ATR			mtxatr;				/* mutex attribute				*/
            PRI			ceilpri;			/* mutex max priority			*/
} T_CMTX;

/*----------- ref_mtx -----------*/
typedef struct t_rmtx {					/* ref_mtx packet					*/
            ID			htskid;				/* mutex locking task ID		*/
            ID			wtskid;				/* waiting task ID				*/
} T_RMTX;

/*---------------------------- messagebuffer -------------------------------*/
#define TSZ_MBF(msgcnt, msgsz) ((msgcnt) * ((msgsz) + sizeof(SIZE)))

/*----------- cre_mbf -----------*/
typedef struct	t_cmbf {				/* cre_mbf packet					*/
			ATR			mbfatr;				/* messagebuffer attribute		*/
			UINT		maxmsz;				/* maximum message size			*/
			SIZE		mbfsz;				/* messagebuffer size			*/
			VP			mbf;				/* messagebuffer start address	*/
} T_CMBF;

/*----------- ref_mbf -----------*/
typedef struct	t_rmbf {				/* ref_mbf packet					*/
			ID			stskid;				/* waiting task ID for sending	*/
			ID			rtskid;				/* waiting task ID for receiving*/
            UINT		smsgcnt;			/* data number in messagebuffer */
            SIZE		fmbfsz;				/* free message area bytes		*/
} T_RMBF;

/*------------------------------- randevu ----------------------------------*/
typedef UW RDVPTN;						/* randevu bit pattern				*/
typedef ID RDVNO;						/* randevu number					*/
#define TBIT_RDVPTN (sizeof(RDVPTN) * 8)/* randevu bit pattern size			*/

/*----------- cre_por -----------*/
typedef struct t_cpor {					/* cre_por packet					*/
            ATR			poratr;				/* port attribute				*/
            UINT		maxcmsz;			/* max size of call message		*/
            UINT		maxrmsz;			/* max size of res message		*/
} T_CPOR;

/*----------- ref_por -----------*/
typedef struct t_rpor {					/* ref_por packet					*/
            ID			ctskid;				/* call waiting task ID			*/
            ID			atskid;				/* res waiting task ID			*/
} T_RPOR;

/*----------- ref_rdv -----------*/
typedef struct t_rrdv {					/* ref_rdv packet					*/
            ID			wtskid;				/* rdv quit waiting task ID		*/
} T_RRDV;

/*------------------------- memorypool management --------------------------*/
/*----------- cre_mpf -----------*/
typedef struct	t_cmpf {				/* cre_mpf packet					*/
			ATR			mpfatr;				/* memorypool attribute			*/
			UINT		blkcnt;				/* getable block count			*/
			UINT		blksz;				/* memory block size			*/
			VP			mpf;				/* fixed memorypool start adr	*/
} T_CMPF;

/*----------- ref_mpf -----------*/
typedef struct	t_rmpf {				/* ref_mpf packet					*/
			ID			wtskid;				/* waiting task ID				*/
			UINT		fblkcnt;			/* total free block count		*/
} T_RMPF;

/*----------- cre_mpl -----------*/
  typedef struct  t_cmpl {				  /* cre_mpl packet					*/
			ATR			mplatr;				/* memorypool attribute			*/
			SIZE		mplsz;				/* memorypool size				*/
			VP			mpl;				/* memorypool start address		*/
} T_CMPL;

/*----------- ref_mpl -----------*/
typedef struct	t_rmpl {				/* ref_mpl packet					*/
			ID			wtskid;				/* waiting task ID				*/
			SIZE		fmplsz;				/* total free size				*/
			UINT		fblksz;				/* maximum immd get free size	*/
} T_RMPL;

/*---------------------------- time management -----------------------------*/
/*----------- def_cyc -----------*/
typedef struct	t_ccyc {				/* cre_cyc packet					*/
			ATR			cycatr;				/* cyclic handler attribute		*/
			VP_INT		exinf;				/* extended information			*/
			FP			cychdr;				/* cyclic handler address		*/
			RELTIM		cyctim;				/* cyclic time					*/
			RELTIM		cycphs;				/* cyclic time phase			*/
} T_CCYC;

/*----------- ref_cyc -----------*/
typedef struct	t_rcyc {				/* ref_cyc packet					*/
            STAT		cycstat;			/* cyclic handler stataus		*/
			RELTIM		lefttim;			/* left time for next wakeup 	*/
} T_RCYC;

/*----------- cycact ------------*/
#define TCY_OFF			(UINT)0				/* disable state				*/
#define TCY_ON			(UINT)1				/* enable state					*/
#define TCY_INI			(UINT)2				/* initialize cycle count		*/

/*----------- def_alm -----------*/
typedef struct	t_calm {				/* cre_alm packet					*/
			ATR			almatr;				/* alarm handler attribute		*/
			VP_INT		exinf;				/* extended information			*/
			FP			almhdr;				/* cyclic handler address		*/
} T_CALM;

/*----------- tmmode ------------*/
#define TTM_ABS			(UINT)0				/* absolute time				*/
#define TTM_REL			(UINT)1				/* relative time				*/

/*----------- ref_alm -----------*/
typedef struct	t_ralm {				/* ref_alm packet					*/
            STAT		almstat;			/* alarm handler stat			*/
            RELTIM		lefttim;			/* left time for next wakeup	*/
} T_RALM;

/*----------- def_ovr -----------*/
typedef struct t_dovr {					/* def_ovr packet					*/
            ATR			ovratr;				/* overrun handler attribute	*/
            FP			ovrhdr;				/* overrun handler func addr	*/
} T_DOVR;

/*----------- ref_ovr -----------*/
typedef UW OVRTIM;						/* processor time					*/

typedef struct t_rovr {					/* ref_ovr packet					*/
            STAT		ovrstat;			/* overrun handler status		*/
            OVRTIM		leftotm;			/* left time for wakeup			*/
} T_ROVR;

/*------------------------ system status management ------------------------*/
/*----------- ref_sys -----------*/
typedef struct	t_rsys	 {				/* ref_sys packet					*/
} T_RSYS;

/*----------------------- service call management --------------------------*/
/*----------- def_svc -----------*/
typedef struct	t_dsvc {				/* def_svc packet					*/
			ATR			svcatr;				/* ex-SVC handler attribute		*/
			FP			svchdr;				/* ex-SVC handler address		*/
} T_DSVC;

/*------------------------ system status management ------------------------*/
typedef ID EXCNO;						/* exception number					*/

/*----------- def_exc -----------*/
typedef struct t_dexc {					/* def_exc packet					*/
            ATR			excatr;				/* CPU exception handler attr	*/
            FP			exchdr;				/* CPU exception handler addr	*/
} T_DEXC;

/*----------- ref_cfg -----------*/
typedef struct	t_rcfg	 {				/* ref_cfg packet					*/
} T_RCFG;

/*----------- ref_ver -----------*/
typedef struct	t_rver	 {				/* ref_ver packet					*/
            UH			maker;				/* maker code					*/
            UH			prid;				/* kernel ID					*/
            UH			spver;				/* ITRON version				*/
            UH			prver;				/* kernel version				*/
            UH			prno[4];			/* kernel management info		*/
} T_RVER;

/****************************************************************************/
/*								API prototype								*/
/****************************************************************************/
	ER		inz_knl(void);
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*							  Task management								*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
	ER 		cre_tsk(ID tskid, T_CTSK *pk_ctsk);
    ER_ID	acre_tsk(T_CTSK *pk_ctsk);
	ER		del_tsk(ID tskid);
    ER		act_tsk(ID tskid);
#define		iact_tsk(tskid) act_tsk((tskid))
    ER_UINT	can_act(ID tskid);
	ER		sta_tsk(ID tskid, VP_INT stacd);
    void	ext_tsk(void);
	void	exd_tsk(void);
	ER		ter_tsk(ID tskid);
	ER		chg_pri(ID tskid, TPRI tskpri);
    ER		get_pri(ID tskid, PRI *p_tskpri);
    ER		ref_tsk(ID tskid, T_RTSK *pk_rtsk);
    ER		ref_tst(ID tskid, T_RTST *pk_rtst);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*						 Task-dependent synchronization						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
	ER		tslp_tsk(TMO tmout);
#define slp_tsk() tslp_tsk(TMO_FEVR)
	ER		wup_tsk(ID tskid);
#define		iwup_tsk(tskid) wup_tsk((tskid))
    ER_UINT	can_wup(ID tskid);
	ER		rel_wai(ID tskid);
#define		irel_wai(tskid) rel_wai((tskid))
	ER		sus_tsk(ID tskid);
	ER		rsm_tsk(ID tskid);
	ER		frsm_tsk(ID tskid);
    ER		dly_tsk(RELTIM dlytim);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*						 Task exception										*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    ER		def_tex(ID tskid, T_DTEX *pk_dtex);
    ER		ras_tex(ID tskid, TEXPTN rasptn);
#define		iras_tex(tskid, rasptn) ras_tex((tskid), (rasptn))
    ER		dis_tex();
    ER		ena_tex();
    BOOL	sns_tex();
    ER		ref_tex(ID tskid, T_RTEX *pk_rtex);
    
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*					  Synchronization and communication						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*--------- semaphore ----------*/
	ER		cre_sem(ID semid, T_CSEM *pk_csem);
    ER		acre_sem(T_CSEM *pk_csem);
	ER		del_sem(ID semid);
	ER		sig_sem(ID semid);
#define		isig_sem(semid) sig_sem((semid))
	ER		twai_sem(ID semid, TMO tmout);
#define		wai_sem(semid) twai_sem((semid), TMO_FEVR)
#define		pol_sem(semid) twai_sem((semid), TMO_POL)
    ER		ref_sem(ID semid, T_RSEM *pk_rsem);

/*--------- eventflag ----------*/
    ER		cre_flg(ID flgid, T_CFLG *pk_cflg);
    ER_ID	acre_flg(T_CFLG *pk_cflg);
    ER		del_flg(ID flgid);
	ER		set_flg(ID flgid, FLGPTN setptn);
#define		iset_flg(flgid, setptn) set_flg((flgid), (setptn))
	ER		clr_flg(ID flgid, UW clrptn);
	ER		twai_flg(ID flgid, FLGPTN waiptn, MODE wfmode, FLGPTN *p_flgptn, TMO tmout);
#define		wai_flg(flgid, waiptn, wfmode, p_flgptn) twai_flg((flgid), (waiptn), (wfmode), (p_flgptn), TMO_FEVR)
	ER		pol_flg(ID flgid, FLGPTN waiptn, MODE wfmode, FLGPTN *p_flgptn);
    ER		ref_flg(ID flgid, T_RFLG *pk_rflg);
    
/*--------- dataque ----------*/
    ER		cre_dtq(ID dtqid, T_CDTQ *pk_cdtq);
    ER_ID	acre_dtq(T_CDTQ *pk_cdtq);
    ER		del_dtq(ID dtqid);
    ER		tsnd_dtq(ID dtqid, VP_INT data, TMO tmout);
#define		snd_dtq(dtqid, data) tsnd_dtq((dtqid), (data), TMO_FEVR)
#define		psnd_dtq(dtqid, data) tsnd_dtq((dtqid), (data), TMO_POL)
#define 	ipsnd_dtq psnd_dtq
    ER		fsnd_dtq(ID dtqid, VP_INT data);
#define		ifsnd_dtq(dtqid, data) fsnd_dtq((dtqid), (data))
    ER		trcv_dtq(ID dtqid, VP_INT *p_data, TMO tmout);
#define		rcv_dtq(dtqid, p_data) trcv_dtq((dtqid), (p_data), TMO_FEVR)
#define		prcv_dtq(dtqid, p_data) trcv_dtq((dtqid), (p_data), TMO_POL)
    ER		ref_dtq(ID dtqid, T_RDTQ *pk_rdtq);
    
/*--------- mailbox ----------*/
	ER		cre_mbx(ID mbxid, T_CMBX *pk_cmbx);
    ER_ID	acre_mbx(T_CMBX *pk_cmbx);
	ER		del_mbx(ID mbxid);
	ER		snd_mbx(ID mbxid, T_MSG *pk_msg);
	ER		trcv_mbx(ID mbxid, T_MSG **ppk_msg, TMO tmout);
#define		rcv_mbx(mbxid, ppk_msg) trcv_mbx((mbxid), (ppk_msg), TMO_FEVR)
#define		prcv_mbx(mbxid, ppk_msg) trcv_mbx((mbxid), (ppk_msg), TMO_POL)
	ER		ref_mbx(ID mbxid, T_RMBX *pk_rmbx);
    
/*--------- mutex ----------*/
    ER		cre_mtx(ID mtxid, T_CMTX *pk_cmtx);
    ER_ID	acre_mtx(T_CMTX *pk_cmtx);
    ER		del_mtx(ID mtxid);
    ER		tloc_mtx(ID mtxid, TMO tmout);
#define		loc_mtx(mtxid) tloc_mtx((mtxid), TMO_FEVR)
#define		ploc_mtx(mtxid) tloc_mtx((mtxid), TMO_POL)
    ER		unl_mtx(ID mtxid);
    ER		ref_mtx(ID mtxid, T_RMTX *pk_rmtx);

/*--------- message buffer ----------*/
    ER		cre_mbf(ID mbfid, T_CMBF *pk_cmbf);
    ER_ID	acre_mbf(T_CMBF *pk_cmbf);
    ER		del_mbf(ID mbfid);
    ER		tsnd_mbf(ID mbfid, VP msg, UINT msgsz, TMO tmout);
#define		snd_mbf(mbfid, msg, msgsz) tsnd_mbf((mbfid), (msg), (msgsz), TMO_FEVR)
#define		psnd_mbf(mbfid, msg, msgsz) tsnd_mbf((mbfid), (msg), (msgsz), TMO_POL)
    ER_UINT	trcv_mbf(ID mbfid, VP msg, TMO tmout);
#define		rcv_mbf(mbfid, msg) trcv_mbf((mbfid), (msg), TMO_FEVR)
#define		prcv_mbf(mbfid, msg) trcv_mbf((mbfid), (msg), TMO_POL)
    ER		ref_mbf(ID mbfid, T_RMBF *pk_rmbf);
    
/*--------- randevu ----------*/
    ER		cre_por(ID porid, T_CPOR *pk_cpor);
    ER_ID	acre_por(T_CPOR *pk_cpor);
    ER		del_por(ID porid);
    ER_UINT	tcal_por(ID porid, RDVPTN calptn, VP msg, UINT cmsgsz, TMO tmoout);
#define		cal_por(porid, calptn, msg, cmsgsz) tcal_por((porid), (calptn), (msg), (cmsgsz), TMO_FEVR)
#define		pcal_por(porid, calptn, msg, cmsgsz) tcal_por((porid), (calptn), (msg), (cmsgsz), TMO_POL)
    ER_UINT	tacp_por(ID porid, RDVPTN acpptn, RDVNO *p_rdvno, VP msg, TMO tmout);
#define		pacp_por(porid, acpptn, p_rdvno, msg) tacp_por((porid), (acpptn), (p_rdvno), (msg), TMO_POL)
    ER		fwd_por(ID porid, RDVPTN calptn, RDVNO rdvno, VP msg, UINT cmsgsz);
    ER		rpl_rdv(RDVNO rdvno, VP msg, UINT rmsgsz);
    ER		ref_por(ID porid, T_RPOR *pk_rpor);
    ER		ref_rdv(RDVNO rdvno, T_RRDV *pk_rrdv);
        
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*							 Interrupt management							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*							 Memorypool management							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*--------- fixed memorypool ----------*/
    ER		cre_mpf(ID mpfid, T_CMPF *pk_cmpf);
    ER_ID	acre_mpf(T_CMPF *pk_cmpf);
    ER		del_mpf(ID mpfid);
    ER		tget_mpf(ID mpfid, VP *p_blk, TMO tmout);
#define		get_mpf(mpfid, p_blk) tget_mpf((mpfid), (p_blk), TMO_FEVR)
#define		pget_mpf(mpfid, p_blk) tget_mpf((mpfid), (p_blk), TMO_POL)
    ER		rel_mpf(ID mpfid, VP blk);
    ER		ref_mpf(ID mpfid, T_RMPF *pk_rmpf);
    
/*--------- valiable memorypool ----------*/
    ER		cre_mpl(ID mplid, T_CMPL *pk_cmpl);
    ER_ID	acre_mpl(T_CMPL *pk_cmpl);
	ER		tget_mpl(ID mplid, INT blksz, VP *p_blk, TMO tmout);
#define		get_mpl(p_blk, mplid, blksz ) tget_mpl((p_blk), (mplid), (blksz), TMO_FEVR)
#define		pget_mpl(p_blk, mplid, blksz ) tget_mpl((p_blk), (mplid), (blksz), TMO_POL)
	ER		rel_mpl(ID mplid, VP blk);
    ER		ref_mpl(ID mplid, T_RMPL *pk_rmpl);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*								Time management								*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*--------- system time management ----------*/
	ER		set_tim(SYSTIM *p_systim);
	ER		get_tim(SYSTIM *p_systim);
	ER		isig_tim();

/*--------- cyclic handler ----------*/
    ER		cre_cyc(ID cycid, T_CCYC *pk_ccyc);
    ER_ID	acre_cyc(T_CCYC *pk_ccyc);
    ER		del_cyc(ID cycid);
    ER		sta_cyc(ID cycid);
    ER		stp_cyc(ID cycid);
    ER		ref_cyc(ID cycid, T_RCYC *pk_rcyc);
    
/*--------- alarm handler ----------*/
    ER		cre_alm(ID almid, T_CALM *pk_calm);
    ER_ID	acre_alm(T_CALM *pk_calm);
    ER		del_alm(ID almid);
    ER		sta_alm(ID almid, RELTIM almtim);
    ER		stp_alm(ID almid);
    ER		ref_alm(ID almid, T_RALM *pk_ralm);
    
/*--------- overrun handler ----------*/
    ER		def_ovr(T_DOVR *pk_dovr);
    ER		sta_ovr(ID tskid, OVRTIM ovrtim);
    ER		stp_ovr(ID tskid);
    ER		ref_ovr(ID tskid, T_ROVR *pk_rovr);
    
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*							   System management							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
	ER		rot_rdq(PRI tskpri);
#define		irot_rdq(tskpri) rot_rdq((tskpri))
	ER		get_tid(ID *p_tskid);
#define		iget_tid(p_tskid) get_tid((p_tskid))
    ER		loc_cpu();
#define		iloc_cpu() loc_cpu()
    ER		unl_cpu();
#define		iunl_cpu() unl_cpu()
    ER		dis_dsp();
    ER		ena_dsp();
    BOOL	sns_ctx();
    BOOL	sns_loc();
    BOOL	sns_dsp();
    BOOL	sns_dpn();
    ER		ref_sys(T_RSYS *pk_rsys);
    
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*							   Interrupt management							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#if 0
    ER		def_inh(INHNO inhno, T_DINH *pk_dinh);
    ER		cre_isr(ID isrid, T_CISR *pk_cisr);
    ER_ID	acre_isr(T_CISR *pk_cisr);
    ER		del_isr(ID isrid);
    ER		ref_isr(ID isrid, T_RISR *pk_risr);
    ER		dis_int(INTNO intno);
    ER		ena_int(INTNO intno);
#endif

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*							   Servicecall management						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    ER		def_svc(FN fncd, T_DSVC *pk_dsvc);
    ER_UINT	cal_svc(FN fncd, VP_INT par1, VP_INT par2, ...);
    
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*							 System status management						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    ER		def_exc(EXCNO excno, T_DEXC *pk_dexc);
    ER		ref_cfg(T_RCFG *pk_rcfg);
	ER		ref_ver(T_RVER *pk_rver);

#endif
