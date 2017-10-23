#include "nvme.h"
#include "bdbm_ftl/dev_dragonfire.h"
#include "dma_df_nand.h"
#include "uatomic.h"
#include "bdbm_ftl/params.h"
#include "stdlib.h"

#include <syslog.h>

/*TODO to declare the used global variables*/
extern bdbm_dev_private_t _dp;

extern NvmeCtrl g_NvmeCtrl;
unsigned long long int gc_phy_addr[70];	// gunjae: 4MB * 70 units assigned to pic_generic driver
uint8_t* ls2_virt_addr[5];
#if (GK_TEST_RD_NAND==2)
uint8_t* gk_virt_addr[70];
#endif	// GK_TEST_RD_NAND

uint8_t first_dma = 0;
void *df_io_completer (void *arg);

atomic_t act_desc_cnt = ATOMIC_INIT(0);
Nand_DmaCtrl nand_DmaCtrl[NAND_TABLE_COUNT];
Desc_Track Desc_trck[TOTAL_NAND_DESCRIPTORS];

#if ((GK_TEST_RD_NAND==3) || (GK_TEST_RD_NAND==4))
void *df_io_rd_completer(void *arg);
Desc_Rd_Track desc_rd_trck[TOTAL_NAND_DESC_RD];	// for fetched data
#endif

#if (GK_TEST_RD_NAND==4)
void *df_io_proc_completer(void *arg);
Desc_Proc_Track desc_proc_trck[DESC_PROC_QUEUE_DEPTH];
#endif

void init_df_dma_mgr (FpgaCtrl *fpga);
void deinit_df_dma_mgr (void);

// gunjae: print functions
void print_bdbm_paddr(bdbm_phyaddr_t pa) { GK_PRINT("chnl[%d] chip[%d] blk[0x%X] page[0x%lX] ", pa.channel_no, pa.chip_no, pa.block_no, pa.page_no); }
void gk_print_nand_desc(bdbm_phyaddr_t row, uint32_t col, uint8_t host_addr, uint32_t len, char* cmd)
{
	GK_PRINT("GK_NAND_DESC: ");
	print_bdbm_paddr(row);
	GK_PRINT("col[0x%lX] host_addr[0x%lX] len[%d] cmd[%s]", col, host_addr, len, cmd);
	GK_PRINT("\n");
}

static inline void __get_lun_target (uint32_t channel_no, uint32_t chip_no, uint8_t *target, uint8_t *lun) 
{
	switch(chip_no){
	case 0:
		*target=0; *lun=0;
		break;
	case 1:
		*target=0; *lun=1;
		break;
	case 2:
		*target=1; *lun=0;
		break;
	case 3:
		*target=1; *lun=1;
		break;
	default:
		printf("%s wrong chip and channel num: %d %d\n", __func__, chip_no, channel_no);
	}
}

static inline void reset_df_dma_descriptors (void)
{
	GK_PRINT("GK: reset_df_dma_descriptors(), n_table[%d], n_desc_per_table[%d]\n", NAND_TABLE_COUNT, NAND_DESC_PER_TABLE);
	int tab_no, loop;
	NvmeCtrl *n = &g_NvmeCtrl;
	nand_csr_reg csr_reset = {0,0,0,1,1,0};

	for (tab_no = 0; tab_no < NAND_TABLE_COUNT ;tab_no++) {
		memcpy ((void *)n->fpga.nand_dma_regs.csr[tab_no],(void *) &csr_reset, sizeof (nand_csr_reg));
		nand_DmaCtrl[tab_no].tail_idx = nand_DmaCtrl[tab_no].head_idx = 0;
		for (loop =0; loop< (NAND_DESC_PER_TABLE*8) ;loop++) {
			memset((uint32_t*)n->fpga.nand_dma_regs.table[tab_no]+loop, 0,sizeof(uint32_t));
		}
	}
}

static uint32_t df_fill_dma_desc (
		void *req_ptr, 
		struct bdbm_phyaddr_t *phy_addr, 
		uint32_t col_addr,
		uint8_t *host_addr, 
		uint32_t len, 
		uint32_t req_flag) 
{
	uint32_t ret = 0;
	uint8_t loop, lun, target;
	uint64_t l_addr;
	nand_descriptor_t desc = {0};
	NvmeCtrl *n = &g_NvmeCtrl;
	uint8_t tab_id = phy_addr->channel_no;	// gunjae: table_id = channel_id
	uint16_t head_idx = nand_DmaCtrl[tab_id].head_idx;
	static uint32_t desc_head_idx =0; /*to be moved into desc-tracker and reset with dma reset calls -TODO*/
#if ((GK_TEST_RD_NAND==3) || (GK_TEST_RD_NAND==4))
	static uint32_t desc_rd_head_idx = 0;	// initialized when starting
	
	uint8_t f_read = 0;
	if (((req_flag & DF_BIT_IS_OOB) && (req_flag & DF_BIT_DATA_IO)) || (req_flag & DF_BIT_STATUS_OP) || (req_flag & DF_BIT_SCAN_BB)) {
		f_read = 0;
	} else {
		if ((host_addr != NULL) && (req_flag & DF_BIT_READ_OP) && (req_flag & DF_BIT_DATA_IO) && (req_flag & DF_BIT_IS_PAGE))
			f_read = 1;
		else
			f_read = 0;
	}
#endif	// GK_TEST_RD_NAND==3 || 4
	__get_lun_target (phy_addr->channel_no, phy_addr->chip_no, &target, &lun);
	do {
	#if ((GK_TEST_RD_NAND==3) || (GK_TEST_RD_NAND==4))
		if (Desc_trck[desc_head_idx].is_used == TRACK_IDX_FREE) {	// gunjae: if head is free, start
			if (f_read==0) {
				break;	// do not check rd status
			} else {
				if (desc_rd_trck[desc_rd_head_idx].is_used==TRACK_IDX_FREE) {
					break;
				}
			}
		}
	#else
		if (Desc_trck[desc_head_idx].is_used == TRACK_IDX_FREE) {	// gunjae: if head is free, start
			break;
		}
	#endif	// GK_TEST_RD_NAND==3 || 4
		usleep(1);
	} while (1);

	do {
		mutex_lock (&nand_DmaCtrl[tab_id].DescSt[head_idx].available);
		if (!nand_DmaCtrl[tab_id].DescSt[head_idx].valid) {
			break;
		}
		mutex_unlock (&nand_DmaCtrl[tab_id].DescSt[head_idx].available);
		usleep(1);
	} while(1);

	desc.row_addr = (lun << 21) | ((phy_addr->block_no & 0xFFF) << 9) | (phy_addr->page_no & 0x1FF);

	desc.column_addr = col_addr;

	desc.buffer_id = 0x0;
	desc.channel_id = 0x0;
	desc.target = target;
	desc.length = len;
	if (req_flag & DF_BIT_READ_OP || req_flag & DF_BIT_STATUS_OP) {
		desc.dir = 0x1;		// gunjae: read??
		if (req_flag & DF_BIT_STATUS_OP) {
			desc.command = CMD_READ_STATUS_ENH;
			desc.ecc = DISABLE_ECC;
		} else if (req_flag & DF_BIT_CONFIG_IO) {
			desc.ecc = DISABLE_ECC;
			desc.command = CMD_READ_PARAMETER;
		} else {
			desc.command = CMD_READ;
		}
	} else if (req_flag & DF_BIT_ERASE_OP) {
		desc.ecc = DISABLE_ECC; 
		desc.command = CMD_BLOCK_ERASE;
	} else if (req_flag & DF_BIT_WRITE_OP) {
		if (req_flag & DF_BIT_MGMT_IO) {
			if (len == 0) {
				desc.no_prog = SET_NO_PROG;
				desc.ecc = DISABLE_ECC;
			} else {
				desc.desc_load = SET_LOAD_BIT;
			}
			if (req_flag & DF_BIT_IS_OOB) {
				desc.command = CMD_CHANGE_WRITE_COL;
			} else {
				desc.command = CMD_PAGE_PROG;
			}
		} else {
			if (req_flag & DF_BIT_IS_OOB) {
				desc.command = CMD_CHANGE_WRITE_COL;
				desc.desc_load = SET_LOAD_BIT;
			} else {
				if (req_flag & DF_BIT_PAGE_END) {
					desc.desc_load = SET_LOAD_BIT;
				}
				desc.no_prog = SET_NO_PROG;   /*No_prog = 1*/
				desc.command = CMD_PAGE_PROG;
			}
		}
	} else if (req_flag & DF_BIT_RESET_OP) {
		desc.ecc = DISABLE_ECC;
		desc.no_data = SET_NO_DATA;
		desc.command = CMD_RESET;
	}
		 

	if (((req_flag & DF_BIT_IS_OOB) && (req_flag & DF_BIT_DATA_IO)) || \
		(req_flag & DF_BIT_STATUS_OP) || (req_flag & DF_BIT_SCAN_BB)) {
		if ((req_flag & DF_BIT_WRITE_OP) && !(req_flag & DF_BIT_STATUS_OP)) {
			l_addr = (uint64_t)host_addr;
			/*memcpy(nand_DmaCtrl[tab_id].DescSt[head_idx].virt_oob, \
					(uint8_t *)cmd_buff->data_buffer, NAND_PAGE_OOB_SIZE);*/
			memcpy(nand_DmaCtrl[tab_id].DescSt[head_idx].virt_oob, \
					(uint8_t *)l_addr, NAND_PAGE_OOB_SIZE);
		}
		desc.data_buff_LSB = ((uint64_t)(nand_DmaCtrl[tab_id].DescSt[head_idx].phy_oob) & 0xffffffff);
		desc.data_buff_MSB = ((uint64_t)(nand_DmaCtrl[tab_id].DescSt[head_idx].phy_oob) >> 32);
	} else {
		if (host_addr != NULL) {
		#if (GK_TEST_RD_NAND >= 2)	// 2, 3, 4
			if ((req_flag & DF_BIT_READ_OP) && (req_flag & DF_BIT_DATA_IO) && (req_flag & DF_BIT_IS_PAGE)) {
				// REQTYPE_READ, new local address becomes destination
				//unsigned int lmem_idx = tab_id + 2;	// start from [2]
				uint8_t *lmem_phy_base;
				uint8_t *lmem_phy;
				lmem_phy_base = (uint8_t *)gc_phy_addr[2];
			#if (GK_TEST_RD_NAND==2)
				lmem_phy = lmem_phy_base + getpagesize()*desc_head_idx;	// total 256, so enough
			#else	// GK_TEST_RD_NAND==3 || 4
				lmem_phy = lmem_phy_base + getpagesize()*desc_rd_head_idx;	// total 256, so enough
			#endif
				desc.data_buff_LSB = ((uint64_t)(lmem_phy) & 0xffffffff);
				desc.data_buff_MSB = ((uint64_t)(lmem_phy) >> 32);
			} else {
				desc.data_buff_LSB = ((uint64_t)(host_addr) & 0xffffffff);
				desc.data_buff_MSB = ((uint64_t)(host_addr) >> 32);
			}
		#else
			desc.data_buff_LSB = ((uint64_t)(host_addr) & 0xffffffff);
			desc.data_buff_MSB = ((uint64_t)(host_addr) >> 32);
		#endif	// GK_TEST_RD_NAND
		}
	}
	desc.irq_en = 0;
	desc.hold = 0;
	desc.desc_id = head_idx+1;
	desc.OwnedByfpga = 1;

	nand_DmaCtrl[tab_id].DescSt[head_idx].req_flag = req_flag;
	nand_DmaCtrl[tab_id].DescSt[head_idx].req_ptr = req_ptr;
	nand_DmaCtrl[tab_id].DescSt[head_idx].valid = 1;
#if 0
	for(loop=0; loop<8; loop++) {
		memcpy((uint32_t *)(nand_DmaCtrl[tab_id].DescSt[head_idx].ptr)+loop, \
				(uint32_t*)&desc+loop, sizeof(uint32_t)); 
	}
#endif
	// gunjae: register dma_descriptor at head
	memcpy(nand_DmaCtrl[tab_id].DescSt[head_idx].ptr, \
				&desc, sizeof(nand_descriptor_t)); 
#if (GK_MEASURE_TIME)
	clock_gettime(CLOCK_MONOTONIC, &(Desc_trck[desc_head_idx].iss_time));
	//Desc_trck[desc_head_idx].iss_clks = clock();
#endif
	if(first_dma == 0) {
		for(loop=0; loop<NAND_TABLE_COUNT; loop++) {
			((nand_csr_reg *)(n->fpga.nand_dma_regs.csr[loop]))->start = 1;
			((nand_csr_reg *)(n->fpga.nand_dma_regs.csr[loop]))->loop = 1;
		}
		first_dma++;
	}

	Desc_trck[desc_head_idx].DescSt_ptr = &(nand_DmaCtrl[tab_id].DescSt[head_idx]);
	Desc_trck[desc_head_idx].is_used = TRACK_IDX_USED;
#if ((GK_TEST_RD_NAND==3) || (GK_TEST_RD_NAND==4))
	if (f_read) {
		desc_rd_trck[desc_rd_head_idx].is_used = TRACK_IDX_USED;
		CIRCULAR_INCR(desc_rd_head_idx, TOTAL_NAND_DESC_RD);	// increase head count
	}
#endif


	mutex_unlock (&nand_DmaCtrl[tab_id].DescSt[head_idx].available);
	atomic_inc(&act_desc_cnt);

	CIRCULAR_INCR(desc_head_idx, TOTAL_NAND_DESCRIPTORS);
	CIRCULAR_INCR(nand_DmaCtrl[tab_id].head_idx, NAND_DESC_PER_TABLE);
	return ret;
}

uint32_t df_setup_nand_desc (
		void *req_ptr, 
		struct bdbm_phyaddr_t *phy_addr, 
		uint32_t col_addr,
		uint8_t *host_addr, 
		uint32_t len, 
		uint32_t req_flag) 
{
	uint32_t ret = 0;

	// gunjae
	df_print_nand_desc(phy_addr, col_addr, host_addr, len, req_flag);
	ret = df_fill_dma_desc (req_ptr, phy_addr, col_addr, host_addr, len, req_flag);

#ifdef GET_CMD_STATUS
	if (((req_flag & DF_BIT_IS_OOB) || (req_flag & DF_BIT_ERASE_OP) || \
			(req_flag & DF_BIT_MGMT_IO && req_ptr != NULL)) && !(req_flag & DF_BIT_READ_OP)) {

		ret |= df_fill_dma_desc (req_ptr, phy_addr, 0, NULL, 1, req_flag|DF_BIT_STATUS_OP);
	}
#endif

	return ret; 
}

static void setup_df_desc_context ()
{
	NvmeCtrl *n = &g_NvmeCtrl;
	uint32_t tab_no = 0, desc_no = 0;
	uint32_t *tab_ptr;
	int i = 0;

	for(tab_no=0; tab_no<NAND_TABLE_COUNT; tab_no++){
		tab_ptr = n->fpga.nand_dma_regs.table[tab_no];
		for(desc_no= 0; desc_no< NAND_DESC_PER_TABLE; desc_no++,i++) {
			nand_DmaCtrl[tab_no].DescSt[desc_no].ptr = (uint8_t *)(tab_ptr + (desc_no * NAND_DESC_SIZE));
			nand_DmaCtrl[tab_no].DescSt[desc_no].req_flag = 0;
			nand_DmaCtrl[tab_no].DescSt[desc_no].virt_oob = ls2_virt_addr[0] + (i * 1024);
			nand_DmaCtrl[tab_no].DescSt[desc_no].phy_oob = (uint8_t*)(gc_phy_addr[0] + (i * 1024));
			nand_DmaCtrl[tab_no].DescSt[desc_no].cmdbuf_virt_addr = (uint64_t)(ls2_virt_addr[0] + 0x200000 + (i * CMD_STRUCT_SIZE));
			nand_DmaCtrl[tab_no].DescSt[desc_no].cmdbuf_phy_addr = (gc_phy_addr[0] + 0x200000+ (i * CMD_STRUCT_SIZE));

			nand_DmaCtrl[tab_no].DescSt[desc_no].valid = 0;
			mutex_init (&nand_DmaCtrl[tab_no].DescSt[desc_no].available, NULL);
		}
	}
}

static void df_dma_default (Nand_DmaRegs *regs)
{
	uint8_t tbl;
	for(tbl = 0 ;tbl < NAND_TABLE_COUNT ; tbl++) {
		nand_DmaCtrl[tbl].head_idx=0;
		nand_DmaCtrl[tbl].tail_idx=0; 

		((nand_csr_reg *)regs->csr[tbl])->reset = 1;
		((nand_csr_reg *)regs->csr[tbl])->reset = 0;
	}
	setup_df_desc_context ();
#if ((GK_TEST_RD_NAND==3) || (GK_TEST_RD_NAND==4))
	unsigned int i;
	for (i = 0; i < TOTAL_NAND_DESC_RD; i++) {
		desc_rd_trck[i].status = 0;
		desc_rd_trck[i].is_used = TRACK_IDX_FREE;
	}
#endif

#if (GK_TEST_RD_NAND==4)
	for (i = 0; i < DESC_PROC_QUEUE_DEPTH; i++) {
		desc_proc_trck[i].is_ready = 0;
		desc_proc_trck[i].is_valid = 0;
	}
#endif
}

#if(KM_TPCH_QUERY6_CMP_ALL)
extern uint8_t * KM_tpch_query6_cmp_all_buffer;
#endif // KM_TPCH_QUERY6_CMP_ALL

#if(KM_TPCH_QUERY14_CMP_ALL)
extern uint8_t * KM_tpch_query14_cmp_all_buffer;
#endif // KM_TPCH_QUERY14_CMP_ALL

#if(KM_TPCH_QUERY1_CMP_ALL)
extern uint8_t * KM_tpch_query1_cmp_all_buffer;
#endif // KM_TPCH_QUERY1_CMP_ALL

/*#if(HV_PREFIX_SIMILARITY_CMP_ALL)
extern uint8_t * KM_prefix_similarity_cmp_all_buffer;
#endif
*/
void init_df_dma_mgr (FpgaCtrl *fpga)
{
	Nand_DmaRegs *regs = &fpga->nand_dma_regs;
	uint32_t loop;

	for (loop=0; loop<NAND_TABLE_COUNT; loop++) {
		*(regs->table_sz[loop]) =  NAND_DESC_PER_TABLE;
	}

	for (loop = 0; loop < 5; loop++) {
		ls2_virt_addr[loop] = mmap_oob_addr(1024,loop);
	}

	for (loop = 0; loop<5; loop++) {
		if(!ls2_virt_addr[loop]) {
			perror("mmap_oob:");
			return;
		}
	}
#if(KM_TPCH_QUERY6_CMP_ALL)
//	KM_tpch_query6_cmp_all_buffer = ls2_virt_addr[4];
	if (posix_memalign((void **)(&KM_tpch_query6_cmp_all_buffer), getpagesize(), 1024*getpagesize())) {
		syslog(LOG_INFO, "can not allocate io payload\n");
	}
#endif // KM_TPCH_QUERY6_CMP_ALL
#if(KM_TPCH_QUERY14_CMP_ALL)
	if (posix_memalign((void **)(&KM_tpch_query14_cmp_all_buffer), getpagesize(), 32*getpagesize())) {
                syslog(LOG_INFO, "can not allocate io payload\n");
        }
#endif // KM_TPCH_QUERY14_CMP_ALL
#if(KM_TPCH_QUERY1_CMP_ALL)
	if (posix_memalign((void **)(&KM_tpch_query1_cmp_all_buffer), getpagesize(), 32*getpagesize())) {
                syslog(LOG_INFO, "can not allocate io payload\n");
        }
#endif // KM_TPCH_QUERY1_CMP_ALL

/*#if(HV_PREFIX_SIMILARITY_CMP_ALL)
	if (posix_memalign((void **)(&KM_prefix_similarity_cmp_all_buffer), getpagesize(), 1024*getpagesize())) {
		syslog(LOG_INFO, "can not allocate io payload\n");
	}
#endif
*/
#if (GK_TEST_RD_NAND==2)
//	uint8_t gk_ret;
//	gk_ret = gk_virt_map();
#endif	// GK_TEST_RD_NAND

	df_dma_default (regs);
}

void deinit_df_dma_mgr (void)
{
	int i;
	for (i=0; i<5; i++) {
		munmap(ls2_virt_addr[i] , (1024*getpagesize()));
	}
#if (GK_TEST_RD_NAND==2)
//	uint8_t gk_ret;
//	gk_ret = gk_virt_unmap();
#endif	// GK_TEST_RD_NAND
}

#if (GK_MEASURE_TIME)
inline void measure_nand_dma_time(struct timespec start, struct timespec end, NvmeCtrl* n)
{
	double time;
	time = ((double)end.tv_sec - (double)start.tv_sec);
	time += ((double)end.tv_nsec - (double)start.tv_nsec) / 1000000000.0;

	n->nand_acc_cnt += 1;
	n->nand_acc_time += time;
	if ((n->nand_acc_cnt > 0) && (n->nand_acc_cnt % 1000==0)) {
		// dma flight time
		double sec_per_acc = (double)(n->nand_acc_time) / n->nand_acc_cnt;
		GK_PRINT("GK_NAND_TIME: %lf / %lld, exec_time = %lf \n", n->nand_acc_time, n->nand_acc_cnt, sec_per_acc);
		// bw
		time = ((double)end.tv_sec - (double)n->nand_bw_init_time.tv_sec);
		time += ((double)end.tv_nsec - (double)n->nand_bw_init_time.tv_nsec) / 1000000000.0;
		GK_PRINT("GK_NAND_BW: %lld / %lf, BW = %lf B/s \n", KERNEL_PAGE_SIZE*1000, time, (double)KERNEL_PAGE_SIZE*1000.0/time);
		//
		n->nand_acc_cnt = 0;
		n->nand_acc_time = 0.0;
		clock_gettime(CLOCK_MONOTONIC, &(n->nand_bw_init_time));
	}
}
#endif	// GK_MEASURE_TIME

void *df_io_completer (void *arg)
{
	NvmeCtrl *n = (NvmeCtrl*)arg;
	nand_csf *csf;
	nand_csf csf_bak;
	nand_csr_reg **csr = (nand_csr_reg **)n->fpga.nand_dma_regs.csr;
	uint64_t desc_tail_idx = 0;
	uint8_t io_stat = 0, tbl, tab_id = 0, status = 0;
	uint32_t err,flag, csf_val = 0;
	uint8_t f_read = 0;	// gunjae: flag for read operation
#if ((GK_TEST_RD_NAND==3) || (GK_TEST_RD_NAND==4))
	uint64_t desc_rd_tail_idx = 0;
#endif

#if (GK_MEASURE_TIME)
	struct timespec fetch_time;	// fetched
	//clock_t ftch_clks;	// fetched
	//clock_t iss_clks;
	//clock_t exec_clks;
	//double sec_per_acc;
	//struct timespec time_start;
	//struct timespec time_end;
	//double time_elapsed;
	
	//clock_gettime(CLOCK_MONOTONIC, &time_start);
#endif

	while (1) {
		do {
			if (Desc_trck[desc_tail_idx].is_used == TRACK_IDX_USED) {
				break;
			}
			usleep(1);
		} while (1);

		do {
			mutex_lock (&(Desc_trck[desc_tail_idx].DescSt_ptr->available));
			if (Desc_trck[desc_tail_idx].DescSt_ptr->valid) {
				break;
			}
			mutex_unlock (&(Desc_trck[desc_tail_idx].DescSt_ptr->available));
			usleep(1);
		}  while (1);

		do {
			csf = (nand_csf *)((uint32_t*)(Desc_trck[desc_tail_idx].DescSt_ptr->ptr) + 7);
			csf_val = *((uint32_t *)csf);
			csf = (nand_csf *)&csf_val;
			usleep(1);
		}while (csf->OwnedByFpga);

		while (csf->OwnedByFpga) {
			usleep(1);
		}

		memcpy (&csf_bak, csf, sizeof(nand_csf));

		if(csf_bak.dma_cmp){
			io_stat = 0;
		} else {
			err = csr[tab_id]->err_code;
			if(err == 1 || err == 2) {  
				printf("READ/WRITE error occured\n");
				io_stat = 1;
			}
		}
		
		f_read = 0;
		flag = Desc_trck[desc_tail_idx].DescSt_ptr->req_flag;
#ifdef GET_CMD_STATUS
		if (flag & DF_BIT_STATUS_OP || flag & DF_BIT_READ_OP) {
			if (flag & DF_BIT_STATUS_OP) {
				if (flag & DF_BIT_WRITE_OP) {
					if (((*(Desc_trck[desc_tail_idx].DescSt_ptr->virt_oob) & 0x42) == 0x42) && \
							((*(Desc_trck[desc_tail_idx].DescSt_ptr->virt_oob) & 0x21) == 0x21)) {
						io_stat = 1;
						printf("ERROR: write operation fails with status : %x\n", \
								*(Desc_trck[desc_tail_idx].DescSt_ptr->virt_oob));
					}
				} else if (flag & DF_BIT_ERASE_OP) {
					if ((*(Desc_trck[desc_tail_idx].DescSt_ptr->virt_oob) & 0x21) == 0x21) {
						io_stat = 1;
						printf("ERROR: erase operation fails with status : %x\n", \
								*(Desc_trck[desc_tail_idx].DescSt_ptr->virt_oob));
					}
				}
				status |= io_stat;
			}

			if ((flag & DF_BIT_DATA_IO)	|| (flag & DF_BIT_GC_OP)) {
				if ((flag & DF_BIT_IS_OOB) || (flag & DF_BIT_ERASE_OP) || \
						((flag & DF_BIT_DATA_IO) && (flag & DF_BIT_READ_OP))) {
				#if (GK_TEST_RD_NAND >= 2)
					if ((flag & DF_BIT_DATA_IO) && (flag & DF_BIT_IS_PAGE) && (flag & DF_BIT_READ_OP)) {
					#if (GK_TEST_RD_NAND==2)
						dm_df_completion_cb_req_read(Desc_trck[desc_tail_idx].DescSt_ptr->req_ptr, status, desc_tail_idx, n);
					#else	// GK_TEST_RD_NAND==3, just update read track queue
						desc_rd_trck[desc_rd_tail_idx].req_ptr = Desc_trck[desc_tail_idx].DescSt_ptr->req_ptr;
						desc_rd_trck[desc_rd_tail_idx].status = status;
						desc_rd_trck[desc_rd_tail_idx].is_used = TRACK_IDX_FETCH;
					#endif
						f_read = 1;
					} else
						dm_df_completion_cb (Desc_trck[desc_tail_idx].DescSt_ptr->req_ptr,status);
				#else
					dm_df_completion_cb (Desc_trck[desc_tail_idx].DescSt_ptr->req_ptr,status);
				#endif	// GK_TEST_RD_NAND
					status = 0;
					io_stat = 0;
				}
			} else if (flag & DF_BIT_SCAN_BB) {
				bdbm_scan_bb_completion_cb (Desc_trck[desc_tail_idx].DescSt_ptr->req_ptr, \
						Desc_trck[desc_tail_idx].DescSt_ptr->virt_oob, status);
			} else if (flag & DF_BIT_MGMT_IO || flag & DF_BIT_SEM_USED) {
				if (Desc_trck[desc_tail_idx].DescSt_ptr->req_ptr != NULL) {
					sem_post((sem_t*)Desc_trck[desc_tail_idx].DescSt_ptr->req_ptr);
				}
				status = 0;
				io_stat = 0;
			}
		}
#else
		if ((flag & DF_BIT_DATA_IO)	|| (flag & DF_BIT_GC_OP)) {
			if ((flag & DF_BIT_IS_OOB) || (flag & DF_BIT_ERASE_OP) || \
					((flag & DF_BITS_DATA_PAGE) && (flag & DF_BIT_READ_OP))) {
				// gunjae: REQTYPE_READ will go here
			#if (GK_TEST_RD_NAND >= 2)
				if ((flag & DF_BIT_DATA_IO) && (flag & DF_BIT_IS_PAGE) && (flag & DF_BIT_READ_OP)) {
				#if (GK_TEST_RD_NAND==2)
					dm_df_completion_cb_req_read(Desc_trck[desc_tail_idx].DescSt_ptr->req_ptr, status, desc_tail_idx, n);
				#else	// GK_TEST_RD_NAND==3, just update read track queue
					desc_rd_trck[desc_rd_tail_idx].req_ptr = Desc_trck[desc_tail_idx].DescSt_ptr->req_ptr;
					desc_rd_trck[desc_rd_tail_idx].status = status;
					desc_rd_trck[desc_rd_tail_idx].is_used = TRACK_IDX_FETCH;
				#endif	
					f_read = 1;
				} else
					dm_df_completion_cb (Desc_trck[desc_tail_idx].DescSt_ptr->req_ptr,status);
			#else
				dm_df_completion_cb (Desc_trck[desc_tail_idx].DescSt_ptr->req_ptr,io_stat);
			#endif	// GK_TEST_RD_NAND
			}
		} else if (flag & DF_BIT_MGMT_IO || flag & DF_BIT_SEM_USED) {
			sem_post((sem_t*)Desc_trck[desc_tail_idx].DescSt_ptr->req_ptr);
		}
#endif

	#if (GK_MEASURE_TIME)
		if (f_read) {
			clock_gettime(CLOCK_MONOTONIC, &fetch_time);
			measure_nand_dma_time(Desc_trck[desc_tail_idx].iss_time, fetch_time, n);
		}
//		ftch_clks = clock();	// fetched
//		iss_clks = Desc_trck[desc_tail_idx].iss_clks;
//		exec_clks = ftch_clks - iss_clks;
//		
//		if (f_read) {
//			n->nand_acc_cnt += 1;
//			n->nand_acc_clks += exec_clks;
//		}
//
//		if ((n->nand_acc_cnt > 0) && (n->nand_acc_cnt % 1000==0)) {
//			clock_gettime(CLOCK_MONOTONIC, &time_end);
//			sec_per_acc = (double)(n->nand_acc_clks) / n->nand_acc_cnt / CLOCKS_PER_SEC;
//			GK_PRINT("GK_NAND_CLKS: %ld / %lld, %lf, rate = %lf B/s\n", n->nand_acc_clks, n->nand_acc_cnt, sec_per_acc, (double)KERNEL_PAGE_SIZE/sec_per_acc);
//			// bw
//			exec_clks = ftch_clks - n->nand_bw_init_clks;
//			GK_PRINT("GK_NAND_BW: %ld / 1000, bw = %lf B/s\n", exec_clks, (double)KERNEL_PAGE_SIZE*1000*CLOCKS_PER_SEC/exec_clks);
//			n->nand_bw_init_clks = clock();
//			
//			time_elapsed = ((double)time_end.tv_nsec - (double)time_start.tv_nsec) / 1000000000.0;
//			time_elapsed += ((double)time_end.tv_sec - (double)time_start.tv_sec);
//			GK_PRINT("GK_NAND_BW: %lf, bw= %lf B/s\n", time_elapsed, (double)KERNEL_PAGE_SIZE*1000/time_elapsed);
//			//time_elapsed = time_end.tv_nsec - time_start.tv_nsec;
//			//time_elapsed += (time_end.tv_sec - time_start.tv_sec) * 1000000000;
//			//GK_PRINT("GK_NAND_BW: %lld, bw= %lf B/s\n", time_elapsed, (double)KERNEL_PAGE_SIZE*1000*1000000000/time_elapsed);
//			//
//			n->nand_acc_cnt = 0;
//			n->nand_acc_clks = 0;
//			clock_gettime(CLOCK_MONOTONIC, &time_start);
//		}
	#endif	// GK_MEASURE_TIME
		Desc_trck[desc_tail_idx].is_used = TRACK_IDX_FREE;
		Desc_trck[desc_tail_idx].DescSt_ptr->valid = 0;
		mutex_unlock (&(Desc_trck[desc_tail_idx].DescSt_ptr->available));

		atomic_dec(&act_desc_cnt);
		CIRCULAR_INCR (desc_tail_idx, TOTAL_NAND_DESCRIPTORS);

	#if ((GK_TEST_RD_NAND==3) || (GK_TEST_RD_NAND==4))
		if (f_read) {
			desc_rd_trck[desc_rd_tail_idx].is_used = TRACK_IDX_FETCH;
			CIRCULAR_INCR (desc_rd_tail_idx, TOTAL_NAND_DESC_RD);
		}
	#endif	// GK_TEST_RD_NAND==3

		if(io_stat) {
#if 0
			reset_df_dma_descriptors();
			for(tbl =0;tbl <NAND_TABLE_COUNT;tbl++){
				csr[tbl]->reset = 1;
				csr[tbl]->reset = 0;
			}
			first_dma =0;
#endif
		}
	}
}

#if ((GK_TEST_RD_NAND==3) || (GK_TEST_RD_NAND==4))
void *df_io_rd_completer(void *arg)
{
	NvmeCtrl *n = (NvmeCtrl*)arg;
	uint64_t tail_idx = 0;
#if (GK_TEST_RD_NAND==4)
	uint64_t desc_proc_head_idx = 0;
	uint8_t can_proc_in_ssd = 0;
	uint32_t req_type;
 #if (IN_PROC_RATE)
	uint8_t n_circular = 0;
	uint8_t en_inproc = 0;
 #endif
#endif

	while (1) {
		do {
			if (desc_rd_trck[tail_idx].is_used == TRACK_IDX_FETCH) {
				// data is fetched in local memory
				break;
			}
			usleep(1);
		} while (1);

	#if ((GK_TEST_RD_NAND==4) && (IN_PROC_RATE))
		en_inproc = 0;
	#if (IN_PROC_RATE==1)
		if (n_circular==1) en_inproc = 1;
	#elif (IN_PROC_RATE==2)
		if ((n_circular >= 1) && (n_circular <= 2)) en_inproc = 1;
	#elif (IN_PROC_RATE==3)
		if ((n_circular >= 1) && (n_circular <= 3)) en_inproc = 1;
	#elif (IN_PROC_RATE==4)
		if ((n_circular >= 1) && (n_circular <= 4)) en_inproc = 1;
	#elif (IN_PROC_RATE==5)
		en_inproc = 1;
	#endif

		if (en_inproc) {
			// check processing slot is enough
			do {
				if (desc_proc_trck[desc_proc_head_idx].is_valid==0) {
					break;
				}
				usleep(1);
			} while(1);
		}
	#endif	// GK_TEST_RD_NAND==4

		//printf("GK: rd_complter, %llu", tail_idx);
		// do memcpy
		//usleep(100000);
		//GK_PRINT("%llu\n", tail_idx);
	#if (GK_TEST_RD_NAND==4)
		can_proc_in_ssd = 0;
		struct bdbm_llm_req_t* ptr_llm_req = (struct bdbm_llm_req_t*)desc_rd_trck[tail_idx].req_ptr;
		req_type = ptr_llm_req->req_type;

	#if (IN_PROC_RATE)	// force in_proc rate
		if (en_inproc==0)
			req_type = REQTYPE_READ;
	#endif	// IN_PROC_RATE
	
		// tentatively, req_type >= 10
		if ((req_type >= 10) && (desc_proc_trck[desc_proc_head_idx].is_valid==0)) {	// empty
			// do proc and send special code, complete command
			desc_proc_trck[desc_proc_head_idx].desc_idx = tail_idx;
			desc_proc_trck[desc_proc_head_idx].req_type = req_type; 
			// do special code transfer
			can_proc_in_ssd = 1;
			dm_df_completion_cb_read_proc( desc_rd_trck[tail_idx].req_ptr, desc_rd_trck[tail_idx].status, tail_idx, n);
		} else {
			// do normal transfer, just send data as normal
			dm_df_completion_cb_read_memcpy( desc_rd_trck[tail_idx].req_ptr, desc_rd_trck[tail_idx].status, tail_idx, n);
		}
	#else	// GK_TEST_RD_NAND==3
		dm_df_completion_cb_req_read( desc_rd_trck[tail_idx].req_ptr, desc_rd_trck[tail_idx].status, tail_idx, n);
	#endif
		// free the tail slot
	#if (GK_TEST_RD_NAND==4)
		if (can_proc_in_ssd==1) {
			desc_proc_trck[desc_proc_head_idx].is_valid = 1;
			CIRCULAR_INCR(desc_proc_head_idx, DESC_PROC_QUEUE_DEPTH);
		}
	#if (IN_PROC_RATE)
		n_circular = (n_circular==4) ? 0: n_circular+1;
	#endif	// IN_PROC_RATE
	#endif	// GK_TEST_RD_NAND==4
		desc_rd_trck[tail_idx].is_used = TRACK_IDX_FREE;
		CIRCULAR_INCR(tail_idx, TOTAL_NAND_DESC_RD);
		//printf("-> %llu", tail_idx);
	}
}
#endif	// 3, 4

#if (GK_TEST_RD_NAND==4)
void *df_inproc_completer(void *arg)
{
	NvmeCtrl *n = (NvmeCtrl*)arg;
	uint64_t tail_idx = 0;

	while (1) {
		do {
			if (desc_proc_trck[tail_idx].is_valid == 1) {
				break;
			}
			usleep(1);
		} while (1);

		// proc functions by req_type
		dm_df_inproc_wrapper(desc_proc_trck[tail_idx].desc_idx, desc_proc_trck[tail_idx].req_type, n);

//		// delay
//	#if (GK_FUNC_DELAY)
//		usleep(GK_FUNC_DELAY);
//	#endif

		// free the tail slot
		desc_proc_trck[tail_idx].is_valid = 0;
		CIRCULAR_INCR(tail_idx, DESC_PROC_QUEUE_DEPTH);
	}
}
#endif	// GK_TEST_RD_NAND==4
