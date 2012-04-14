/**
 *******************************************************************************
 ** Copyright (c) 2011-2012                                                   **
 **                                                                           **
 **   Integrated Device Technology, Inc.                                      **
 **   Intel Corporation                                                       **
 **   LSI Corporation                                                         **
 **                                                                           **
 ** All rights reserved.                                                      **
 **                                                                           **
 *******************************************************************************
 **                                                                           **
 ** Redistribution and use in source and binary forms, with or without        **
 ** modification, are permitted provided that the following conditions are    **
 ** met:                                                                      **
 **                                                                           **
 **   1. Redistributions of source code must retain the above copyright       **
 **      notice, this list of conditions and the following disclaimer.        **
 **                                                                           **
 **   2. Redistributions in binary form must reproduce the above copyright    **
 **      notice, this list of conditions and the following disclaimer in the  **
 **      documentation and/or other materials provided with the distribution. **
 **                                                                           **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS   **
 ** IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, **
 ** THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR    **
 ** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR         **
 ** CONTRIBUTORS BE LIABLE FOR ANY DIRECT,INDIRECT, INCIDENTAL, SPECIAL,      **
 ** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       **
 ** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        **
 ** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    **
 ** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      **
 ** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        **
 ** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              **
 **                                                                           **
 ** The views and conclusions contained in the software and documentation     **
 ** are those of the authors and should not be interpreted as representing    **
 ** official policies, either expressed or implied, of Intel Corporation,     **
 ** Integrated Device Technology Inc., or Sandforce Corporation.              **
 **                                                                           **
 *******************************************************************************
**/

/*
 * File: nvmeIo.c
 */

#include "precomp.h"

/*******************************************************************************
 * ProcessIo
 *
 * @brief Routine for processing an I/O request (both internal and externa)
 *        and setting up all the necessary info. Then, calls NVMeIssueCmd to
 *        issue the command to the controller.
 *
 * @param AdapterExtension - pointer to device extension
 * @param SrbExtension - SRB extension for this command
 * @param QueueType - type of queue (admin or I/O)
 *
 * @return BOOLEAN
 *     TRUE - command was processed successfully
 *     FALSE - If anything goes wrong
 ******************************************************************************/
BOOLEAN
ProcessIo(
    __in PNVME_DEVICE_EXTENSION pAdapterExtension,
    __in PNVME_SRB_EXTENSION pSrbExtension,
    __in NVME_QUEUE_TYPE QueueType
)
{
    PNVMe_COMMAND pNvmeCmd;
    ULONG StorStatus;
    BOOLEAN returnStatus;
    PCMD_INFO pCmdInfo = NULL;
    PROCESSOR_NUMBER ProcNumber;
    USHORT SubQueue = 0;
    USHORT CplQueue = 0;
    PQUEUE_INFO pQI = &pAdapterExtension->QueueInfo;
    PSUB_QUEUE_INFO pSQI = pQI->pSubQueueInfo;

    StorStatus = StorPortGetCurrentProcessorNumber((PVOID)pAdapterExtension,
                                                   &ProcNumber);
    if (StorStatus != STOR_STATUS_SUCCESS)
        return FALSE;

    /* save off the usbmitting core info for CT learning purposes */
    pSrbExtension->procNum = ProcNumber;

    /* 1 - Select Queue based on CPU */
    if (QueueType == NVME_QUEUE_TYPE_IO) {

        returnStatus = NVMeMapCore2Queue(pAdapterExtension,
                                         &ProcNumber,
                                         &SubQueue,
                                         &CplQueue);
        if (returnStatus == FALSE)
            return FALSE;

    } else {
        /* It's an admin queue */
        SubQueue = 0;
        CplQueue = 0;
    }

    /* 2 - Choose CID for the CMD_ENTRY */
    StorStatus = NVMeGetCmdEntry(pAdapterExtension,
                                 SubQueue,
                                 (PVOID)pSrbExtension,
                                 &pCmdInfo);
    if (StorStatus != STOR_STATUS_SUCCESS) {
        if (pSrbExtension->pSrb != NULL) {
            pSrbExtension->pSrb->SrbStatus = SRB_STATUS_BUSY;
            IO_StorPortNotification(RequestComplete,
                                    pAdapterExtension,
                                    pSrbExtension->pSrb);
        }

        return FALSE;
    }

    pNvmeCmd = &pSrbExtension->nvmeSqeUnit;
#pragma prefast(suppress:6011,"This pointer is not NULL")
    pNvmeCmd->CDW0.CID = (USHORT)pCmdInfo->CmdID;

#ifdef DBL_BUFF
    /*
     * For reads/writes, create PRP list in pre-allocated
     * space describing the dbl buff location... make sure that
     * we do not double buffer NVMe Flush commands.
     */
    if ((QueueType == NVME_QUEUE_TYPE_IO) &&
        (pNvmeCmd->CDW0.OPC != NVM_FLUSH)) {
        LONG len = pSrbExtension->pSrb->DataTransferLength;
        ULONG i = 1;
        PUINT64 pPrpList = (PUINT64)pCmdInfo->pPRPList;

        ASSERT(len <= DBL_BUFF_SZ);

        if (len <= (PAGE_SIZE * 2)) {
            pNvmeCmd->PRP1 = pCmdInfo->dblPhy.QuadPart;
            if (len > PAGE_SIZE) {
                pNvmeCmd->PRP2 = pCmdInfo->dblPhy.QuadPart + PAGE_SIZE;
            } else {
                pNvmeCmd->PRP2 = 0;
            }
        } else {
            pNvmeCmd->PRP1 = pCmdInfo->dblPhy.QuadPart;
            len -= PAGE_SIZE;
            pNvmeCmd->PRP2 = pCmdInfo->prpListPhyAddr.QuadPart;

            while (len > 0) {
                *pPrpList = pCmdInfo->dblPhy.QuadPart + (PAGE_SIZE * i);
                len -= PAGE_SIZE;
                pPrpList++;
                i++;
            }
        }

        /* Pre-allacted so this had better be true! */
        ASSERT(IS_SYS_PAGE_ALIGNED(pNvmeCmd->PRP1));

        // Get Virtual address, only for read or write
        if (IS_CMD_DATA_IN(pNvmeCmd->CDW0.OPC) ||
            IS_CMD_DATA_OUT(pNvmeCmd->CDW0.OPC)) {
            /*
             * Save the dblBuff location, Srb databuff location and len
             * all in one handy location in the srb ext
             */
            StorStatus = StorPortGetSystemAddress(pAdapterExtension,
                                                  pSrbExtension->pSrb,
                                                  &pSrbExtension->pSrbDataVir);

            ASSERT(StorStatus == STOR_STATUS_SUCCESS);

            pSrbExtension->dataLen = pSrbExtension->pSrb->DataTransferLength;
            pSrbExtension->pDblVir = pCmdInfo->pDblVir;

            /*
             * For a write, copy data to the dbl buff, read data will
             * be copied out in the ISR
             */
            if (IS_CMD_DATA_OUT(pNvmeCmd->CDW0.OPC)) {
                StorPortMoveMemory(pSrbExtension->pDblVir,
                                   pSrbExtension->pSrbDataVir,
                                   pSrbExtension->dataLen);
            }
        }
    }
#else /* DBL_BUFF */
    /*
     * 3 - If a PRP list is used, copy the buildIO prepared list to the
     * preallocated memory location and update the entry not the pCmdInfo is a
     * stack var but contains a to the pre allocated mem which is what we're
     * updating.
     */
    if (pSrbExtension->prpList[0] != 0) {
        pNvmeCmd->PRP2 = pCmdInfo->prpListPhyAddr.QuadPart;
        StorPortMoveMemory((PVOID)pCmdInfo->pPRPList,
                           (PVOID)&pSrbExtension->prpList[0],
                           (pSrbExtension->numberOfPrpEntries*sizeof(UINT64)));
    }
#endif /* DBL_BUFF */

    /* 4 - Issue the Command */
    StorStatus = NVMeIssueCmd(pAdapterExtension, SubQueue, pNvmeCmd);
    ASSERT(StorStatus == STOR_STATUS_SUCCESS);

    if (StorStatus != STOR_STATUS_SUCCESS) {
        NVMeCompleteCmd(pAdapterExtension,
                        SubQueue,
                        NO_SQ_HEAD_CHANGE,
                        pNvmeCmd->CDW0.CID,
                        (PVOID)pSrbExtension);

        if (pSrbExtension->pSrb != NULL) {
            pSrbExtension->pSrb->SrbStatus = SRB_STATUS_BUSY;
            IO_StorPortNotification(RequestComplete,
                                    pAdapterExtension,
                                    pSrbExtension->pSrb);
        }
        return FALSE;
    }

    /*
     * In crashdump we poll on admin command completions
     * in order to allow our init state machine to function.
     * We don't poll on IO commands as storport will poll
     * for us and call our ISR.
     */
    if ((pAdapterExtension->ntldrDump == TRUE) &&
        (QueueType == NVME_QUEUE_TYPE_ADMIN)   &&
        (StorStatus == STOR_STATUS_SUCCESS)) {
        ULONG pollCount = 0;

        while (FALSE == NVMeIsrMsix(pAdapterExtension, NVME_ADMIN_MSG_ID)) {
            NVMeCrashDelay(pAdapterExtension->DriverState.CheckbackInterval);
            if (++pollCount > DUMP_POLL_CALLS) {
                /* a polled admin command timeout is considered fatal */
                pAdapterExtension->DriverState.DriverErrorStatus |=
                    (1 << FATAL_POLLED_ADMIN_CMD_FAILURE);
                pAdapterExtension->DriverState.NextDriverState = NVMeStateFailed;
                /*
                 * depending on whether the timer driven thread is dead or not
                 * this error may get loggged twice
                 */
                NVMeLogError(pAdapterExtension,
                    (ULONG)pAdapterExtension->DriverState.DriverErrorStatus);
                    return FALSE;
            }
        }
        return TRUE;
    }

    return TRUE;
} /* ProcessIo */

/*******************************************************************************
 * NVMeCompleteCmd
 *
 * @brief NVMeCompleteCmd gets called to recover the context saved in the
 *        associated CMD_ENTRY structure with the specificed CmdID. Normally
 *        this routine is called when the caller is about to complete the
 *        request and notify StorPort.
 *
 * @param pAE - Pointer to hardware device extension.
 * @param QueueID - Which submission queue to recover the context from
 * @param CmdID - The acquired CmdID used to de-reference the CMD_ENTRY
 * @param pContext - Caller prepared buffer to save the original context
 *
 * @return VOID
 ******************************************************************************/
VOID NVMeCompleteCmd(
    PNVME_DEVICE_EXTENSION pAE,
    USHORT QueueID,
    SHORT NewHead,
    USHORT CmdID,
    PVOID pContext
)
{
    PQUEUE_INFO pQI = &pAE->QueueInfo;
    PSUB_QUEUE_INFO pSQI = NULL;
    PCMD_ENTRY pCmdEntry = NULL;

#ifdef DBL_BUFF
    UCHAR opcode = 0;
    opcode = ((PNVME_SRB_EXTENSION)pCmdEntry->Context)->nvmeSqeUnit.CDW0.OPC;
#endif /* DBL_BUFF */

    /* Make sure the parameters are valid */
    ASSERT((QueueID <= pQI->NumSubIoQCreated) && (pContext != NULL));

    /*
     * Identify the target submission queue/cmd entry
     * and update the head pointer for the SQ if needed
     */
    pSQI = pQI->pSubQueueInfo + QueueID;

    if (NewHead != NO_SQ_HEAD_CHANGE)
        pSQI->SubQHeadPtr = NewHead;

    pCmdEntry = ((PCMD_ENTRY)pSQI->pCmdEntry) + CmdID;

    /* Ensure the command entry had been acquired */
    ASSERT(pCmdEntry->Pending == TRUE);

    /*
     * Return the original context -- this is a pointer to srb extension
     * (sanity check first that it is not NULL)
     */
    ASSERT(pCmdEntry->Context != NULL);

    *((ULONG_PTR *)pContext) = (ULONG_PTR)pCmdEntry->Context;

#ifdef DBL_BUFF
    /*
     * For non admin command read, need to copy from the dbl buff to the
     * SRB data buff
     */
    if ((QueueID > 0) && IS_CMD_DATA_IN(opcode)) {
        PNVME_SRB_EXTENSION pSrbExt = (PNVME_SRB_EXTENSION)pCmdEntry->Context;

        ASSERT(pSrbExt);

        StorPortMoveMemory(pSrbExt->pSrbDataVir,
                           pSrbExt->pDblVir,
                           pSrbExt->dataLen);
    }
#endif /* DBL_BUFF */

    /* Clear the fields of CMD_ENTRY before appending back to free list */
    pCmdEntry->Pending = FALSE;
    pCmdEntry->Context = 0;

    InsertTailList(&pSQI->FreeQList, &pCmdEntry->ListEntry);
} /* NVMeCompleteCmd */

/*******************************************************************************
 * NVMeDetectPendingCmds
 *
 * @brief NVMeDetectPendingCmds gets called to check for commands that may still
 *        be pending. Called when the caller is about to shutdown per S3 or S4.
 *
 * @param pAE - Pointer to hardware device extension.
 *
 * @return BOOLEAN
 *     TRUE if commands are detected that are still pending
 *     FALSE if no commands pending
 ******************************************************************************/
BOOLEAN NVMeDetectPendingCmds(
    PNVME_DEVICE_EXTENSION pAE
)
{
    PQUEUE_INFO pQI = &pAE->QueueInfo;
    PSUB_QUEUE_INFO pSQI = NULL;
    PCMD_ENTRY pCmdEntry = NULL;
    USHORT CmdID;
    USHORT QueueID = 0;
    PNVME_SRB_EXTENSION pSrbExtension = NULL;

    /* Search all IO queues - start after admin queue */
    for (QueueID = 1; QueueID <= pQI->NumSubIoQCreated; QueueID++) {
        pSQI = pQI->pSubQueueInfo + QueueID;

        for (CmdID = 0; CmdID < pSQI->SubQEntries; CmdID++) {
            pCmdEntry = ((PCMD_ENTRY)pSQI->pCmdEntry) + CmdID;
            if (pCmdEntry->Pending == TRUE) {
                pSrbExtension = (PNVME_SRB_EXTENSION)pCmdEntry->Context;

                /*
                 * Since pending is set, these fields should have
                 * values as well.
                 */
                ASSERT((pSrbExtension != NULL) &&
                       (pSrbExtension->pSrb != NULL));

                StorPortDebugPrint(INFO,
                    "NVMeDetectPendingCmds: pending commands detected\n");

                if ((pSrbExtension != NULL) &&
                    (pSrbExtension->pSrb != NULL))
                    return TRUE;
            }
        }
    }

    return FALSE;
} /* NVMeDetectPendingCmds */
