#include <pthread.h>
#include <string.h>
#include <malloc.h>
#include "itron.h"
#include "depend.h"
#include "common.h"

/***** constant definision *****/
#define CNT_FREF_CHAIN 16
enum {
	ATR_MEM_FREE,
	ATR_MEM_BUSY,
	ATR_MEM_END
} ATR_MEM;

/***** structure definision *****/
typedef struct _mblk_header_t {			/* memory block header				*/
		UW magic;						/* magic code (0xdeadbeef) 			*/
	struct _mblk_header_t *prev;		/* prev. block header				*/
	struct _mblk_header_t *next;		/* next block header				*/
	UINT len;							/* this block's length				*/
	ID task;							/* request task						*/
        UB using;                                                       /* using attribute                                      */
	UB reserve;
} T_BLK_HDR;

typedef struct {						/* fixed memory pool management info*/
	BOOL created;						/* created flag						*/
	ATR mpfatr;							/* memory pool attribute			*/
	UINT blkcnt;						/* memory block count				*/
	UINT blksz;							/* block size						*/
	UINT freecnt;						/* free memory block count			*/
	BOOL useheap;						/* use heap memory					*/
	VP mpf;								/* start of memory pool area		*/
	T_BLK_HDR *free;					/* free block chains				*/
	pthread_mutex_t mutex;				/* management info access mutex		*/
} T_MPF_MAN;

typedef struct {						/* memory pool management info		*/
	BOOL created;						/* created flag						*/
	ATR mplatr;							/* memory pool attribute			*/
	SIZE mplsz;							/* memory pool size					*/
	BOOL useheap;						/* use heap memory					*/
	VP mpl;								/* start of memory pool area		*/
	pthread_mutex_t mutex;				/* management info access mutex		*/
} T_MPL_MAN;

/***** global valiable *****/
static T_MPF_MAN gsMpfman[TMAX_MPF_ID];
static T_MPL_MAN gsMplman[TMAX_MPL_ID];
static pthread_mutex_t gsMpfmtx, gsMplmtx;

/***** functions *****/
ER inz_mpf(void)
{
	memset(&gsMpfman[0], 0, sizeof(gsMpfman));
	PCHK(pthread_mutex_init(&gsMpfmtx, NULL) == 0, E_SYS);
	return E_OK;
}

ER cre_mpf(
	ID mpfid,
	T_CMPF *pk_cmpf
	)
{
	ER res, err;
	T_BLK_HDR *blk;
	T_MPF_MAN *man;
	INT i;
	
	PCHK(1 <= mpfid && mpfid <= TMAX_MPF_ID, E_ID);
	PCHK(pk_cmpf != NULL, E_PAR);
	PCHK(TA_TFIFO <= pk_cmpf->mpfatr && pk_cmpf->mpfatr <= TA_CEILING, E_RSATR);
	PCHK(pk_cmpf->mpfatr == TA_TFIFO || pk_cmpf->mpfatr == TA_TPRI, E_PAR);
	PCHK(pthread_mutex_lock(&gsMpfmtx) == 0, E_SYS);
	man = &gsMpfman[mpfid];
	EREXIT(res, (man->created == FALSE)? E_OK : E_OBJ, ERR_EXIT);
	EREXIT(res, (pthread_mutex_init(&man->mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
	EREXIT(res, (cre_oqe(TCD_OBJ_MPF, mpfid, (pk_cmpf->mpfatr == TA_TFIFO)? TA_TFIFO : TA_TPRI) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
	man->mpfatr = pk_cmpf->mpfatr;
	man->blkcnt = pk_cmpf->blkcnt;
	man->blksz = pk_cmpf->blksz;
	man->freecnt = pk_cmpf->blkcnt;
	if (pk_cmpf->mpf != NULL){
		man->mpf = pk_cmpf->mpf;
		man->useheap = FALSE;
	} else {
		man->mpf = malloc((sizeof(T_BLK_HDR) + pk_cmpf->blksz) * pk_cmpf->blkcnt);
		man->useheap = TRUE;
		if (man->mpf == NULL) goto ERR_EXIT3;
	}
	man->free = (T_BLK_HDR *)man->mpf;
	/* initialize mpf block */
	for (i = 0, blk = man->mpf; i < pk_cmpf->blkcnt; i++, blk = blk->next){
		if (i == 0){						/* first block */
			blk->prev = NULL;
		}
		blk->len = pk_cmpf->blksz;
		blk->using = ATR_MEM_FREE;
		if (i == pk_cmpf->blkcnt - 1){		/* last block */
			blk->next = NULL;
		}else{
			blk->next = (T_BLK_HDR *)((UB *)blk + sizeof(T_BLK_HDR) + pk_cmpf->blksz);
			blk->next->prev = blk;
		}
	}
	man->created = TRUE;
	goto ERR_EXIT;
ERR_EXIT3:
	err = del_oqe(TCD_OBJ_MPF, mpfid);
ERR_EXIT2:
	err = pthread_mutex_destroy(&man->mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMpfmtx) == 0, E_SYS);
	return res;
}


ER_ID acre_mpf(
	T_CMPF *pk_cmpf
	)
{
	ER_ID res, err;
	ID mpfid;
	T_MPF_MAN *man;
	T_BLK_HDR *blk;
	INT i;
	
	PCHK(pk_cmpf != NULL, E_PAR);
	PCHK(pk_cmpf->mpfatr == TA_TFIFO || pk_cmpf->mpfatr == TA_TPRI, E_PAR);
	PCHK(pthread_mutex_lock(&gsMpfmtx) == 0, E_SYS);
	for (mpfid=1; mpfid<=TMAX_MPF_ID; mpfid++){
		if (gsMpfman[mpfid].created == TRUE){
			continue;
		}
		man = &gsMpfman[mpfid];
		EREXIT(res, (pthread_mutex_init(&man->mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
		EREXIT(res, (cre_oqe(TCD_OBJ_MPF, mpfid, (pk_cmpf->mpfatr == TA_TFIFO)? TA_TFIFO : TA_TPRI) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
		man->mpfatr = pk_cmpf->mpfatr;
		man->blkcnt = pk_cmpf->blkcnt;
		man->blksz = pk_cmpf->blksz;
		man->freecnt = pk_cmpf->blkcnt;
		if (pk_cmpf->mpf != NULL){
			man->mpf = pk_cmpf->mpf;
			man->useheap = FALSE;
		} else {
			man->mpf = malloc((sizeof(T_BLK_HDR) + pk_cmpf->blksz) * pk_cmpf->blkcnt);
			man->useheap = TRUE;
			if (man->mpf == NULL) goto ERR_EXIT3;
		}
		man->free = (T_BLK_HDR *)man->mpf;
		/* initialize mpf block */
		for (i = 0, blk = man->mpf; i < pk_cmpf->blkcnt; i++, blk = blk->next){
			if (i == 0){						/* first block */
				blk->prev = NULL;
			}
			blk->len = pk_cmpf->blksz;
			blk->using = ATR_MEM_FREE;
			if (i == pk_cmpf->blkcnt - 1){		/* last block */
				blk->next = NULL;
			}else{
				blk->next = (T_BLK_HDR *)((UB *)blk + sizeof(T_BLK_HDR) + pk_cmpf->blksz);
				blk->next->prev = blk;
			}
		}
//		blk->next = NULL;
		man->created = TRUE;
		break;
	}
	res = (mpfid > TMAX_MPF_ID)? E_NOID : (ER_ID)mpfid;
	goto ERR_EXIT;
ERR_EXIT3:
	err = del_oqe(TCD_OBJ_MPF, mpfid);
ERR_EXIT2:
	err = pthread_mutex_destroy(&man->mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMpfmtx) == 0, E_SYS);
	return res;
}


ER del_mpf(
	ID mpfid
	)
{
	ER res;
	INT err;
	ID tskid;
	T_MPF_MAN *man;
	
	PCHK(1 <= mpfid && mpfid <= TMAX_MPF_ID, E_ID);
	PCHK(pthread_mutex_lock(&gsMpfmtx) == 0, E_SYS);
	man = &gsMpfman[mpfid];
	EREXIT(res, (pthread_mutex_lock(&man->mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	EREXIT(res, (man->created == TRUE)? E_OK : E_NOEXS, ERR_EXIT);
	man->created = FALSE;
	EREXIT(res, (pthread_mutex_unlock(&man->mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	while (deq_oqe(TCD_OBJ_MPF, mpfid, &tskid, NULL) == E_OK && tskid != TSK_NONE){
		res |= sys_wup_tsk(tskid, TFN_DEL_MPF, TTS_DLT);
	}
	res = del_oqe(TCD_OBJ_MPF, mpfid);
	EREXIT(res, (pthread_mutex_destroy(&man->mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	if (man->useheap){
		free(man->mpf);
	}
	memset(man, 0, sizeof(T_MPF_MAN));
	goto ERR_EXIT2;
ERR_EXIT:
	err = pthread_mutex_unlock(&man->mutex);
ERR_EXIT2:
	PCHK(pthread_mutex_unlock(&gsMpfmtx) == 0, E_SYS);
	return res;
}


ER tget_mpf(
	ID mpfid,
	VP *p_blk,
	TMO tmout
	)
{
	ER res, err;
	T_BLK_HDR *blk;
	T_MPF_MAN *man;
	
	PCHK(1 <= mpfid && mpfid <= TMAX_MPF_ID, E_ID);
	CHK_EXIST(res, err, gsMpfmtx, gsMpfman[mpfid]);
	man = &gsMpfman[mpfid];
RETRY:
	if (man->free != NULL){
		blk = man->free;
		EREXIT(res, blk->using == ATR_MEM_BUSY, ERR_EXIT);
		man->free = blk->next;
		if (man->free != NULL){
			man->free->prev = NULL;
		}
		man->freecnt--;
		blk->using = ATR_MEM_BUSY;
		blk->prev = NULL;
		blk->next = NULL;
		*p_blk = ((UB *)blk + sizeof(T_BLK_HDR));
		res = E_OK;
	}else if (tmout == TMO_POL){
		res = E_TMOUT;
	}else{
		EREXIT(res, wai_oqe(TCD_OBJ_MPF, mpfid, TSK_SELF, NULL, tmout, TFN_TGET_MPF, &man->mutex), ERR_EXIT);
		tmout = 0;	// if timed out event then return E_TMOUT
		goto RETRY;
	}
ERR_EXIT:
	err = pthread_mutex_unlock(&man->mutex);
	return res;
}


ER rel_mpf(
	ID mpfid,
	VP p_blk
	)
{
	ER res, err;
	T_BLK_HDR *blk;
	ID tskid;
	VP p_data;
	T_MPF_MAN *man;
	
	PCHK(1 <= mpfid && mpfid <= TMAX_MPF_ID, E_ID);
	CHK_EXIST(res, err, gsMpfmtx, gsMpfman[mpfid]);
	blk = (T_BLK_HDR *)((UB *)p_blk - sizeof(T_BLK_HDR));
	man = &gsMpfman[mpfid];
	EREXIT(res, (blk->len == man->blksz && blk->using == ATR_MEM_BUSY)? E_OK : E_PAR, ERR_EXIT);
	man->freecnt++;
	blk->next = man->free;
	if (blk->next != NULL){
		blk->next->prev = blk;
	}
	blk->prev = NULL;
	blk->using = ATR_MEM_FREE;
	man->free = blk;
	EREXIT(res, deq_oqe(TCD_OBJ_MPF, mpfid, &tskid, &p_data), ERR_EXIT);
	if (tskid != TSK_NONE){
		res = sys_wup_tsk(tskid, TFN_REL_MPF, 0);
	}
ERR_EXIT:
	err = pthread_mutex_unlock(&man->mutex);
	return res;
}


ER ref_mpf(
	ID mpfid,
	T_RMPF *pk_rmpf
	)
{
	ER res, err;
	ID tskid;
	VP p_data;
	T_MPF_MAN *man;
	
	PCHK(1 <= mpfid && mpfid <= TMAX_MPF_ID, E_ID);
	CHK_EXIST(res, err, gsMpfmtx, gsMpfman[mpfid]);
	man = &gsMpfman[mpfid];
	EREXIT(res, ref_oqe(TCD_OBJ_MPF, mpfid, 0, &tskid, &p_data), ERR_EXIT);
	pk_rmpf->wtskid = tskid;
	pk_rmpf->fblkcnt = man->freecnt;
ERR_EXIT:
	err = pthread_mutex_unlock(&man->mutex);
	return res;
}


ER inz_mpl(void)
{
	memset(&gsMplman[0], 0, sizeof(gsMplman));
	PCHK(pthread_mutex_init(&gsMplmtx, NULL) == 0, E_SYS);
	return E_OK;
}

ER cre_mpl(
	ID mplid,
	T_CMPL *pk_cmpl
	)
{
	ER res, err;
	T_BLK_HDR *blk;
	T_MPL_MAN *man;
	
	PCHK(1 <= mplid && mplid <= TMAX_MPL_ID, E_ID);
	PCHK(pk_cmpl != NULL, E_PAR);
	PCHK(pk_cmpl->mplatr == TA_TFIFO || pk_cmpl->mplatr == TA_TPRI, E_PAR);
	PCHK(pthread_mutex_lock(&gsMplmtx) == 0, E_SYS);
	man = &gsMplman[mplid];
	EREXIT(res, (man->created == FALSE)? E_OK : E_OBJ, ERR_EXIT);
	EREXIT(res, (pthread_mutex_init(&man->mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
	EREXIT(res, (cre_oqe(TCD_OBJ_MPL, mplid, (pk_cmpl->mplatr == TA_TFIFO)? TA_TFIFO : TA_TPRI) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
	man->mplatr = pk_cmpl->mplatr;
	man->mplsz = pk_cmpl->mplsz;
	if (pk_cmpl->mpl != NULL){
		man->mpl = pk_cmpl->mpl;
		man->useheap = FALSE;
	} else {
		man->mpl = malloc(pk_cmpl->mplsz);
		man->useheap = TRUE;
		if (man->mpl == NULL) goto ERR_EXIT3;
	}
	/* initialize mpl block */
	blk = man->mpl;
	blk->magic = 0xdeadbeef;
	blk->prev = NULL;
	blk->next = NULL;
	blk->len = man->mplsz - sizeof(T_BLK_HDR);
	man->created = TRUE;
	goto ERR_EXIT;
ERR_EXIT3:
	err = del_oqe(TCD_OBJ_MPL, mplid);
ERR_EXIT2:
	err = pthread_mutex_destroy(&man->mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMplmtx) == 0, E_SYS);
	return res;
}


ER_ID acre_mpl(
	T_CMPL *pk_cmpl
	)
{
	ER_ID res, err;
	ID mplid;
	T_BLK_HDR *blk;
	T_MPL_MAN *man;
	
	PCHK(pk_cmpl != NULL, E_PAR);
	PCHK(pk_cmpl->mplatr == TA_TFIFO || pk_cmpl->mplatr == TA_TPRI, E_PAR);
	PCHK(pthread_mutex_lock(&gsMplmtx) == 0, E_SYS);
	for (mplid=1; mplid<=TMAX_MPL_ID; mplid++){
		if (gsMplman[mplid].created == TRUE){
			continue;
		}
		man = &gsMplman[mplid];
		EREXIT(res, (pthread_mutex_init(&man->mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
		EREXIT(res, (cre_oqe(TCD_OBJ_MPL, mplid, (pk_cmpl->mplatr == TA_TFIFO)? TA_TFIFO : TA_TPRI) == E_OK)? E_OK : E_SYS, ERR_EXIT2);
		man->mplatr = pk_cmpl->mplatr;
		man->mplsz = pk_cmpl->mplsz;
		if (pk_cmpl->mpl != NULL){
			man->mpl = pk_cmpl->mpl;
			man->useheap = FALSE;
		} else {
			man->mpl = malloc(pk_cmpl->mplsz);
			man->useheap = TRUE;
			if (man->mpl == NULL) goto ERR_EXIT3;
		}
		/* initialize mpl block */
		blk = man->mpl;
		blk->magic = 0xdeadbeef;
		blk->prev = NULL;
		blk->next = NULL;
		blk->len = man->mplsz - sizeof(T_BLK_HDR);
		man->created = TRUE;
		break;
	}
	res = (mplid > TMAX_MPL_ID)? E_NOID : (ER_ID)mplid;
	goto ERR_EXIT;
ERR_EXIT3:
	err = del_oqe(TCD_OBJ_MPL, mplid);
ERR_EXIT2:
	err = pthread_mutex_destroy(&man->mutex);
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsMplmtx) == 0, E_SYS);
	return res;
}


ER del_mpl(
	ID mplid
	)
{
	ER res;
	INT err;
	ID tskid;
	T_MPL_MAN *man;
	
	PCHK(1 <= mplid && mplid <= TMAX_MPL_ID, E_ID);
	PCHK(pthread_mutex_lock(&gsMplmtx) == 0, E_SYS);
	man = &gsMplman[mplid];
	EREXIT(res, (pthread_mutex_lock(&man->mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	EREXIT(res, (man->created == TRUE)? E_OK : E_NOEXS, ERR_EXIT);
	man->created = FALSE;
	EREXIT(res, (pthread_mutex_unlock(&man->mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	while (deq_oqe(TCD_OBJ_MPL, mplid, &tskid, NULL) == E_OK && tskid != TSK_NONE){
		res |= sys_wup_tsk(tskid, TFN_DEL_MPL, TTS_DLT);
	}
	res = del_oqe(TCD_OBJ_MPL, mplid);
	EREXIT(res, (pthread_mutex_destroy(&man->mutex) == 0)? E_OK : E_SYS, ERR_EXIT2);
	if (man->useheap){
		free(man->mpl);
	}
	memset(man, 0, sizeof(T_MPL_MAN));
	goto ERR_EXIT2;
ERR_EXIT:
	err = pthread_mutex_unlock(&man->mutex);
ERR_EXIT2:
	PCHK(pthread_mutex_unlock(&gsMplmtx) == 0, E_SYS);
	return res;
}


ER tget_mpl(
	ID mplid,
	INT blksz,
	VP *p_blk,
	TMO tmout
	)
{
	ER res, err;
	T_BLK_HDR *blk, *nextbak;
	SIZE lenbak;
	T_MPL_MAN *man;
	
	PCHK(1 <= mplid && mplid <= TMAX_MPL_ID, E_ID);
	CHK_EXIST(res, err, gsMplmtx, gsMplman[mplid]);
		man = &gsMplman[mplid];
	blk = man->mpl;
	*p_blk = NULL;
	blksz = ((blksz + 3) / 4) * 4;
RETRY:
	do { /* search memory block */
		if (blk->magic != 0xdeadbeef){
		  res = E_SYS;
		  goto ERR_EXIT;
		}
		if (blk->len > blksz + sizeof(T_BLK_HDR) && blk->using == ATR_MEM_FREE){ /* we can use a blk */
			lenbak = blk->len;
			nextbak = blk->next;
			blk->using = ATR_MEM_BUSY;
			get_tid(&blk->task);
			/* note: blk->len doesn't include a "blk" header */
			if ((nextbak != NULL && lenbak - blk->len - sizeof(T_BLK_HDR) > sizeof(T_BLK_HDR)) || /* can split */
				(nextbak == NULL && (UB *)blk + blksz + sizeof(T_BLK_HDR) * 2 < (UB *)man->mpl + man->mplsz)){
				blk->next = (T_BLK_HDR *)((UB *)blk + blksz + sizeof(T_BLK_HDR));
				blk->len = blksz;
				blk->next->magic = 0xdeadbeef;
				blk->next->prev = blk;
				blk->next->next = nextbak;
				if (blk->next->next != NULL){
					blk->next->next->prev = blk->next;
				}
				blk->next->len = lenbak - blk->len - sizeof(T_BLK_HDR); /* next block free space length */
				blk->next->using = ATR_MEM_FREE;
			}
			*p_blk = ((UB *)blk + sizeof(T_BLK_HDR));
			res = E_OK;
		} else {
			blk = blk->next;
		}
	} while (*p_blk == NULL && blk != NULL);
	if (*p_blk == NULL){
		if (tmout == TMO_POL){
			res = E_TMOUT;
		}else{
			EREXIT(res, wai_oqe(TCD_OBJ_MPL, mplid, TSK_SELF, NULL, tmout, TFN_TGET_MPL, &man->mutex), ERR_EXIT);
			tmout = 0;	// if timed out event then return E_TMOUT
			goto RETRY;
		}
	}
ERR_EXIT:
	err = pthread_mutex_unlock(&man->mutex);
	return res;
}


ER rel_mpl(
	ID mplid,
	VP p_blk
	)
{
	ER res, err;
	T_BLK_HDR *blk, *blk2;
	ID tskid;
	VP p_data;
	T_MPL_MAN *man;
	
	blk = (T_BLK_HDR *)((UB *)p_blk - sizeof(T_BLK_HDR));
	PCHK(1 <= mplid && mplid <= TMAX_MPL_ID, E_ID);
	PCHK(blk->magic == 0xdeadbeef, E_PAR);
	CHK_EXIST(res, err, gsMplmtx, gsMplman[mplid]);
	man = &gsMplman[mplid];
	EREXIT(res, (((blk->next == NULL) || (blk->next->prev == blk)) && ((blk->prev == NULL) || (blk->prev->next == blk)) && blk->using == ATR_MEM_BUSY)? E_OK : E_PAR, ERR_EXIT);
	blk->using = ATR_MEM_FREE;
	/* merge free block */
	if (blk->next != NULL && blk->next->using == ATR_MEM_FREE){
		blk2 = blk->next;
		blk->next = blk2->next;
		blk->len += blk2->len + sizeof(T_BLK_HDR);
		if (blk->next != NULL){
			blk->next->prev = blk;
		}
	}
	if (blk->prev != NULL && blk->prev->using == ATR_MEM_FREE){
		blk2 = blk->prev;
		blk2->next = blk->next;
		blk2->len += blk->len + sizeof(T_BLK_HDR);
		if (blk2->next != NULL){
			blk2->next->prev = blk2;
		}
	}
	EREXIT(res, deq_oqe(TCD_OBJ_MPL, mplid, &tskid, &p_data), ERR_EXIT);
	if (tskid != TSK_NONE){
		res = sys_wup_tsk(tskid, TFN_REL_MPL, 0);
	}
ERR_EXIT:
	err = pthread_mutex_unlock(&man->mutex);
	return res;
}


ER ref_mpl(
	ID mplid,
	T_RMPL *pk_rmpl
	)
{
	ER res, err;
	ID tskid;
	T_BLK_HDR *blk;
	VP p_data;
	
	PCHK(1 <= mplid && mplid <= TMAX_MPL_ID, E_ID);
	CHK_EXIST(res, err, gsMplmtx, gsMplman[mplid]);
	EREXIT(res, ref_oqe(TCD_OBJ_MPL, mplid, 0, &tskid, &p_data), ERR_EXIT);
	pk_rmpl->wtskid = tskid;
	for (blk = (T_BLK_HDR *)gsMplman[mplid].mpl, pk_rmpl->fmplsz = 0, pk_rmpl->fblksz = 0; blk != NULL; blk = blk->next){
		if (blk->using == ATR_MEM_FREE){
			pk_rmpl->fmplsz += blk->len;
			if (pk_rmpl->fblksz < blk->len){
				pk_rmpl->fblksz = blk->len;
			}
		}
	}
ERR_EXIT:
	err = pthread_mutex_unlock(&gsMplman[mplid].mutex);
	return res;
}


