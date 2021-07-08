#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "itron.h"
#include "depend.h"
#include "common.h"

/***** structure definision *****/
typedef struct {						/* semaphore management structure	*/
	BOOL created;						/* semaphore object created			*/
	UINT count;							/* semaphore counter				*/
	UINT max;							/* max semaphore count				*/
    pthread_mutex_t mutex;				/* management info access mutex		*/
} T_SEM_MAN;

typedef struct {						/* event flag management structure	*/
	BOOL created;						/* event flag object created		*/
	ATR flgatr;							/* event flag attribute				*/
	FLGPTN flgptn;						/* event flag pattern				*/
    pthread_mutex_t mutex;				/* management info access mutex		*/
} T_FLG_MAN;

typedef struct {						/* event flag wait info				*/
	MODE wfmode;						/* event flag wait mode				*/
	FLGPTN waiptn;						/* event flag wait pattern			*/
	FLGPTN *p_flgptn;					/* event flag released value		*/
} T_FLG_WINF;

typedef struct {						/* dataque management structure		*/
	BOOL created;						/* dataque object created			*/
	UINT dtqcnt;						/* dataque count					*/
	VP dtq;								/* dataque area						*/
	BOOL malloced;						/* malloced dataque area			*/
	UINT enq;							/* enqueue pointer					*/
	UINT ent;							/* number of entry data				*/
    pthread_mutex_t mutex;				/* management info access mutex		*/
} T_DTQ_MAN;

typedef struct {						/* mailbox management structure		*/
	BOOL created;						/* mailbox object created			*/
	ATR mbxatr;							/* mailbox attribute				*/
	PRI maxmpri;						/* mailbox sending message primax	*/
	T_MSG_PRIHD *mprihd;				/* mailbox message queue header area*/
    pthread_mutex_t mutex;				/* management info access mutex		*/
} T_MBX_MAN;

/***** global valiable *****/
static volatile T_SEM_MAN gsSemman[TMAX_SEM_ID];
static volatile T_FLG_MAN gsFlgman[TMAX_FLG_ID];
static volatile T_DTQ_MAN gsDtqman[TMAX_DTQ_ID];
static volatile T_MBX_MAN gsMbxman[TMAX_MBX_ID];
static volatile pthread_mutex_t gsSemmtx, gsFlgmtx, gsDtqmtx, gsMbxmtx;

/***** local function prototype *****/
    
    
/***** functions *****/

/***** semaphore *****/
ER inz_sem(void)
{
	memset(gsSemman, 0, sizeof(gsSemman));
	PCHK(pthread_mutex_init(&gsSemmtx, NULL) == 0, E_SYS);
	return E_OK;
}

ER cre_sem(
	ID semid,
	T_CSEM *pk_csem
	)
{
	ER res, err;
	
	PCHK(1 <= semid && semid <= TMAX_SEM_ID, E_ID);
	PCHK(pk_csem != NULL && pk_csem->isemcnt <= pk_csem->maxsem, E_PAR);
	PCHK(0 < pk_csem->maxsem && pk_csem->maxsem <= TMAX_MAXSEM, E_PAR);
	PCHK((pk_csem->sematr & ~(TA_TFIFO | TA_TPRI)) == 0, E_RSATR);
	PCHK(pthread_mutex_lock(&gsSemmtx) == 0, E_SYS);
	EREXIT(res, (gsSemman[semid].created == FALSE)? E_OK : E_OBJ, ERR_EXIT);
    EREXIT(res, (pthread_mutex_init(&gsSemman[semid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
	EREXIT(res, (cre_oqe(TCD_OBJ_SEM, semid, pk_csem->sematr) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
	gsSemman[semid].created = TRUE;
	gsSemman[semid].count = pk_csem->isemcnt;
	gsSemman[semid].max = pk_csem->maxsem;
	goto ERR_EXIT;
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsSemman[semid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsSemmtx) == 0, E_SYS);
	return res;
}


ER_ID acre_sem(
	T_CSEM *pk_csem
	)
{
	ER_ID res, err;
	ID semid;
	
	PCHK(pk_csem != NULL && pk_csem->isemcnt <= pk_csem->maxsem, E_PAR);
	PCHK(0 < pk_csem->maxsem && pk_csem->maxsem <= TMAX_MAXSEM, E_PAR);
	PCHK((pk_csem->sematr & ~(TA_TFIFO | TA_TPRI)) == 0, E_RSATR);
	PCHK(pthread_mutex_lock(&gsSemmtx) == 0, E_SYS);
	for (semid=1; semid<=TMAX_SEM_ID; semid++){
		if (gsSemman[semid].created == TRUE){
			continue;
		}
		EREXIT(res, (pthread_mutex_init(&gsSemman[semid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
		EREXIT(res, (cre_oqe(TCD_OBJ_SEM, semid, pk_csem->sematr) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
		gsSemman[semid].created = TRUE;
		gsSemman[semid].count = pk_csem->isemcnt;
		gsSemman[semid].max = pk_csem->maxsem;
		break;
	}
	res = (semid > TMAX_SEM_ID)? E_NOID : (ER_ID)semid;
	goto ERR_EXIT;
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsSemman[semid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsSemmtx) == 0, E_SYS);
	return res;
}


ER del_sem(
	ID semid
	)
{
	ER res;
	INT err;
	ID tskid;
	
	PCHK(1 <= semid && semid <= TMAX_SEM_ID, E_ID);
	PCHK(pthread_mutex_lock(&gsSemmtx) == 0, E_SYS);
	EREXIT(res, (pthread_mutex_lock(&gsSemman[semid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	EREXIT(res, (gsSemman[semid].created == TRUE)? E_OK : E_NOEXS, ERR_EXIT);
	gsSemman[semid].created = FALSE;
	EREXIT(res, (pthread_mutex_unlock(&gsSemman[semid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	while (deq_oqe(TCD_OBJ_SEM, semid, &tskid, NULL) == E_OK && tskid != TSK_NONE){
		res |= sys_wup_tsk(tskid, TFN_DEL_SEM, TTS_DLT);
	}
	res = chg_pri(TSK_SELF, TMAX_TSK_PRI);	/* for waiting low priority task wake up */
	res = del_oqe(TCD_OBJ_SEM, semid);
	EREXIT(res, (pthread_mutex_destroy(&gsSemman[semid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	memset(&gsSemman[semid], 0, sizeof(T_SEM_MAN));
	goto ERR_EXIT2;
ERR_EXIT:
	err = pthread_mutex_unlock(&gsSemman[semid].mutex);
ERR_EXIT2:
	PCHK(pthread_mutex_unlock(&gsSemmtx) == 0, E_SYS);
	return res;
}


ER sig_sem(
	ID semid
	)
{
	ER res, err;
	ID tskid;
	
	PCHK(1 <= semid && semid <= TMAX_SEM_ID, E_ID);
	CHK_EXIST(res, err, gsSemmtx, gsSemman[semid]);
	EREXIT(res, (gsSemman[semid].count < gsSemman[semid].max)? E_OK : E_QOVR, ERR_EXIT);
	gsSemman[semid].count++;
	EREXIT(res, deq_oqe(TCD_OBJ_SEM, semid, &tskid, NULL), ERR_EXIT);
	if (tskid != TSK_NONE){
		res = sys_wup_tsk(tskid, TFN_TWAI_SEM, 0);
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsSemman[semid].mutex) == 0, E_SYS);
	return res;
}


ER twai_sem(
	ID semid,
	TMO tmout
	)
{
	ER res, err;
	
	PCHK(1 <= semid && semid <= TMAX_SEM_ID, E_ID);
	PCHK(0 <= tmout || tmout == TMO_FEVR, E_PAR);
	CHK_EXIST(res, err, gsSemmtx, gsSemman[semid]);
	if (gsSemman[semid].count == 0){
		EREXIT(res, wai_oqe(TCD_OBJ_SEM, semid, TSK_SELF, NULL, tmout, TFN_TWAI_SEM, &gsSemman[semid].mutex), ERR_EXIT);
	}
	gsSemman[semid].count--;
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsSemman[semid].mutex) == 0, E_SYS);
	return res;
}


ER ref_sem(
	ID semid,
	T_RSEM *pk_rsem
	)
{
	ER res, err;
	
	PCHK(1 <= semid && semid <= TMAX_SEM_ID, E_ID);
	PCHK(pk_rsem != NULL, E_PAR);
	CHK_EXIST(res, err, gsSemmtx, gsSemman[semid]);
	res = ref_oqe(TCD_OBJ_SEM, semid, 0, &pk_rsem->wtskid, NULL);
	pk_rsem->semcnt = gsSemman[semid].count;
	res = (pk_rsem->wtskid != TSK_NONE && pk_rsem->semcnt != 0)? E_SYS : E_OK;
	PCHK(pthread_mutex_unlock(&gsSemman[semid].mutex) == 0, E_SYS);
	return res;
}


/***** event flag *****/
ER inz_flg(void)
{
	memset(&gsFlgman[0], 0, sizeof(gsFlgman));
	PCHK(pthread_mutex_init(&gsFlgmtx, NULL) == 0, E_SYS);
	return E_OK;
}

ER cre_flg(
	ID flgid,
	T_CFLG *pk_cflg
	)
{
	ER res, err;
	
	PCHK(1 <= flgid && flgid <= TMAX_FLG_ID, E_ID);
	PCHK(pk_cflg != NULL, E_PAR);
	PCHK((pk_cflg->flgatr & ~(TA_TFIFO | TA_TPRI | TA_WSGL | TA_WMUL)) == 0, E_RSATR);
	PCHK(pthread_mutex_lock(&gsFlgmtx) == 0, E_SYS);
	EREXIT(res, (gsFlgman[flgid].created == FALSE)? E_OK : E_OBJ, ERR_EXIT);
    EREXIT(res, (pthread_mutex_init(&gsFlgman[flgid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
	if (pk_cflg->flgatr & TA_WMUL){
		EREXIT(res, (cre_oqe(TCD_OBJ_FLG, flgid, pk_cflg->flgatr) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
	}
	gsFlgman[flgid].created = TRUE;
	gsFlgman[flgid].flgatr = pk_cflg->flgatr;
	gsFlgman[flgid].flgptn = pk_cflg->iflgptn;
	goto ERR_EXIT;
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsFlgman[flgid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsFlgmtx) == 0, E_SYS);
	return res;
}


ER_ID acre_flg(
	T_CFLG *pk_cflg
	)
{
	ER_ID res, err;
	ID flgid;
	
	PCHK(pk_cflg != NULL, E_PAR);
	PCHK((pk_cflg->flgatr & ~(TA_TFIFO | TA_TPRI | TA_WSGL | TA_WMUL)) == 0, E_RSATR);
	PCHK(pthread_mutex_lock(&gsFlgmtx) == 0, E_SYS);
	for (flgid=1; flgid<=TMAX_FLG_ID; flgid++){
		if (gsFlgman[flgid].created == TRUE){
			continue;
		}
		EREXIT(res, (pthread_mutex_init(&gsFlgman[flgid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
		EREXIT(res, (cre_oqe(TCD_OBJ_FLG, flgid, pk_cflg->flgatr) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
		gsFlgman[flgid].created = TRUE;
		gsFlgman[flgid].flgatr = pk_cflg->flgatr;
		gsFlgman[flgid].flgptn = pk_cflg->iflgptn;
		break;
	}
	res = (flgid > TMAX_FLG_ID)? E_NOID : (ER_ID)flgid;
	goto ERR_EXIT;
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsFlgman[flgid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsFlgmtx) == 0, E_SYS);
	return res;
}


ER del_flg(
	ID flgid
	)
{
	ER res;
	INT err;
	ID tskid;
	T_FLG_WINF *winf;
	
	PCHK(1 <= flgid && flgid <= TMAX_FLG_ID, E_ID);
	PCHK(pthread_mutex_lock(&gsFlgmtx) == 0, E_SYS);
	EREXIT(res, (pthread_mutex_lock(&gsFlgman[flgid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	EREXIT(res, (gsFlgman[flgid].created == TRUE)? E_OK : E_NOEXS, ERR_EXIT);
	gsFlgman[flgid].created = FALSE;
	EREXIT(res, (pthread_mutex_unlock(&gsFlgman[flgid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	while (deq_oqe(TCD_OBJ_FLG, flgid, &tskid, (VP *)&winf) == E_OK && tskid != TSK_NONE){
		free(winf);
		res |= sys_wup_tsk(tskid, TFN_DEL_FLG, TTS_DLT);
	}
	res = chg_pri(TSK_SELF, TMAX_TSK_PRI);	/* for waiting low priority task wake up */
	res = del_oqe(TCD_OBJ_FLG, flgid);
	EREXIT(res, (pthread_mutex_destroy(&gsFlgman[flgid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	memset(&gsFlgman[flgid], 0, sizeof(T_FLG_MAN));
	goto ERR_EXIT2;
ERR_EXIT:
	err = pthread_mutex_unlock(&gsFlgman[flgid].mutex);
ERR_EXIT2:
	PCHK(pthread_mutex_unlock(&gsFlgmtx) == 0, E_SYS);
	return res;
}


ER set_flg(
	ID flgid,
	FLGPTN setptn
	)
{
	ER res, err;
	ID tskid;
	T_FLG_WINF *winf;
	INT i;
	
	PCHK(1 <= flgid && flgid <= TMAX_FLG_ID, E_ID);
	CHK_EXIST(res, err, gsFlgmtx, gsFlgman[flgid]);
	gsFlgman[flgid].flgptn |= setptn;
	for (i=0, res = E_OK; ref_oqe(TCD_OBJ_FLG, flgid, i, &tskid, (VP *)&winf) == E_OK && tskid != TSK_NONE; i++){
		if ((winf->wfmode == TWF_ANDW && (winf->waiptn & gsFlgman[flgid].flgptn) == winf->waiptn) ||
			(winf->wfmode == TWF_ORW && (winf->waiptn & gsFlgman[flgid].flgptn) != 0)){
			res = get_oqe(TCD_OBJ_FLG, flgid, i, &tskid, (VP *)&winf);
			*(winf->p_flgptn) = gsFlgman[flgid].flgptn;
			free(winf);
			res = sys_wup_tsk(tskid, TFN_SET_FLG, 0);
			if (gsFlgman[flgid].flgatr & TA_CLR){
				gsFlgman[flgid].flgptn = 0;
				break;
			}
		}
	}
	PCHK(pthread_mutex_unlock(&gsFlgman[flgid].mutex) == 0, E_SYS);
	return res;
}


ER clr_flg(
	ID flgid,
	FLGPTN clrptn
	)
{
	ER res, err;
	
	PCHK(1 <= flgid && flgid <= TMAX_FLG_ID, E_ID);
	CHK_EXIST(res, err, gsFlgmtx, gsFlgman[flgid]);
	gsFlgman[flgid].flgptn &= clrptn;
	PCHK(pthread_mutex_unlock(&gsFlgman[flgid].mutex) == 0, E_SYS);
	return res;
}


ER twai_flg(
	ID flgid,
	FLGPTN waiptn,
	MODE wfmode,
	FLGPTN *p_flgptn,
	TMO tmout
	)
{
	ER res, err;
	T_FLG_WINF *winf;
	ID tskid;
	
	PCHK(1 <= flgid && flgid <= TMAX_FLG_ID, E_ID);
	PCHK(p_flgptn != NULL && waiptn != 0, E_PAR);
	CHK_EXIST(res, err, gsFlgmtx, gsFlgman[flgid]);
	if ((wfmode == TWF_ANDW && (waiptn & gsFlgman[flgid].flgptn) == waiptn) ||
		(wfmode == TWF_ORW && (waiptn & gsFlgman[flgid].flgptn) != 0)){
		*p_flgptn = gsFlgman[flgid].flgptn;
	}else{
		EREXIT(res, (tmout != TMO_POL)? E_OK : E_TMOUT, ERR_EXIT);
		if (!(gsFlgman[flgid].flgatr & TA_WMUL)){
			EREXIT(res, ref_oqe(TCD_OBJ_FLG, flgid, 0, &tskid, NULL), ERR_EXIT);
			EREXIT(res, (tskid == TSK_NONE)? E_OK : E_ILUSE, ERR_EXIT);
		}
		EREXIT(res, ((winf = malloc(sizeof(T_FLG_WINF))) != NULL)? E_OK : E_SYS, ERR_EXIT);
		winf->wfmode = wfmode;
		winf->waiptn = waiptn;
		winf->p_flgptn = p_flgptn;
		res = wai_oqe(TCD_OBJ_FLG, flgid, TSK_SELF, winf, tmout, TFN_TWAI_FLG, &gsFlgman[flgid].mutex);
		if (res != E_OK){
			free(winf);
			goto ERR_EXIT;
		}
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsFlgman[flgid].mutex) == 0, E_SYS);
	return res;
}


ER ref_flg(
	ID flgid,
	T_RFLG *pk_rflg
	)
{
	ER res, err;
	ID tskid;
	
	PCHK(1 <= flgid && flgid <= TMAX_FLG_ID, E_ID);
	PCHK(pk_rflg != NULL, E_PAR);
	CHK_EXIST(res, err, gsFlgmtx, gsFlgman[flgid]);
	EREXIT(res, ref_oqe(TCD_OBJ_FLG, flgid, 0, &tskid, NULL), ERR_EXIT);
	pk_rflg->wtskid = tskid;
	pk_rflg->flgptn = gsFlgman[flgid].flgptn;
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsFlgman[flgid].mutex) == 0, E_SYS);
	return res;
}


/***** data queue *****/
ER inz_dtq(void)
{
	memset(&gsDtqman[0], 0, sizeof(gsDtqman));
	PCHK(pthread_mutex_init(&gsDtqmtx, NULL) == 0, E_SYS);
	return E_OK;
}

ER cre_dtq(
	ID dtqid,
	T_CDTQ *pk_cdtq
	)
{
	ER res, err;
	
	PCHK(1 <= dtqid && dtqid <= TMAX_DTQ_ID, E_ID);
	PCHK(pk_cdtq != NULL && pk_cdtq->dtqcnt <= (UINT)((INT)-1), E_PAR);
	PCHK((pk_cdtq->dtqatr & ~(TA_TFIFO | TA_TPRI)) == 0, E_RSATR);
	PCHK(pthread_mutex_lock(&gsDtqmtx) == 0, E_SYS);
	EREXIT(res, (gsDtqman[dtqid].created == FALSE)? E_OK : E_OBJ, ERR_EXIT);
    EREXIT(res, (pthread_mutex_init(&gsDtqman[dtqid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
	EREXIT(res, (cre_oqe(TCD_OBJ_DTQS, dtqid, pk_cdtq->dtqatr) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
	EREXIT(res, (cre_oqe(TCD_OBJ_DTQR, dtqid, pk_cdtq->dtqatr) == E_OK)? E_OK : E_SYS, ERR_EXIT3);
	gsDtqman[dtqid].created = TRUE;
	gsDtqman[dtqid].dtqcnt = pk_cdtq->dtqcnt;
	if (pk_cdtq->dtq == NULL){
		gsDtqman[dtqid].dtq = malloc(TSZ_DTQ * (pk_cdtq->dtqcnt)? pk_cdtq->dtqcnt : 1);
		gsDtqman[dtqid].malloced = TRUE;
	}else{
		gsDtqman[dtqid].dtq = pk_cdtq->dtq;
	}
	gsDtqman[dtqid].enq = 0;
	gsDtqman[dtqid].ent = 0;
	res = (gsDtqman[dtqid].dtq != NULL)? E_OK : E_NOMEM;
	goto ERR_EXIT;
ERR_EXIT3:
	err = del_oqe(TCD_OBJ_DTQS, dtqid);
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsDtqman[dtqid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsDtqmtx) == 0, E_SYS);
	return res;
}


ER_ID acre_dtq(
	T_CDTQ *pk_cdtq
	)
{
	ER_ID res, err;
	ID dtqid;
	
	PCHK(pk_cdtq != NULL && pk_cdtq->dtqcnt <= (UINT)((INT)-1), E_PAR);
	PCHK((pk_cdtq->dtqatr & ~(TA_TFIFO | TA_TPRI)) == 0, E_RSATR);
	PCHK(pthread_mutex_lock(&gsDtqmtx) == 0, E_SYS);
	for (dtqid=1; dtqid<=TMAX_DTQ_ID; dtqid++){
		if (gsDtqman[dtqid].created == TRUE){
			continue;
		}
		EREXIT(res, (pthread_mutex_init(&gsDtqman[dtqid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
		EREXIT(res, (cre_oqe(TCD_OBJ_DTQS, dtqid, pk_cdtq->dtqatr) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
		EREXIT(res, (cre_oqe(TCD_OBJ_DTQR, dtqid, pk_cdtq->dtqatr) == E_OK)? E_OK : E_SYS, ERR_EXIT3);
		gsDtqman[dtqid].created = TRUE;
		gsDtqman[dtqid].dtqcnt = pk_cdtq->dtqcnt;
		if (pk_cdtq->dtq == NULL){
			gsDtqman[dtqid].dtq = malloc(TSZ_DTQ * (pk_cdtq->dtqcnt)? pk_cdtq->dtqcnt : 1);
			gsDtqman[dtqid].malloced = TRUE;
		}else{
			gsDtqman[dtqid].dtq = pk_cdtq->dtq;
		}
		gsDtqman[dtqid].enq = 0;
		gsDtqman[dtqid].ent = 0;
		res = (gsDtqman[dtqid].dtq != NULL)? E_OK : E_NOMEM;
		break;
	}
	res = (dtqid > TMAX_DTQ_ID)? E_NOID : (ER_ID)dtqid;
ERR_EXIT3:
	err = del_oqe(TCD_OBJ_DTQS, dtqid);
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsDtqman[dtqid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsDtqmtx) == 0, E_SYS);
	return res;
}


ER del_dtq(
	ID dtqid
	)
{
	ER res;
	INT err;
	ID tskid;
	
	PCHK(1 <= dtqid && dtqid <= TMAX_SEM_ID, E_ID);
	PCHK(pthread_mutex_lock(&gsDtqmtx) == 0, E_SYS);
	EREXIT(res, (pthread_mutex_lock(&gsDtqman[dtqid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	EREXIT(res, (gsDtqman[dtqid].created == TRUE)? E_OK : E_NOEXS, ERR_EXIT);
	gsDtqman[dtqid].created = FALSE;
	EREXIT(res, (pthread_mutex_unlock(&gsDtqman[dtqid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	while (deq_oqe(TCD_OBJ_DTQS, dtqid, &tskid, NULL) == E_OK && tskid != TSK_NONE){
		res |= sys_wup_tsk(tskid, TFN_DEL_DTQ, TTS_DLT);
	}
	while (deq_oqe(TCD_OBJ_DTQR, dtqid, &tskid, NULL) == E_OK && tskid != TSK_NONE){
		res |= sys_wup_tsk(tskid, TFN_DEL_DTQ, TTS_DLT);
	}
	res = chg_pri(TSK_SELF, TMAX_TSK_PRI);	/* for waiting low priority task wake up */
	res = del_oqe(TCD_OBJ_DTQS, dtqid);
	res = del_oqe(TCD_OBJ_DTQR, dtqid);
	EREXIT(res, (pthread_mutex_destroy(&gsDtqman[dtqid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	memset(&gsDtqman[dtqid], 0, sizeof(T_DTQ_MAN));
	goto ERR_EXIT2;
ERR_EXIT:
	err = pthread_mutex_unlock(&gsDtqman[dtqid].mutex);
ERR_EXIT2:
	PCHK(pthread_mutex_unlock(&gsDtqmtx) == 0, E_SYS);
	return res;
}


ER tsnd_dtq(
	ID dtqid,
	VP_INT data,
	TMO tmout
	)
{
	ER res, err;
	UINT *dtq;
	ID tskid;
	
	PCHK(1 <= dtqid && dtqid <= TMAX_DTQ_ID, E_ID);
	PCHK(0 <= tmout || tmout == TMO_FEVR, E_PAR);
	CHK_EXIST(res, err, gsDtqmtx, gsDtqman[dtqid]);
	dtq = (UINT *)gsDtqman[dtqid].dtq;
RETRY:
	if (gsDtqman[dtqid].dtqcnt == 0){
		EREXIT(res, deq_oqe(TCD_OBJ_DTQR, dtqid, &tskid, NULL), ERR_EXIT);
		if (tskid != TSK_NONE){
			dtq[0] = (UINT)data;
			res = sys_wup_tsk(tskid, TFN_TSND_DTQ, 0);
		}else{
			EREXIT(res, wai_oqe(TCD_OBJ_DTQS, dtqid, TSK_SELF, NULL, tmout, TFN_TSND_DTQ, &gsDtqman[dtqid].mutex), ERR_EXIT);
			goto RETRY;
		}
	}else if ((gsDtqman[dtqid].ent + 1) > gsDtqman[dtqid].dtqcnt){
		EREXIT(res, wai_oqe(TCD_OBJ_DTQS, dtqid, TSK_SELF, NULL, tmout, TFN_TSND_DTQ, &gsDtqman[dtqid].mutex), ERR_EXIT);
		tmout = 0;	// if timed out event then return E_TMOUT
		goto RETRY;
	}else{
		dtq[gsDtqman[dtqid].enq] = (UINT)data;
		gsDtqman[dtqid].enq = (gsDtqman[dtqid].enq + 1) % gsDtqman[dtqid].dtqcnt;
		gsDtqman[dtqid].ent++;
		EREXIT(res, deq_oqe(TCD_OBJ_DTQR, dtqid, &tskid, NULL), ERR_EXIT);
		if (tskid != TSK_NONE){
			res = sys_wup_tsk(tskid, TFN_TSND_DTQ, 0);
		}
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsDtqman[dtqid].mutex) == 0, E_SYS);
	return res;
}


ER fsnd_dtq(
	ID dtqid,
	VP_INT data
	)
{
	ER res, err;
	UINT *dtq;
	ID tskid;
	
	PCHK(1 <= dtqid && dtqid <= TMAX_DTQ_ID, E_ID);
	CHK_EXIST(res, err, gsDtqmtx, gsDtqman[dtqid]);
	EREXIT(res, (gsDtqman[dtqid].dtqcnt > 0)? E_OK : E_ILUSE, ERR_EXIT);
	dtq = (UINT *)gsDtqman[dtqid].dtq;
	if ((gsDtqman[dtqid].ent + 1) > gsDtqman[dtqid].dtqcnt){
		gsDtqman[dtqid].ent--;
	}
	dtq[gsDtqman[dtqid].enq] = (UINT)data;
	gsDtqman[dtqid].enq = (gsDtqman[dtqid].enq + 1) % gsDtqman[dtqid].dtqcnt;
	gsDtqman[dtqid].ent++;
	EREXIT(res, deq_oqe(TCD_OBJ_DTQR, dtqid, &tskid, NULL), ERR_EXIT);
	if (tskid != TSK_NONE){
		res = sys_wup_tsk(tskid, TFN_FSND_DTQ, 0);
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsDtqman[dtqid].mutex) == 0, E_SYS);
	return res;
}


ER trcv_dtq(
	ID dtqid,
	VP_INT *data,
	TMO tmout
	)
{
	ER res, err;
	UINT *dtq;
	ID tskid;
	
	PCHK(1 <= dtqid && dtqid <= TMAX_DTQ_ID, E_ID);
	PCHK(0 <= tmout || tmout == TMO_FEVR, E_PAR);
	CHK_EXIST(res, err, gsDtqmtx, gsDtqman[dtqid]);
	dtq = (UINT *)gsDtqman[dtqid].dtq;
	EREXIT(res, deq_oqe(TCD_OBJ_DTQS, dtqid, &tskid, NULL), ERR_EXIT);
	if (tskid != TSK_NONE){
		res = sys_wup_tsk(tskid, TFN_TRCV_DTQ, 0);
	}
RETRY:
	if (gsDtqman[dtqid].dtqcnt == 0){
		EREXIT(res, wai_oqe(TCD_OBJ_DTQR, dtqid, TSK_SELF, NULL, tmout, TFN_TRCV_DTQ, &gsDtqman[dtqid].mutex), ERR_EXIT);
		*((UINT *)data) = dtq[(gsDtqman[dtqid].enq + gsDtqman[dtqid].ent) % gsDtqman[dtqid].dtqcnt];
	}else if (gsDtqman[dtqid].ent > gsDtqman[dtqid].dtqcnt){
		EREXIT(res, wai_oqe(TCD_OBJ_DTQR, dtqid, TSK_SELF, NULL, tmout, TFN_TRCV_DTQ, &gsDtqman[dtqid].mutex), ERR_EXIT);
		tmout = 0;	// if timed out event then return E_TMOUT
		goto RETRY;
	}else{
		*((UINT *)data) = dtq[(gsDtqman[dtqid].enq + gsDtqman[dtqid].ent) % gsDtqman[dtqid].dtqcnt];
		gsDtqman[dtqid].ent--;
		EREXIT(err, deq_oqe(TCD_OBJ_DTQS, dtqid, &tskid, NULL), ERR_EXIT);
		if (tskid != TSK_NONE){
			res = sys_wup_tsk(tskid, TFN_TRCV_DTQ, 0);
		}
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsDtqman[dtqid].mutex) == 0, E_SYS);
	return res;
}


ER ref_dtq(
	ID dtqid,
	T_RDTQ *pk_rdtq
	)
{
	ER res, err;
	
	PCHK(1 <= dtqid && dtqid <= TMAX_DTQ_ID, E_ID);
	PCHK(pk_rdtq != NULL, E_PAR);
	CHK_EXIST(res, err, gsDtqmtx, gsDtqman[dtqid]);
	EREXIT(res, ref_oqe(TCD_OBJ_DTQS, dtqid, 0, &pk_rdtq->stskid, NULL), ERR_EXIT);
	EREXIT(res, ref_oqe(TCD_OBJ_DTQR, dtqid, 0, &pk_rdtq->rtskid, NULL), ERR_EXIT);
	pk_rdtq->sdtqcnt = gsDtqman[dtqid].ent;
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsDtqman[dtqid].mutex) == 0, E_SYS);
	return res;
}


/***** mail box *****/
ER inz_mbx(void)
{
	memset(&gsMbxman[0], 0, sizeof(gsMbxman));
	PCHK(pthread_mutex_init(&gsMbxmtx, NULL) == 0, E_SYS);
	return E_OK;
}

ER cre_mbx(
	ID mbxid,
	T_CMBX *pk_cmbx
	)
{
	ER res, err;
	
	PCHK(1 <= mbxid && mbxid <= TMAX_MBX_ID, E_ID);
	PCHK(pk_cmbx != NULL && 0 <= pk_cmbx->maxmpri && pk_cmbx->maxmpri <= TMAX_MPRI, E_PAR);
	PCHK((pk_cmbx->mbxatr & ~(TA_TFIFO | TA_TPRI | TA_MFIFO | TA_MPRI)) == 0, E_RSATR);
	PCHK(pthread_mutex_lock(&gsMbxmtx) == 0, E_SYS);
	EREXIT(res, (gsMbxman[mbxid].created == FALSE)? E_OK : E_OBJ, ERR_EXIT);
    EREXIT(res, (pthread_mutex_init(&gsMbxman[mbxid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
	EREXIT(res, (cre_oqe(TCD_OBJ_MBX, mbxid, pk_cmbx->mbxatr) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
	gsMbxman[mbxid].created = TRUE;
	gsMbxman[mbxid].mbxatr = pk_cmbx->mbxatr;
	gsMbxman[mbxid].maxmpri = pk_cmbx->maxmpri;
	gsMbxman[mbxid].mprihd = (pk_cmbx->mprihd != NULL)? 
		pk_cmbx->mprihd : malloc(TSZ_MPRIHD(pk_cmbx->maxmpri));
	if (gsMbxman[mbxid].mprihd != NULL){
		memset(gsMbxman[mbxid].mprihd, 0, TSZ_MPRIHD(pk_cmbx->maxmpri));
		res = E_OK;
	}else{
		res = E_NOMEM;
	}
	goto ERR_EXIT;
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsMbxman[mbxid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMbxmtx) == 0, E_SYS);
	return res;
}


ER_ID acre_mbx(
	T_CMBX *pk_cmbx
	)
{
	ER_ID res, err;
	ID mbxid;
	
	PCHK(pk_cmbx != NULL && 0 <= pk_cmbx->maxmpri && pk_cmbx->maxmpri <= TMAX_MPRI, E_PAR);
	PCHK((pk_cmbx->mbxatr & ~(TA_TFIFO | TA_TPRI | TA_MFIFO | TA_MPRI)) == 0, E_RSATR);
	PCHK(pthread_mutex_lock(&gsMbxmtx) == 0, E_SYS);
	for (mbxid=1; mbxid<=TMAX_DTQ_ID; mbxid++){
		if (gsMbxman[mbxid].created == TRUE){
			continue;
		}
		EREXIT(res, (pthread_mutex_init(&gsMbxman[mbxid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
		EREXIT(res, (cre_oqe(TCD_OBJ_MBX, mbxid, pk_cmbx->mbxatr) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
		gsMbxman[mbxid].created = TRUE;
		gsMbxman[mbxid].mbxatr = pk_cmbx->mbxatr;
		gsMbxman[mbxid].maxmpri = pk_cmbx->maxmpri;
		gsMbxman[mbxid].mprihd = (pk_cmbx->mprihd != NULL)? 
			pk_cmbx->mprihd : malloc(TSZ_MPRIHD(pk_cmbx->maxmpri));
		if (gsMbxman[mbxid].mprihd != NULL){
			memset(gsMbxman[mbxid].mprihd, 0, TSZ_MPRIHD(pk_cmbx->maxmpri));
			res = E_OK;
		}else{
			res = E_NOMEM;
		}
		break;
	}
	res = (mbxid > TMAX_MBX_ID)? E_NOID : (ER_ID)mbxid;
	goto ERR_EXIT;
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsMbxman[mbxid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMbxmtx) == 0, E_SYS);
	return res;
}


ER del_mbx(
	ID mbxid
	)
{
	ER res;
	INT err;
	ID tskid;
	
	PCHK(1 <= mbxid && mbxid <= TMAX_SEM_ID, E_ID);
	PCHK(pthread_mutex_lock(&gsMbxmtx) == 0, E_SYS);
	EREXIT(res, (pthread_mutex_lock(&gsMbxman[mbxid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	EREXIT(res, (gsMbxman[mbxid].created == TRUE)? E_OK : E_NOEXS, ERR_EXIT);
	gsMbxman[mbxid].created = FALSE;
	EREXIT(res, (pthread_mutex_unlock(&gsMbxman[mbxid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	while (deq_oqe(TCD_OBJ_MBX, mbxid, &tskid, NULL) == E_OK && tskid != TSK_NONE){
		res |= sys_wup_tsk(tskid, TFN_DEL_MBX, TTS_DLT);
	}
	res = chg_pri(TSK_SELF, TMAX_TSK_PRI);	/* for waiting low priority task wake up */
	res = del_oqe(TCD_OBJ_MBX, mbxid);
	EREXIT(res, (pthread_mutex_destroy(&gsMbxman[mbxid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	memset(&gsMbxman[mbxid], 0, sizeof(T_DTQ_MAN));
	goto ERR_EXIT2;
ERR_EXIT:
	err = pthread_mutex_unlock(&gsMbxman[mbxid].mutex);
ERR_EXIT2:
	PCHK(pthread_mutex_unlock(&gsMbxmtx) == 0, E_SYS);
	return res;
}


ER snd_mbx(
	ID mbxid,
	T_MSG *pk_msg
	)
{
	ER res, err;
	T_MSG_PRIHD *mque;
	ID tskid;
	
	PCHK(1 <= mbxid && mbxid <= TMAX_MBX_ID, E_ID);
	PCHK(pk_msg != NULL, E_PAR);
	CHK_EXIST(res, err, gsMbxmtx, gsMbxman[mbxid]);
	EREXIT(res, ((gsMbxman[mbxid].mbxatr & TA_MPRI) == 0 ||
		((gsMbxman[mbxid].mbxatr & TA_MPRI) == 1 && 
			0 <= ((T_MSG_PRI *)pk_msg)->msgpri && ((T_MSG_PRI *)pk_msg)->msgpri <= TMAX_MPRI))? E_OK : E_PAR, 
		ERR_EXIT);
	mque = (gsMbxman[mbxid].mbxatr & TA_MPRI)? 
		&gsMbxman[mbxid].mprihd[((T_MSG_PRI *)pk_msg)->msgpri] : gsMbxman[mbxid].mprihd;
	pk_msg->nextmsg = NULL;
	if (mque->top == NULL){				/* first msg */
		mque->top = pk_msg;
		mque->end = pk_msg;
	}else{
		mque->end->nextmsg = pk_msg;
		mque->end = pk_msg;
	}
	EREXIT(res, deq_oqe(TCD_OBJ_MBX, mbxid, &tskid, NULL), ERR_EXIT);
	if (tskid != TSK_NONE){
		res = sys_wup_tsk(tskid, TFN_SND_MBX, 0);
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMbxman[mbxid].mutex) == 0, E_SYS);
	return res;
}


ER trcv_mbx(
	ID mbxid,
	T_MSG **ppk_msg,
	TMO tmout
	)
{
	ER res, err;
	BOOL waited;
	INT i;
	
	PCHK(1 <= mbxid && mbxid <= TMAX_MBX_ID, E_ID);
	PCHK(ppk_msg != NULL, E_PAR);
	CHK_EXIST(res, err, gsMbxmtx, gsMbxman[mbxid]);
	res = E_SYS;
	waited = FALSE;
	if (gsMbxman[mbxid].mbxatr & TA_MPRI){
		while(res == E_SYS){
			for (i=0; i<gsMbxman[mbxid].maxmpri; i++){
				if (gsMbxman[mbxid].mprihd[i].top != NULL){
					*ppk_msg = gsMbxman[mbxid].mprihd[i].top;
					gsMbxman[mbxid].mprihd[i].top = (*ppk_msg)->nextmsg;
					if (gsMbxman[mbxid].mprihd[i].top == NULL){	/* no msg */
						gsMbxman[mbxid].mprihd[i].end = NULL;
					}
					res = E_OK;
					break;
				}
			}
			if (res == E_SYS && !waited){
				EREXIT(res, wai_oqe(TCD_OBJ_MBX, mbxid, TSK_SELF, NULL, tmout, TFN_TRCV_MBX, &gsMbxman[mbxid].mutex), ERR_EXIT);
				waited = TRUE;
				res = E_SYS;
			}else if (waited){
				goto ERR_EXIT;
			}
		}
	}else{
		while (res == E_SYS){
			if (gsMbxman[mbxid].mprihd->top != NULL){
				*ppk_msg = gsMbxman[mbxid].mprihd->top;
				gsMbxman[mbxid].mprihd->top = (*ppk_msg)->nextmsg;
				res = E_OK;
			}else if (res == E_SYS && !waited){
				EREXIT(res, wai_oqe(TCD_OBJ_MBX, mbxid, TSK_SELF, NULL, tmout, TFN_TRCV_MBX, &gsMbxman[mbxid].mutex), ERR_EXIT);
				waited = TRUE;
				res = E_SYS;
			}else if (waited){
				goto ERR_EXIT;
			}
		}
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMbxman[mbxid].mutex) == 0, E_SYS);
	return res;
}


ER ref_mbx(
	ID mbxid,
	T_RMBX *pk_rmbx
	)
{
	ER res, err;
	INT i;
	
	PCHK(1 <= mbxid && mbxid <= TMAX_MBX_ID, E_ID);
	PCHK(pk_rmbx != NULL, E_PAR);
	CHK_EXIST(res, err, gsMbxmtx, gsMbxman[mbxid]);
	EREXIT(res, ref_oqe(TCD_OBJ_MBX, mbxid, 0, &pk_rmbx->wtskid, NULL), ERR_EXIT);
	if (gsMbxman[mbxid].mbxatr & TA_MPRI){
		for (i=0; i<gsMbxman[mbxid].maxmpri; i++){
			if (gsMbxman[mbxid].mprihd[i].top != NULL){
				pk_rmbx->pk_msg = gsMbxman[mbxid].mprihd[i].top;
			}
		}
	}else{
		pk_rmbx->pk_msg = gsMbxman[mbxid].mprihd->top;
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMbxman[mbxid].mutex) == 0, E_SYS);
	return res;
}


