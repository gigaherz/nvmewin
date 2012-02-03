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
 * File: nvmeStat.c
 */

#include "precomp.h"

/*******************************************************************************
 * NVMeCallArbiter 
 * 
 * @brief Calls the init state machine arbiter either via timer callback or
 *        directly depending on crashdump or not
 * 
 * @param pAE - Pointer to adapter device extension.
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeCallArbiter(
    PNVME_DEVICE_EXTENSION pAE
)
{
    if (pAE->ntldrDump == FALSE) {
        StorPortNotification(RequestTimerCall,
                             pAE,
                             NVMeRunning,
                             pAE->StartState.CheckbackInterval);
    } else {
        NVMeCrashDelay(pAE->StartState.CheckbackInterval);
        NVMeRunning(pAE);
    }
} /* NVMeCallArbiter */

/*******************************************************************************
 * NVMeCrashDelay
 * 
 * @brief Will delay, by spinning, the amount of time specified without relying
 *        on API not available in crashdump mode.  Should not be used outside of
 *        crashdump mode.
 * 
 * @param delayInUsec - uSec to delay
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeCrashDelay(
    ULONG delayInUsec
)
{
    LARGE_INTEGER currTime;
    LARGE_INTEGER startTime;
    LARGE_INTEGER stopTime;

    /*
     * StorPortQuerySystemTime is updated every .01 and our delay should be 
     * min .05 sec.
     */
    StorPortQuerySystemTime(&startTime);

    delayInUsec *= MICRO_TO_NANO;
    stopTime.QuadPart = startTime.QuadPart + delayInUsec;

    do {
        StorPortQuerySystemTime(&currTime);
    } while (currTime.QuadPart < stopTime.QuadPart);
} /* NVMeCrashDelay */

/*******************************************************************************
 * NVMeRunningStartAttempt
 *
 * @brief NVMeRunningStartAttempt is the entry point of state machine. It is
 *        called to initialize and start the state machine. It returns the
 *        returned status from NVMeRunning to the callers.
 * 
 * @param pAE - Pointer to adapter device extension.
 * @param resetDriven - Boolean to determine if reset driven
 * @param pResetSrb - Pointer to SRB for reset
 * 
 * @return BOOLEAN
 *     TRUE: When the starting state is completed successfully
 *     FALSE: If the starting state had been determined as a failure
 ******************************************************************************/
BOOLEAN NVMeRunningStartAttempt(
    PNVME_DEVICE_EXTENSION pAE,
    BOOLEAN resetDriven,
    PSCSI_REQUEST_BLOCK pResetSrb
)
{
    /* Set up the timer interval (time per DPC callback) */
    pAE->StartState.CheckbackInterval = STORPORT_TIMER_CB_us;

    /* Initializes the state machine and its variables */
    pAE->StartState.StartErrorStatus = 0;
    pAE->StartState.NextStartState = NVMeWaitOnRDY;
    pAE->StartState.StateChkCount = 0;
    pAE->StartState.IdentifyNamespaceFetched = 0;
    pAE->StartState.InterruptCoalescingSet = FALSE;
    pAE->StartState.ConfigLbaRangeNeeded = FALSE;
    pAE->StartState.LbaRangeExamined = 0;
    pAE->StartState.NumAERsIssued = 0;
    pAE->StartState.TimeoutCounter = 0;
    pAE->StartState.resetDriven = resetDriven;
    pAE->StartState.pResetSrb = pResetSrb;

    /* Zero out SQ cn CQ counters */
    pAE->QueueInfo.NumSubIoQCreated = 0;
    pAE->QueueInfo.NumCplIoQCreated = 0;
    pAE->QueueInfo.NumSubIoQAllocFromAdapter = 0;
    pAE->QueueInfo.NumCplIoQAllocFromAdapter = 0;

    /* Zero out the LUN extensions and reset the counter as well */
    memset((PVOID)pAE->lunExtensionTable[0],
           0,
           sizeof(NVME_LUN_EXTENSION) * MAX_NAMESPACES);

    /*
     * Now, starts state machine by calling NVMeRunning
     * We won't accept IOs until the machine finishes and if it
     * fails to finish we'll never accept IOs and simply log an error
     */
    NVMeRunning(pAE);

    return (TRUE);
} /* NVMeRunningStartAttempt */

/*******************************************************************************
 * NVMeStallExecution
 *
 * @brief Stalls for the # of usecs specified
 * 
 * @param pAE - Pointer to adapter device extension.
 * @param microSeconds - time to stall
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeStallExecution(
    PNVME_DEVICE_EXTENSION pAE,
    ULONG microSeconds
)
{
    ULONG i;

    if (pAE->ntldrDump == FALSE) {
        for (i=0; i < microSeconds / MILLI_TO_MICRO; i++) {
            StorPortStallExecution(MILLI_TO_MICRO - 1);
        }
    } else {
        NVMeCrashDelay(microSeconds);
    }
}

/*******************************************************************************
 * NVMeRunning
 *
 * @brief NVMeRunning is called to dispatch the processing depending on the next
 *        state. It can be called by NVMeRunningStartAttempt or Storport to call
 *        the associated function.
 * 
 * @param pAE - Pointer to adapter device extension.
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeRunning(
    PNVME_DEVICE_EXTENSION pAE
)
{
    /*
     * Check timeout, if we fail to start (or recover from reset) then
     * we leave the controller in this state (NextStartState) and we
     * won't accept any IO.  We'll also log an error.
     */
    if (++pAE->StartState.TimeoutCounter == STATE_MACHINE_TIMEOUT_CALLS) {
        pAE->StartState.StartErrorStatus |= (1 << START_STATE_TIMEOUT_FAILURE);
        pAE->StartState.NextStartState = NVMeStartFailed;
    }

    /*
     * Otherwise, go to the next state in the Start State Machine
     * transitions are managed either in the state handler or
     * in the completion routines and executed via DPC (except crasdump)
     * calling back into this arbiter
     */
    switch (pAE->StartState.NextStartState) {
        case NVMeStartFailed:
            StorPortLogError(pAE,
                             NULL,
                             0,
                             0,
                             0,
                             SP_INTERNAL_ADAPTER_ERROR,
                             (ULONG)pAE->StartState.StartErrorStatus);

            NVMeFreeBuffers(pAE);
        break;
        case NVMeWaitOnRDY:
            NVMeRunningWaitOnRDY(pAE);
        break;
        case NVMeWaitOnIdentifyCtrl:
            NVMeRunningWaitOnIdentifyCtrl(pAE);
        break;
        case NVMeWaitOnIdentifyNS:
            NVMeRunningWaitOnIdentifyNS(pAE);
        break;
        case NVMeWaitOnSetFeatures:
            NVMeRunningWaitOnSetFeatures(pAE);
        break;
        case NVMeWaitOnSetupQueues:
            NVMeRunningWaitOnSetupQueues(pAE);
        break;
        case NVMeWaitOnAER:
            NVMeRunningWaitOnAER(pAE);
        break;
        case NVMeWaitOnIoCQ:
            NVMeRunningWaitOnIoCQ(pAE);
        break;
        case NVMeWaitOnIoSQ:
            NVMeRunningWaitOnIoSQ(pAE);
        break;
        case NVMeStartComplete:
            pAE->RecoveryAttemptPossible = TRUE;

            if (pAE->StartState.resetDriven) {
                /* If this was at the request of the host, complete that Srb */
                if (pAE->StartState.pResetSrb != NULL) {
                    pAE->StartState.pResetSrb->SrbStatus = SRB_STATUS_SUCCESS;

                    IO_StorPortNotification(RequestComplete,
                                            pAE,
                                            pAE->StartState.pResetSrb);
                }

                /* Ready again for host commands */
                StorPortReady(pAE);
            }
        break;
        default:
        break;
    } /* end switch */
} /* NVMeRunning */

/*******************************************************************************
 * NVMeRunningWaitOnRDY
 *
 * @brief NVMeRunningWaitOnRDY is called to verify if the adapter is enabled and
 *        ready to process commands. It is called by NVMeRunning when the state
 *        machine is in NVMeWaitOnRDY state.
 * 
 * @param pAE - Pointer to adapter device extension.
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeRunningWaitOnRDY(
    PNVME_DEVICE_EXTENSION pAE
)
{
    NVMe_CONTROLLER_CONFIGURATION CC;
    NVMe_CONTROLLER_STATUS        CSTS;

    /*
     * Checking to see if we're enabled yet, watching the timeout value
     * we read from the controller CAP register (StateChkCount is incr
     * in uSec in this case)
     */
    if ((pAE->StartState.StateChkCount > pAE->uSecCrtlTimeout) ||
        (pAE->StartState.StartErrorStatus)) {
        NVMeRunningStartFailed( pAE );
        return;
    }

    /*
     * Look for signs of life. If it's ready, set NextStartState to proceed to 
     * next state. Otherwise, wait for 1 sec only for crashdump case otherwise 
     * ask Storport to call again in normal driver case.
     */
    CSTS.AsUlong =
        StorPortReadRegisterUlong(pAE, &pAE->pCtrlRegister->CSTS.AsUlong);

    if (CSTS.RDY == 1) {
#ifdef CHATHAM
        CC.AsUlong =
            StorPortReadRegisterUlong(pAE, (PULONG)(&pAE->pCtrlRegister->CC));
        CC.IOCQIE = CC.ACQIE = 1;
        StorPortWriteRegisterUlong(pAE,
                                   (PULONG)(&pAE->pCtrlRegister->CC),
                                   CC.AsUlong);
#endif /* CHATHAM */
        pAE->StartState.NextStartState = NVMeWaitOnIdentifyCtrl;
        pAE->StartState.StateChkCount = 0;
    } else {
        pAE->StartState.NextStartState = NVMeWaitOnRDY;
        pAE->StartState.StateChkCount += pAE->StartState.CheckbackInterval;
    }

    NVMeCallArbiter(pAE);
} /* NVMeRunningWaitOnRDY */

/*******************************************************************************
 * NVMeRunningWaitOnIdentifyCtrl
 *
 * @brief NVMeRunningWaitOnIdentifyCtrl is called to issue Identify command to
 *        retrieve Controller structures.
 * 
 * @param pAE - Pointer to adapter device extension.
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeRunningWaitOnIdentifyCtrl(
    PNVME_DEVICE_EXTENSION pAE
)
{
    /*
     * Issue Identify command for the very first time If failed, fail the state 
     * machine 
     */
#ifndef CHATHAM
    if (NVMeGetIdentifyStructures(pAE, IDEN_CONTROLLER) == FALSE) {
        NVMeRunningStartFailed(pAE);

        return;
    }
#else /* CHATHAM */
    {
    PADMIN_IDENTIFY_NAMESPACE pIdenNS = NULL;
    ADMIN_IDENTIFY_FORMAT_DATA fData = {0};
    PNVME_LUN_EXTENSION pLunExt = NULL;

    memset(&pAE->controllerIdentifyData, 0, sizeof(ADMIN_IDENTIFY_CONTROLLER));

    pAE->controllerIdentifyData.VID = 0x8086;
    pAE->controllerIdentifyData.NN = 1;

#define CHATHAM_SERIAL "S123"
#define CHATHAM_MODEL "M123"
#define CHATHAM_FWREV "F123"

    RtlCopyMemory((UINT8*)&pAE->controllerIdentifyData.SN[0],
                  CHATHAM_SERIAL,
                  strlen(CHATHAM_SERIAL));
    RtlCopyMemory((UINT8*)&pAE->controllerIdentifyData.MN[0],
                  CHATHAM_MODEL,
                  strlen(CHATHAM_MODEL));
    RtlCopyMemory((UINT8*)&pAE->controllerIdentifyData.FR[0],
                  CHATHAM_FWREV,
                  strlen(CHATHAM_FWREV));

    pIdenNS = &pAE->lunExtensionTable[00]->identifyData;

    memset(pIdenNS, 0, sizeof(ADMIN_IDENTIFY_NAMESPACE));
    pIdenNS->NSZE = ChathamNlb;
    pIdenNS->NCAP = ChathamNlb;
    pIdenNS->NUSE = ChathamNlb;
    fData.LBADS = 9;
    pIdenNS->LBAFx[0] = fData;

    pAE->QueueInfo.NumSubIoQAllocFromAdapter = CHATHAM_NR_QUEUES;
    pAE->QueueInfo.NumCplIoQAllocFromAdapter = CHATHAM_NR_QUEUES;

    pLunExt = pAE->lunExtensionTable[0];
    pLunExt->ExposeNamespace = TRUE;
    pLunExt->ReadOnly = FALSE;

    pAE->StartState.NextStartState = NVMeWaitOnSetupQueues;
    pAE->StartState.StateChkCount = 0;
    NVMeCallArbiter(pAE);
    }
#endif /* CHATHAM */
} /* NVMeRunningWaitOnIdentifyCtrl */

/*******************************************************************************
 * NVMeRunningWaitOnIdentifyNS
 *
 * @brief NVMeRunningWaitOnIdentifyNS is called to issue Identify command to
 *        retrieve Namespace structures.
 * 
 * @param pAE - Pointer to adapter device extension.
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeRunningWaitOnIdentifyNS(
    PNVME_DEVICE_EXTENSION pAE
)
{
    /*
     * Issue an identify command.  The completion handler will keep us at this
     * state if there are more identifies needed based on what the ctlr told us
     * If failed, fail the state machine
     *
     * Please note that NN of Controller structure is 1-based.
     */
    if (NVMeGetIdentifyStructures(pAE, 
            pAE->StartState.IdentifyNamespaceFetched + 1) == FALSE) {
        NVMeRunningStartFailed(pAE);

        return;
    }
} /* NVMeRunningWaitOnIdentifyNS */

/*******************************************************************************
 * NVMeRunningWaitOnSetupQueues
 *
 * @brief Called as part of init state machine to perform alloc of queues and
 *        setup of the resouce table
 * 
 * @param pAE - Pointer to adapter device extension.
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeRunningWaitOnSetupQueues(
    PNVME_DEVICE_EXTENSION pAE
)
{
    PQUEUE_INFO pQI = &pAE->QueueInfo;
    USHORT QueueID;
    ULONG Status = STOR_STATUS_SUCCESS;

    /*
     * 1. Allocate IO queues
     * 2. Initialize IO queues
     * 3. Complete Resource Table
     *
     * If not, wait for 1 sec only for crashdump case or ask Storport to call
     * again in normal driver case
     */

    /* Allocate IO queues memory if they weren't already allocated */
    if (pAE->IoQueuesAllocated == FALSE) {
        if (NVMeAllocIoQueues( pAE ) == FALSE) {
            NVMeRunningStartFailed( pAE );
            return;
        }

        pAE->IoQueuesAllocated = TRUE;
    }

    /*
     * Now we have all resources in place, complete resource mapping table now.
     */
    if (pAE->ResourceTableMapped == FALSE) {
        NVMeCompleteResMapTbl(pAE);
        pAE->ResourceTableMapped = TRUE;
    }

    /* Once we have the queue memory allocated, initialize them */
    for (QueueID = 1; QueueID <= pQI->NumSubIoQAllocated; QueueID++) {
        /* Initialize all Submission queues/entries */
        Status = NVMeInitSubQueue(pAE, QueueID);
        if (Status != STOR_STATUS_SUCCESS) {
            NVMeRunningStartFailed(pAE);
            return;
        }
    }

    for (QueueID = 1; QueueID <= pQI->NumCplIoQAllocated; QueueID++) {
        /* Initialize all Completion queues/entries */
        Status = NVMeInitCplQueue(pAE, QueueID);
        if (Status != STOR_STATUS_SUCCESS) {
            NVMeRunningStartFailed(pAE);
            return;
        }
    }

    for (QueueID = 1; QueueID <= pQI->NumSubIoQAllocated; QueueID++) {
        /* Initialize all CMD_ENTRY entries */
        Status = NVMeInitCmdEntries(pAE, QueueID);
        if (Status != STOR_STATUS_SUCCESS) {
            NVMeRunningStartFailed(pAE);
            return;
        }
    }

    /* 
     * Skip NVMeWaitOnAER since the NVMe QEMU emulator completes these commands 
     *immediately pAE->StartState.NextStartState = NVMeWaitOnAER; 
     */
    pAE->StartState.NextStartState = NVMeWaitOnIoCQ;
    pAE->StartState.StateChkCount = 0;

    NVMeCallArbiter(pAE);
} /* NVMeRunningWaitOnSetupQueues */

/*******************************************************************************
 * NVMeRunningWaitOnSetFeatures
 *
 * @brief NVMeRunningWaitOnSetFeatures is called to issue the following
 *        commands:
 *
 *        1. Set Features command (Interrupt Coalescing, Feature ID#8)
 *        2. Set Features command (Number of Queues, Feature ID#7)
 *        3. For each existing Namespace, Get Features (LBA Range Type) first.
 *           When its Type is 00b and NLB matches the size of the Namespace,
 *           isssue Set Features (LBA Range Type) to configure:
 *             a. its Type as Filesystem,
 *             b. can be overwritten, and
 *             c. to be visible
 * 
 * @param pAE - Pointer to adapter device extension.
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeRunningWaitOnSetFeatures(
    PNVME_DEVICE_EXTENSION pAE
)
{
    PQUEUE_INFO pQI = &pAE->QueueInfo;

    /*
     * There are multiple steps hanlded in this state as they're all
     * grouped into 'set feature' type things.  This simplifies adding more
     * set features in the future as jst this sub-state machine needs updating
     */
    if (pAE->StartState.InterruptCoalescingSet == FALSE) {
        if (NVMeSetIntCoalescing(pAE) == FALSE) {
            NVMeRunningStartFailed(pAE);
            return;
        }
    } else if (pQI->NumSubIoQAllocFromAdapter == 0) {
        if (NVMeAllocQueueFromAdapter(pAE) == FALSE) {
            NVMeRunningStartFailed(pAE);
            return;
        }
    } else {
        if(NVMeAccessLbaRangeEntry(pAE) == FALSE) {
            NVMeRunningStartFailed(pAE);
            return;
        }
    }
} /* NVMeRunningWaitOnSetFeatures */

/*******************************************************************************
 * NVMeRunningWaitOnAER
 *
 * @brief NVMeRunningWaitOnAER is called to issue Asynchronous Event Request
 *        commands.
 * 
 * @param pAE - Pointer to adapter device extension.
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeRunningWaitOnAER(
    PNVME_DEVICE_EXTENSION pAE
)
{
    /*
     * Issue four Asynchronous Event Request commands by default
     * As long as it can issue one command, proceed to next state.
     * If failed to issue any, fail the state machine
     */
    if (NVMeIssueAERs( pAE, DFT_ASYNC_EVENT_REQ_NUMBER ) == 0) {
        NVMeRunningStartFailed( pAE );

        return;
    }
} /* NVMeRunningWaitOnAER */

/*******************************************************************************
 * NVMeRunningWaitOnIoCQ
 *
 * @brief NVMeRunningWaitOnIoCQ gets called to create IO completion queues via
 *        issuing Create IO Completion Queue command(s)
 * 
 * @param pAE - Pointer to adapter device extension.
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeRunningWaitOnIoCQ(
    PNVME_DEVICE_EXTENSION pAE
)
{
    PQUEUE_INFO pQI = &pAE->QueueInfo;

    /*
     * Issue Create IO Completion Queue commands when first called
     * If failed, fail the state machine
     */
    if (NVMeCreateCplQueue(pAE, (USHORT)pQI->NumCplIoQCreated + 1 ) == FALSE) {
        NVMeRunningStartFailed( pAE );

        return;
    }
} /* NVMeRunningWaitOnIoCQ */

/*******************************************************************************
 * NVMeRunningWaitOnIoSQ
 *
 * @brief NVMeRunningWaitOnIoSQ gets called to create IO submission queues via
 *        issuing Create IO Submission Queue command(s)
 * 
 * @param pAE - Pointer to adapter device extension.
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeRunningWaitOnIoSQ(
    PNVME_DEVICE_EXTENSION pAE
)
{
    PQUEUE_INFO pQI = &pAE->QueueInfo;

    /*
     * Issue Create IO Submission Queue commands when first called
     * If failed, fail the state machine
     */
    if (NVMeCreateSubQueue(pAE, (USHORT)pQI->NumSubIoQCreated + 1) == FALSE) {
        NVMeRunningStartFailed(pAE);

        return;
    }
} /* NVMeRunningWaitOnIoSQ */

/*******************************************************************************
 * NVMeRunningStartFailed
 *
 * @brief NVMeRunningStartFailed gets called to mark down the failure of state
 *        machine.
 * 
 * @param pAE - Pointer to adapter device extension.
 * 
 * @return VOID
 ******************************************************************************/
VOID NVMeRunningStartFailed(
    PNVME_DEVICE_EXTENSION pAE
)
{
    /* Update state of Start State Machine as failure */
    ASSERT(FALSE);

    StorPortDebugPrint(ERROR, "Start State Machine Fails.\n");

    pAE->StartState.NextStartState = NVMeStartFailed;
    NVMeCallArbiter(pAE);

    return;
} /* NVMeRunningStartFailed */
