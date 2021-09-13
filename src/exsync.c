#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "itron.h"
#include "depend.h"
#include "common.h"

/***** structure definision *****/
typedef struct {						/* mutex management structure	*/
    pthread_mutex_t mutex;				/* management info access mutex	*/
	BOOL created;						/* mutex object created			*/
	ATR  mtxatr;						/* attribute					*/
	PRI  mtx_max;						/* reservable task's max pri	*/
	PRI  res_max;						/* reserved task's pri max		*/
	ID   lockedtsk;						/* locking task's ID			*/
} T_MTX_MAN;

typedef struct {						/* message entry					*/
	SIZE msgsz;							/* message size						*/
	UB msg[1];							/* message							*/
} T_MBF_MSG;

typedef struct {						/* msgbuf management structure		*/
	BOOL created;						/* msgbuf object created			*/
	ATR mbfatr;							/* msgbuf attribute					*/
	UINT maxmsz;						/* max message size					*/
	VP mbf;								/* msgbuf area						*/
	SIZE mbfsz;							/* msgbuf size						*/
	T_MBF_MSG *enq;						/* enqueue pointer					*/
	T_MBF_MSG *deq;						/* dequeue pointer					*/
    pthread_mutex_t mutex;				/* management info access mutex		*/
} T_MBF_MAN;

typedef struct {						/* rendezvous port management structure	*/
	BOOL created;						/* port object created				*/
	UINT maxcmsz;						/* max call message size			*/
	UINT maxrmsz;						/* max response message size		*/
    pthread_mutex_t mutex;				/* management info access mutex		*/
} T_POR_MAN;

typedef struct {						/* established randezvous info structure */
	BOOL use;							/* using flag 						*/
	RDVPTN rdvptn;						/* accept randezvous pattern 		*/
	VP msg;								/* message area 					*/
	UINT msgsz;							/* message size 					*/
	ID ctskid;							/* calling task id 					*/
} T_RDV_MAN;

/***** global valiable *****/
static T_MTX_MAN gsMtxman[TMAX_MTX_ID + 1];
static T_MBF_MAN gsMbfman[TMAX_MBF_ID + 1];
static T_POR_MAN gsPorman[TMAX_POR_ID + 1];
static T_RDV_MAN gsRdvman[TMAX_RDV_NO + 1];
static RDVNO gRdvnum;
static pthread_mutex_t gsMtxmtx, gsMbfmtx, gsRdvmtx;

/***** functions *****/
ER inz_mtx(void)
{
	memset(&gsMtxman[0], 0, sizeof(gsMtxman));
	PCHK(pthread_mutex_init(&gsMtxmtx, NULL) == 0, E_SYS);
	return E_OK;
}

ER cre_mtx(
	ID mtxid,
	T_CMTX *pk_cmtx
	)
{
	ER res, err;
	
	PCHK(1 <= mtxid && mtxid <= TMAX_MTX_ID, E_ID);
	PCHK(pk_cmtx != NULL, E_PAR);
	PCHK(TA_TFIFO <= pk_cmtx->mtxatr && pk_cmtx->mtxatr <= TA_CEILING, E_RSATR);
	PCHK(pk_cmtx->mtxatr == TA_CEILING && TMIN_MPRI <= pk_cmtx->ceilpri && pk_cmtx->ceilpri <= TMAX_MPRI, E_PAR);
	PCHK(pthread_mutex_lock(&gsMtxmtx) == 0, E_SYS);
	EREXIT(res, (gsMtxman[mtxid].created == FALSE)? E_OK : E_OBJ, ERR_EXIT);
    EREXIT(res, (pthread_mutex_init(&gsMtxman[mtxid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
	EREXIT(res, (cre_oqe(TCD_OBJ_MTX, mtxid, (pk_cmtx->mtxatr == TA_TFIFO)? TA_TFIFO : TA_TPRI) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
	gsMtxman[mtxid].created = TRUE;
	gsMtxman[mtxid].res_max = 0;
	gsMtxman[mtxid].mtx_max = pk_cmtx->ceilpri;
	gsMtxman[mtxid].mtxatr = pk_cmtx->mtxatr;
	goto ERR_EXIT;
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsMtxman[mtxid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMtxmtx) == 0, E_SYS);
	return res;
}


ER_ID acre_mtx(
	T_CMTX *pk_cmtx
	)
{
	ER_ID res, err;
	ID mtxid;
	
	PCHK(pk_cmtx != NULL, E_PAR);
	PCHK(TA_TFIFO <= pk_cmtx->mtxatr && pk_cmtx->mtxatr <= TA_CEILING, E_RSATR);
	PCHK(pk_cmtx->mtxatr == TA_CEILING && TMIN_MPRI <= pk_cmtx->ceilpri && pk_cmtx->ceilpri <= TMAX_MPRI, E_PAR);
	PCHK(pthread_mutex_lock(&gsMtxmtx) == 0, E_SYS);
	for (mtxid=1; mtxid<=TMAX_MTX_ID; mtxid++){
		if (gsMtxman[mtxid].created == TRUE){
			continue;
		}
		EREXIT(res, (pthread_mutex_init(&gsMtxman[mtxid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
		EREXIT(res, (cre_oqe(TCD_OBJ_MTX, mtxid, (pk_cmtx->mtxatr == TA_TFIFO)? TA_TFIFO : TA_TPRI) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
		gsMtxman[mtxid].created = TRUE;
		gsMtxman[mtxid].res_max = 0;
		gsMtxman[mtxid].mtx_max = pk_cmtx->ceilpri;
		gsMtxman[mtxid].mtxatr = pk_cmtx->mtxatr;
		break;
	}
	res = (mtxid > TMAX_MTX_ID)? E_NOID : (ER_ID)mtxid;
	goto ERR_EXIT;
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsMtxman[mtxid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMtxmtx) == 0, E_SYS);
	return res;
}


ER del_mtx(
	ID mtxid
	)
{
	ER res;
	INT err;
	ID tskid;
	
	PCHK(1 <= mtxid && mtxid <= TMAX_MTX_ID, E_ID);
	PCHK(pthread_mutex_lock(&gsMtxmtx) == 0, E_SYS);
	EREXIT(res, (pthread_mutex_lock(&gsMtxman[mtxid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	EREXIT(res, (gsMtxman[mtxid].created == TRUE)? E_OK : E_NOEXS, ERR_EXIT);
	gsMtxman[mtxid].created = FALSE;
	EREXIT(res, (pthread_mutex_unlock(&gsMtxman[mtxid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	if (gsMtxman[mtxid].lockedtsk != TSK_NONE){
		res |= tsk_unl_mtx(gsMtxman[mtxid].lockedtsk, mtxid);
		res |= sys_wup_tsk(gsMtxman[mtxid].lockedtsk, TFN_DEL_MTX, TTS_DLT);
	}
	while (deq_oqe(TCD_OBJ_MTX, mtxid, &tskid, NULL) == E_OK && tskid != TSK_NONE){
		res |= tsk_unl_mtx(tskid, mtxid);
		res |= sys_wup_tsk(tskid, TFN_DEL_MTX, TTS_DLT);
	}
	res = del_oqe(TCD_OBJ_MTX, mtxid);
	EREXIT(res, (pthread_mutex_destroy(&gsMtxman[mtxid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	memset(&gsMtxman[mtxid], 0, sizeof(T_MTX_MAN));
	goto ERR_EXIT2;
ERR_EXIT:
	err = pthread_mutex_unlock(&gsMtxman[mtxid].mutex);
ERR_EXIT2:
	PCHK(pthread_mutex_unlock(&gsMtxmtx) == 0, E_SYS);
	return res;
}


ER tloc_mtx(
	ID mtxid,
	TMO tmout
	)
{
	ID myid;
	T_RTSK rtsk;
	ER res, err;
	
	PCHK(1 <= mtxid && mtxid <= TMAX_MTX_ID, E_ID);
	PCHK(0 <= tmout || tmout == TMO_FEVR, E_PAR);
	PCHK(get_tid(&myid) == E_OK, E_SYS);
	PCHK(ref_tsk(myid, &rtsk) == E_OK, E_SYS);
	CHK_EXIST(res, err, gsMtxmtx, gsMtxman[mtxid]);
	EREXIT(res, (gsMtxman[mtxid].mtxatr == TA_CEILING && rtsk.tskbpri >= gsMtxman[mtxid].mtx_max)? E_OK : E_ILUSE, ERR_EXIT);
	EREXIT(res, tsk_loc_mtx(myid, mtxid, gsMtxman[mtxid].mtx_max), ERR_EXIT);
	if (gsMtxman[mtxid].lockedtsk != TSK_NONE){
		if (gsMtxman[mtxid].mtxatr == TA_INHERIT && gsMtxman[mtxid].res_max > rtsk.tskpri){
			gsMtxman[mtxid].res_max = rtsk.tskpri;
			EREXIT(res, chg_nowpri(gsMtxman[mtxid].lockedtsk, rtsk.tskpri), ERR_EXIT);
		}
		EREXIT(res, wai_oqe(TCD_OBJ_MTX, mtxid, TSK_SELF, NULL, tmout, TFN_TLOC_MTX, &gsMtxman[mtxid].mutex), ERR_EXIT);
	}else{
		gsMtxman[mtxid].res_max = rtsk.tskpri;
	}
	if (gsMtxman[mtxid].mtxatr == TA_CEILING && gsMtxman[mtxid].mtx_max < rtsk.tskpri){
			EREXIT(res, chg_nowpri(myid, gsMtxman[mtxid].mtx_max), ERR_EXIT);
	}
	gsMtxman[mtxid].lockedtsk = myid;
	res = err;
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMtxman[mtxid].mutex) == 0, E_SYS);
	return res;
}


ER unl_mtx(
	ID mtxid
	)
{
	ER res, err;
	ID tskid;
	
	PCHK(1 <= mtxid && mtxid <= TMAX_MTX_ID, E_ID);
	PCHK(get_tid(&tskid) == E_OK, E_SYS);
	CHK_EXIST(res, err, gsMtxmtx, gsMtxman[mtxid]);
	EREXIT(res, (gsMtxman[mtxid].lockedtsk == tskid)? E_OK : E_ILUSE, ERR_EXIT);
	EREXIT(res, tsk_unl_mtx(tskid, mtxid), ERR_EXIT);
	EREXIT(res, deq_oqe(TCD_OBJ_MTX, mtxid, &tskid, NULL), ERR_EXIT);
	if (tskid != TSK_NONE){
		res = sys_wup_tsk(tskid, TFN_UNL_MTX, 0);
	}else{
		gsMtxman[mtxid].lockedtsk = TSK_NONE;
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMtxman[mtxid].mutex) == 0, E_SYS);
	return res;
}


ER ref_mtx(
	ID mtxid,
	T_RMTX *pk_rmtx
	)
{
	ER res, err;
	
	PCHK(1 <= mtxid && mtxid <= TMAX_MTX_ID, E_ID);
	PCHK(pk_rmtx != NULL, E_PAR);
	CHK_EXIST(res, err, gsMtxmtx, gsMtxman[mtxid]);
	res = ref_oqe(TCD_OBJ_MTX, mtxid, 0, &pk_rmtx->wtskid, NULL);
	pk_rmtx->htskid = gsMtxman[mtxid].lockedtsk;
	res = (pk_rmtx->wtskid != TSK_NONE && pk_rmtx->htskid == TSK_NONE)? E_SYS : E_OK;
	PCHK(pthread_mutex_unlock(&gsMtxman[mtxid].mutex) == 0, E_SYS);
	return res;
}


/***** message buffer *****/
ER inz_mbf(void)
{
	memset(&gsMbfman[0], 0, sizeof(gsMbfman));
	PCHK(pthread_mutex_init(&gsMbfmtx, NULL) == 0, E_SYS);
	return E_OK;
}

ER cre_mbf(
	ID mbfid,
	T_CMBF *pk_cmbf
	)
{
	ER res, err;
	
	PCHK(1 <= mbfid && mbfid <= TMAX_MBF_ID, E_ID);
	PCHK(pk_cmbf != NULL && 0 < pk_cmbf->maxmsz && (pk_cmbf->maxmsz <= pk_cmbf->mbfsz || pk_cmbf->mbfsz  == 0), E_PAR);
	PCHK(pk_cmbf->maxmsz % sizeof(UW) == 0 && pk_cmbf->mbfsz % sizeof(UW) == 0, E_PAR);
	PCHK((pk_cmbf->mbfatr & ~(TA_TFIFO | TA_TPRI)) == 0, E_RSATR);
	PCHK(pthread_mutex_lock(&gsMbfmtx) == 0, E_SYS);
	EREXIT(res, (gsMbfman[mbfid].created == FALSE)? E_OK : E_OBJ, ERR_EXIT);
	EREXIT(res, (gsMbfman[mbfid].mbf == NULL)? E_OK : E_SYS, ERR_EXIT);
    EREXIT(res, (pthread_mutex_init(&gsMbfman[mbfid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
	EREXIT(res, (cre_oqe(TCD_OBJ_MBFS, mbfid, pk_cmbf->mbfatr) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
	EREXIT(res, (cre_oqe(TCD_OBJ_MBFR, mbfid, pk_cmbf->mbfatr) == E_OK)? E_OK : E_SYS, ERR_EXIT3);
	gsMbfman[mbfid].created = TRUE;
	gsMbfman[mbfid].mbfatr = pk_cmbf->mbfatr;
	gsMbfman[mbfid].maxmsz = pk_cmbf->maxmsz;
	gsMbfman[mbfid].mbfsz = pk_cmbf->mbfsz;
	gsMbfman[mbfid].mbf = malloc((pk_cmbf->mbfsz)? pk_cmbf->mbfsz : pk_cmbf->maxmsz);
	gsMbfman[mbfid].enq = gsMbfman[mbfid].mbf;
	gsMbfman[mbfid].enq->msgsz = 0;
	gsMbfman[mbfid].deq = gsMbfman[mbfid].mbf;
	gsMbfman[mbfid].deq->msgsz = 0;
	if (gsMbfman[mbfid].mbf != NULL) {
		res = E_OK;
		goto ERR_EXIT;
	} else {
		res = E_NOMEM;
		gsMbfman[mbfid].created = FALSE;
	}
	err = del_oqe(TCD_OBJ_MBFR, mbfid);
ERR_EXIT3:
	err = del_oqe(TCD_OBJ_MBFS, mbfid);
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsMbfman[mbfid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMbfmtx) == 0, E_SYS);
	return res;
}


ER_ID acre_mbf(
	T_CMBF *pk_cmbf
	)
{
	ER_ID res, err;
	ID mbfid;
	
	PCHK(pk_cmbf != NULL && pk_cmbf->mbfsz > 0, E_PAR);
    PCHK(pk_cmbf->maxmsz % sizeof(UW) == 0 && pk_cmbf->mbfsz % sizeof(UW) == 0, E_PAR);
	PCHK((pk_cmbf->mbfatr & ~(TA_TFIFO | TA_TPRI)) == 0, E_RSATR);
	PCHK(pthread_mutex_lock(&gsMbfmtx) == 0, E_SYS);
	for (mbfid=1; mbfid<=TMAX_MBF_ID; mbfid++){
		if (gsMbfman[mbfid].created == TRUE){
			continue;
		}
		EREXIT(res, (pthread_mutex_init(&gsMbfman[mbfid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
		EREXIT(res, (cre_oqe(TCD_OBJ_MBFS, mbfid, pk_cmbf->mbfatr) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
		EREXIT(res, (cre_oqe(TCD_OBJ_MBFR, mbfid, pk_cmbf->mbfatr) == E_OK)? E_OK : E_SYS, ERR_EXIT3);
		gsMbfman[mbfid].created = TRUE;
		gsMbfman[mbfid].mbfatr = pk_cmbf->mbfatr;
		gsMbfman[mbfid].maxmsz = pk_cmbf->maxmsz;
		gsMbfman[mbfid].mbfsz = pk_cmbf->mbfsz;
		if (pk_cmbf->mbf == NULL){
			gsMbfman[mbfid].mbf = malloc((pk_cmbf->mbfsz)? pk_cmbf->mbfsz : pk_cmbf->maxmsz);
		}else{
			gsMbfman[mbfid].mbf = pk_cmbf->mbf;
		}
		gsMbfman[mbfid].enq = gsMbfman[mbfid].mbf;
		gsMbfman[mbfid].enq->msgsz = 0;
		gsMbfman[mbfid].deq = gsMbfman[mbfid].mbf;
		gsMbfman[mbfid].deq->msgsz = 0;
		if (gsMbfman[mbfid].mbf != NULL) {
			res = (ER_ID)mbfid;
			goto ERR_EXIT;
		} else {
			res = E_NOMEM;
			gsMbfman[mbfid].created = FALSE;
			goto ERR_EXIT4;
		}
		break;
	}
	res = E_NOID;
	goto ERR_EXIT;
ERR_EXIT4:
	err = del_oqe(TCD_OBJ_MBFR, mbfid);
ERR_EXIT3:
	err = del_oqe(TCD_OBJ_MBFS, mbfid);
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsMbfman[mbfid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMbfmtx) == 0, E_SYS);
	return res;
}


ER del_mbf(
	ID mbfid
	)
{
	ER res;
	INT err;
	ID tskid;
	
	PCHK(1 <= mbfid && mbfid <= TMAX_SEM_ID, E_ID);
	PCHK(pthread_mutex_lock(&gsMbfmtx) == 0, E_SYS);
	EREXIT(res, (pthread_mutex_lock(&gsMbfman[mbfid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	EREXIT(res, (gsMbfman[mbfid].created == TRUE)? E_OK : E_NOEXS, ERR_EXIT);
	gsMbfman[mbfid].created = FALSE;
	EREXIT(res, (pthread_mutex_unlock(&gsMbfman[mbfid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	while (deq_oqe(TCD_OBJ_MBFS, mbfid, &tskid, NULL) == E_OK && tskid != TSK_NONE){
		res |= sys_wup_tsk(tskid, TFN_DEL_MBF, TTS_DLT);
	}
	while (deq_oqe(TCD_OBJ_MBFR, mbfid, &tskid, NULL) == E_OK && tskid != TSK_NONE){
		res |= sys_wup_tsk(tskid, TFN_DEL_MBF, TTS_DLT);
	}
	res = del_oqe(TCD_OBJ_MBFS, mbfid);
	res = del_oqe(TCD_OBJ_MBFR, mbfid);
	free(gsMbfman[mbfid].mbf);
	EREXIT(res, (pthread_mutex_destroy(&gsMbfman[mbfid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	memset(&gsMbfman[mbfid], 0, sizeof(T_MBF_MAN));
	goto ERR_EXIT2;
ERR_EXIT:
	err = pthread_mutex_unlock(&gsMbfman[mbfid].mutex);
ERR_EXIT2:
	PCHK(pthread_mutex_unlock(&gsMbfmtx) == 0, E_SYS);
	return res;
}


ER tsnd_mbf(
	ID mbfid,
	VP msg,
	UINT msgsz,
	TMO tmout
	)
{
	ER res, err;
	UB *mbf;
	UW enqof, deqof, free_all, free_split;
	ID tskid;
	PRI tskpri, wtskpri;
	
	PCHK(1 <= mbfid && mbfid <= TMAX_MBF_ID, E_ID);
	PCHK(msg != NULL && 0 < msgsz && (0 <= tmout || tmout == TMO_FEVR), E_PAR);
	CHK_EXIST(res, err, gsMbfmtx, gsMbfman[mbfid]);
	msgsz = ((msgsz + 3) / 4) * 4;
	EREXIT(res, (msgsz <= gsMbfman[mbfid].maxmsz)? E_OK : E_PAR, ERR_EXIT);
	mbf = (UB *)gsMbfman[mbfid].mbf;
RETRY:
	if (gsMbfman[mbfid].mbfsz == 0){
		EREXIT(res, deq_oqe(TCD_OBJ_MBFR, mbfid, &tskid, NULL), ERR_EXIT);
		if (tskid != TSK_NONE){
			memcpy(gsMbfman[mbfid].enq->msg, msg, msgsz);
			gsMbfman[mbfid].enq->msgsz = msgsz;
			res = sys_wup_tsk(tskid, TFN_TSND_MBF, 0);
		}else if (tmout == TMO_POL){
			res = E_TMOUT;
		}else{
			EREXIT(res, wai_oqe(TCD_OBJ_MBFS, mbfid, TSK_SELF, (VP)msgsz, tmout, TFN_TSND_MBF, &gsMbfman[mbfid].mutex), ERR_EXIT);
			tmout = 0;	// if timed out event then return E_TMOUT
			goto RETRY;
		}
	}else{
		enqof = (UW)((UB *)gsMbfman[mbfid].enq - mbf);
		deqof = (UW)((UB *)gsMbfman[mbfid].deq - mbf);
		if (enqof >= deqof){
			free_split = gsMbfman[mbfid].mbfsz - enqof;
			free_all = deqof + free_split;
		}else{
			free_split = deqof - enqof;
			free_all = free_split;
		}
		tskid = TSK_NONE;
		err = ref_oqe(TCD_OBJ_MBFS, mbfid, 0, &tskid, NULL);
		if (err == E_OK && tskid != TSK_NONE){
			EREXIT(res, get_pri(tskid, &tskpri), ERR_EXIT);
		}
		EREXIT(res, get_pri(TSK_SELF, &wtskpri), ERR_EXIT);
		if (free_all >= msgsz + sizeof(SIZE)
			&& (err == E_NOEXS || (gsMbfman[mbfid].mbfatr == TA_TPRI && err == E_OK && tskid != TSK_NONE && tskpri > wtskpri))){
			if (free_split < msgsz + sizeof(SIZE)){
				if (sizeof(SIZE) < free_split){
					memcpy(gsMbfman[mbfid].enq->msg, msg, free_split - sizeof(SIZE));
					memcpy(mbf, &(((UB *)msg)[free_split - sizeof(SIZE)]), msgsz - free_split + sizeof(SIZE));
				}else{
                    memcpy(mbf, msg, msgsz);
				}
			}else{
				memcpy(gsMbfman[mbfid].enq->msg, msg, msgsz);
			}
			gsMbfman[mbfid].enq->msgsz = msgsz;
			gsMbfman[mbfid].enq = (T_MBF_MSG *)((UB *)gsMbfman[mbfid].mbf + (enqof + msgsz + sizeof(SIZE)) % gsMbfman[mbfid].mbfsz);
			gsMbfman[mbfid].enq->msgsz = 0;
			EREXIT(res, deq_oqe(TCD_OBJ_MBFR, mbfid, &tskid, NULL), ERR_EXIT);
			if (tskid != TSK_NONE){
				res = sys_wup_tsk(tskid, TFN_TSND_MBF, 0);
			}
		}else if (tmout == TMO_POL){
			res = E_TMOUT;
		}else{
			EREXIT(res, wai_oqe(TCD_OBJ_MBFS, mbfid, TSK_SELF, (VP)msgsz, tmout, TFN_TSND_MBF, &gsMbfman[mbfid].mutex), ERR_EXIT);
			tmout = 0;	// if timed out event then return E_TMOUT
			goto RETRY;
		}
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMbfman[mbfid].mutex) == 0, E_SYS);
	return res;
}


ER_UINT trcv_mbf(
	ID mbfid,
	VP msg,
	TMO tmout
	)
{
	ER res, err;
	ID tskid;
    UW enqof, deqof, msg_all, msg_split;
	SIZE msgsz;
	
	PCHK(1 <= mbfid && mbfid <= TMAX_MBF_ID, E_ID);
	PCHK((0 <= tmout || tmout == TMO_FEVR) && msg != NULL, E_PAR);
	CHK_EXIST(res, err, gsMbfmtx, gsMbfman[mbfid]);
RETRY:
	if (gsMbfman[mbfid].mbfsz == 0){
		EREXIT(res, deq_oqe(TCD_OBJ_MBFS, mbfid, &tskid, NULL), ERR_EXIT);
		if (tskid != TSK_NONE){
			res = sys_wup_tsk(tskid, TFN_TRCV_MBF, 0);
		}
		EREXIT(res, wai_oqe(TCD_OBJ_MBFR, mbfid, TSK_SELF, NULL, tmout, TFN_TRCV_MBF, &gsMbfman[mbfid].mutex), ERR_EXIT);
		memcpy(msg, gsMbfman[mbfid].mbf, gsMbfman[mbfid].maxmsz);
	}else if (gsMbfman[mbfid].deq == gsMbfman[mbfid].enq){
		EREXIT(res, wai_oqe(TCD_OBJ_MBFR, mbfid, TSK_SELF, NULL, tmout, TFN_TRCV_MBF, &gsMbfman[mbfid].mutex), ERR_EXIT);
		tmout = 0;	// if timed out event then return E_TMOUT because wai_oqe reject tmout=0 request
		goto RETRY;
	}else{
		res = gsMbfman[mbfid].deq->msgsz;
		enqof = (UW)((UB *)gsMbfman[mbfid].enq - (UB *)gsMbfman[mbfid].mbf);
		deqof = (UW)((UB *)gsMbfman[mbfid].deq - (UB *)gsMbfman[mbfid].mbf);
        if (enqof >= deqof){
            msg_split = enqof - deqof;
        }else{
            msg_split = gsMbfman[mbfid].mbfsz - deqof;
        }
        if (msg_split < gsMbfman[mbfid].deq->msgsz + sizeof(SIZE)){
        	if (msg_split > sizeof(SIZE)){
                memcpy(msg, gsMbfman[mbfid].deq->msg, msg_split - sizeof(SIZE));
                memcpy(&(((UB *)msg)[msg_split - sizeof(SIZE)]), gsMbfman[mbfid].mbf, gsMbfman[mbfid].deq->msgsz - msg_split + sizeof(SIZE));
        	}else{
                memcpy(msg, gsMbfman[mbfid].mbf, gsMbfman[mbfid].deq->msgsz);
        	}
        }else{
        	memcpy(msg, gsMbfman[mbfid].deq->msg, gsMbfman[mbfid].deq->msgsz);
        }
		gsMbfman[mbfid].deq = (T_MBF_MSG *)((UB *)gsMbfman[mbfid].mbf + (deqof + gsMbfman[mbfid].deq->msgsz + sizeof(SIZE)) % gsMbfman[mbfid].mbfsz);
		deqof = (UW)((UB *)gsMbfman[mbfid].deq - (UB *)gsMbfman[mbfid].mbf);
        if (enqof >= deqof){
            msg_split = enqof - deqof;
            msg_all = msg_split;
        }else{
            msg_split = gsMbfman[mbfid].mbfsz - deqof;
            msg_all = deqof + msg_split;
        }
		EREXIT(err, ref_oqe(TCD_OBJ_MBFS, mbfid, 0, &tskid, (VP *)&msgsz), ERR_EXIT);
		if (tskid != TSK_NONE && msgsz <= msg_all - sizeof(SIZE)){
			err = sys_wup_tsk(tskid, TFN_TRCV_MBF, 0);
		}
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMbfman[mbfid].mutex) == 0, E_SYS);
	return res;
}


ER ref_mbf(
	ID mbfid,
	T_RMBF *pk_rmbf
	)
{
	ER res, err;
	
	PCHK(1 <= mbfid && mbfid <= TMAX_MBF_ID, E_ID);
	PCHK(pk_rmbf != NULL, E_PAR);
	CHK_EXIST(res, err, gsMbfmtx, gsMbfman[mbfid]);
	EREXIT(res, ref_oqe(TCD_OBJ_MBFS, mbfid, 0, &pk_rmbf->stskid, NULL), ERR_EXIT);
	EREXIT(res, ref_oqe(TCD_OBJ_MBFR, mbfid, 0, &pk_rmbf->rtskid, NULL), ERR_EXIT);
	pk_rmbf->smsgcnt = (gsMbfman[mbfid].enq >  gsMbfman[mbfid].deq)? 
		gsMbfman[mbfid].enq - gsMbfman[mbfid].deq : 
		gsMbfman[mbfid].enq + gsMbfman[mbfid].mbfsz - gsMbfman[mbfid].deq;
	pk_rmbf->fmbfsz = pk_rmbf->smsgcnt * gsMbfman[mbfid].maxmsz;
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMbfman[mbfid].mutex) == 0, E_SYS);
	return res;
}


/***** rendezvous *****/
ER inz_rdv(void)
{
	memset(&gsPorman[0], 0, sizeof(gsPorman));
	PCHK(pthread_mutex_init(&gsRdvmtx, NULL) == 0, E_SYS);
	gRdvnum = 0;
	return E_OK;
}

ER cre_por(
	ID porid,
	T_CPOR *pk_cpor
	)
{
	ER res, err;
	
	PCHK(1 <= porid && porid <= TMAX_MBF_ID, E_ID);
	PCHK(pk_cpor != NULL && 0 <= pk_cpor->maxcmsz && pk_cpor->maxcmsz <= TMAX_POR_CMSZ
		&& 0 <= pk_cpor->maxrmsz && pk_cpor->maxrmsz <= TMAX_POR_RMSZ, E_PAR);
	PCHK(pk_cpor->poratr == TA_TFIFO || pk_cpor->poratr == TA_TPRI, E_RSATR);
	PCHK(pthread_mutex_lock(&gsRdvmtx) == 0, E_SYS);
	EREXIT(res, (gsPorman[porid].created == FALSE)? E_OK : E_OBJ, ERR_EXIT);
    EREXIT(res, (pthread_mutex_init(&gsPorman[porid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
	EREXIT(res, (cre_oqe(TCD_OBJ_PORC, porid, pk_cpor->poratr) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
	EREXIT(res, (cre_oqe(TCD_OBJ_PORA, porid, pk_cpor->poratr) == E_OK)? E_OK : E_SYS, ERR_EXIT3);
	gsPorman[porid].created = TRUE;
	gsPorman[porid].maxcmsz = pk_cpor->maxcmsz;
	gsPorman[porid].maxrmsz = pk_cpor->maxrmsz;
	res = E_OK;
	goto ERR_EXIT;
ERR_EXIT3:
	err = del_oqe(TCD_OBJ_PORC, porid);
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsPorman[porid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsRdvmtx) == 0, E_SYS);
	return res;
}


ER_ID acre_por(
	T_CPOR *pk_cpor
	)
{
	ER_ID res, err;
	ID porid;
	
	PCHK(pk_cpor != NULL && 0 <= pk_cpor->maxcmsz && pk_cpor->maxcmsz <= TMAX_POR_CMSZ
		&& 0 <= pk_cpor->maxrmsz && pk_cpor->maxrmsz <= TMAX_POR_RMSZ, E_PAR);
	PCHK(pk_cpor->poratr == TA_TFIFO || pk_cpor->poratr == TA_TPRI, E_RSATR);
	PCHK(pthread_mutex_lock(&gsRdvmtx) == 0, E_SYS);
	for (porid=1; porid<=TMAX_POR_ID; porid++){
		if (gsPorman[porid].created == TRUE){
			continue;
		}
		EREXIT(res, (gsPorman[porid].created == FALSE)? E_OK : E_OBJ, ERR_EXIT);
		EREXIT(res, (pthread_mutex_init(&gsPorman[porid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
		EREXIT(res, (cre_oqe(TCD_OBJ_PORC, porid, pk_cpor->poratr) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
		EREXIT(res, (cre_oqe(TCD_OBJ_PORA, porid, pk_cpor->poratr) == E_OK)? E_OK : E_SYS, ERR_EXIT3);
		gsPorman[porid].created = TRUE;
		gsPorman[porid].maxcmsz = pk_cpor->maxcmsz;
		gsPorman[porid].maxrmsz = pk_cpor->maxrmsz;
		res = E_OK;
		goto ERR_EXIT;
	}
	res = E_NOID;
	goto ERR_EXIT;
ERR_EXIT3:
	err = del_oqe(TCD_OBJ_PORC, porid);
ERR_EXIT2:
	err = pthread_mutex_destroy(&gsPorman[porid].mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsRdvmtx) == 0, E_SYS);
	return res;
}


ER del_por(
	ID porid
	)
{
	ER res;
	INT err;
	ID tskid;
	
	PCHK(1 <= porid && porid <= TMAX_POR_ID, E_ID);
	PCHK(pthread_mutex_lock(&gsRdvmtx) == 0, E_SYS);
	EREXIT(res, (pthread_mutex_lock(&gsPorman[porid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	EREXIT(res, (gsPorman[porid].created == TRUE)? E_OK : E_NOEXS, ERR_EXIT);
	gsPorman[porid].created = FALSE;
	EREXIT(res, (pthread_mutex_unlock(&gsPorman[porid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	while (deq_oqe(TCD_OBJ_MBFS, porid, &tskid, NULL) == E_OK && tskid != TSK_NONE){
		res |= sys_wup_tsk(tskid, TFN_DEL_POR, TTS_DLT);
	}
	while (deq_oqe(TCD_OBJ_MBFR, porid, &tskid, NULL) == E_OK && tskid != TSK_NONE){
		res |= sys_wup_tsk(tskid, TFN_DEL_POR, TTS_DLT);
	}
	res = del_oqe(TCD_OBJ_PORC, porid);
	res = del_oqe(TCD_OBJ_PORA, porid);
	EREXIT(res, (pthread_mutex_destroy(&gsPorman[porid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	memset(&gsPorman[porid], 0, sizeof(T_POR_MAN));
	goto ERR_EXIT2;
ERR_EXIT:
	err = pthread_mutex_unlock(&gsPorman[porid].mutex);
ERR_EXIT2:
	PCHK(pthread_mutex_unlock(&gsRdvmtx) == 0, E_SYS);
	return res;
}


ER_UINT tcal_por(
	ID porid,
	RDVPTN calptn,
	VP msg,
	UINT cmsgsz,
	TMO tmout
	)
{
	ER_UINT res, err;
	ID tskid, myid;
	T_RDV_MAN *rdv;
	INT i;
	
	PCHK(1 <= porid && porid <= TMAX_POR_ID, E_ID);
	PCHK(msg != NULL && (0 < tmout || tmout == TMO_FEVR) && calptn != 0, E_PAR);
	CHK_EXIST(res, err, gsRdvmtx, gsPorman[porid]);
	EREXIT(res, (cmsgsz <= TMAX_POR_CMSZ)? E_OK : E_PAR, ERR_EXIT);
	while (1){
		for (i=0; (res = ref_oqe(TCD_OBJ_PORA, porid, i, &tskid, (VP *)&rdv)) == E_OK; i++){
			if (rdv != NULL && (rdv->rdvptn & calptn)){
				EREXIT(res, get_tid(&myid), ERR_EXIT);
				EREXIT(res, rmv_oqe(TCD_OBJ_PORA, porid, tskid, NULL), ERR_EXIT);
				rdv->msg = msg;
				rdv->msgsz = cmsgsz;
				rdv->ctskid = myid;
				res = sys_wup_tsk(tskid, TFN_TCAL_POR, 0);
				res = sys_wai_tsk(TCD_OBJ_PORC, porid, tmout, TFN_TCAL_POR, &gsPorman[porid].mutex);
				res = (res == E_OK)? rdv->msgsz : res;
				rdv->use = FALSE;
				goto ERR_EXIT;
			}
		}
		EREXIT(res, wai_oqe(TCD_OBJ_PORC, porid, TSK_SELF, &calptn, tmout, TFN_TCAL_POR, &gsPorman[porid].mutex), ERR_EXIT);
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsPorman[porid].mutex) == 0, E_SYS);
	return res;
}


ER_UINT tacp_por(
	ID porid,
	RDVPTN acpptn,
	RDVNO *p_rdvno,
	VP msg,
	TMO tmout
	)
{
	ER res, err;
	ID tskid;
	INT i;
	RDVPTN calptn;
	
	PCHK(1 <= porid && porid <= TMAX_POR_ID, E_ID);
	PCHK((0 <= tmout || tmout == TMO_FEVR) && msg != NULL, E_PAR);
	CHK_EXIST(res, err, gsRdvmtx, gsPorman[porid]);
	for (i=0; (res = ref_oqe(TCD_OBJ_PORC, porid, i, &tskid, (VP *)&calptn)) == E_OK; i++){
		if (calptn & acpptn){
			res = sys_wup_tsk(tskid, TFN_TACP_POR, 0);
		}
	}
	gsRdvman[gRdvnum].rdvptn = acpptn;
	gsRdvman[gRdvnum].msg = NULL;
	gsRdvman[gRdvnum].msgsz = 0;
	gsRdvman[gRdvnum].ctskid = TSK_NONE;
	gRdvnum = (gRdvnum + 1) % TMAX_RDV_NO;
	*p_rdvno = gRdvnum;
	EREXIT(res, wai_oqe(TCD_OBJ_PORA, porid, TSK_SELF, &gsRdvman[gRdvnum], tmout, TFN_TACP_POR, &gsPorman[porid].mutex), ERR_EXIT);
	memcpy(msg, gsRdvman[*p_rdvno].msg, gsRdvman[*p_rdvno].msgsz);
	res = gsRdvman[*p_rdvno].msgsz;
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsPorman[porid].mutex) == 0, E_SYS);
	return res;
}


ER fwd_por(
	ID porid,
	RDVPTN calptn,
	RDVNO rdvno,
	VP msg,
	UINT cmsgsz
	)
{
	ER_UINT res, err;
	ID tskid, myid;
	T_RDV_MAN *rdv;
	INT i;
	
	PCHK(1 <= porid && porid <= TMAX_POR_ID, E_ID);
	PCHK(0 <= rdvno && rdvno <= TMAX_RDV_NO, E_PAR);
	PCHK(msg != NULL && calptn != 0, E_PAR);
	CHK_EXIST(res, err, gsRdvmtx, gsPorman[porid]);
	EREXIT(res, (cmsgsz <= TMAX_POR_CMSZ)? E_OK : E_PAR, ERR_EXIT);
	while (1){
		for (i=0; (res = ref_oqe(TCD_OBJ_PORA, porid, i, &tskid, (VP *)&rdv)) == E_OK; i++){
			if (rdv != NULL && (rdv->rdvptn & calptn)){
				EREXIT(res, get_tid(&myid), ERR_EXIT);
				EREXIT(res, rmv_oqe(TCD_OBJ_PORA, porid, tskid, NULL), ERR_EXIT);
				rdv->msg = gsRdvman[rdvno].msg;
				rdv->msgsz = gsRdvman[rdvno].msgsz;
				memcpy(rdv->msg, msg, cmsgsz);
				rdv->ctskid = gsRdvman[rdvno].ctskid;
				res = sys_wup_tsk(tskid, TFN_TCAL_POR, 0);
				goto ERR_EXIT;
			}
		}
		EREXIT(res, enq_oqe(TCD_OBJ_PORC, porid, gsRdvman[rdvno].ctskid, &calptn), ERR_EXIT);
	}
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsPorman[porid].mutex) == 0, E_SYS);
	return res;
}


ER rpl_rdv(
	RDVNO rdvno,
	VP msg,
	UINT rmsgsz
	)
{
	PCHK(0 <= rdvno && rdvno <= TMAX_RDV_NO && gsRdvman[rdvno].use, E_OBJ);
	PCHK(msg != NULL && 0 <= rmsgsz && rmsgsz <= TMAX_POR_RMSZ, E_PAR);
	memcpy(gsRdvman[rdvno].msg, msg, rmsgsz);
	gsRdvman[rdvno].msgsz = rmsgsz;
	gsRdvman[rdvno].use = FALSE;
	return sys_wup_tsk(gsRdvman[rdvno].ctskid, TFN_RPL_RDV, 0);
}


ER ref_por(
	ID porid,
	T_RPOR *pk_rmbf
	)
{
	ER res, err;
	
	PCHK(1 <= porid && porid <= TMAX_POR_ID, E_ID);
	PCHK(pk_rmbf != NULL, E_PAR);
	CHK_EXIST(res, err, gsRdvmtx, gsPorman[porid]);
	EREXIT(res, ref_oqe(TCD_OBJ_PORC, porid, 0, &pk_rmbf->ctskid, NULL), ERR_EXIT);
	EREXIT(res, ref_oqe(TCD_OBJ_PORA, porid, 0, &pk_rmbf->atskid, NULL), ERR_EXIT);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsPorman[porid].mutex) == 0, E_SYS);
	return res;
}


ER ref_rdv(
	RDVNO rdvno,
	T_RRDV *pk_rrdv
	)
{	
	PCHK(pk_rrdv != NULL, E_PAR);
	pk_rrdv->wtskid = (gsRdvman[rdvno % TMAX_RDV_NO].use)? gsRdvman[rdvno % TMAX_RDV_NO].ctskid : TSK_NONE;
	return E_OK;
}

