/*
 * Copyright (c) 2013, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Intel Corporation nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _NUMATOP_PERF_H
#define	_NUMATOP_PERF_H

#include <sys/types.h>
#include <inttypes.h>
#include <pthread.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SMPL_PERIOD_INFINTE				0xFFFFFFFFFFFFFFULL
#define SMPL_PERIOD_DEFAULT_CORE_CLK	SMPL_PERIOD_INFINTE
#define SMPL_PERIOD_DEFAULT_RMA			10000
#define SMPL_PERIOD_DEFAULT_LMA			10000
#define SMPL_PERIOD_DEFAULT_CLK			10000000
#define SMPL_PERIOD_DEFAULT_IR			10000000
#define SMPL_PERIOD_MIN_CORE_CLK		SMPL_PERIOD_INFINTE
#define SMPL_PERIOD_MIN_RMA				5000
#define SMPL_PERIOD_MIN_LMA				5000
#define SMPL_PERIOD_MIN_CLK				1000000
#define SMPL_PERIOD_MIN_IR				1000000
#define SMPL_PERIOD_MAX_CORE_CLK		SMPL_PERIOD_INFINTE
#define SMPL_PERIOD_MAX_RMA				100000
#define SMPL_PERIOD_MAX_LMA				100000
#define SMPL_PERIOD_MAX_CLK				100000000
#define SMPL_PERIOD_MAX_IR				100000000

#define PERF_FD_NUM		NCPUS_MAX * COUNT_NUM
#define PERF_WAIT_NSEC	5
#define PERF_INTVAL_MIN_MS	1000
#define PERF_REC_NUM	512

typedef enum {
	PERF_STATUS_IDLE = 0,
	PERF_STATUS_PROFILING_STARTED,
	PERF_STATUS_PROFILING_PART_STARTED,
	PERF_STATUS_PROFILING_FAILED,
	PERF_STATUS_LL_STARTED,
	PERF_STATUS_LL_FAILED,
} perf_status_t;

typedef enum {
	PERF_INVALID_ID = 0,
	PERF_PROFILING_START_ID,
	PERF_PROFILING_SMPL_ID,
	PERF_PROFILING_PARTPAUSE_ID,
	PERF_PROFILING_RESTORE_ID,
	PERF_LL_START_ID,
	PERF_LL_SMPL_ID,
	PERF_STOP_ID,
	PERF_QUIT_ID
} perf_taskid_t;

typedef struct _task_quit {
	perf_taskid_t task_id;
} task_quit_t;

typedef struct _task_allstop {
	perf_taskid_t task_id;
} task_allstop_t;

typedef struct _task_profiling {
	perf_taskid_t task_id;
} task_profiling_t;

typedef struct _task_partpause {
	perf_taskid_t task_id;
	count_id_t keeprun_id;
} task_partpause_t;

typedef struct _task_restore {
	perf_taskid_t task_id;
	count_id_t keeprun_id;
} task_restore_t;

typedef struct _task_ll {
	perf_taskid_t task_id;
	pid_t pid;
	int lwpid;
} task_ll_t;

typedef union _perf_task {
	task_quit_t quit;
	task_allstop_t allstop;
	task_profiling_t profiling;
	task_partpause_t partpause;
	task_restore_t restore;
	task_ll_t ll;
} perf_task_t;

typedef struct _perf_callchain {
	unsigned int ip_num;
	uint64_t ips[IP_NUM];
} perf_callchain_t;

typedef struct _perf_llrec {
	uint64_t addr;
	uint64_t cpu;
	uint64_t latency;
	perf_callchain_t callchain;
} perf_llrec_t;

typedef struct _perf_llrecgrp {
	perf_llrec_t *rec_arr;
	int nrec_cur;
	int nrec_max;
	int cursor;
} perf_llrecgrp_t;

typedef struct _perf_chainrec {
	uint64_t count_value;
	perf_callchain_t callchain;	
} perf_chainrec_t;

typedef struct _perf_chainrecgrp {
	perf_chainrec_t *rec_arr;
	int nrec_cur;
	int nrec_max;
	int cursor;
} perf_chainrecgrp_t;

typedef struct _perf_countchain {
	perf_chainrecgrp_t chaingrps[COUNT_NUM];
} perf_countchain_t;

#define	TASKID(task_addr) \
	(*(perf_taskid_t *)(task_addr))

#define	TASKID_SET(task_addr, task_id) \
	((*(perf_taskid_t *)(task_addr)) = (task_id))

#define PERF_PROFILING_STARTED \
	(s_perf_ctl.status == PERF_STATUS_PROFILING_STARTED)

typedef struct _perf_ctl {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_mutex_t status_mutex;
	pthread_cond_t status_cond;	
	perf_status_t status;
	pthread_t thr;
	perf_task_t task;
	boolean_t inited;
	uint64_t last_ms;
} perf_ctl_t;

typedef struct _perf_cpu {
	int cpuid;
	int fds[COUNT_NUM];
	int group_idx;
	int map_len;
	int map_mask;
	void *map_base;
	boolean_t hit;
	boolean_t hotadd;
	boolean_t hotremove;
	count_value_t countval_last;
} perf_cpu_t;

typedef int (*pfn_perf_cpu_op_t)(struct _perf_cpu *, void *);

extern precise_type_t g_precise;

extern int perf_init(void);
extern void perf_fini(void);
extern int perf_allstop(void);
extern boolean_t perf_profiling_started(void);
extern int perf_profiling_start(void);
extern int perf_profiling_smpl(void);
extern int perf_profiling_partpause(count_id_t);
extern int perf_profiling_restore(count_id_t);
extern boolean_t perf_ll_started(void);
extern int perf_ll_start(void);
extern int perf_ll_smpl(pid_t, int);
extern void perf_llrecgrp_reset(perf_llrecgrp_t *);
extern void perf_cpuarr_init(perf_cpu_t *, int, boolean_t);
extern void perf_cpuarr_fini(perf_cpu_t *, int, boolean_t);
extern int perf_cpuarr_refresh(perf_cpu_t *, int, int *, int, boolean_t);
extern void perf_countchain_reset(perf_countchain_t *);

#ifdef __cplusplus
}
#endif

#endif /* _NUMATOP_PERF_H */
