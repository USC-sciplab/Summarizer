#ifndef __DF_DM_H
#define __DF_DM_H
#include "bdbm_drv.h"
#include "../nvme.h"
extern struct bdbm_dm_inf_t _dm_dragonfire_inf;

uint32_t dm_df_probe (struct bdbm_drv_info* bdi);
uint32_t dm_df_open (struct bdbm_drv_info* bdi);
void dm_df_close (struct bdbm_drv_info* bdi); 

/**
 * @Func    	: dm_df_make_req
 * @Brief   	: Act as the entry point to the DM layer for IO commands which will
 * 				  be accessed by IO thread.
 *  @input  	: bdi			- Structure which contains drive information and 
 *  			              	  major parametrs used in each layer.
 *  			  ptr_llm_req	- Pointer to the LLM request.
 *  @return     : Return zero on success and non zero on failure.
 */
uint32_t dm_df_make_req (struct bdbm_drv_info* bdi, struct bdbm_llm_req_t **ptr_llm_req);


/**
 * @Func    	: dm_df_end_req
 * @Brief   	: Act as the exit point from the DM layer for IO commands which will
 * 			  	  be accessed by completion thread.
 *  @input  	: bdi			- Structure which contains drive information and 
 *  			              	  major parametrs used in each layer.
 *  			  ptr_llm_req	- Pointer to the LLM request.
 *  @return     : Return zero on success and non zero on failure.
 */
void dm_df_end_req (struct bdbm_drv_info* bdi, struct bdbm_llm_req_t* ptr_llm_req);

void dm_df_completion_cb (void *req_ptr, uint8_t ret);
#if (GK_TEST_RD_NAND)
void dm_df_completion_cb_req_read (void *req_ptr, uint8_t ret, uint32_t desc_idx, NvmeCtrl *n);
#endif  // GK_TEST_RD_NAND
#if (GK_TEST_RD_NAND==4)
void dm_df_completion_cb_read_proc(void *req_ptr, uint8_t ret, uint32_t desc_idx, NvmeCtrl* n);
void dm_df_completion_cb_read_memcpy(void *req_ptr, uint8_t ret, uint32_t desc_idx, NvmeCtrl* n);
void dm_df_inproc_wrapper(uint32_t desc_idx, uint32_t req_type, NvmeCtrl *n);
#endif	// GK_TEST_RD_NAND==4
#if (GK_REQ_TYPE)
void dm_df_read_calc0 (uint8_t *buf);
void dm_df_add10 (uint8_t *buf);
#endif
#if (KM_TPCH_QUERY6)
void dm_df_read_tpch_query6_read (uint8_t *buf, NvmeCtrl *n);
#endif // KM_TPCH_QUERY6
#if (KM_TPCH_QUERY14)
void dm_df_read_tpch_query14_read1(uint8_t *buf);
void dm_df_read_tpch_query14_read2(uint8_t *buf);
#endif
#if (KM_TPCH_QUERY6_CMP_ALL)
void dm_df_read_tpch_query6_cpm_all_read (uint8_t *buf, NvmeCtrl *n);
#endif	// KM_TPCH_QUERY_CMP_ALL
#if (KM_TPCH_QUERY1)
void dm_df_read_tpch_query1_cmp_all_read(uint8_t *buf);
#endif // KM_TPCH_QUERY1
#if (TE_AFC_STICKY)
void dm_df_afc_sticky_read(uint8_t *buf);
#endif	// TE_AFC_STICKY

#if (HV_PREFIX_SIMILARITY)
void dm_df_read_prefix_similarity_read (uint8_t *buf);
#endif // PREFIX_SIMILARITY

#if (HV_PREFIX_SIMILARITY_CMP_ALL)
void dm_df_read_prefix_similarity_cmp_all_read (uint8_t *buf);
#endif	// PREFIX_SIMILARITY_CMP_ALL

uint32_t dm_df_load (struct bdbm_drv_info* bdi, void *pmt, void *abm);
uint32_t dm_df_store (struct bdbm_drv_info* bdi);
#endif


