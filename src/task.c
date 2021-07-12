#include <pthread.h>
#include <string.h>
#include "itron.h"
#include "depend.h"
#define _COMMON_DEF_
#include "common.h"

/***** constant definision *****/
#define TCNT_LOC_MTX (TMAX_MTX_ID + 31)/32
#define TCD_MTX_NOLOC (-1)
#define TCD_MTX_NONE 0

/***** structure definision *****/
typedef struct {						/* task locking mutex info struct	*/
	ID mtxid;							/* mutex id							*/
	PRI maxpri;							/* T_CEILING atr mutex's tsk pri	*/
} T_TSK_MTX;

typedef struct {						/* task management structure 		*/
	ID tskid;							/* itron task ID for pthread		*/
	PRI tskbpri;						/* itron base priority				*/
	PRI tskpri;							/* task now priority				*/
	pthread_key_t key;					/* POSIX ID key						*/
	pthread_t pth;						/* POSIX thread structure			*/
	pthread_attr_t pthatr;				/* POSIX thread attribute structure */
    pthread_mutex_t mutex;				/* task dependent synchronization	*/
	pthread_cond_t cond;				/* task dependent synchronization	*/
    SYSTIM waitim;         				/* task wait time					*/
	ID loctsk;							/* task info locker task			*/
	UW locnt;							/* task info lock count				*/
    W fn;								/* task wait request func			*/
	TCD_OBJ objcd;						/* task waiting obj code			*/
	ID objid;							/* task waiting obj id				*/
	STAT stat;							/* task status						*/
	T_CTSK ctsk;						/* task create info					*/
	VP_INT stacd;						/* task start code					*/
	UW actcnt;							/* act_tsk request counter			*/
    UW waicnt;							/* tsk force wait request counter	*/
    UW wupcnt;							/* tsk wake up request counter		*/
	BOOL exd;							/* exd_tsk called					*/
	PRI mtxmin;							/* TA_CEILING lowest pri			*/
	T_TSK_MTX loc[TCNT_LOC_MTX];		/* locking mutex list				*/
} T_TSK_MAN;

/***** global valiable *****/
static int giPrimin, giPrimax;
static T_TSK_MAN gsTskman[TMAX_TSK_ID];
static pthread_key_t gsKey;
static const ER (*gftRelWai[])(ID taskid) =	/* object release func */
    {
    };
    
/***** local function prototype *****/
ER isig_tsk(void);

static ER tsk_lock(
    ID tskid
    );
static ER tsk_unlock(
    ID tskid
    );
static ER cre_tskobj(
	ID tskid,
	PRI itskpri
	);
static void tsk_destructor(
	VP p_tskid
	);
static TASK tsk_thread(
	VP p_tskid
	);
    
/***** functions *****/
void inz_tsk(void)
{
	INT err;
	
	memset(&gsTskman[0], 0, sizeof(gsTskman));
	giPrimin = sched_get_priority_min(SCHED_FIFO);
	giPrimax = sched_get_priority_max(SCHED_FIFO);
    err = pthread_key_create(&gsKey, tsk_destructor);
}

ER cre_tsk(
	ID tskid,
	T_CTSK *pk_ctsk
	)
{
	ER err;
	
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat == 0, E_OBJ);
	PCHK(pk_ctsk != NULL && giPrimin <= pk_ctsk->itskpri && pk_ctsk->itskpri < giPrimax, E_PAR);
	PCHK(pk_ctsk->task != NULL && pk_ctsk->stksz < TMAX_STKSZ, E_PAR);
	PCHK(TA_HLNG <= (pk_ctsk->tskatr & ~TA_ACT) && (pk_ctsk->tskatr & ~TA_ACT) <= TA_ASM, E_RSATR);
	err = cre_tskobj(tskid, pk_ctsk->itskpri);
	PCHK(err == E_OK, err);
	gsTskman[tskid].stat = TTS_DMT;
	gsTskman[tskid].ctsk = *pk_ctsk;
	gsTskman[tskid].tskbpri = pk_ctsk->itskpri;
	gsTskman[tskid].tskpri = pk_ctsk->itskpri;
	if ((pk_ctsk->tskatr & TA_ACT) == 0){
		return E_OK;
	}else{
		return act_tsk(tskid);
	}
}


ER_ID acre_tsk(
	T_CTSK *pk_ctsk
	)
{
	ER_ID i;																		/* loop counter */
	ER err;
	
	PCHK(pk_ctsk != NULL && giPrimin <= pk_ctsk->itskpri && pk_ctsk->itskpri < giPrimax, E_PAR);
	PCHK(pk_ctsk->task != NULL && pk_ctsk->stksz < TMAX_STKSZ, E_PAR);
	PCHK(TA_HLNG <= (pk_ctsk->tskatr & ~TA_ACT) && (pk_ctsk->tskatr & ~TA_ACT) <= TA_ASM, E_RSATR);
	for (i = 1; i < TMAX_TSK_ID; i++){
		if (gsTskman[i].stat == 0){
			break;
		}
	}
	PCHK(i < TMAX_TSK_ID, E_NOID);
	err = cre_tskobj(i, pk_ctsk->itskpri);
	PCHK(err == E_OK, err);
	gsTskman[i].stat = TTS_DMT;
	gsTskman[i].ctsk = *pk_ctsk;
	gsTskman[i].tskbpri = pk_ctsk->itskpri;
	gsTskman[i].tskpri = pk_ctsk->itskpri;
	if ((pk_ctsk->tskatr & TA_ACT) != 0){
		err = act_tsk(i);
	}
	PCHK(err == E_OK, err);
	return i;
}


ER del_tsk(
	ID tskid
	)
{
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
	PCHK(gsTskman[tskid].stat == TTS_DMT, E_OBJ);
	memset(&gsTskman[tskid], 0, sizeof(T_TSK_MAN));
	return E_OK;
}

ER act_tsk(
	ID tskid
	)
{
	int err;																		/* POSIX call result */
	
	if (tskid == TSK_SELF){
		PCHK(get_tid(&tskid) == E_OK, E_SYS);
	}else{
		PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
		PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
	}
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
	if (gsTskman[tskid].stat == TTS_DMT){
		gsTskman[tskid].stacd = gsTskman[tskid].ctsk.exinf;
		PCHK((err = pthread_create(&gsTskman[tskid].pth, &gsTskman[tskid].pthatr, tsk_thread, (VP)tskid)) == 0, E_SYS);
		gsTskman[tskid].stat = TTS_RUN;
	}else{
		PCHK((gsTskman[tskid].actcnt + 1) != 0, E_QOVR);
		gsTskman[tskid].actcnt++;
	}
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
	return E_OK;
}

#if 0
ER iact_tsk(
	ID tskid
	)
{
	int err;																		/* POSIX call result */
	
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
	if (gsTskman[tskid].stat == TTS_DMT){
		PCHK(pthread_create(&gsTskman[tskid].pth, NULL, tsk_thread, (VP)tskid) != 0, E_SYS);
		gsTskman[tskid].stat = TTS_RUN;
	}else{
		PCHK((gsTskman[tskid].actcnt + 1) == 0, E_QOVR);
		gsTskman[tskid].actcnt++;
	}
	return E_OK;
}
#endif

ER_UINT can_act(
	ID tskid
	)
{
	ER_UINT res;																/* result */
	
	if (tskid == TSK_SELF){
		PCHK(get_tid(&tskid) != E_OK, E_SYS);
	}else{
		PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
		PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
	}
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
	res = (ER_UINT)gsTskman[tskid].actcnt;
	gsTskman[tskid].actcnt = 0;
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
	return res;
}

ER sta_tsk(
	ID tskid,
	VP_INT stacd
	)
{
//	PCHK(tskid == TSK_SELF, E_OBJ);
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
	PCHK(gsTskman[tskid].stat == TTS_DMT, E_OBJ);
	gsTskman[tskid].stacd = stacd;
	PCHK(pthread_create(&gsTskman[tskid].pth, NULL, tsk_thread, (VP)tskid) == 0, E_SYS);
	gsTskman[tskid].stat = TTS_RUN;
	return E_OK;
}

void ext_tsk()
{
	ID tskid;
	
	if (get_tid(&tskid) != E_OK) return;
    gsTskman[tskid].exd = FALSE;
	pthread_exit(NULL);
}


void exd_tsk()
{
	ID tskid;
	
    if (get_tid(&tskid) != E_OK) return;
    gsTskman[tskid].exd = TRUE;
	pthread_exit(NULL);
}

ER ter_tsk(
	ID tskid
	)
{
	ER err;																		/* ITRON call result */
	
    PCHK(get_tid(&tskid) == E_OK, E_SYS);
    PCHK(tskid != tskid, E_ILUSE);
	PCHK(gsTskman[tskid].stat != TTS_DMT, E_OBJ);
	gsTskman[tskid].exd = FALSE;
	err = (ER)pthread_cancel(gsTskman[tskid].pth);
    return err;
}

ER chg_pri(
	ID tskid,
	PRI tskpri
	)
{
    int err;
	struct sched_param pthprm;
    
    if (tskid == TSK_SELF){
        PCHK(get_tid(&tskid) == E_OK, E_SYS);
    }
    if (tskpri == TPRI_INI){
        tskpri = gsTskman[tskid].ctsk.itskpri;
    }
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
	PCHK(1 <= tskpri && tskpri <= TMAX_TSK_PRI, E_PAR);
	PCHK(gsTskman[tskid].stat != TTS_DMT, E_OBJ);
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
	EREXIT(err, (tskpri >= gsTskman[tskid].mtxmin)? E_OK : E_ILUSE, ERR_EXIT);
	if (gsTskman[tskid].tskbpri == gsTskman[tskid].tskpri){
		EREXIT(err, pthread_attr_getschedparam(&gsTskman[tskid].pthatr, &pthprm), ERR_EXIT);
		pthprm.sched_priority = ((giPrimin + tskpri) > giPrimax)? giPrimax : (giPrimin + tskpri);
		err = pthread_attr_setschedparam(&gsTskman[tskid].pthatr, &pthprm);
		if (err == 0){
			gsTskman[tskid].tskbpri = tskpri;
			gsTskman[tskid].tskpri = tskpri;
			if ((gsTskman[tskid].stat & TTS_WAI) && gsTskman[tskid].objcd != 0){
				/* pending */
			}else{
				/* not imprement (use POSIX scheduler policy) */
			}
		}
    }else{
		gsTskman[tskid].tskbpri = tskpri;
	}
ERR_EXIT:
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return (err == ENOSYS)? E_NOSPT : (err == 0)? E_OK : (err == EINVAL)? E_ILUSE : E_PAR;
}

ER chg_nowpri(
	ID tskid,
	PRI tskpri
	)
{
    int err;
	struct sched_param pthprm;
    
    if (tskid == TSK_SELF){
        PCHK(get_tid(&tskid) == E_OK, E_SYS);
    }
    if (tskpri == TPRI_INI){
        tskpri = gsTskman[tskid].ctsk.itskpri;
    }
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
	PCHK(1 <= tskpri && tskpri <= TMAX_TSK_PRI, E_PAR);
	PCHK(gsTskman[tskid].stat != TTS_DMT, E_OBJ);
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
	EREXIT(err, (tskpri >= gsTskman[tskid].mtxmin)? E_OK : E_ILUSE, ERR_EXIT);
	EREXIT(err, pthread_attr_getschedparam(&gsTskman[tskid].pthatr, &pthprm), ERR_EXIT);
	pthprm.sched_priority = ((giPrimin + tskpri) > giPrimax)? giPrimax : (giPrimin + tskpri);
	err = pthread_attr_setschedparam(&gsTskman[tskid].pthatr, &pthprm);
	if (err == 0){
		gsTskman[tskid].tskpri = tskpri;
	}
ERR_EXIT:
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return (err == ENOSYS)? E_NOSPT : (err == 0)? E_OK : (err == EINVAL)? E_ILUSE : E_PAR;
}

ER get_pri(
    ID tskid,
    PRI *p_tskpri
    )
{
    if (tskid == TSK_SELF){
        PCHK(get_tid(&tskid) == E_OK, E_SYS);
    }
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
    PCHK(p_tskpri != NULL, E_PAR);
	PCHK(gsTskman[tskid].stat != TTS_DMT, E_OBJ);
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
    *p_tskpri = gsTskman[tskid].tskpri;
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return E_OK;
}

ER ref_tsk(
    ID tskid,
    T_RTSK *pk_rtsk
    )
{
    if (tskid == TSK_SELF){
        PCHK(get_tid(&tskid) == E_OK, E_SYS);
    }
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
    PCHK(pk_rtsk != NULL, E_PAR);
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
    pk_rtsk->tskstat = gsTskman[tskid].stat;
    pk_rtsk->tskpri = gsTskman[tskid].tskpri;
    pk_rtsk->tskbpri = gsTskman[tskid].tskbpri;
    pk_rtsk->tskwait = 0;
    pk_rtsk->wobjid = 0;
    pk_rtsk->lefttmo = 0;
    pk_rtsk->actcnt = gsTskman[tskid].actcnt;
    pk_rtsk->wupcnt = 0;
    pk_rtsk->suscnt = 0;
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return E_OK;
}

ER ref_tst(
    ID tskid,
    T_RTST *pk_rtst
    )
{
    
    if (tskid == TSK_SELF){
        PCHK(get_tid(&tskid) == E_OK, E_SYS);
    }
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
    PCHK(pk_rtst != NULL, E_PAR);
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
    pk_rtst->tskstat = gsTskman[tskid].stat;
    pk_rtst->tskwait = 0;
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return E_OK;
}


ER tslp_tsk(
    TMO tmout
    )
{
    ER res;
    INT err;
    ID tskid;
    
    PCHK(get_tid(&tskid) == E_OK, E_SYS);
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
    if (gsTskman[tskid].wupcnt > 0){
        gsTskman[tskid].wupcnt--;
        res = E_RLWAI;
    }else{
        if (tmout == TMO_POL){
            res = E_OK;
        }else{
            gsTskman[tskid].stat |= TTS_WAI;
            gsTskman[tskid].fn = TFN_TSLP_TSK;
            gsTskman[tskid].waitim.utime = (tmout == TMO_FEVR)? -1 : 0;
            gsTskman[tskid].waitim.ltime = tmout;
            while (gsTskman[tskid].stat & TTS_WAI){
			   err = pthread_cond_wait(&gsTskman[tskid].cond, &gsTskman[tskid].mutex);
			   res = (err != 0)? E_SYS : (gsTskman[tskid].stat & TTS_FWU)? E_RLWAI : E_OK;
			}
            gsTskman[tskid].stat &= ~TTS_FWU;
        }
    }
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return res;
}


ER wup_tsk(
    ID tskid
    )
{
    ER res;
    INT err;
    
    if (tskid == TSK_SELF){
        PCHK(get_tid(&tskid) == E_OK, E_SYS);
    }
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
	PCHK(gsTskman[tskid].stat != TTS_DMT, E_OBJ);
    
    /* check wake upable status */
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
    if ((gsTskman[tskid].stat & TTS_WAI) && gsTskman[tskid].fn == TFN_TSLP_TSK){
        gsTskman[tskid].stat &= ~TTS_WAI;											/* reset flags */
        gsTskman[tskid].stat &= ~TTS_FWU;
        err = pthread_cond_signal(&gsTskman[tskid].cond);
        res = (err == 0)? E_OK : E_SYS;
    }else{
        if (gsTskman[tskid].wupcnt + 1 > TMAX_WUPCNT){
            res = E_QOVR;
        }else{
            res = E_OK;
            gsTskman[tskid].wupcnt++;
        }
    }
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return res;
}


ER_UINT can_wup(
    ID tskid
    )
{
    UINT cnt;
    
    if (tskid == TSK_SELF){
        PCHK(get_tid(&tskid) == E_OK, E_SYS);
    }
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
	PCHK(gsTskman[tskid].stat != TTS_DMT, E_OBJ);
    
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
    cnt = gsTskman[tskid].wupcnt;
    gsTskman[tskid].wupcnt = 0;
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return cnt;
}


ER rel_wai(
    ID tskid
    )
{
    ER res;
    INT err;
    
    if (tskid == TSK_SELF){
        PCHK(get_tid(&tskid) == E_OK, E_SYS);
    }
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
	PCHK(gsTskman[tskid].stat & TTS_WAS, E_OBJ);
    
    /* check wake upable status */
    if (gsTskman[tskid].fn != TFN_TSLP_TSK){									/* not wake up wait */
        gftRelWai[-gsTskman[tskid].fn](tskid);									/* release object */
    }
    gsTskman[tskid].stat |= TTS_FWU;
    gsTskman[tskid].stat &= ~TTS_WAI;											/* reset flags */
    err = pthread_cond_signal(&gsTskman[tskid].cond);
    res = (err == 0)? E_OK : E_SYS;
    return res;
}


ER sus_tsk(
    ID tskid
    )
{
    ER res;
    INT err;
	struct sched_param pthprm;
    
    if (tskid == TSK_SELF){
        PCHK(get_tid(&tskid) == E_OK, E_SYS);
    }
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
    if (gsTskman[tskid].stat & TTS_SCL){
        res = E_CTX;
        goto ERR_EXIT;
    }
    if (gsTskman[tskid].waicnt + 1 <= TMAX_SUSCNT){
        err = pthread_attr_getschedparam(&gsTskman[tskid].pthatr, &pthprm);
        if (err != 0){
            res = E_SYS;
            goto ERR_EXIT;
        }
        pthprm.sched_priority = giPrimax;
        err = pthread_attr_setschedparam(&gsTskman[tskid].pthatr, &pthprm);
        if (err != 0){
            res = E_SYS;
            goto ERR_EXIT;
        }
        gsTskman[tskid].stat |= TTS_SUS;
        gsTskman[tskid].waicnt++;
    }else{
        res = E_QOVR;
    }
ERR_EXIT:
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return res;
}
    
ER rsm_tsk(
    ID tskid
    )
{
    ER res;
    INT err;
	struct sched_param pthprm;
    
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
    PCHK((gsTskman[tskid].stat & TTS_SUS) == 0, E_OBJ);
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
    if (gsTskman[tskid].waicnt - 1 == 0){
        err = pthread_attr_getschedparam(&gsTskman[tskid].pthatr, &pthprm);
        if (err != 0){
            res = E_SYS;
            goto ERR_EXIT;
        }
        pthprm.sched_priority = giPrimax;
        err = pthread_attr_setschedparam(&gsTskman[tskid].pthatr, &pthprm);
        if (err != 0){
            res = E_SYS;
            goto ERR_EXIT;
        }
        gsTskman[tskid].stat &= ~TTS_SUS;
    }
    gsTskman[tskid].waicnt--;
ERR_EXIT:
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return res;
}
    
ER frsm_tsk(
    ID tskid
    )
{
    ER res;
    INT err;
	struct sched_param pthprm;
    
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
    PCHK((gsTskman[tskid].stat & TTS_SUS) == 0, E_OBJ);
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
    err = pthread_attr_getschedparam(&gsTskman[tskid].pthatr, &pthprm);
    if (err != 0){
        res = E_SYS;
        goto ERR_EXIT;
    }
    pthprm.sched_priority = giPrimax;
    err = pthread_attr_setschedparam(&gsTskman[tskid].pthatr, &pthprm);
    if (err != 0){
        res = E_SYS;
        goto ERR_EXIT;
    }
    gsTskman[tskid].stat &= ~TTS_SUS;
    gsTskman[tskid].waicnt = 0;
ERR_EXIT:
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return res;
}
    
ER dly_tsk(
    RELTIM dlytim
    )
{
    ER res;
    INT err;
    ID tskid;
    
    PCHK(get_tid(&tskid) == E_OK, E_SYS);
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
    gsTskman[tskid].stat |= TTS_WAI;
    gsTskman[tskid].fn = TFN_DLY_TSK;
    gsTskman[tskid].waitim = dlytim;
    if (dlytim.utime == 0 && dlytim.ltime == 0){
        gsTskman[tskid].waitim.utime = -1;
    }
    while (gsTskman[tskid].stat & TTS_WAI){
		err = pthread_cond_wait(&gsTskman[tskid].cond, &gsTskman[tskid].mutex);
		res = (err != 0)? E_SYS : E_RLWAI;
	}
    res = (err == 0)? E_RLWAI : (err == ETIMEDOUT)? E_OK : E_SYS;
    gsTskman[tskid].stat &= ~TTS_FWU;
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return res;
}


ER get_tid(
    ID *p_tskid
    )
{
    ER res;
    ID *tid;
    
    tid = (ID *)pthread_getspecific(gsKey);
    if (tid == NULL){
        res = E_SYS;
        *p_tskid = TSK_NONE;
    }else{
        res = E_OK;
        *p_tskid = *tid;
    }
    return res;
}


/***** sysytem routine *****/
ER sys_wai_tsk(
	TCD_OBJ objcd,
	ID objid,
    TMO tmout,
    W fn,
    pthread_mutex_t *mutex
    )
{
    ID tskid;
    ER res;
    INT err;

    PCHK(get_tid(&tskid) == E_OK, E_SYS);
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
	PCHK(tmout != TMO_POL, E_TMOUT);
	PCHK(pthread_mutex_unlock(mutex) == 0, E_SYS);
	gsTskman[tskid].stat |= TTS_WAI;
	gsTskman[tskid].fn = fn;
	gsTskman[tskid].objcd = objcd;
	gsTskman[tskid].objid = objid;
	gsTskman[tskid].waitim.utime = (tmout == TMO_FEVR)? -1 : 0;
	gsTskman[tskid].waitim.ltime = tmout;
	while (gsTskman[tskid].stat & TTS_WAI){
		err = pthread_cond_wait(&gsTskman[tskid].cond, &gsTskman[tskid].mutex);
		res = (err != 0)? E_SYS : (gsTskman[tskid].stat & TTS_FWU)? E_RLWAI :
			(gsTskman[tskid].stat & TTS_DLT)? E_DLT : E_OK;
	}
	gsTskman[tskid].objcd = 0;
	gsTskman[tskid].objid = 0;
	gsTskman[tskid].stat &= ~TTS_FWU;
	gsTskman[tskid].stat &= ~TTS_WAI;
	res = (pthread_mutex_lock(mutex) == 0)? res : E_SYS;
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return res;
}

ER sys_wup_tsk(
    ID tskid,
	FN fn,
	UW tskst
    )
{
    ER res;
    INT err;
    
    if (tskid == TSK_SELF){
        PCHK(get_tid(&tskid) == E_OK, E_SYS);
    }
	PCHK(1 <= tskid && tskid <= TMAX_TSK_ID, E_ID);
	PCHK(gsTskman[tskid].stat != 0, E_NOEXS);
	PCHK(gsTskman[tskid].stat != TTS_DMT, E_OBJ);
    
    /* check wake upable status */
    PCHK(tsk_lock(tskid) == E_OK, E_SYS);
    PCHK(gsTskman[tskid].stat & TTS_WAI, E_OBJ);
	gsTskman[tskid].fn = fn;
	gsTskman[tskid].stat |= tskst;
	gsTskman[tskid].stat &= ~TTS_WAI;
    err = pthread_cond_signal(&gsTskman[tskid].cond);
    res = (err == 0)? E_OK : E_SYS;
    PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
    return res;
}

ER tsk_loc_mtx(
	ID tskid,
	ID mtxid,
	PRI maxpri
	)
{
	INT i;
	ER res = E_OK;
	
	PCHK(tsk_lock(tskid) == E_OK, E_SYS);
	for (i=0; i < TCNT_LOC_MTX && gsTskman[tskid].loc[i].mtxid != TCD_MTX_NONE && gsTskman[tskid].loc[i].mtxid != TCD_MTX_NOLOC; i++){
		if (gsTskman[tskid].loc[i].mtxid == mtxid){
			res = E_ILUSE;
			goto LOCKING;
		}
	}
	if (i < TCNT_LOC_MTX){
		gsTskman[tskid].loc[i].mtxid = mtxid;
		gsTskman[tskid].loc[i].maxpri = maxpri;
		if (gsTskman[tskid].tskpri > maxpri && maxpri >= TMIN_TPRI){
			res = chg_nowpri(tskid, maxpri);
		}
	}
LOCKING:
	PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
	return res;
}


ER tsk_unl_mtx(
	ID tskid,
	ID mtxid
	)
{
	INT i, j;
	ER res = E_ILUSE;
	
	PCHK(tsk_lock(tskid) == E_OK, E_SYS);
	for (i=0, gsTskman[tskid].mtxmin=0; i < TCNT_LOC_MTX && gsTskman[tskid].loc[i].mtxid != TCD_MTX_NONE; i++){
		if (gsTskman[tskid].loc[i].mtxid == mtxid){
			gsTskman[tskid].loc[i].mtxid = TCD_MTX_NOLOC;
			gsTskman[tskid].loc[i].maxpri = TCD_MTX_NONE;
			res = E_OK;
			continue;
		}else if (gsTskman[tskid].loc[i].mtxid != TCD_MTX_NOLOC && gsTskman[tskid].loc[i].maxpri > gsTskman[tskid].mtxmin){
			gsTskman[tskid].mtxmin = gsTskman[tskid].loc[i].maxpri;
		}
	}
	if (res == E_OK){
		for (j = i-1; j >= 0 && gsTskman[tskid].loc[j].mtxid == TCD_MTX_NOLOC; j--){
			gsTskman[tskid].loc[j].mtxid = TCD_MTX_NONE;
		}
		if (j < 0){
			res = chg_nowpri(tskid, gsTskman[tskid].tskbpri);
		}
	}
	PCHK(tsk_unlock(tskid) == E_OK, E_SYS);
	return res;
}

ER isig_tsk(void)
{
	int tskid;
	ER res, err;

	for (tskid = 0, res = E_OK; tskid < TMAX_TSK_ID; tskid++){
		if (tsk_lock(tskid) != E_OK){
			continue;
		}
		if ((gsTskman[tskid].stat & TTS_WAS) != 0 && gsTskman[tskid].waitim.utime >= 0){
			if (gsTskman[tskid].waitim.ltime == 1 && gsTskman[tskid].waitim.utime == 0){
				gsTskman[tskid].waitim.ltime = 0;
				gsTskman[tskid].stat &= ~TTS_WAI;                                                                                       /* reset flags */
				gsTskman[tskid].stat &= ~TTS_FWU;
				err = pthread_cond_signal(&gsTskman[tskid].cond);
				res = (err == 0)? res : E_SYS;
			}else{
				if (gsTskman[tskid].waitim.ltime == 0){
					gsTskman[tskid].waitim.utime--;
				}
				gsTskman[tskid].waitim.ltime--;
			}
		}
        tsk_unlock(tskid);
	}
	return res;
}

/***** sub routine *****/
static ER tsk_lock(
    ID tskid
    )
{
    ER res;
    INT err = 0;

    if (tskid == TSK_SELF){
        PCHK(get_tid(&tskid) == E_OK, E_SYS);
    }
//	if (gsTskman[tskid].loctsk != tskid || gsTskman[tskid].locnt == 0){		// 2015.03.18 comment out
		err = pthread_mutex_lock(&gsTskman[tskid].mutex);
//	}
	gsTskman[tskid].loctsk = tskid;
	gsTskman[tskid].locnt++;
    res = (err == 0)? E_OK : E_SYS;
    return res;
}


static ER tsk_unlock(
    ID tskid
    )
{
    ER res;
    INT err = 0;

    if (tskid == TSK_SELF){
        PCHK(get_tid(&tskid) == E_OK, E_SYS);
    }
	gsTskman[tskid].locnt = (gsTskman[tskid].locnt > 0)? gsTskman[tskid].locnt - 1 : 0;
//	if (gsTskman[tskid].locnt == 0){
		gsTskman[tskid].loctsk = TSK_NONE;
		err = pthread_mutex_unlock(&gsTskman[tskid].mutex);
//	}
    res = (err == 0)? E_OK : E_SYS;
    return res;
}


static ER cre_tskobj(
	ID tskid,
	PRI itskpri
	)
{
	int err;																		/* POSIX call result */
	struct sched_param pthprm;
	
	gsTskman[tskid].tskid = tskid;
	err = pthread_attr_init(&gsTskman[tskid].pthatr);
	PCHK(err == 0, E_NOMEM);
	err = pthread_attr_getschedparam(&gsTskman[tskid].pthatr, &pthprm);
	PCHK(err == 0, E_NOMEM);
	pthprm.sched_priority = ((giPrimin + itskpri) > giPrimax)? giPrimax : (giPrimin + itskpri);
	EREXIT(err, pthread_attr_setdetachstate(&gsTskman[tskid].pthatr,  PTHREAD_CREATE_DETACHED), ERR_EXIT1);
//    EREXIT(err, pthread_attr_setschedpolicy(&gsTskman[tskid].pthatr, SCHED_FIFO), ERR_EXIT1);
//	EREXIT(err, pthread_attr_setschedparam(&gsTskman[tskid].pthatr, &pthprm), ERR_EXIT1);
    EREXIT(err, pthread_mutex_init(&gsTskman[tskid].mutex, NULL), ERR_EXIT2);
    EREXIT(err, pthread_cond_init(&gsTskman[tskid].cond, NULL), ERR_EXIT3);
	return E_OK;
ERR_EXIT3:
    err = pthread_mutex_destroy(&gsTskman[tskid].mutex);
ERR_EXIT2:
	err = pthread_key_delete(gsTskman[tskid].key);
ERR_EXIT1:
	err = pthread_attr_destroy(&gsTskman[tskid].pthatr);
	return E_SYS;
}

static void tsk_destructor(
	VP p_tskid
	)
{
	ID tskid;
	int i, err;																		/* POSIX call result */
	
	tskid = *((ID *)p_tskid);
	for (i=0; i<TCNT_LOC_MTX && gsTskman[tskid].loc[i].mtxid != TCD_MTX_NONE; i++){
		if (gsTskman[tskid].loc[i].mtxid > 0){
			err = rmv_oqe(TCD_OBJ_MTX, gsTskman[tskid].loc[i].mtxid, tskid, NULL);
			gsTskman[tskid].loc[i].mtxid = TCD_MTX_NOLOC;
		}
	}
	if (gsTskman[tskid].exd == FALSE){
		if (gsTskman[tskid].actcnt > 0){
			err = pthread_create(&gsTskman[tskid].pth, NULL, tsk_thread, (VP)tskid);
			if (err == 0){
				gsTskman[tskid].actcnt--;
			}
		}else{
			gsTskman[tskid].stat = TTS_DMT;
		}
	}else{
		gsTskman[tskid].stat = TTS_DMT;
		err = del_tsk(tskid);
	}
}

static TASK tsk_thread(
	VP p_tskid
	)
{
	ID tskid;
	int err;

	tskid = (ID)p_tskid;
	EREXIT(err, pthread_setspecific(gsKey, &gsTskman[tskid].tskid), ERR_EXIT);
	gsTskman[tskid].ctsk.task(gsTskman[tskid].stacd);
	return;
ERR_EXIT:
	pthread_exit(NULL);
}
