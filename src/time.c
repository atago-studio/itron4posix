#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include "itron.h"
#include "depend.h"
#include "common.h"

/***** structure definision *****/
typedef struct {						/* cyclic handler management info	*/
	BOOL created;						/* created flag						*/
	BOOL started;						/* handler started flag				*/
	ATR cycatr;							/* cyclic handler attribute			*/
	VP_INT exinf;						/* extention information			*/
	FP cychdr;							/* handler pointer					*/
	RELTIM cyctim;						/* cyclic handler wakeup cycle		*/
	RELTIM cycphs;						/* cyclic handler wakeup phase		*/
	RELTIM count;						/* cyclic counter					*/
	pthread_mutex_t mutex;				/* management info access mutex		*/
} T_CYC_MAN;

typedef struct {						/* alarm handler management info 	*/
	BOOL created;						/* created flag						*/
	BOOL started;						/* handler started flag				*/
	ATR almatr;							/* alatm handler attribute			*/
	VP_INT exinf;						/* extention information			*/
	FP almhdr;							/* handler pointer					*/
	RELTIM lefttim;						/* left time to wakeup				*/
	pthread_mutex_t mutex;				/* management info access mutex		*/
} T_ALM_MAN;

/***** global valiable *****/
static T_CYC_MAN gsCycman[TMAX_CYC_ID];
static T_ALM_MAN gsAlmman[TMAX_ALM_ID];
static pthread_mutex_t gsCycmtx, gsAlmmtx;
static SYSTIM gSystim;

/***** macro *****/
#define inc_systim(tim) \
	tim.ltime++; \
	if (tim.ltime == 0){ \
		tim.utime++; \
	}

#define dec_systim(tim) \
	if (tim.ltime == 0){ \
		tim.utime--; \
	} \
	tim.ltime--

#define sub_systim(tim1, tim2, tim3) \
	tim1.utime = tim2.utime - tim3.utime; \
	tim1.ltime = tim2.ltime - tim3.ltime

#define cmp_systim(tim1, tim2) \
	(tim1.utime == tim2.utime && tim1.ltime == tim2.ltime)

#define clr_systim(tim) \
    tim.utime = 0; \
    tim.ltime = 0

#define IS_SYSTIM0(tim) (tim.utime == 0 && tim.ltime == 0)

/***** prottypes *****/
extern ER isig_tsk(void);
static ER isig_cyc(void);
static ER isig_alm(void);
//static ER isig_ovr(void);

/***** functions *****/
/* System Time Management */
ER inz_tim(void)
{
	memset(&gSystim, 0, sizeof(gSystim));
	return E_OK;
}


ER set_tim(
	SYSTIM *p_systim
	)
{
	PCHK(p_systim != NULL, E_PAR);
	gSystim = *p_systim;
	return E_OK;
}


ER get_tim(
	SYSTIM *p_systim
	)
{
	PCHK(p_systim != NULL, E_PAR);
	*p_systim = gSystim;
	return E_OK;
}


ER isig_tim(void)
{
	ER res;
	
	/* Systim increment */
	if (gSystim.ltime > gSystim.ltime + SYSTIC){
		gSystim.utime++;
	}
	gSystim.ltime += SYSTIC;
	
	/* Task time update */
	res = isig_tsk();

	/* Cyclic handler time update */
	res |= isig_cyc();
	
	/* Alarm handler time update */
	res |= isig_alm();
	/* Over run handler time update */
	
	return res;
}

/* Cyclic Handler */
ER inz_cyc(void)
{
	memset(&gsCycman[0], 0, sizeof(gsCycman));
	PCHK(pthread_mutex_init(&gsCycmtx, NULL) == 0, E_SYS);
	return E_OK;
}


ER cre_cyc(
	ID cycid,
	T_CCYC *pk_ccyc
	)
{
	ER res;
	
	PCHK(1 <= cycid && cycid <= TMAX_CYC_ID, E_ID);
	PCHK(pk_ccyc != NULL, E_PAR);
	PCHK(!(pk_ccyc->cycatr & (TA_ASM | TA_PHS)), E_NOSPT);
	PCHK(pk_ccyc->cycatr & ~(TA_HLNG | TA_STA), E_PAR);
	PCHK(pthread_mutex_lock(&gsCycmtx) == 0, E_SYS);
	EREXIT(res, (gsCycman[cycid].created == FALSE)? E_OK : E_OBJ, ERR_EXIT);
    EREXIT(res, (pthread_mutex_init(&gsCycman[cycid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
	gsCycman[cycid].created = TRUE;
	gsCycman[cycid].started = (pk_ccyc->cycatr & TA_STA)? TRUE : FALSE;
	gsCycman[cycid].cycatr = pk_ccyc->cycatr;
	gsCycman[cycid].exinf = pk_ccyc->exinf;
	gsCycman[cycid].cychdr = pk_ccyc->cychdr;
	gsCycman[cycid].cyctim = pk_ccyc->cyctim;
	gsCycman[cycid].cycphs = pk_ccyc->cycphs;
	memset (&gsCycman[cycid].count, 0, sizeof(RELTIM));
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsCycmtx) == 0, E_SYS);
	return res;
}


ER_ID acre_cyc(
	T_CCYC *pk_ccyc
	)
{
	ER_ID res, i;
	
	PCHK(pk_ccyc != NULL, E_PAR);
	PCHK(!(pk_ccyc->cycatr & (TA_ASM | TA_PHS)), E_NOSPT);
	PCHK(!(pk_ccyc->cycatr & ~(TA_HLNG | TA_STA)), E_PAR);
	PCHK(pthread_mutex_lock(&gsCycmtx) == 0, E_SYS);
	for (i = 1; i <= TMAX_CYC_ID; i++){
		if (gsCycman[i].created == TRUE){
			continue;
		}
	    EREXIT(res, (pthread_mutex_init(&gsCycman[i].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
		gsCycman[i].created = TRUE;
		gsCycman[i].started = (pk_ccyc->cycatr & TA_STA)? TRUE : FALSE;
		gsCycman[i].cycatr = pk_ccyc->cycatr;
		gsCycman[i].exinf = pk_ccyc->exinf;
		gsCycman[i].cychdr = pk_ccyc->cychdr;
		gsCycman[i].cyctim = pk_ccyc->cyctim;
		gsCycman[i].cycphs = pk_ccyc->cycphs;
		memset (&gsCycman[i].count, 0, sizeof(RELTIM));
		break;
	}
	res = (res > TMAX_CYC_ID)? E_NOID : i;
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsCycmtx) == 0, E_SYS);
	return res;
}


ER del_cyc(
	ID cycid
	)
{
	ER res;
	
	PCHK(1 <= cycid && cycid <= TMAX_CYC_ID, E_ID);
	PCHK(pthread_mutex_lock(&gsCycmtx) == 0, E_SYS);
	EREXIT(res, (pthread_mutex_lock(&gsCycman[cycid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT);
	EREXIT(res, (gsCycman[cycid].created == TRUE)? E_OK : E_NOEXS, ERR_EXIT);
	gsCycman[cycid].created = FALSE;
	EREXIT(res, (pthread_mutex_unlock(&gsCycman[cycid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT);
	EREXIT(res, (pthread_mutex_destroy(&gsCycman[cycid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT);
	memset(&gsCycman[cycid], 0, sizeof(T_CYC_MAN));
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsCycmtx) == 0, E_SYS);
	return res;
}


ER sta_cyc(
	ID cycid
	)
{
	ER res, err;
	
	PCHK(1 <= cycid && cycid <= TMAX_CYC_ID, E_ID);
	CHK_EXIST(res, err, gsCycmtx, gsCycman[cycid]);
	gsCycman[cycid].started = TRUE;
	err = pthread_mutex_unlock(&gsCycman[cycid].mutex);
	return res;
}


ER stp_cyc(
	ID cycid
	)
{
	ER res, err;
	
	PCHK(1 <= cycid && cycid <= TMAX_CYC_ID, E_ID);
	CHK_EXIST(res, err, gsCycmtx, gsCycman[cycid]);
	gsCycman[cycid].started = FALSE;
	err = pthread_mutex_unlock(&gsCycman[cycid].mutex);
	return res;
}


ER ref_cyc(
	ID cycid,
	T_RCYC *pk_rcyc
	)
{
	ER res, err;
	
	PCHK(1 <= cycid && cycid <= TMAX_CYC_ID, E_ID);
	PCHK(pk_rcyc != NULL, E_PAR);
	CHK_EXIST(res, err, gsCycmtx, gsCycman[cycid]);
	pk_rcyc->cycstat = (gsCycman[cycid].started)? TCYC_STA : TCYC_STP;
	sub_systim(pk_rcyc->lefttim, gsCycman[cycid].cyctim, gsCycman[cycid].count);
	err = pthread_mutex_unlock(&gsCycman[cycid].mutex);
	return res;
}


static ER isig_cyc(void)
{
	INT i;
	ER res;
	
	for (i = 1; i <= TMAX_CYC_ID; i++){
		if (gsCycman[i].created){
			EREXIT(res, (pthread_mutex_lock(&gsCycman[i].mutex) == 0)? E_OK : E_SYS, ERR_EXIT);
			if (gsCycman[i].started){
				inc_systim(gsCycman[i].count);
				if (cmp_systim(gsCycman[i].cyctim, gsCycman[i].count)){	/* count over */
					gsCycman[i].cychdr(gsCycman[i].exinf);
					clr_systim(gsCycman[i].count);
				}
			}
			EREXIT(res, (pthread_mutex_unlock(&gsCycman[i].mutex) == 0)? E_OK : E_SYS, ERR_EXIT);
		}
	}
ERR_EXIT:
	return res;
}


/* Alarm Handler */
ER inz_alm(void)
{
	memset(&gsAlmman[0], 0, sizeof(gsAlmman));
	PCHK(pthread_mutex_init(&gsAlmmtx, NULL) == 0, E_SYS);
	return E_OK;
}


ER cre_alm(
	ID almid,
	T_CALM *pk_calm
	)
{
	ER res;
	
	PCHK(1 <= almid && almid <= TMAX_ALM_ID, E_ID);
	PCHK(pk_calm != NULL, E_PAR);
	PCHK(pk_calm->almatr & TA_HLNG, E_NOSPT);
	PCHK(pthread_mutex_lock(&gsAlmmtx) == 0, E_SYS);
	EREXIT(res, (gsAlmman[almid].created == FALSE)? E_OK : E_OBJ, ERR_EXIT);
    EREXIT(res, (pthread_mutex_init(&gsAlmman[almid].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
	gsAlmman[almid].created = TRUE;
	gsAlmman[almid].started = FALSE;
	gsAlmman[almid].almatr = pk_calm->almatr;
	gsAlmman[almid].exinf = pk_calm->exinf;
	gsAlmman[almid].almhdr = pk_calm->almhdr;
	gsAlmman[almid].lefttim.ltime = 0;
	gsAlmman[almid].lefttim.utime = 0;
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsAlmmtx) == 0, E_SYS);
	return res;
}


ER_ID acre_alm(
	T_CALM *pk_calm
	)
{
	ER_ID res, i;
	
	PCHK(pk_calm != NULL, E_PAR);
	PCHK(pk_calm->almatr & TA_HLNG, E_NOSPT);
	PCHK(pthread_mutex_lock(&gsAlmmtx) == 0, E_SYS);
	for (i = 1; i <= TMAX_ALM_ID; i++){
		if (gsAlmman[i].created == TRUE){
			continue;
		}
	    EREXIT(res, (pthread_mutex_init(&gsAlmman[i].mutex, NULL) == 0)? E_OK : E_SYS, ERR_EXIT);
		gsAlmman[i].created = TRUE;
		gsAlmman[i].started = FALSE;
		gsAlmman[i].almatr = pk_calm->almatr;
		gsAlmman[i].exinf = pk_calm->exinf;
		gsAlmman[i].almhdr = pk_calm->almhdr;
		gsAlmman[i].lefttim.ltime = 0;
		gsAlmman[i].lefttim.utime = 0;
		break;
	}
	res = (res > TMAX_ALM_ID)? E_NOID : i;
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsAlmmtx) == 0, E_SYS);
	return res;
}


ER del_alm(
	ID almid
	)
{
	ER res;
	
	PCHK(1 <= almid && almid <= TMAX_ALM_ID, E_ID);
	PCHK(pthread_mutex_lock(&gsAlmmtx) == 0, E_SYS);
	EREXIT(res, (pthread_mutex_lock(&gsAlmman[almid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT);
	EREXIT(res, (gsAlmman[almid].created == TRUE)? E_OK : E_NOEXS, ERR_EXIT);
	gsAlmman[almid].created = FALSE;
	EREXIT(res, (pthread_mutex_unlock(&gsAlmman[almid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT);
	EREXIT(res, (pthread_mutex_destroy(&gsAlmman[almid].mutex) == 0)? E_OK : E_SYS, ERR_EXIT);
	memset(&gsAlmman[almid], 0, sizeof(T_ALM_MAN));
ERR_EXIT:
	PCHK(pthread_mutex_unlock(&gsAlmmtx) == 0, E_SYS);
	return res;
}


ER sta_alm(
	ID almid,
	RELTIM almtim
	)
{
	ER res, err;
	
	PCHK(1 <= almid && almid <= TMAX_ALM_ID, E_ID);
	CHK_EXIST(res, err, gsAlmmtx, gsAlmman[almid]);
	gsAlmman[almid].started = TRUE;
	gsAlmman[almid].lefttim = almtim;
	err = pthread_mutex_unlock(&gsAlmman[almid].mutex);
	return res;
}


ER stp_alm(
	ID almid
	)
{
	ER res, err;
	
	PCHK(1 <= almid && almid <= TMAX_ALM_ID, E_ID);
	CHK_EXIST(res, err, gsAlmmtx, gsAlmman[almid]);
	gsAlmman[almid].started = FALSE;
	err = pthread_mutex_unlock(&gsAlmman[almid].mutex);
	return res;
}


ER ref_alm(
	ID almid,
	T_RALM *pk_ralm
	)
{
	ER res, err;
	
	PCHK(1 <= almid && almid <= TMAX_ALM_ID, E_ID);
	PCHK(pk_ralm != NULL, E_PAR);
	CHK_EXIST(res, err, gsAlmmtx, gsAlmman[almid]);
	pk_ralm->almstat = (gsAlmman[almid].started)? TALM_STA : TALM_STP;
	pk_ralm->lefttim = gsAlmman[almid].lefttim;
	err = pthread_mutex_unlock(&gsAlmman[almid].mutex);
	return res;
}

static ER isig_alm(void)
{
	INT i;
	ER res;
	
	for (i = 1; i <= TMAX_ALM_ID; i++){
		if (gsAlmman[i].created){
			EREXIT(res, (pthread_mutex_lock(&gsAlmman[i].mutex) == 0)? E_OK : E_SYS, ERR_EXIT);
			if (gsAlmman[i].started){
				dec_systim(gsAlmman[i].lefttim);
				if (IS_SYSTIM0(gsAlmman[i].lefttim)){	/* timeout */
					gsAlmman[i].almhdr(gsAlmman[i].exinf);
					gsAlmman[i].started = 0;
				}
			}
			EREXIT(res, (pthread_mutex_unlock(&gsAlmman[i].mutex) == 0)? E_OK : E_SYS, ERR_EXIT);
		}
	}
ERR_EXIT:
	return res;
}
