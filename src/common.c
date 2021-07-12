#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "itron.h"
#include "depend.h"
#include "common.h"

/*
 * Waiting queue
 */

extern void inz_tsk(void);
extern ER inz_sem(void);
extern ER inz_flg(void);
extern ER inz_dtq(void);
extern ER inz_mbx(void);
extern ER inz_mpf(void);
extern ER inz_mpl(void);
extern ER inz_mtx(void);
extern ER inz_mbf(void);
extern ER inz_rdv(void);
extern ER inz_tim(void);
extern ER inz_cyc(void);
extern ER inz_alm(void);

/***** structure definision *****/
typedef struct t_oqe_inf {				/* Waiting task info				*/
	struct t_oqe_inf *next;				/* next info						*/
	ID tskid;							/* Waiting task ID					*/
	PRI tskpri;							/* Waiting task priority			*/
	VP data;							/* data								*/
} T_OQE_INF;

/* There is no mutex, because same obj will exclusive access by obj's mutex */
typedef struct t_oqe_man {				/* Objext Wait Queue management		*/
	struct t_oqe_man *next;				/* next management info				*/
	T_OQE_INF *top;						/* first wait task info				*/
	T_OQE_INF *end;						/* end wait task info				*/
	ID objid;							/* object id number					*/
	ATR objatr;							/* waiting attribute				*/
} T_OQE_MAN;

/***** global valiable *****/
static T_OQE_MAN *gsOqeman[TCD_OBJ_END];

/***** macro *****/
#define SRH_QUE(objcd, objid, p_man) \
	for (p_man = gsOqeman[objcd]; p_man != NULL && p_man->objid != objid; p_man = p_man->next)
    
/***** functions *****/
ER inz_knl(void)
{
	ER res;

	ERRET(res, inz_oqe());
	inz_tsk();
	ERRET(res, inz_sem());
	ERRET(res, inz_flg());
	ERRET(res, inz_dtq());
	ERRET(res, inz_mbx());
	ERRET(res, inz_mpf());
	ERRET(res, inz_mpl());
	ERRET(res, inz_mtx());
	ERRET(res, inz_mbf());
	ERRET(res, inz_rdv());
	ERRET(res, inz_tim());
	ERRET(res, inz_cyc());
	ERRET(res, inz_alm());
	return E_OK;
}
ER inz_oqe(void)
{
	memset(&gsOqeman[0], 0, sizeof(gsOqeman));
	return E_OK;
}

ER cre_oqe(
	TCD_OBJ objcd,
	ID objid,
	ATR objatr
	)
{
	T_OQE_MAN *man, *now;
	
	man = malloc(sizeof(T_OQE_MAN));
	memset(man, 0, sizeof(T_OQE_MAN));
	man->objid = objid;
	man->objatr = objatr;
	if (gsOqeman[objcd] == NULL){
		gsOqeman[objcd] = man;
	}else{
		now = gsOqeman[objcd];
		while (now->next != NULL){
			PCHK(now->objid != objid, E_SYS);
			now = now->next;
		}
		now->next = man;
	}
	return E_OK;
}

ER del_oqe(
	TCD_OBJ objcd,
	ID objid
	)
{
	T_OQE_MAN *now, *prev;
	
	for (prev = gsOqeman[objcd], now = prev;;prev = now, now = now->next){
		if (now->objid == objid){
			prev->next = now->next;
			free(now);
			return E_OK;
		}else if (now->next == NULL){
			return E_SYS;
		}
	}
	return E_NOEXS;
}

ER enq_oqe(
	TCD_OBJ objcd,
	ID objid,
	ID tskid,
	VP data
	)
{
	T_OQE_MAN *que;
	T_OQE_INF *prev, *next, *ins;
	ER res;
	T_RTSK rtsk;
	
	if (tskid == TSK_SELF){
		ERRET(res, get_tid(&tskid));
	}
	ERRET(res, ref_tsk(tskid, &rtsk));
	SRH_QUE(objcd, objid, que);
	PCHK(que != NULL, E_SYS);
	PCHK((ins = (T_OQE_INF *)malloc(sizeof(T_OQE_INF))) != NULL, E_NOMEM);
	memset(ins, 0, sizeof(T_OQE_INF));
	ins->tskid = tskid;
	ins->tskpri = rtsk.tskpri;
	ins->data = data;
	ins->next = NULL;
	if (que->objatr & TA_TPRI){
		for (prev = que->top, next = prev; next != NULL; prev = next, next = next->next){
			if (next->tskpri > rtsk.tskpri){
				ins->next = next;
				prev->next = ins;
				break;
			}
		}
		if (next == NULL){
			if (que->end != NULL){
				prev = que->end;
				prev->next = ins;
				que->end = ins;
			}else{
				que->top = ins;
				que->end = ins;
			}
		}
	}else{
		if (que->end != NULL){
			prev = que->end;
			prev->next = ins;
			que->end = ins;
		}else{
			que->top = ins;
			que->end = ins;
		}
	}
	return E_OK;
}

ER deq_oqe(
	TCD_OBJ objcd,
	ID objid,
	ID *p_tskid,				/* if return value is TSK_NONE then queue empty */
	VP *pp_data					/* if NULL then ignore this param */
	)
{
	T_OQE_MAN *que;
	T_OQE_INF *get;

	SRH_QUE(objcd, objid, que);
	PCHK(que != NULL, E_SYS);
	get = que->top;
	if (get != NULL){
		*p_tskid = get->tskid;
		if (pp_data != NULL) *pp_data = get->data;
		que->top = get->next;
		free(get);
	}else{
		*p_tskid = TSK_NONE;
		if (pp_data != NULL) *pp_data = NULL;
	}
	if (que->top == NULL){
		que->end = NULL;
	}
	return E_OK;
}

ER ref_oqe(
	TCD_OBJ objcd,
	ID objid,
	INT offset,
	ID *p_tskid,
	VP *pp_data					/* if NULL then ignore this param */
	)
{
	T_OQE_MAN *que;
	T_OQE_INF *get;
	INT i;

	
	SRH_QUE(objcd, objid, que);
	PCHK(que != NULL, E_SYS);
	for (i=0, get=que->top; i<offset && get!=NULL; i++, get=get->next);
	PCHK(i==offset && get != NULL, E_NOEXS);
	*p_tskid = get->tskid;
	if (pp_data != NULL) *pp_data = get->data;
	return E_OK;
}

ER get_oqe(						/* get mideum position data from queue */
	TCD_OBJ objcd,
	ID objid,
	INT offset,
	ID *p_tskid,
	VP *pp_data					/* if NULL then ignore this param */
	)
{
	T_OQE_MAN *que;
	T_OQE_INF *get, *prev;
	INT i;

	SRH_QUE(objcd, objid, que);
	PCHK(que != NULL, E_ID);
	for (i=0, prev=get=que->top; i<offset && get!=NULL; i++, prev=get, get=get->next);
	PCHK(i==offset && get != NULL, E_NOEXS);
	*p_tskid = get->tskid;
	if (pp_data != NULL) *pp_data = get->data;
	if (get == que->top){
		que->top = get->next;
		if (que->top == NULL) que->end = NULL;
	}else{
		prev->next = get->next;
		if (prev->next == NULL) que->end = prev;
	}
	free(get);
	return E_OK;
}

ER srh_oqe(						/* search waiting task */
	TCD_OBJ objcd,
	ID objid,
	INT *offset,				/* found position */
	ID tskid,					/* search task id (if TSK_SELF then ignore this param) */
	VP pk_data					/* search data pattern (if pk_tskid isn't TSK_SELF then ignore this param) */
	)
{
	T_OQE_MAN *que;
	T_OQE_INF *get;

	SRH_QUE(objcd, objid, que);
	PCHK(que != NULL, E_SYS);
	for (*offset=0, get=que->top; get != NULL; (*offset)++, get=get->next){
		if ((tskid != TSK_SELF && tskid == get->tskid) ||(tskid == TSK_SELF && pk_data == get->data)){
			break;
		}
	}
	PCHK(get != NULL, E_NOEXS);
	return E_OK;
}

ER rmv_oqe(						/* search & remove waiting task */
	TCD_OBJ objcd,
	ID objid,
	ID tskid,					/* search task id (if TSK_SELF then ignore this param) */
	VP pk_data					/* search data pattern (if pk_tskid isn't TSK_SELF then ignore this param) */
	)
{
	T_OQE_MAN *que;
	T_OQE_INF *get, *prev;

	SRH_QUE(objcd, objid, que);
	PCHK(que != NULL, E_ID);
	for (get=que->top, prev=que->top; get != NULL; prev=get, get=get->next){
		if ((tskid != TSK_SELF && tskid == get->tskid) ||(tskid == TSK_SELF && *((UW *)pk_data) == *((UW *)get->data))){
			if (prev != get){
				prev->next = get->next;
			}
			if (que->top == get) que->top = get->next;
			if (get->next == NULL){
				if (prev != get){
					que->end = prev;
				}else{
					que->end = NULL;
				}
			}
			free(get);
			return E_OK;
		}
	}
	return E_NOEXS;
}

ER wai_oqe(
	TCD_OBJ objcd,
	ID objid,
	ID tskid,
	VP data,
	TMO tmout,
	FN fn,
    pthread_mutex_t *pk_mutex	
	)
{
	ER res;
	
	PCHK(tmout != TMO_POL, E_TMOUT);
	if (tskid == TSK_SELF) ERRET(res, get_tid(&tskid));
	PCHK((res = enq_oqe(objcd, objid, tskid, data)) == E_OK, res);
	res = sys_wai_tsk(objcd, objid, tmout, fn, pk_mutex);
	if (res != E_OK && res != E_DLT) PCHK(rmv_oqe(objcd, objid, tskid, NULL) == E_OK, E_SYS);
	return res;
}

