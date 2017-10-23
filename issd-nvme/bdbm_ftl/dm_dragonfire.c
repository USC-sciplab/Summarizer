#include "bdbm_drv.h"
#include "params.h"
#include "dm_dragonfire.h"
#include "dev_dragonfire.h"
#include "../dma_df_nand.h"
#include "platform.h"

#if (KM_TPCH_QUERY6 || KM_TPCH_QUERY6_CMP_ALL || KM_TPCH_QUERY14 || KM_TPCH_QUERY14_CMP_ALL ||  KM_TPCH_QUERY1 || KM_TPCH_QUERY1_CMP_ALL || TE_AFC_STICKY || HV_PREFIX_SIMILARITY || HV_PREFIX_SIMILARITY_CMP_ALL)
#include "string.h"
#include "stdbool.h"
#endif // (KM_TPCH_QUERY6 || KM_TPCH_QUERY6_CMP_ALL || KM_TPCH_QUERY14 || KM_TPCH_QUERY14_CMP_ALL ||  KM_TPCH_QUERY1 || KM_TPCH_QUERY1_CMP_ALL || TE_AFC_STICKY)

#if (GK_TEST_RD_NAND)
#define HOST_OUTBOUND_ADDR  0x1400000000        // Outbound iATU Address, from common.c
extern unsigned long long int gc_phy_addr[70];
extern uint8_t *ls2_virt_addr[5];
extern uint8_t *gk_virt_addr[70];
#endif	// GK_TEST_RD_NAND

struct bdbm_dm_inf_t _dm_dragonfire_inf = {
	.ptr_private = NULL,
	.probe = dm_df_probe,
	.open = dm_df_open,
	.close = dm_df_close,
	.make_req = dm_df_make_req,
	.end_req = dm_df_end_req,
	.load = dm_df_load,
	.store = dm_df_store,
};

bdbm_dev_private_t _dp;
extern struct bdbm_drv_info* _bdi;

inline memcpy_split( uint64_t dst, uint8_t* src, unsigned int sz, unsigned int n_split )
{
	unsigned int i;
	uint64_t prp_split;
	uint8_t *lmem_virt_split;
	for (i = 0; i < n_split; i++) {
		prp_split = dst + (sz / n_split);
		lmem_virt_split = src + (sz / n_split);
	#if (GK_DMA_DELAY)
		usleep(GK_DMA_DELAY);
	#endif
		memcpy( (void*)prp_split, (void*)lmem_virt_split, (sz / n_split) );
	}
}

#if (GK_MEASURE_TIME)
//inline void measure_ls2_exec_clk(clock_t clks, NvmeCtrl* n)
//{
//	n->ls2_acc_cnt += 1;
//	n->ls2_acc_clks += clks;
//	
//	if ((n->ls2_acc_cnt > 0) && (n->ls2_acc_cnt % 1000==0)) {
//		double sec_per_acc = (double)(n->ls2_acc_clks) / n->ls2_acc_cnt / CLOCKS_PER_SEC;
//		GK_PRINT("GK_LS2_CLKS: %ld / %lld, exec_time = %lf \n", n->ls2_acc_clks, n->ls2_acc_cnt, sec_per_acc);
//		n->ls2_acc_cnt = 0;
//		n->ls2_acc_clks = 0;
//	}
//}
//
//inline void measure_ls2_exec_time(struct timespec start, struct timespec end, NvmeCtrl* n)
//{
//	double time;
//	time = ((double)end.tv_sec - (double)start.tv_sec);
//	time += ((double)end.tv_nsec - (double)start.tv_nsec) / 1000000000.0;
//
//	n->ls2_acc_cnt += 1;
//	n->ls2_acc_time += time;
//	if ((n->ls2_acc_cnt > 0) && (n->ls2_acc_cnt % 1000==0)) {
//		double sec_per_acc = (double)(n->ls2_acc_time) / n->ls2_acc_cnt;
//		GK_PRINT("GK_LS2_TIME: %lf / %lld, exec_time = %lf \n", n->ls2_acc_time, n->ls2_acc_cnt, sec_per_acc);
//		// bw
//		time = ((double)end.tv_sec - (double)n->ls2_bw_init_time.tv_sec);
//		time += ((double)end.tv_nsec - (double)n->ls2_bw_init_time.tv_nsec) / 1000000000.0;
//		GK_PRINT("GK_LS2_BW: %lld / %lf, BW = %lf B/s \n", KERNEL_PAGE_SIZE*1000, time, (double)KERNEL_PAGE_SIZE*1000.0/time);
//		//
//		n->ls2_acc_cnt = 0;
//		n->ls2_acc_time = 0.0;
//		clock_gettime(CLOCK_MONOTONIC, &(n->ls2_bw_init_time));
//	}
//}
//
//inline void measure_func_exec_clk(clock_t clks, NvmeCtrl* n)
//{
//	n->func_acc_cnt += 1;
//	n->func_acc_clks += clks;
//	
//	if ((n->func_acc_cnt > 0) && (n->func_acc_cnt % 1000==0)) {
//		double sec_per_acc = (double)(n->func_acc_clks) / n->func_acc_cnt / CLOCKS_PER_SEC;
//		GK_PRINT("GK_FUNC_CLKS: %ld / %lld, exec_time = %lf \n", n->func_acc_clks, n->func_acc_cnt, sec_per_acc);
//		n->func_acc_cnt = 0;
//		n->func_acc_clks = 0;
//	}
//}
//
//inline void measure_func_exec_time(struct timespec start, struct timespec end, NvmeCtrl* n)
//{
//	double time;
//	time = ((double)end.tv_sec - (double)start.tv_sec);
//	time += ((double)end.tv_nsec - (double)start.tv_nsec) / 1000000000.0;
//
//	n->func_acc_cnt += 1;
//	n->func_acc_time += time;
//	if ((n->func_acc_cnt > 0) && (n->func_acc_cnt % 1000==0)) {
//		double sec_per_acc = (double)(n->func_acc_time) / n->func_acc_cnt;
//		GK_PRINT("GK_FUNC_TIME: %lf / %lld, exec_time = %lf \n", n->func_acc_time, n->func_acc_cnt, sec_per_acc);
//		n->func_acc_cnt = 0;
//		n->func_acc_time = 0.0;
//	}
//}
#endif	// GK_MEASURE_TIME

#if (GK_TEST_RD_NAND==4)
// send the special code noticing SSD is working for this page
void dm_df_completion_cb_read_proc(void *req_ptr, uint8_t ret, uint32_t desc_idx, NvmeCtrl* n)
{
	struct bdbm_llm_req_t* ptr_llm_req = (struct bdbm_llm_req_t*)req_ptr;

	uint8_t *lmem_virt;
	lmem_virt = (uint8_t*)malloc( 4*sizeof(uint8_t) );
	memset(lmem_virt, 0xFF, 4);
	//lmem_virt = ls2_virt_addr[2] + getpagesize()*desc_idx;
	
	uint8_t *prp;
	prp = ptr_llm_req->pptr_kpgs[0];
	
	uint64_t prp1;
	prp1 = (uint64_t)prp;
	prp1 = prp1 - HOST_OUTBOUND_ADDR;
	prp1 = prp1 + n->host.io_mem.addr;

#if (GK_MEASURE_TIME)
	set_start_time( &(n->et_ls2_ctrl) );
#endif	// GK_MEASURE_TIME
	
	memcpy((void*)prp1, (void*)lmem_virt, 4);

#if (GK_MEASURE_TIME)
	add_exec_time( &(n->et_ls2_ctrl) );
	if ( n->et_ls2_ctrl.iter==1000 ) {
		print_exec_time( &(n->et_ls2_ctrl) );
		print_exec_bw( &(n->et_ls2_ctrl) );
		init_time( &(n->et_ls2_ctrl) );
	}
#endif	// GK_MEASURE_TIME

	free(lmem_virt);
	ptr_llm_req->ret = ret;
	_bdi->ptr_dm_inf->end_req (_bdi, ptr_llm_req);
}

void dm_df_completion_cb_read_memcpy(void *req_ptr, uint8_t ret, uint32_t desc_idx, NvmeCtrl* n)
{
	struct bdbm_llm_req_t* ptr_llm_req = (struct bdbm_llm_req_t*)req_ptr;

	uint8_t *lmem_virt;
	lmem_virt = ls2_virt_addr[2] + getpagesize()*desc_idx;
	
	uint8_t *prp;
	prp = ptr_llm_req->pptr_kpgs[0];
	
	uint64_t prp1;
	prp1 = (uint64_t)prp;
	prp1 = prp1 - HOST_OUTBOUND_ADDR;
	prp1 = prp1 + n->host.io_mem.addr;

#if (GK_MEASURE_TIME)
	set_start_time( &(n->et_ls2_data) );
#endif	// GK_MEASURE_TIME

#if (GK_SPLIT_HOST_TX)
	memcpy_split(prp1, lmem_virt, KERNEL_PAGE_SIZE, GK_SPLIT_HOST_TX);
#else
 #if (GK_DMA_DELAY)
	usleep(GK_DMA_DELAY);
 #endif	// GK_DMA_DELAY
	memcpy((void*)prp1, (void*)lmem_virt, KERNEL_PAGE_SIZE);	// it is stored in nand_param as subpage_size, equivalent to KERNEL_PAGE_SIZE
#endif	// GK_SPLIT_HOST_TX

#if (GK_MEASURE_TIME)
	add_exec_time( &(n->et_ls2_data) );
	if ( n->et_ls2_data.iter==1000 ) {
		print_exec_time( &(n->et_ls2_data) );
		print_exec_bw( &(n->et_ls2_data) );
		init_time( &(n->et_ls2_data) );
	}
#endif	// GK_MEASURE_TIME

	ptr_llm_req->ret = ret;
	_bdi->ptr_dm_inf->end_req (_bdi, ptr_llm_req);
}

void dm_df_inproc_wrapper(uint32_t desc_idx, uint32_t req_type, NvmeCtrl *n)
{
	uint8_t *lmem_virt;
	lmem_virt = ls2_virt_addr[2] + getpagesize()*desc_idx;

#if (GK_MEASURE_TIME)
	set_start_time( &(n->et_proc) );
#endif	// GK_MEASURE_TIME

#if (KM_TPCH_QUERY6)
	if(req_type==REQTYPE_TPCH_QUERY6_READ) {
	#if (GK_PROC_DELAY_QUERY6)
		usleep(GK_PROC_DELAY_QUERY6);
	#endif
		dm_df_read_tpch_query6_read(lmem_virt, n);
	}
#endif	// KM_TPCH_QUERY6
#if (KM_TPCH_QUERY6_CMP_ALL)
	if(req_type==REQTYPE_TPCH_QUERY6_CMP_ALL) {
	#if (GK_PROC_DELAY_QUERY6)
		usleep(GK_PROC_DELAY_QUERY6);
	#endif
		dm_df_read_tpch_query6_cpm_all_read(lmem_virt, n);
	}
#endif // KM_TPCH_QUERY6_CMP_ALL


#if (HV_PREFIX_SIMILARITY)
    if(req_type==REQTYPE_PREFIX_SIMILARITY_READ){
    #if(GK_PROC_DELAY_JOIN)
        usleep(GK_PROC_DELAY_JOIN);
    #endif
		dm_df_read_prefix_similarity_read(lmem_virt);
    }
#endif
#if (HV_PREFIX_SIMILARITY_CMP_ALL)
    if(req_type==REQTYPE_PREFIX_SIMILARITY_CMP_ALL){
    #if(GK_PROC_DELAY_JOIN)
        usleep(GK_PROC_DELAY_JOIN);
    #endif
		dm_df_read_prefix_similarity_cmp_all_read(lmem_virt);
    }
#endif


#if (KM_TPCH_QUERY1)
	if(req_type==REQTYPE_TPCH_QUERY1_READ)
	{
	#if (GK_PROC_DELAY_QUERY1)
		usleep(GK_PROC_DELAY_QUERY1);
	#endif
		dm_df_read_tpch_query1_read(lmem_virt);
	}
#endif	// KM_TPCH_QUERY1
#if (KM_TPCH_QUERY1_CMP_ALL)
	if(req_type==REQTYPE_TPCH_QUERY1_CMP_ALL)
	{
	#if (GK_PROC_DELAY_QUERY1)
		usleep(GK_PROC_DELAY_QUERY1);
	#endif
		dm_df_read_tpch_query1_cmp_all_read(lmem_virt);
	}
#endif // KM_TPCH_QUERY1_CMP_ALL

#if (KM_TPCH_QUERY14)
	if(req_type==REQTYPE_TPCH_QUERY14_READ1 || req_type==REQTYPE_TPCH_QUERY14_READ2 )
	{
	#if (GK_PROC_DELAY_QUERY14)
		usleep(GK_PROC_DELAY_QUERY14);
	#endif
		if(req_type==REQTYPE_TPCH_QUERY14_READ1)
			dm_df_read_tpch_query14_read1(lmem_virt);
		else if(req_type==REQTYPE_TPCH_QUERY14_READ2)
			dm_df_read_tpch_query14_read2(lmem_virt);
	}
#endif	// KM_TPCH_QUERY14
#if (KM_TPCH_QUERY14_CMP_ALL)
	if(req_type==REQTYPE_TPCH_QUERY14_CMP_ALL_READ1 || req_type==REQTYPE_TPCH_QUERY14_CMP_ALL_READ2 )
	{
	#if (GK_PROC_DELAY_QUERY14)
		usleep(GK_PROC_DELAY_QUERY14);
	#endif
		if(req_type==REQTYPE_TPCH_QUERY14_CMP_ALL_READ1)
			dm_df_read_tpch_query14_cmp_all_read1(lmem_virt);
		else if(req_type==REQTYPE_TPCH_QUERY14_CMP_ALL_READ2)
			dm_df_read_tpch_query14_cmp_all_read2(lmem_virt);
	}
#endif	// KM_TPCH_QUERY14_CMP_ALL

#if (TE_AFC_STICKY)
	if(req_type==REQTYPE_AFC_STICKY)
	{
	// TODO: not sure what this for
	//#if (TE_PROC_DELAY_AFC) 
    	//    usleep(TE_PROC_DELAY_AFC);
	//#endif
	    dm_df_afc_sticky_read(lmem_virt);
	}
#endif // TE_AFC_STICKY

#if (GK_MEASURE_TIME)
	add_exec_time( &(n->et_proc) );
    if ( n->et_proc.iter==1000 ) {
        print_exec_time( &(n->et_proc) );
        init_time( &(n->et_proc) );
    }
#endif

}
#endif	// GK_TEST_RD_NAND==4

// gunjae: original function
void dm_df_completion_cb (void *req_ptr, uint8_t ret)
{
	struct bdbm_llm_req_t* ptr_llm_req = (struct bdbm_llm_req_t*)req_ptr;

	ptr_llm_req->ret = ret;
	_bdi->ptr_dm_inf->end_req (_bdi, ptr_llm_req);
}

#if (GK_TEST_RD_NAND)
void dm_df_completion_cb_req_read (void *req_ptr, uint8_t ret, uint32_t desc_idx, NvmeCtrl* n)
{
	struct bdbm_llm_req_t* ptr_llm_req = (struct bdbm_llm_req_t*)req_ptr;

	uint8_t *lmem_virt;
	lmem_virt = ls2_virt_addr[2] + getpagesize()*desc_idx;
	//GK_REQ_PRINT("GK: read data = %X\n", *lmem_virt);
	// copy local data to host memory
	uint8_t *prp;
	prp = ptr_llm_req->pptr_kpgs[0];

/////////////////////////////////////////////////////////////////
// ISSD custom commands implemented here
/////////////////////////////////////////////////////////////////
//#if (GK_MEASURE_TIME)
//	struct timespec func_start_time, func_end_time;
//	double func_time_elapsed;
////	clock_t func_start_clks, func_end_clks, func_exec_clks;
//#endif

#if (GK_MEASURE_TIME)
	set_start_time( &(n->et_proc) );
#endif	// GK_MEASURE_TIME

#if (GK_REQ_TYPE)
	if (ptr_llm_req->req_type==REQTYPE_READ_CALC0) {
		//printf("CALLED...\n");
		dm_df_add10(lmem_virt);	// C++
		//dm_df_read_calc0(lmem_virt);
	}
#endif // GK_REQ_TYPE
#if (KM_TPCH_QUERY1)
	if(ptr_llm_req->req_type==REQTYPE_TPCH_QUERY1_READ)
	{
//		syslog(LOG_INFO, "lpa = %llu \n", *(ptr_llm_req->lpa));
	#if (GK_PROC_DELAY_QUERY1)
		usleep(GK_PROC_DELAY_QUERY1);
	#endif
		dm_df_read_tpch_query1_read(lmem_virt);
	}
#endif	// KM_TPCH_QUERY1
#if (KM_TPCH_QUERY1_CMP_ALL)
	if(ptr_llm_req->req_type==REQTYPE_TPCH_QUERY1_CMP_ALL)
	{
	#if (GK_PROC_DELAY_QUERY1)
		usleep(GK_PROC_DELAY_QUERY1);
	#endif
		dm_df_read_tpch_query1_cmp_all_read(lmem_virt);
	}
#endif // KM_TPCH_QUERY1_CMP_ALL

#if (KM_TPCH_QUERY14)
	if(ptr_llm_req->req_type==REQTYPE_TPCH_QUERY14_READ1 || ptr_llm_req->req_type==REQTYPE_TPCH_QUERY14_READ2 )
	{
//		syslog(LOG_INFO, "lpa = %llu \n", *(ptr_llm_req->lpa));
	#if (GK_PROC_DELAY_QUERY14)
		usleep(GK_PROC_DELAY_QUERY14);
	#endif
		if(ptr_llm_req->req_type==REQTYPE_TPCH_QUERY14_READ1)
			dm_df_read_tpch_query14_read1(lmem_virt);
		else if(ptr_llm_req->req_type==REQTYPE_TPCH_QUERY14_READ2)
			dm_df_read_tpch_query14_read2(lmem_virt);
	}
#endif	// KM_TPCH_QUERY14

#if (KM_TPCH_QUERY14_CMP_ALL)
	if(ptr_llm_req->req_type==REQTYPE_TPCH_QUERY14_CMP_ALL_READ1 || ptr_llm_req->req_type==REQTYPE_TPCH_QUERY14_CMP_ALL_READ2 )
	{
//		syslog(LOG_INFO, "lpa = %llu \n", *(ptr_llm_req->lpa));
	#if (GK_PROC_DELAY_QUERY14)
		usleep(GK_PROC_DELAY_QUERY14);
	#endif
		if(ptr_llm_req->req_type==REQTYPE_TPCH_QUERY14_CMP_ALL_READ1)
			dm_df_read_tpch_query14_cmp_all_read1(lmem_virt);
		else if(ptr_llm_req->req_type==REQTYPE_TPCH_QUERY14_CMP_ALL_READ2)
			dm_df_read_tpch_query14_cmp_all_read2(lmem_virt);
	}
#endif	// KM_TPCH_QUERY14_CMP_ALL

#if (KM_TPCH_QUERY6)
	if(ptr_llm_req->req_type==REQTYPE_TPCH_QUERY6_READ)
	{
	#if (GK_PROC_DELAY_QUERY6)
		usleep(GK_PROC_DELAY_QUERY6);
	#endif
		dm_df_read_tpch_query6_read(lmem_virt, n);
	}
#endif	// KM_TPCH_QUERY6
#if (KM_TPCH_QUERY6_CMP_ALL)
	if(ptr_llm_req->req_type==REQTYPE_TPCH_QUERY6_CMP_ALL)
	{
	#if (GK_PROC_DELAY_QUERY6)
		usleep(GK_PROC_DELAY_QUERY6);
	#endif
		dm_df_read_tpch_query6_cpm_all_read(lmem_virt, n);
	}
#endif // KM_TPCH_QUERY6_CMP_ALL
#if (TE_AFC_STICKY)
	if(ptr_llm_req->req_type==REQTYPE_AFC_STICKY)
	{
	// TODO: not sure what this for
	//#if (TE_PROC_DELAY_AFC) 
    	//    usleep(TE_PROC_DELAY_AFC);
	//#endif
	    dm_df_afc_sticky_read(lmem_virt);
	}
#endif // TE_AFC_STICKY


#if (HV_PREFIX_SIMILARITY)
	if(ptr_llm_req->req_type==REQTYPE_PREFIX_SIMILARITY_READ)
	{
	#if (GK_PROC_DELAY_PREFIX_SIMILARITY)
		usleep(GK_PROC_DELAY_PREFIX_SIMILARITY);
	#endif
		dm_df_read_prefix_similarity_read(lmem_virt);
	}
#endif
#if (HV_PREFIX_SIMILARITY_CMP_ALL)
	if(ptr_llm_req->req_type==REQTYPE_PREFIX_SIMILARITY_READ_CMP_ALL)
	{
	#if (GK_PROC_DELAY_PREFIX_SIMILARITY)
		usleep(GK_PROC_DELAY_PREFIX_SIMILARITY);
	#endif
		dm_df_read_prefix_similarity_cmp_all_read(lmem_virt);
	}
#endif


#if (GK_MEASURE_TIME)
	add_exec_time( &(n->et_proc) );
    if ( n->et_proc.iter==1000 ) {
        print_exec_time( &(n->et_proc) );
        init_time( &(n->et_proc) );
    }
#endif	// GK_MEASURE_TIME
//////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Transfer to LS2
///////////////////////////////////////////////////////////////////
	//NvmeCtrl *n = &g_NvmeCtrl;
	uint64_t prp1;
	prp1 = (uint64_t)prp;
	prp1 = prp1 - HOST_OUTBOUND_ADDR;
	prp1 = prp1 + n->host.io_mem.addr;
	//printf("GK: read data = %llX src=0x%llX dst=0x%llX (dst_pcie=0x%llX) host_io=0x%llX\n",
	//	*(uint64_t*)lmem_virt, (void*)lmem_virt, prp1, (void*)prp, n->host.io_mem.addr);
	//	//*(uint64_t*)lmem_virt, (void*)lmem_virt, (void*)prp1, (void*)prp, (void*)(n->host.io_mem.addr));

#if (KM_TPCH_QUERY6 || KM_TPCH_QUERY6_CMP_ALL || KM_TPCH_QUERY14 || KM_TPCH_QUERY14_CMP_ALL || KM_TPCH_QUERY1 || KM_TPCH_QUERY1_CMP_ALL || TE_AFC_STICKY || HV_PREFIX_SIMILARITY || HV_PREFIX_SIMILARITY_CMP_ALL)
	if(ptr_llm_req->req_type==REQTYPE_TPCH_QUERY6_READ || ptr_llm_req->req_type == REQTYPE_TPCH_QUERY6_CMP_ALL || ptr_llm_req->req_type == REQTYPE_TPCH_QUERY14_READ1 || ptr_llm_req->req_type == REQTYPE_TPCH_QUERY14_READ2 || ptr_llm_req->req_type == REQTYPE_TPCH_QUERY14_CMP_ALL_READ1 || ptr_llm_req->req_type == REQTYPE_TPCH_QUERY14_CMP_ALL_READ2 || ptr_llm_req->req_type == REQTYPE_TPCH_QUERY1_READ || ptr_llm_req->req_type == REQTYPE_TPCH_QUERY1_CMP_ALL || ptr_llm_req->req_type == REQTYPE_AFC_STICKY || ptr_llm_req->req_type == REQTYPE_PREFIX_SIMILARITY_READ || ptr_llm_req->req_type == REQTYPE_PREFIX_SIMILARITY_CMP_ALL)
	{
	}
	else{
	#if (GK_MEASURE_TIME)
		set_start_time( &(n->et_ls2_data) );
	#endif	// GK_MEASURE_TIME
 	#if (GK_SPLIT_HOST_TX)
		memcpy_split(prp1, lmem_virt, KERNEL_PAGE_SIZE, GK_SPLIT_HOST_TX);
 	#else
 	 #if (GK_DMA_DELAY)
		usleep(GK_DMA_DELAY);
	 #endif	// GK_DMA_DELAY
		//memcpy((void *)prp, (void *)lmem_virt, KERNEL_PAGE_SIZE);	// it is stored in nand_param as subpage_size, equivalent to KERNEL_PAGE_SIZE
		memcpy((void*)prp1, (void*)lmem_virt, KERNEL_PAGE_SIZE);	// it is stored in nand_param as subpage_size, equivalent to KERNEL_PAGE_SIZE
		//printf("GK: after memcpy ");
 	#endif	// GK_SPLIT_HOST_TX
	#if (GK_MEASURE_TIME)
		add_exec_time( &(n->et_ls2_data) );
		if ( n->et_ls2_data.iter==1000 ) {
			print_exec_time( &(n->et_ls2_data) );
			print_exec_bw( &(n->et_ls2_data) );
			init_time( &(n->et_ls2_data) );
		}
	#endif	// GK_MEASURE_TIME
	}
#else
	// split transfer size
 #if (GK_MEASURE_TIME)
	set_start_time( &(n->et_ls2_data) );
 #endif	// GK_MEASURE_TIME
 #if (GK_SPLIT_HOST_TX)
	memcpy_split(prp1, lmem_virt, KERNEL_PAGE_SIZE, GK_SPLIT_HOST_TX);
 #else
	// tentatively
	#if (GK_DMA_DELAY)
	usleep(GK_DMA_DELAY);
	#endif	// GK_DMA_DELAY
	//memcpy((void *)prp, (void *)lmem_virt, KERNEL_PAGE_SIZE);	// it is stored in nand_param as subpage_size, equivalent to KERNEL_PAGE_SIZE
	memcpy((void*)prp1, (void*)lmem_virt, KERNEL_PAGE_SIZE);	// it is stored in nand_param as subpage_size, equivalent to KERNEL_PAGE_SIZE
	//printf("GK: after memcpy ");
 #endif	// GK_SPLIT_HOST_TX
 #if (GK_MEASURE_TIME)
	add_exec_time( &(n->et_ls2_data) );
	if ( n->et_ls2_data.iter==1000 ) {
		print_exec_time( &(n->et_ls2_data) );
		print_exec_bw( &(n->et_ls2_data) );
		init_time( &(n->et_ls2_data) );
	}
 #endif	// GK_MEASURE_TIME

#endif // (KM_TPCH_QUERY6 || KM_TPCH_QUERY6_CMP_ALL || KM_TPCH_QUERY14 || KM_TPCH_QUERY14_CMP_ALL || KM_TPCH_QUERY1 || KM_TPCH_QUERY1_CMP_ALL || TE_AFC_STICKY)

//#if (GK_MEASURE_TIME)
//	clock_t ftch_clks = clock();
//	clock_t exec_clks = ftch_clks - iss_clks;
//	n->ls2_acc_cnt += 1;
//	n->ls2_acc_clks += exec_clks;
//
//	if ((n->ls2_acc_cnt > 0) && (n->ls2_acc_cnt % 1000==0)) {
//		double sec_per_acc = (double)(n->ls2_acc_clks) / n->ls2_acc_cnt / CLOCKS_PER_SEC;
//		GK_PRINT("GK_LS2_CLKS: %ld / %lld, %lf, rate = %lf B/s\n", n->ls2_acc_clks, n->ls2_acc_cnt, sec_per_acc, (double)KERNEL_PAGE_SIZE/sec_per_acc);
//		// bw
//		exec_clks = ftch_clks - n->ls2_bw_init_clks;
//		GK_PRINT("GK_LS2_BW: %ld / 1000, bw = %lf B/s\n", exec_clks, (double)KERNEL_PAGE_SIZE*1000*CLOCKS_PER_SEC/exec_clks);
//		n->ls2_bw_init_clks = clock();
//		//
//		n->ls2_acc_cnt = 0;
//		n->ls2_acc_clks = 0;
//	}
//#endif	// GK_MEASURE_TIME
	ptr_llm_req->ret = ret;
	_bdi->ptr_dm_inf->end_req (_bdi, ptr_llm_req);
}
#endif	// GK_TEST_RD_NAND

#if (GK_REQ_TYPE)
void dm_df_read_calc0 (uint8_t *buf)
{
	// dummy function to add the first 10 bytes
	uint64_t sum = 0;
	uint8_t *p_tmp;
    int i;
	for (i=0; i < 10; i++) {
		p_tmp = (buf + i);
		sum += *p_tmp;
	}
	memcpy(buf, &sum, sizeof(uint64_t));
}
#endif

#if (KM_TPCH_QUERY6 || KM_TPCH_QUERY6_CMP_ALL)
char KM_tpch_query6_date[20];
unsigned int KM_tpch_query6_quantity;
double KM_tpch_query6_discount;
unsigned int KM_tpch_query6_current_query;
void convert(char *date, int *year, int *month, int *day)
{
	char *Year = (char *)calloc(5,1);
	memcpy((void *)Year, (void *)date, 4);
	*year = atoi(Year);
	char *Month = (char *)calloc(3,1);
	memcpy((void *)Month, (void *)(date+4), 2);
	*month = atoi(Month);
	char *Day = (char *)calloc(3,1);
	memcpy((void *)Day, (void *)(date+6), 2);
	*day = atoi(Day);
	free(Year);
	free(Month);
	free(Day);
}

bool compareDates(char *str1, char *str2, int interval)
{
	int year1, year2, month1, month2, day1, day2;
	convert(str1, &year1, &month1, &day1);
	convert(str2, &year2, &month2, &day2);
	if(interval == 0)
	{
		if(year2 < year1) return 1;
		else if(year2 > year1) return 0;
		else {
			if(month2 < month1) return 1;
			else if(month2 > month1) return 0;
			else {
				if(day2 < day1) return 1;
				else if(day2 > day1) return 0;
				else {
					return 0;
				}
			}
		}
	}
	else {
		if(year2 < (year1+interval)) return 1;
		else if(year2 > (year1+interval)) return 0;
		else {
			if(month2 < month1) return 1;
			else if(month2 > month1) return 0;
			else {
				if(day2 < day1) return 1;
				else if(day2 > day1) return 0;
				else {
					return 0;
				}
			}
		}
	}
}
#endif // (KM_TPCH_QUERY6 || KM_TPCH_QUERY6_CMP_ALL)

#if(KM_TPCH_QUERY6)
double KM_tpch_query6_revenue = 0;
void dm_df_read_tpch_query6_read (uint8_t *buf, NvmeCtrl *n)
{
	unsigned int i = 0;
//	for(i = 75; i < 85; i++)
//		syslog(LOG_INFO, "%u %c ", i, buf[i]);
//	syslog(LOG_INFO, "\n");
//	syslog(LOG_INFO, "KM_TPCH_QUERY6: Entering into dm_df_read_tpch_query6_read function\n");

	// Function to analyse the query and sum the revenue in a global variable
	int offset = 0;
	char *l_shipdate = (char *)calloc(9,1);
	double *l_discount = (double *)calloc(1,sizeof(double));
	double *l_quantitiy = (double *)calloc(1,sizeof(double));
	double *l_extendedprice = (double *)calloc(1,sizeof(double));
//#if (GK_MEASURE_TIME)
//	set_start_time( &(n->et_proc) );
//#endif	// GK_MEASURE_TIME
//#if (GK_FUNC_DELAY)
//	usleep(GK_FUNC_DELAY);
//#endif	// GK_FUNC_DELAY
	while(1)
	{
		memcpy((char *)l_shipdate, (char *)(buf+offset+50), 8);
		memcpy((char *)l_discount, (char *)(buf+offset+32), 8);
		memcpy((char *)l_quantitiy, (char *)(buf+offset+16), 8);

		if( (((compareDates(l_shipdate, KM_tpch_query6_date, 0) == 1) && (compareDates(KM_tpch_query6_date, l_shipdate, 1) == 1)) || (strcmp(l_shipdate, KM_tpch_query6_date) == 0)) && (*l_discount > (KM_tpch_query6_discount - 0.01) ) && (*l_discount < (KM_tpch_query6_discount + 0.01)) && (*l_quantitiy < KM_tpch_query6_quantity))
		{
			memcpy((char *)l_extendedprice, (char *)(buf+offset+24), 8);
			KM_tpch_query6_revenue += *l_extendedprice * *l_discount;
		}
		
		offset += 153;
		if((offset + 153) >= (4096) )
		{
//			cout << j << endl;
			offset = 0;
			break;
		}
	}
//#if (GK_MEASURE_TIME)
//	add_exec_time( &(n->et_proc) );
//	if ( n->et_proc.iter==1000 ) {
//		print_exec_time( &(n->et_proc) );
//		init_time( &(n->et_proc) );
//	}
//#endif	// GK_MEASURE_TIME
	free(l_shipdate);
	free(l_discount);
	free(l_quantitiy);
	free(l_extendedprice);
}
#endif //KM_TPCH_QUERY6

#if (KM_TPCH_QUERY6_CMP_ALL)
//Get the page, apply all the comparisons on it, store only the expenditure and other thing used for the sum at the buffer pointer..Great!!
uint8_t * KM_tpch_query6_cmp_all_buffer;
unsigned int KM_tpch_query6_cmp_all_buffer_pageNum, KM_tpch_query6_cmp_all_buffer_page_position;
unsigned int KM_tpch_query6_cmp_all_numRecords;
//Should not do memcpy after every page, should do at the end
void dm_df_read_tpch_query6_cpm_all_read (uint8_t *buf, NvmeCtrl *n)
{
	unsigned int i = 0;
//	for(i = 75; i < 85; i++)
//		syslog(LOG_INFO, "%u %c ", i, buf[i]);
//	syslog(LOG_INFO, "\n");

	int offset = 0;
	char *l_shipdate = (char *)calloc(9,1);
	double *l_discount = (double *)calloc(1,sizeof(double));
	double *l_quantitiy = (double *)calloc(1,sizeof(double));
	double *l_extendedprice = (double *)calloc(1,sizeof(double));
//#if (GK_MEASURE_TIME)
//	set_start_time( &(n->et_proc) );
//#endif	// GK_MEASURE_TIME
//#if (GK_FUNC_DELAY)
//	usleep(GK_FUNC_DELAY);
//#endif	// GK_FUNC_DELAY
	while(1)
	{
		memcpy((char *)l_shipdate, (char *)(buf+offset+50), 8);
		memcpy((char *)l_discount, (char *)(buf+offset+32), 8);
		memcpy((char *)l_quantitiy, (char *)(buf+offset+16), 8);

		if( (((compareDates(l_shipdate, KM_tpch_query6_date, 0) == 1) && (compareDates(KM_tpch_query6_date, l_shipdate, 1) == 1)) || (strcmp(l_shipdate, KM_tpch_query6_date) == 0)) && (*l_discount > (KM_tpch_query6_discount - 0.01) ) && (*l_discount < (KM_tpch_query6_discount + 0.01)) && (*l_quantitiy < KM_tpch_query6_quantity))
		{	
		//	syslog(LOG_INFO, "Entering after condition\n");
			unsigned int position = KM_tpch_query6_cmp_all_buffer_page_position;
			if(KM_tpch_query6_cmp_all_buffer_pageNum == 0) position += 4;
			if(position + 16 >= 4096)
			{
				KM_tpch_query6_cmp_all_buffer_pageNum += 1;
				KM_tpch_query6_cmp_all_buffer_page_position = 0;
				position = 0;
			}
			memcpy((char *)(KM_tpch_query6_cmp_all_buffer+KM_tpch_query6_cmp_all_buffer_pageNum*4096 + position), (char *)(buf+offset+24), 8);
			memcpy((char *)(KM_tpch_query6_cmp_all_buffer+KM_tpch_query6_cmp_all_buffer_pageNum*4096 + position + 8), (char *)l_discount, 8);
			KM_tpch_query6_cmp_all_buffer_page_position += 16;
			KM_tpch_query6_cmp_all_numRecords += 1;
//			memcpy((char *)l_extendedprice, (char *)(buf+offset+24), 8);
//			KM_tpch_query6_revenue += *l_extendedprice * *l_discount;
		}
		offset += 153;
		if((offset + 153) >= (4096) )
		{
			offset = 0;
			break;
		}
	}
//#if (GK_MEASURE_TIME)
//	add_exec_time( &(n->et_proc) );
//	if ( n->et_proc.iter==1000 ) {
//		print_exec_time( &(n->et_proc) );
//		init_time( &(n->et_proc) );
//	}
//#endif	// GK_MEASURE_TIME

//	syslog(LOG_INFO, "%u %u %u\n", KM_tpch_query6_cmp_all_buffer_pageNum, KM_tpch_query6_cmp_all_buffer_page_position, KM_tpch_query6_cmp_all_numRecords);
	free(l_shipdate);
	free(l_discount);
	free(l_quantitiy);
	free(l_extendedprice);
}
#endif //KM_TPCH_QUERY6_CMP_ALL

uint32_t dm_df_probe (struct bdbm_drv_info *bdi)
{
	int ret;
	bdi->ptr_dm_inf->ptr_private = (void*)&_dp;
	memcpy((void*)&_dp.nand, (void*)&bdi->ptr_bdbm_params->nand, sizeof(bdbm_device_params_t));
	ret = bdbm_get_dimm_status (&_dp);
	bdbm_bug_on (ret < 0);
	memcpy((void*)&bdi->ptr_bdbm_params->nand, (void*)&_dp.nand, sizeof(bdbm_device_params_t));
	ret = setup_new_ssd_context (&_dp);
	bdbm_bug_on (ret < 0);
	return ret;
}

uint32_t dm_df_open (struct bdbm_drv_info *bdi)
{                                                                                         
	return 0;
}

void dm_df_close (struct bdbm_drv_info *bdi)
{
}

// gunjae: this function makes requests (MAKE_REQ)
uint32_t dm_df_make_req (struct bdbm_drv_info *bdi, struct bdbm_llm_req_t **pptr_llm_req)
{
	/*TODO to elect the advance command from the basic command*/
	uint32_t *flag = NULL;
	uint32_t ret = 0, llm_cnt, nr_llm_reqs;
	struct nand_params* np = (struct nand_params*)BDBM_GET_NAND_PARAMS(bdi);
	uint64_t hlm_len;
	uint8_t page_off;
	uint8_t nr_kp_per_fp = np->page_main_size/KERNEL_PAGE_SIZE; 

	if (pptr_llm_req[0]->req_type == REQTYPE_GC_READ || \
		pptr_llm_req[0]->req_type == REQTYPE_GC_WRITE || \
		pptr_llm_req[0]->req_type == REQTYPE_GC_ERASE) {
		struct bdbm_hlm_req_gc_t *hlm_req;
		hlm_req = (struct bdbm_hlm_req_gc_t *)(pptr_llm_req[0]->ptr_hlm_req);
		hlm_len = hlm_req->nr_reqs;
		nr_llm_reqs = hlm_req->nr_reqs;
	} else {
		struct bdbm_hlm_req_t *hlm_req;
		hlm_req = (struct bdbm_hlm_req_t *)(pptr_llm_req[0]->ptr_hlm_req);
		hlm_len = hlm_req->len;
		nr_llm_reqs = hlm_req->nr_llm_reqs;
	}
	
	flag = (uint32_t *)bdbm_malloc(sizeof(uint32_t) * nr_llm_reqs);

	if(flag == NULL) {
		bdbm_error("DM Flag memory allocation failed");
		return 1;
	}

//#if (GK_TEST_RD_NAND==1)	// -> failed
//	// gunjae: test for read data from nand flash
//	// gunjae: malloc local mem for read buffer (for one subpage)
//	local_mem_t lmem = {0};
//	bdbm_alloc_pv(&lmem, np->subpage_size);
//	sem_t waiter;
//	sem_init(&waiter, 0, 0);
//	
//	uint32_t gk_ret = 0;
//	uint32_t gk_data_read = 0xFFFFFFFF;
//#endif	// GK_TEST_RD_NAND

	for (llm_cnt = 0; llm_cnt < nr_llm_reqs; llm_cnt++) {
		struct bdbm_llm_req_t *llm_req = pptr_llm_req[llm_cnt];
		switch (llm_req->req_type) {
			case REQTYPE_READ:
			case REQTYPE_WRITE:
		#if (GK_TEST_RD_DUMMY)
			case REQTYPE_READ_DUMMY:
		#endif
		#if (GK_REQ_TYPE)
			case REQTYPE_READ_CALC0:
		#endif // GK_REQ_TYPE 
		#if (KM_TPCH_QUERY6)
			case REQTYPE_TPCH_QUERY6_READ:
		#endif // KM_TPCH_QUERY6
		#if (KM_TPCH_QUERY6_CMP_ALL)
			case REQTYPE_TPCH_QUERY6_CMP_ALL:
		#endif // KM_TPCH_QUERY6_CMP_ALL
		#if (KM_TPCH_QUERY14)
			case REQTYPE_TPCH_QUERY14_READ1:
			case REQTYPE_TPCH_QUERY14_READ2:
		#endif // KM_TPCH_QUERY14
		#if (KM_TPCH_QUERY14_CMP_ALL)
			case REQTYPE_TPCH_QUERY14_CMP_ALL_READ1:
			case REQTYPE_TPCH_QUERY14_CMP_ALL_READ2:
		#endif // KM_TPCH_QUERY14_CMP_ALL
		#if (TE_AFC_STICKY)
			case REQTYPE_AFC_STICKY:
		#endif // TE_AFC_STICKY
		#if (KM_TPCH_QUERY1)
			case REQTYPE_TPCH_QUERY1_READ:
		#endif // KM_TPCH_QUERY1
		#if (KM_TPCH_QUERY1_CMP_ALL)
			case REQTYPE_TPCH_QUERY1_CMP_ALL:
		#endif // KM_TPCH_QUERY1_CMP_ALL
       
        #if (HV_PREFIX_SIMILARITY)
            case REQTYPE_PREFIX_SIMILARITY_READ:
        #endif
        #if (HV_PREFIX_SIMILARITY_CMP_ALL)
            case REQTYPE_PREFIX_SIMILARITY_CMP_ALL:
        #endif


				flag[llm_cnt] = DF_BIT_DATA_IO;
				break;
			case REQTYPE_GC_READ:
			case REQTYPE_GC_WRITE:
			case REQTYPE_GC_ERASE:
				flag[llm_cnt] = DF_BIT_GC_OP;
				break;
		}
	}
	//uint8_t gk_host_addr;
	for (llm_cnt = 0; llm_cnt < nr_llm_reqs && !ret; llm_cnt++) {
		struct bdbm_llm_req_t *llm_req = pptr_llm_req[llm_cnt];
		switch (llm_req->req_type) {
			case REQTYPE_READ:
		#if (GK_REQ_TYPE)
			case REQTYPE_READ_CALC0:
		#endif
		#if (KM_TPCH_QUERY6)
			case REQTYPE_TPCH_QUERY6_READ:
		#endif
		#if (KM_TPCH_QUERY6_CMP_ALL)
			case REQTYPE_TPCH_QUERY6_CMP_ALL:
		#endif // KM_TPCH_QUERY6_CMP_ALL
		#if (KM_TPCH_QUERY14)
			case REQTYPE_TPCH_QUERY14_READ1:
			case REQTYPE_TPCH_QUERY14_READ2:
		#endif // KM_TPCH_QUERY14
#if (KM_TPCH_QUERY14_CMP_ALL)
			case REQTYPE_TPCH_QUERY14_CMP_ALL_READ1:
			case REQTYPE_TPCH_QUERY14_CMP_ALL_READ2:
#endif // KM_TPCH_QUERY14_CMP_ALL
		#if (KM_TPCH_QUERY1)
			case REQTYPE_TPCH_QUERY1_READ:
		#endif // KM_TPCH_QUERY1
		#if (KM_TPCH_QUERY1_CMP_ALL)
			case REQTYPE_TPCH_QUERY1_CMP_ALL:
		#endif // KM_TPCH_QUERY1_CMP_ALL
		#if (TE_AFC_STICKY)
			case REQTYPE_AFC_STICKY:
		#endif // TE_AFC_STICKY
        #if (HV_PREFIX_SIMILARITY)
            case REQTYPE_PREFIX_SIMILARITY_READ:
        #endif
        #if (HV_PREFIX_SIMILARITY_CMP_ALL)
            case REQTYPE_PREFIX_SIMILARITY_CMP_ALL:
        #endif
            //#if (GK_TEST_RD_NAND==1) // -> failed
			//	//sem_init (&waiter, 0, 0);
            //    gk_ret = df_setup_nand_rd_desc (
			//			(void*)&waiter,
			//			llm_req->phyaddr,
			//			llm_req->offset * np->subpage_size,
			//			lmem.phy,	// physical address of local memory
			//			np->subpage_size,
			//			flag[llm_cnt] | DF_BIT_IS_PAGE);
			//	sem_wait (&waiter);
			//	gk_data_read = *(uint32_t *)lmem.virt;
			//	GK_REQ_PRINT("GK_RD_NAND: llm[%d] = %0xlX\n", llm_cnt, gk_data_read);
			//#endif	// GK_TEST_RD_NAND
            
				// gunjae -> segmentation fault why?
				//gk_host_addr = *llm_req->pptr_kpgs[0];
				//gk_print_nand_desc(*llm_req->phyaddr, llm_req->offset * np->subpage_size, gk_host_addr, np->subpage_size, "read");
				ret = df_setup_nand_rd_desc (
						(void*)llm_req,
						llm_req->phyaddr,
						llm_req->offset * np->subpage_size,
						llm_req->pptr_kpgs[0],	// physical address
						np->subpage_size,
						flag[llm_cnt] | DF_BIT_IS_PAGE);
				break;
			case REQTYPE_GC_READ:
				for (page_off=0; page_off < nr_kp_per_fp; page_off++) {
					if (llm_req->kpg_flags[page_off] == MEMFLAG_KMAP_PAGE) {
						ret |= df_setup_nand_rd_desc (
								(void*)llm_req,
								llm_req->phyaddr,
								page_off * np->subpage_size,
								llm_req->pptr_kpgs[page_off],
								np->subpage_size,
								flag[llm_cnt] | DF_BIT_IS_PAGE);
					}
				}
				ret |= df_setup_nand_rd_desc (
						(void*)llm_req,
						llm_req->phyaddr,
						(uint32_t)np->page_main_size,
						llm_req->phy_ptr_oob,
						np->page_oob_size,
						flag[llm_cnt] | DF_BIT_IS_OOB);
				break;

			case REQTYPE_WRITE:
				for (page_off=0; page_off < nr_kp_per_fp; page_off++) {
					if ((llm_req->kpg_flags[page_off + 1] != MEMFLAG_KMAP_PAGE) || (page_off == (nr_kp_per_fp-1))) {
						ret |= df_setup_nand_wr_desc (
								(void*)llm_req,
								llm_req->phyaddr,
								page_off * np->subpage_size,
								llm_req->pptr_kpgs[page_off],
								np->subpage_size*(page_off+1),
								flag[llm_cnt] | DF_BIT_IS_PAGE | DF_BIT_PAGE_END);
						break;
					}
					ret |= df_setup_nand_wr_desc (
							(void*)llm_req,
							llm_req->phyaddr,
							page_off * np->subpage_size,
							llm_req->pptr_kpgs[page_off],
							np->subpage_size*(page_off+1),
							flag[llm_cnt] | DF_BIT_IS_PAGE);
				}

				ret |= df_setup_nand_wr_desc (
						(void*)llm_req,
						llm_req->phyaddr,
						(uint32_t)np->page_main_size,
						llm_req->ptr_oob,
						np->page_oob_size,
						flag[llm_cnt] | DF_BIT_IS_OOB);
				break;

			case REQTYPE_GC_WRITE:
				for (page_off=0; page_off < nr_kp_per_fp; page_off++) {
					if ((llm_req->kpg_flags[page_off+1] != MEMFLAG_KMAP_PAGE) || (page_off == (nr_kp_per_fp-1))) {
						ret |= df_setup_nand_wr_desc (
								(void*)llm_req,
								llm_req->phyaddr,
								page_off * np->subpage_size,
								llm_req->pptr_kpgs[page_off],
								np->subpage_size*(page_off+1),
								flag[llm_cnt] | DF_BIT_IS_PAGE | DF_BIT_PAGE_END);
						break;
					}
						
					ret |= df_setup_nand_wr_desc (
							(void*)llm_req,
							llm_req->phyaddr,
							page_off * np->subpage_size,
							llm_req->pptr_kpgs[page_off],
							np->subpage_size*(page_off+1),
							flag[llm_cnt] | DF_BIT_IS_PAGE);
				}

				ret |= df_setup_nand_wr_desc (
						(void*)llm_req,
						llm_req->phyaddr,
						(uint32_t)np->page_main_size,
						llm_req->phy_ptr_oob,
						np->page_oob_size,
						flag[llm_cnt] | DF_BIT_IS_OOB);
				break;

			case REQTYPE_GC_ERASE:
				ret = df_setup_nand_erase_desc (
						(void*)llm_req,
						llm_req->phyaddr,
						0,
						NULL,
						0,
						flag[llm_cnt]);
				break;

			case REQTYPE_READ_DUMMY:
			#if (GK_TEST_RD_DUMMY)
				ret = df_setup_nand_rd_desc (
						(void*)llm_req,
						llm_req->phyaddr,
						llm_req->offset * np->subpage_size,
						llm_req->pptr_kpgs[0],	// physical address
						np->subpage_size,
						flag[llm_cnt] | DF_BIT_IS_PAGE);
				break;
			#endif	// GK_TEST_RD_DUMMY
			case REQTYPE_TRIM:
				ret = 0;
				dm_df_completion_cb ((void*)llm_req, 0);
				break;

			default:
				bdbm_error ("invalid command");
				ret = 1;
				break;
		}
	}
//#if (GK_TEST_RD_NAND==1) -> failed
//	sem_destroy(&waiter);
//	bdbm_free_pv(&lmem);
//#endif	// GK_TEST_RD_NAND
	bdbm_free(flag);
	return ret;			
}

void dm_df_end_req (struct bdbm_drv_info* bdi, struct bdbm_llm_req_t* ptr_llm_req)
{
	bdbm_bug_on (ptr_llm_req == NULL);
	bdi->ptr_llm_inf->end_req (bdi, ptr_llm_req);
}

uint32_t dm_df_load (struct bdbm_drv_info* bdi, void *pmt, void *bai) 
{
	uint8_t punits,ret;
	bdbm_nblock_st_t *iobb_pools;

	_dp.pmt = (bdbm_pme_t*)pmt;
	_dp.bai = (bdbm_abm_info_t*)bai;

	_dp.bmi[MGMTB_BMT] = (bdbm_nblock_st_t *)_dp.bai->blocks;


	ret = bdbm_ftl_load_from_flash (&_dp);
	iobb_pools = _dp.bmi[MGMTB_IOBB_POOL];

	for (punits = 0; punits < _dp.mgmt_blkptr[MGMTB_IOBB_POOL].nEntries; punits++,iobb_pools++) {
		_dp.bai->punits_iobb_pool[punits] = iobb_pools->block;
	}

	if (_dp.dimm_status != BDBM_ST_LOAD_SUCCESS) {
		ret = 1;
	}

	return ret;
}

uint32_t dm_df_store (struct bdbm_drv_info* bdi)
{
	return bdbm_ftl_store_to_flash (&_dp);
}
