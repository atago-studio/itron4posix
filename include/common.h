/* kernel common internal define */

/***** Constants *****/
typedef enum {							/* OBJECT CODE for object wait queue	*/
	TCD_OBJ_SEM,						/* semaphore							*/
	TCD_OBJ_FLG,						/* event flag							*/
	TCD_OBJ_DTQS,						/* data queue send wait					*/
	TCD_OBJ_DTQR,						/* data queue receive wait				*/
	TCD_OBJ_MBX,						/* message box receive wait				*/
	TCD_OBJ_MTX,						/* mutex								*/
	TCD_OBJ_MBFS,						/* message buffer send wait				*/
	TCD_OBJ_MBFR,						/* message buffer receive wait			*/
	TCD_OBJ_PORC,						/* rendezvous call wait					*/
	TCD_OBJ_PORA,						/* rendezvous accept wait				*/
	TCD_OBJ_MPF,						/* fixed length memory pool				*/
	TCD_OBJ_MPL,						/* valiable length memory pool			*/
	TCD_OBJ_END							/* for renge check						*/
} TCD_OBJ;

/***** Tables *****/
#ifdef _COMMON_DEF_
const UW BIT[32] = {					/* bit location							*/
	0x00000001, 0x00000002, 0x00000004, 0x00000008, 
	0x00000010, 0x00000020, 0x00000040, 0x00000080,
	0x00000100, 0x00000200, 0x00000400, 0x00000800,
	0x00001000, 0x00002000, 0x00004000, 0x00008000,
	0x00010000, 0x00020000, 0x00040000, 0x00080000,
	0x00100000, 0x00200000, 0x00400000, 0x00800000,
	0x01000000, 0x02000000, 0x04000000, 0x08000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000
	};
#else
extern const UW BIT[32];
#endif

/***** Macros *****/
#define PCHK(cond, ret)	if (!(cond)) return ret
#define EREXIT(res, cond, dest) if ((res = cond) != E_OK) goto dest
#define ERRET(res, func) if ((res = func) != E_OK) return res;
#define CHK_EXIST(res, err, all_mtx, mng) \
	res = pthread_mutex_lock(&all_mtx); \
	if (res == 0){ \
		if (mng.created){ \
			res = (pthread_mutex_lock(&mng.mutex))? E_SYS : E_OK; \
		} else { \
			res = E_NOEXS; \
		} \
		err = pthread_mutex_unlock(&all_mtx); \
		res = (res)? res : err; \
		if (res) return res; \
	}

/***** Funcs *****/
ER chg_nowpri(
	ID tskid,
	PRI tskpri
	);

/* Object Wait Func */
ER sys_wai_tsk(
	TCD_OBJ objcd,
	ID objid,
    TMO tmout,
    W fn,
    pthread_mutex_t *mutex
    );
ER sys_wup_tsk(
    ID tskid,
	FN fn,
	UW tskst
    );

/* task depend mutex operation */
ER tsk_loc_mtx(
	ID tskid,
	ID mtxid,
	PRI maxpri
	);
ER tsk_unl_mtx(
	ID tskid,
	ID mtxid
	);

/* Object Wait Queue */
ER inz_oqe(void);
ER cre_oqe(
	TCD_OBJ objcd,
	ID objid,
	ATR objatr
	);
ER del_oqe(
	TCD_OBJ objcd,
	ID objid
	);
ER enq_oqe(
	TCD_OBJ objcd,
	ID objid,
	ID tskid,
	VP data
	);
ER deq_oqe(
	TCD_OBJ objcd,
	ID objid,
	ID *p_tskid,				/* if return value is TSK_NONE then queue empty */
	VP *pp_data					/* if NULL then ignore this param */
	);
ER ref_oqe(
	TCD_OBJ objcd,
	ID objid,
	INT offset,
	ID *p_tskid,
	VP *pp_data					/* if NULL then ignore this param */
	);
ER get_oqe(						/* get mideum position data from queue */
	TCD_OBJ objcd,
	ID objid,
	INT offset,
	ID *p_tskid,
	VP *pp_data					/* if NULL then ignore this param */
	);
ER srh_oqe(						/* search waiting task */
	TCD_OBJ objcd,
	ID objid,
	INT *offset,				/* found position */
	ID tskid,					/* search task id (if TSK_SELF then ignore this param) */
	VP pk_data					/* search data pattern (if pk_tskid isn't TSK_SELF then ignore this param) */
	);
ER rmv_oqe(						/* search & remove waiting task */
	TCD_OBJ objcd,
	ID objid,
	ID tskid,					/* search task id (if TSK_SELF then ignore this param) */
	VP pk_data					/* search data pattern (if pk_tskid isn't TSK_SELF then ignore this param) */
	);
ER wai_oqe(
	TCD_OBJ objcd,
	ID objid,
	ID tskid,
	VP data,
	TMO tmout,
	FN fn,
    pthread_mutex_t *pk_mutex	
	);
