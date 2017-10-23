# Notes for issd-nvme

1. Main function - nvme.c
 - It looks like main() is first called when iSSD is booted up
 - functions nvme_process_reg() and nvme_process_db(), what are they?
 - In nvme_process_db(), what is CQ or SQ? (NvmeCQ or NvmeSQ)
 - SQ: submission queue, CQ: completion queue
 - nvme_io_cmd(): function to parse nvme commands, then nvme_equeue_req_completion() is called

2. Miscellaneous
 - CLEAN_EXIT process is called when SIGINT is asserted, so "kill -2" is appropriate

3. Request generation
 - nvme.c:: SQ->mq_send ... mq_receive->io_preocessor ... CQ
 - io_processor():: generate block IO requests (bio)
   hierarchy: nvme_bio_request -> host_make_req -> hlm_make_req -> ftl -> llm_make_req -> dm_make_req
   after request generation is completed, nvme_enqueue_req_completion is called (CQ is updated)
