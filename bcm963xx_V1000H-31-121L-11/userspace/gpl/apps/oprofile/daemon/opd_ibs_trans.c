/**
 * @file daemon/opd_ibs_trans.c
 * AMD Family10h Instruction Based Sampling (IBS) translation.
 *
 * @remark Copyright 2008 OProfile authors
 * @remark Read the file COPYING
 *
 * @author Jason Yeh <jason.yeh@amd.com>
 * @author Paul Drongowski <paul.drongowski@amd.com>
 * @author Suravee Suthikulpanit <suravee.suthikulpanit@amd.com>
 * Copyright (c) 2008 Advanced Micro Devices, Inc.
 */

#include "opd_ibs.h"
#include "opd_ibs_macro.h"
#include "opd_ibs_trans.h"
#include "opd_trans.h"
#include "opd_printf.h"

#include <stdlib.h>
#include <stdio.h>

#define MAX_EVENTS_PER_GROUP	32

/*
 * --------------------- OP DERIVED FUNCTION
 */
void trans_ibs_fetch (struct transient * trans, unsigned int selected_flag, unsigned int size)
{
	struct ibs_fetch_sample * trans_fetch = ((struct ibs_sample*)(trans->ext))->fetch;
	unsigned int i, j, mask = 1;

	for (i = IBS_FETCH_BASE, j =0 ; i <= IBS_FETCH_END && j < size ; i++, mask = mask << 1) {

		if ((selected_flag & mask) == 0)
			continue;

		j++;

		switch (i) {

		case DE_IBS_FETCH_ALL:
			/* IBS all fetch samples (kills + attempts) */
			AGG_IBS_EVENT(DE_IBS_FETCH_ALL);
			break;

		case DE_IBS_FETCH_KILLED:
			/* IBS killed fetches ("case 0") -- All interesting event
			 * flags are clear */
			if (IBS_FETCH_KILLED(trans_fetch))
				AGG_IBS_EVENT(DE_IBS_FETCH_KILLED);
			break;

		case DE_IBS_FETCH_ATTEMPTED:
			/* Any non-killed fetch is an attempted fetch */
			AGG_IBS_EVENT(DE_IBS_FETCH_ATTEMPTED);
			break;

		case DE_IBS_FETCH_COMPLETED:
			if (IBS_FETCH_FETCH_COMPLETION(trans_fetch))
				/* IBS Fetch Completed */
				AGG_IBS_EVENT(DE_IBS_FETCH_COMPLETED);
			break;

		case DE_IBS_FETCH_ABORTED:
			if (!IBS_FETCH_FETCH_COMPLETION(trans_fetch))
				/* IBS Fetch Aborted */
				AGG_IBS_EVENT(DE_IBS_FETCH_ABORTED);
			break;

		case DE_IBS_L1_ITLB_HIT:
			/* IBS L1 ITLB hit */
			if (IBS_FETCH_L1_TLB_HIT(trans_fetch))
				AGG_IBS_EVENT(DE_IBS_L1_ITLB_HIT);
			break;

		case DE_IBS_ITLB_L1M_L2H:
			/* IBS L1 ITLB miss and L2 ITLB hit */
			if (IBS_FETCH_ITLB_L1M_L2H(trans_fetch))
				AGG_IBS_EVENT(DE_IBS_ITLB_L1M_L2H);
			break;

		case DE_IBS_ITLB_L1M_L2M:
			/* IBS L1 & L2 ITLB miss; complete ITLB miss */
			if (IBS_FETCH_ITLB_L1M_L2M(trans_fetch))
				AGG_IBS_EVENT(DE_IBS_ITLB_L1M_L2M);
			break;

		case DE_IBS_IC_MISS:
			/* IBS instruction cache miss */
			if (IBS_FETCH_INST_CACHE_MISS(trans_fetch))
				AGG_IBS_EVENT(DE_IBS_IC_MISS);
			break;

		case DE_IBS_IC_HIT:
			/* IBS instruction cache hit */
			if (IBS_FETCH_INST_CACHE_HIT(trans_fetch))
				AGG_IBS_EVENT(DE_IBS_IC_HIT);
			break;

		case DE_IBS_FETCH_4K_PAGE:
			if (IBS_FETCH_PHYS_ADDR_VALID(trans_fetch)
			    && IBS_FETCH_TLB_PAGE_SIZE(trans_fetch) ==  L1TLB4K)
				AGG_IBS_EVENT(DE_IBS_FETCH_4K_PAGE);
			break;

		case DE_IBS_FETCH_2M_PAGE:
			if (IBS_FETCH_PHYS_ADDR_VALID(trans_fetch)
			    && IBS_FETCH_TLB_PAGE_SIZE(trans_fetch) ==  L1TLB2M)
				AGG_IBS_EVENT(DE_IBS_FETCH_2M_PAGE);
			break;

		case DE_IBS_FETCH_1G_PAGE:
			if (IBS_FETCH_PHYS_ADDR_VALID(trans_fetch)
			    && IBS_FETCH_TLB_PAGE_SIZE(trans_fetch) ==  L1TLB1G)
				AGG_IBS_EVENT(DE_IBS_FETCH_1G_PAGE);
			break;

		case DE_IBS_FETCH_XX_PAGE:
			break;

		case DE_IBS_FETCH_LATENCY:
			if (IBS_FETCH_FETCH_LATENCY(trans_fetch))
				AGG_IBS_COUNT(DE_IBS_FETCH_LATENCY,
					      IBS_FETCH_FETCH_LATENCY(trans_fetch));
			break;
		default:
			break;
		}
	}
}

/*
 * --------------------- OP DERIVED FUNCTION
 */
void trans_ibs_op (struct transient * trans, unsigned int selected_flag, unsigned int size)
{
	struct ibs_op_sample * trans_op = ((struct ibs_sample*)(trans->ext))->op;
	unsigned int i, j, mask = 1;

	for (i = IBS_OP_BASE, j =0 ; i <= IBS_OP_END && j < size ; i++, mask = mask << 1) {

		if ((selected_flag & mask) == 0)
			continue;

		j++;

		switch (i) {

		case DE_IBS_OP_ALL:
			/* All IBS op samples */
			AGG_IBS_EVENT(DE_IBS_OP_ALL);
			break;

		case DE_IBS_OP_TAG_TO_RETIRE:
			/* Tally retire cycle counts for all sampled macro-ops
			 * IBS tag to retire cycles */
			if (IBS_OP_TAG_TO_RETIRE_CYCLES(trans_op))
				AGG_IBS_COUNT(DE_IBS_OP_TAG_TO_RETIRE,
					IBS_OP_TAG_TO_RETIRE_CYCLES(trans_op));
			break;

		case DE_IBS_OP_COMP_TO_RETIRE:
			/* IBS completion to retire cycles */
			if (IBS_OP_COM_TO_RETIRE_CYCLES(trans_op))
				AGG_IBS_COUNT(DE_IBS_OP_COMP_TO_RETIRE,
					IBS_OP_COM_TO_RETIRE_CYCLES(trans_op));
			break;

		case DE_IBS_BRANCH_RETIRED:
			if (IBS_OP_OP_BRANCH_RETIRED(trans_op))
				/* IBS Branch retired op */
				AGG_IBS_EVENT(DE_IBS_BRANCH_RETIRED) ;
			break;

		case DE_IBS_BRANCH_MISP:
			if (IBS_OP_OP_BRANCH_RETIRED(trans_op)
			    /* Test branch-specific event flags */
			    /* IBS mispredicted Branch op */
			    && IBS_OP_OP_BRANCH_MISPREDICT(trans_op))
				AGG_IBS_EVENT(DE_IBS_BRANCH_MISP) ;
			break;

		case DE_IBS_BRANCH_TAKEN:
			if (IBS_OP_OP_BRANCH_RETIRED(trans_op)
			    /* IBS taken Branch op */
			    && IBS_OP_OP_BRANCH_TAKEN(trans_op))
				AGG_IBS_EVENT(DE_IBS_BRANCH_TAKEN);
			break;

		case DE_IBS_BRANCH_MISP_TAKEN:
			if (IBS_OP_OP_BRANCH_RETIRED(trans_op)
			    /* IBS mispredicted taken branch op */
			    && IBS_OP_OP_BRANCH_TAKEN(trans_op)
			    && IBS_OP_OP_BRANCH_MISPREDICT(trans_op))
				AGG_IBS_EVENT(DE_IBS_BRANCH_MISP_TAKEN);
			break;

		case DE_IBS_RETURN:
			if (IBS_OP_OP_BRANCH_RETIRED(trans_op)
			    /* IBS return op */
			    && IBS_OP_OP_RETURN(trans_op))
				AGG_IBS_EVENT(DE_IBS_RETURN);
			break;

		case DE_IBS_RETURN_MISP:
			if (IBS_OP_OP_BRANCH_RETIRED(trans_op)
			    /* IBS mispredicted return op */
			    && IBS_OP_OP_RETURN(trans_op)
			    && IBS_OP_OP_BRANCH_MISPREDICT(trans_op))
				AGG_IBS_EVENT(DE_IBS_RETURN_MISP);
			break;

		case DE_IBS_RESYNC:
			/* Test for a resync macro-op */
			if (IBS_OP_OP_BRANCH_RESYNC(trans_op))
				AGG_IBS_EVENT(DE_IBS_RESYNC);
			break;
		default:
			break;
		}
	}
}


/*
 * --------------------- OP LS DERIVED FUNCTION
 */
void trans_ibs_op_ls (struct transient * trans, unsigned int selected_flag, unsigned int size)
{
	struct ibs_op_sample * trans_op = ((struct ibs_sample*)(trans->ext))->op;
	unsigned int i, j, mask = 1;

	/* Preliminary check */
	if (!IBS_OP_IBS_LD_OP(trans_op) && !IBS_OP_IBS_ST_OP(trans_op))
		return;


	for (i = IBS_OP_LS_BASE, j =0 ; i <= IBS_OP_LS_END && j < size ; i++, mask = mask << 1) {

		if ((selected_flag & mask) == 0)
			continue;

		j++;

		switch (i) {

		case DE_IBS_LS_ALL_OP:
			/* Count the number of LS op samples */
			AGG_IBS_EVENT(DE_IBS_LS_ALL_OP) ;
			break;

		case DE_IBS_LS_LOAD_OP:
			if (IBS_OP_IBS_LD_OP(trans_op))
				/* TALLy an IBS load derived event */
				AGG_IBS_EVENT(DE_IBS_LS_LOAD_OP) ;
			break;

		case DE_IBS_LS_STORE_OP:
			if (IBS_OP_IBS_ST_OP(trans_op))
				/* Count and handle store operations */
				AGG_IBS_EVENT(DE_IBS_LS_STORE_OP);
			break;

		case DE_IBS_LS_DTLB_L1H:
			if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
			    && !IBS_OP_IBS_DC_L1_TLB_MISS(trans_op))
				/* L1 DTLB hit -- This is the most frequent case */
				AGG_IBS_EVENT(DE_IBS_LS_DTLB_L1H);
			break;

		case DE_IBS_LS_DTLB_L1M_L2H:
			/* l2_translation_size = 1 */
			if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
			    && IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)
			    && !IBS_OP_IBS_DC_L2_TLB_MISS(trans_op))
				/* L1 DTLB miss, L2 DTLB hit */
				AGG_IBS_EVENT(DE_IBS_LS_DTLB_L1M_L2H);
			break;

		case DE_IBS_LS_DTLB_L1M_L2M:
			if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
			    && IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)
			    && IBS_OP_IBS_DC_L2_TLB_MISS(trans_op))
				/* L1 DTLB miss, L2 DTLB miss */
				AGG_IBS_EVENT(DE_IBS_LS_DTLB_L1M_L2M);
			break;

		case DE_IBS_LS_DC_MISS:
			if (IBS_OP_IBS_DC_MISS(trans_op))
				AGG_IBS_EVENT(DE_IBS_LS_DC_MISS);
			break;

		case DE_IBS_LS_DC_HIT:
			if (!IBS_OP_IBS_DC_MISS(trans_op))
				AGG_IBS_EVENT(DE_IBS_LS_DC_HIT);
			break;

		case DE_IBS_LS_MISALIGNED:
			if (IBS_OP_IBS_DC_MISS_ACC(trans_op))
				AGG_IBS_EVENT(DE_IBS_LS_MISALIGNED);
			break;

		case DE_IBS_LS_BNK_CONF_LOAD:
			if (IBS_OP_IBS_DC_LD_BNK_CON(trans_op))
				AGG_IBS_EVENT(DE_IBS_LS_BNK_CONF_LOAD);
			break;

		case DE_IBS_LS_BNK_CONF_STORE:
			if (IBS_OP_IBS_DC_ST_BNK_CON(trans_op))
				AGG_IBS_EVENT(DE_IBS_LS_BNK_CONF_STORE);
			break;

		case DE_IBS_LS_STL_FORWARDED:
			if (IBS_OP_IBS_LD_OP(trans_op)
			    /* Data forwarding info are valid only for load ops */
			    && IBS_OP_IBS_DC_ST_TO_LD_FWD(trans_op))
				AGG_IBS_EVENT(DE_IBS_LS_STL_FORWARDED) ;
			break;

		case DE_IBS_LS_STL_CANCELLED:
			if (IBS_OP_IBS_LD_OP(trans_op))
			if (IBS_OP_IBS_DC_ST_TO_LD_CAN(trans_op))
				AGG_IBS_EVENT(DE_IBS_LS_STL_CANCELLED) ;
			break;

		case DE_IBS_LS_UC_MEM_ACCESS:
			if (IBS_OP_IBS_DC_UC_MEM_ACC(trans_op))
				AGG_IBS_EVENT(DE_IBS_LS_UC_MEM_ACCESS);
			break;

		case DE_IBS_LS_WC_MEM_ACCESS:
			if (IBS_OP_IBS_DC_WC_MEM_ACC(trans_op))
				AGG_IBS_EVENT(DE_IBS_LS_WC_MEM_ACCESS);
			break;

		case DE_IBS_LS_LOCKED_OP:
			if (IBS_OP_IBS_LOCKED_OP(trans_op))
				AGG_IBS_EVENT(DE_IBS_LS_LOCKED_OP);
			break;

		case DE_IBS_LS_MAB_HIT:
			if (IBS_OP_IBS_DC_MAB_HIT(trans_op))
				AGG_IBS_EVENT(DE_IBS_LS_MAB_HIT);
			break;

		case DE_IBS_LS_L1_DTLB_4K:
			/* l1_translation */
			if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
			    && !IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)

			    && !IBS_OP_IBS_DC_L1_TLB_HIT_2MB(trans_op)
			    && !IBS_OP_IBS_DC_L1_TLB_HIT_1GB(trans_op))
				/* This is the most common case, unfortunately */
				AGG_IBS_EVENT(DE_IBS_LS_L1_DTLB_4K) ;
			break;

		case DE_IBS_LS_L1_DTLB_2M:
			/* l1_translation */
			if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
			    && !IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)

			    && IBS_OP_IBS_DC_L1_TLB_HIT_2MB(trans_op))
				/* 2M L1 DTLB page translation */
				AGG_IBS_EVENT(DE_IBS_LS_L1_DTLB_2M);
			break;

		case DE_IBS_LS_L1_DTLB_1G:
			/* l1_translation */
			if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
			    && !IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)

			    && !IBS_OP_IBS_DC_L1_TLB_HIT_2MB(trans_op)
			    && IBS_OP_IBS_DC_L1_TLB_HIT_1GB(trans_op))
				/* 1G L1 DTLB page translation */
				AGG_IBS_EVENT(DE_IBS_LS_L1_DTLB_1G);
			break;

		case DE_IBS_LS_L1_DTLB_RES:
			break;

		case DE_IBS_LS_L2_DTLB_4K:
			/* l2_translation_size = 1 */
			if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
			    && IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)
			    && !IBS_OP_IBS_DC_L2_TLB_MISS(trans_op)

			    /* L2 DTLB page translation */
			    && !IBS_OP_IBS_DC_L2_TLB_HIT_2MB(trans_op)
			    && !IBS_OP_IBS_DC_L2_TLB_HIT_1GB(trans_op))
				/* 4K L2 DTLB page translation */
				AGG_IBS_EVENT(DE_IBS_LS_L2_DTLB_4K);
			break;

		case DE_IBS_LS_L2_DTLB_2M:
			/* l2_translation_size = 1 */
			if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
			    && IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)
			    && !IBS_OP_IBS_DC_L2_TLB_MISS(trans_op)

			    /* L2 DTLB page translation */
			    && IBS_OP_IBS_DC_L2_TLB_HIT_2MB(trans_op)
			    && !IBS_OP_IBS_DC_L2_TLB_HIT_1GB(trans_op))
				/* 2M L2 DTLB page translation */
				AGG_IBS_EVENT(DE_IBS_LS_L2_DTLB_2M);
			break;

		case DE_IBS_LS_L2_DTLB_1G:
			/* l2_translation_size = 1 */
			if (IBS_OP_IBS_DC_LIN_ADDR_VALID(trans_op)
			    && IBS_OP_IBS_DC_L1_TLB_MISS(trans_op)
			    && !IBS_OP_IBS_DC_L2_TLB_MISS(trans_op)

			    /* L2 DTLB page translation */
			    && !IBS_OP_IBS_DC_L2_TLB_HIT_2MB(trans_op)
			    && IBS_OP_IBS_DC_L2_TLB_HIT_1GB(trans_op))
				/* 2M L2 DTLB page translation */
				AGG_IBS_EVENT(DE_IBS_LS_L2_DTLB_1G);
			break;

		case DE_IBS_LS_L2_DTLB_RES2:
			break;

		case DE_IBS_LS_DC_LOAD_LAT:
			if (IBS_OP_IBS_LD_OP(trans_op)
			    /* If the load missed in DC, tally the DC load miss latency */
			    && IBS_OP_IBS_DC_MISS(trans_op))
				/* DC load miss latency is only reliable for load ops */
				AGG_IBS_COUNT(DE_IBS_LS_DC_LOAD_LAT,
					      IBS_OP_DC_MISS_LATENCY(trans_op)) ;
			break;

		default:
			break;
		}
	}
}

/*
 * --------------------- OP NB DERIVED FUNCTION
 *
 * NB data is only guaranteed reliable for load operations
 * that miss in L1 and L2 cache. NB data arrives too late
 * to be reliable for store operations
 */
void trans_ibs_op_nb (struct transient * trans, unsigned int selected_flag, unsigned int size)
{
	struct ibs_op_sample * trans_op = ((struct ibs_sample*)(trans->ext))->op;
	unsigned int i, j, mask = 1;

	/* Preliminary check */
	if (!IBS_OP_IBS_LD_OP(trans_op))
		return;

	if (!IBS_OP_IBS_DC_MISS(trans_op))
		return;

	if (IBS_OP_NB_IBS_REQ_SRC(trans_op) == 0)
		return;

	for (i = IBS_OP_NB_BASE, j =0 ; i <= IBS_OP_NB_END && j < size ; i++, mask = mask << 1) {

		if ((selected_flag & mask) == 0)
			continue;

		j++;

		switch (i) {

		case DE_IBS_NB_LOCAL:
			if (!IBS_OP_NB_IBS_REQ_DST_PROC(trans_op))
				/* Request was serviced by local processor */
				AGG_IBS_EVENT(DE_IBS_NB_LOCAL) ;
			break;

		case DE_IBS_NB_REMOTE:
			if (IBS_OP_NB_IBS_REQ_DST_PROC(trans_op))
				/* Request was serviced by remote processor */
				AGG_IBS_EVENT(DE_IBS_NB_REMOTE) ;
			break;

		case DE_IBS_NB_LOCAL_L3:
			if (!IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
			    && (IBS_OP_NB_IBS_REQ_SRC(trans_op) == 0x1))
				AGG_IBS_EVENT(DE_IBS_NB_LOCAL_L3);
			break;

		case DE_IBS_NB_LOCAL_CACHE:
			if (!IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
			    && (IBS_OP_NB_IBS_REQ_SRC(trans_op) == 0x2))
				AGG_IBS_EVENT(DE_IBS_NB_LOCAL_CACHE);
			break;

		case DE_IBS_NB_REMOTE_CACHE:
			if (IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
			    && (IBS_OP_NB_IBS_REQ_SRC(trans_op) == 0x2))
				AGG_IBS_EVENT(DE_IBS_NB_REMOTE_CACHE) ;
			break;

		case DE_IBS_NB_LOCAL_DRAM:
			if (!IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
			    && (IBS_OP_NB_IBS_REQ_SRC(trans_op) == 0x3))
				AGG_IBS_EVENT(DE_IBS_NB_LOCAL_DRAM);
			break;

		case DE_IBS_NB_REMOTE_DRAM:
			if (IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
			    && (IBS_OP_NB_IBS_REQ_SRC(trans_op) == 0x3))
				AGG_IBS_EVENT(DE_IBS_NB_REMOTE_DRAM) ;
			break;

		case DE_IBS_NB_LOCAL_OTHER:
			if (!IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
			    && (IBS_OP_NB_IBS_REQ_SRC(trans_op) == 0x7))
				AGG_IBS_EVENT(DE_IBS_NB_LOCAL_OTHER);
			break;

		case DE_IBS_NB_REMOTE_OTHER:
			if (IBS_OP_NB_IBS_REQ_DST_PROC(trans_op)
			    && (IBS_OP_NB_IBS_REQ_SRC(trans_op) == 0x7))
				AGG_IBS_EVENT(DE_IBS_NB_REMOTE_OTHER) ;
			break;

		case DE_IBS_NB_CACHE_STATE_M:
			if ((IBS_OP_NB_IBS_REQ_SRC(trans_op) == 0x2)
			    && !IBS_OP_NB_IBS_CACHE_HIT_ST(trans_op))
				AGG_IBS_EVENT(DE_IBS_NB_CACHE_STATE_M) ;
			break;

		case DE_IBS_NB_CACHE_STATE_O:
			if ((IBS_OP_NB_IBS_REQ_SRC(trans_op) == 0x2)
			    && IBS_OP_NB_IBS_CACHE_HIT_ST(trans_op))
				AGG_IBS_EVENT(DE_IBS_NB_CACHE_STATE_O) ;
			break;

		case DE_IBS_NB_LOCAL_LATENCY:
			if (!IBS_OP_NB_IBS_REQ_DST_PROC(trans_op))
				/* Request was serviced by local processor */
				AGG_IBS_COUNT(DE_IBS_NB_LOCAL_LATENCY,
					      IBS_OP_DC_MISS_LATENCY(trans_op));
			break;

		case DE_IBS_NB_REMOTE_LATENCY:
			if (IBS_OP_NB_IBS_REQ_DST_PROC(trans_op))
				/* Request was serviced by remote processor */
				AGG_IBS_COUNT(DE_IBS_NB_REMOTE_LATENCY,
					      IBS_OP_DC_MISS_LATENCY(trans_op));
			break;

		default:
			break;
		}
	}
}
