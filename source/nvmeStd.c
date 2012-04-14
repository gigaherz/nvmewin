/**
 *******************************************************************************
 ** Copyright (c) 2011-2012                                                   **
 **                                                                           **
 **   Integrated Device Technology, Inc.                                      **
 **   Intel Corporation                                                       **
 **   LSI Corporation
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
 * File: nvmeStd.c
 */

#include "precomp.h"

/*******************************************************************************
 * DriverEntry
 *
 * @brief Driver entry point for Storport Miniport driver.
 *
 * @param DriverObject - The driver object associated with miniport driver
 * @param RegistryPath - Used when registering with Storport driver
 *
 * @eturn ULONG
 *     Status indicating whether successfully registering with Storport driver.
 ******************************************************************************/
ULONG DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
)
{
    HW_INITIALIZATION_DATA hwInitData = { 0 };
    ULONG Status = 0;

    /* DbgBreakPoint(); */

    /* Set size of hardware initialization structure. */
    hwInitData.HwInitializationDataSize = sizeof(HW_INITIALIZATION_DATA);

    /* Identify required miniport entry point routines. */
    hwInitData.HwInitialize = NVMeInitialize;
    hwInitData.HwStartIo = NVMeStartIo;
    hwInitData.HwInterrupt = NVMeIsrIntx;
    hwInitData.HwFindAdapter = NVMeFindAdapter;
    hwInitData.HwResetBus = NVMeResetBus;
    hwInitData.HwAdapterControl = NVMeAdapterControl;
    hwInitData.HwBuildIo = NVMeBuildIo;

    /* Specifiy adapter specific information. */
    hwInitData.AutoRequestSense = TRUE;
    hwInitData.NeedPhysicalAddresses = TRUE;
#ifndef CHATHAM
    hwInitData.NumberOfAccessRanges = NVME_ACCESS_RANGES;
#else
    hwInitData.NumberOfAccessRanges = 2;
#endif
    hwInitData.AdapterInterfaceType = PCIBus;
    hwInitData.MapBuffers = STOR_MAP_NON_READ_WRITE_BUFFERS;
    hwInitData.TaggedQueuing = TRUE;
    hwInitData.MultipleRequestPerLu = TRUE;
    hwInitData.HwDmaStarted = NULL;
    hwInitData.HwAdapterState = NULL;

    /* Set required extension sizes. */
    hwInitData.DeviceExtensionSize = sizeof(NVME_DEVICE_EXTENSION);
    hwInitData.SrbExtensionSize = sizeof(NVME_SRB_EXTENSION);

    /* Call StorPortInitialize to register with hwInitData */
    Status = StorPortInitialize(DriverObject,
                                RegistryPath,
                                &hwInitData,
                                NULL );

    StorPortDebugPrint(INFO,
                       "StorPortInitialize returns Status(0x%x)\n",
                       Status);

    return (Status);
} /* DriverEntry */

/*******************************************************************************
 * NVMeFindAdapter
 *
 * @brief This function gets called to fill in the Port Configuration
 *        Information structure that indicates more capabillites the adapter
 *        supports.
 *
 * @param Context - Pointer to hardware device extension.
 * @param Reserved1 - Unused.
 * @param Reserved2 - Unused.
 * @param ArgumentString - DriverParameter string.
 * @param pPCI - Pointer to PORT_CONFIGURATION_INFORMATION structure.
 * @param Reserved3 - Unused.
 *
 * @return ULONG
 *     Returns status based upon results of adapter parameter acquisition.
 ******************************************************************************/
ULONG
NVMeFindAdapter(
    PVOID Context,
    PVOID Reserved1,
    PVOID Reserved2,
    PCSTR ArgumentString,
    PPORT_CONFIGURATION_INFORMATION pPCI,
    UCHAR* Reserved3
)
{
    PNVME_DEVICE_EXTENSION pAE = Context;
    PACCESS_RANGE pMM_Range;
    PRES_MAPPING_TBL pRMT = NULL;

    UNREFERENCED_PARAMETER( Reserved1 );
    UNREFERENCED_PARAMETER( Reserved2 );
    UNREFERENCED_PARAMETER( Reserved3 );

    /* Initialize the hardware device extension structure. */
    memset ((void*)pAE, 0, sizeof(NVME_DEVICE_EXTENSION));

    /*
     * Get memory-mapped access range information
     * NVMe Adapter needs to request one access range for accessing
     * NVMe Controller registers
     */
    pMM_Range = NULL;
    pMM_Range = &(*(pPCI->AccessRanges))[0];

    /*
     * If desired access range is not available or
     * RangeLength isn't enough to cover all regisgters, including
     * one Admin queue pair and one IO queue pair, or
     * it's not memory-mapped, return now with SP_RETURN_NOT_FOUND
     */
    if ((pMM_Range == NULL)                             ||
        (pMM_Range->RangeLength <
         (NVME_DB_START + 2 * sizeof(NVMe_QUEUE_PAIR))) ||
        (pMM_Range->RangeInMemory == FALSE)) {
        /*
         * If no access range granted, treat it as error case and return
         * Otherwise, jump out of the loop
         */
        return (SP_RETURN_NOT_FOUND);
    }

    /* Mapping BAR memory to the virtual address of Control registers */
    pAE->pCtrlRegister = (PNVMe_CONTROLLER_REGISTERS)StorPortGetDeviceBase(pAE,
                          pPCI->AdapterInterfaceType,
                          pPCI->SystemIoBusNumber,
                          pMM_Range->RangeStart,
                          pMM_Range->RangeLength,
                          FALSE);

    if (pAE->pCtrlRegister == NULL) {
        return (SP_RETURN_NOT_FOUND);
    } else {
        /* Print out where it is */
        StorPortDebugPrint(INFO,
                           "Access Range, VirtualAddr=0x%llX.\n",
                           pAE->pCtrlRegister);
    }

#ifdef CHATHAM
    pAE->pChathamRegs = (PVOID)pAE->pCtrlRegister;
    pMM_Range = &(*(pPCI->AccessRanges))[1];
    pAE->pCtrlRegister = StorPortGetDeviceBase(pAE,
                                               pPCI->AdapterInterfaceType,
                                               pPCI->SystemIoBusNumber,
                                               pMM_Range->RangeStart,
                                               pMM_Range->RangeLength,
                                               FALSE);
#endif

    /*
     * Parse the ArgumentString to find out if it's a normal driver loading
     * If so, go on the collection resource information. Otherwise, set up
     * PORT_CONFIGURATION_INFORMATION structure and return
     */
    if (NVMeStrCompare("dump=1", ArgumentString) == TRUE)
        pAE->ntldrDump = TRUE;

    /*
     * Pre-program with default values in case of failure in accessing Registry
     * Defaultly, it can support up to 16 LUNs per target
     */
    pAE->InitInfo.Namespaces = DFT_NAMESPACES;

    /* Max transfer size is 128KB by default */
    pAE->InitInfo.MaxTxSize = DFT_TX_SIZE;
    pAE->PRPListSize = ((pAE->InitInfo.MaxTxSize / PAGE_SIZE) * sizeof(UINT64));

    /* 128 entries by default for Admin queue. */
    pAE->InitInfo.AdQEntries = DFT_AD_QUEUE_ENTRIES;

    /* 1024 entries by default for IO queues. */
    pAE->InitInfo.IoQEntries = DFT_IO_QUEUE_ENTRIES;

    /* Interrupt coalescing by default: 8 millisecond/16 completions. */
    pAE->InitInfo.IntCoalescingTime = DFT_INT_COALESCING_TIME;
    pAE->InitInfo.IntCoalescingEntry = DFT_INT_COALESCING_ENTRY;

    /* Information for accessing pciCfg space */
    pAE->SystemIoBusNumber  =  pPCI->SystemIoBusNumber;
    pAE->SlotNumber         =  pPCI->SlotNumber;

    /*
     * Access Registry and enumerate NUMA/cores topology when normal driver is
     * being loaded.
     */
    if (pAE->ntldrDump == FALSE) {
        /* Call NVMeFetchRegistry to retrieve all designated values */
        NVMeFetchRegistry(pAE);

        /*
         * Get the CPU Affinity of current system and construct NUMA table,
         * including if NUMA supported, how many CPU cores, NUMA nodes, etc
         */
        if (NVMeEnumNumaCores(pAE) == FALSE)
            return (SP_RETURN_NOT_FOUND);

        /*
         * Allocate buffer for MSI_MESSAGE_TBL structure array. If fails, return
         * FALSE.
         */
        pRMT = &pAE->ResMapTbl;
        pRMT->pMsiMsgTbl = (PMSI_MESSAGE_TBL)
            NVMeAllocatePool(pAE, (pRMT->NumActiveCores + 1) *
                             sizeof(MSI_MESSAGE_TBL));

        if (pRMT->pMsiMsgTbl == NULL)
            return (SP_RETURN_NOT_FOUND);

#ifdef COMPLETE_IN_DPC
        /*
         * Allocate buffer for DPC completiong array. If fails, return
         * FALSE.
         */
        pAE->NumDpc = pRMT->NumActiveCores + 1;
        pAE->pDpcArray = NVMeAllocatePool(pAE,
            pAE->NumDpc * sizeof(STOR_DPC));

        if ( pAE->pDpcArray == NULL )
            return (SP_RETURN_NOT_FOUND);
#endif

    }

    /* Populate all PORT_CONFIGURATION_INFORMATION fields... */
    pPCI->MaximumTransferLength = pAE->InitInfo.MaxTxSize;
    pPCI->NumberOfPhysicalBreaks = pAE->InitInfo.MaxTxSize / PAGE_SIZE;
    pPCI->NumberOfBuses = 1;
    pPCI->ScatterGather = TRUE;
    pPCI->AlignmentMask = BUFFER_ALIGNMENT_MASK;  /* Double WORD Aligned */
    pPCI->CachesData = FALSE;

    /* Support SRB_FUNCTION_RESET_DEVICE */
    pPCI->ResetTargetSupported = TRUE;

    /* Set the supported number of Targets per bus. */
    pPCI->MaximumNumberOfTargets = 1;

    /* Set the supported number of LUNs per target. */
    pPCI->MaximumNumberOfLogicalUnits = (UCHAR)pAE->InitInfo.Namespaces;

    /* Set driver to run in full duplex mode */
    pPCI->SynchronizationModel = StorSynchronizeFullDuplex;

    /* Specify the size of SrbExtension */
    pPCI->SrbExtensionSize = sizeof(NVME_SRB_EXTENSION);

    /* For 64-bit systems, controller supports 64-bit addressing, */
    if (pPCI->Dma64BitAddresses == SCSI_DMA64_SYSTEM_SUPPORTED)
        pPCI->Dma64BitAddresses = SCSI_DMA64_MINIPORT_SUPPORTED;

    pPCI->InterruptSynchronizationMode = InterruptSynchronizePerMessage;

    /* Specify NVMe MSI/MSI-X ISR here */
    pPCI->HwMSInterruptRoutine = NVMeIsrMsix;

    /* Confirm with Storport that device is found */
    return(SP_RETURN_FOUND);
} /* NVMeFindAdapter */

/*******************************************************************************
 * NVMePassiveInitialize
 *
 * @brief NVMePassiveInitialize gets called to do the following for the
 *        Controller:
 *
 *        1. Allocate memory buffers for Admin and IO queues
 *        2. Initialize the queues
 *        3. Initialize/Enable the adapter
 *        4. Construct resource mapping table
 *        5. Issue Admin commands for the initialization
 *        6. Initialize DPCs for command completions that need to free memory
 *        7. Enter Start State machine for other initialization
 *
 * @param Context - Pointer to hardware device extension.
 *
 * @return BOOLEAN
 *     TRUE - If all resources are allocated and initialized properly
 *     FALSE - If anything goes wrong
 ******************************************************************************/
BOOLEAN NVMePassiveInitialize(
    PVOID Context
)
{
    PNVME_DEVICE_EXTENSION pAE = Context;
    ULONG Status = STOR_STATUS_SUCCESS;
    PQUEUE_INFO pQI = &pAE->QueueInfo;
    PRES_MAPPING_TBL pRMT = &pAE->ResMapTbl;
    ULONG Lun;
    ULONG i;

    /* Ensure the Context is valid first */
    if (pAE == NULL)
        return (FALSE);

    /*
     * Based on the number of active cores in the system, allocate sub/cpl queue
     * info structure array first. The total number of structures should be the
     * number of active cores plus one (Admin queue).
     */
    pQI->pSubQueueInfo =
        (PSUB_QUEUE_INFO)NVMeAllocatePool(pAE, sizeof(SUB_QUEUE_INFO) *
                                          (pRMT->NumActiveCores + 1));

    if (pQI->pSubQueueInfo == NULL) {
        /* Free the allocated SUB_QUEUE_INFO structure memory */
        NVMeFreeBuffers(pAE);
        return (FALSE);
    }

    pQI->pCplQueueInfo =
        (PCPL_QUEUE_INFO)NVMeAllocatePool(pAE, sizeof(CPL_QUEUE_INFO) *
                                          (pRMT->NumActiveCores + 1));

    if (pQI->pCplQueueInfo == NULL) {
        /* Free the allocated SUB_QUEUE_INFO structure memory */
        NVMeFreeBuffers(pAE);
        return (FALSE);
    }

    /*
     * Allocate Admin queue first from NUMA node#0 by default If failed, return
     * failure.
     */
    Status = NVMeAllocQueues(pAE,
                             0,
                             pAE->InitInfo.AdQEntries,
                             0);

    if (Status != STOR_STATUS_SUCCESS) {
        /* Free the allocated SUB/CPL_QUEUE_INFO structures memory */
        NVMeFreeBuffers(pAE);
        return (FALSE);
    }

    /* Mark down the actual number of entries allocated for Admin queue */
    pQI->pSubQueueInfo->SubQEntries = pQI->NumAdQEntriesAllocated;
    pQI->pCplQueueInfo->CplQEntries = pQI->NumAdQEntriesAllocated;

    Status = NVMeInitAdminQueues(pAE);
    if (Status != STOR_STATUS_SUCCESS) {
        NVMeFreeBuffers(pAE);
        return (FALSE);
    }

    /* Allocate one SRB Extension for Start State Machine command submissions */
    pAE->DriverState.pSrbExt = NVMeAllocatePool(pAE, sizeof(NVME_SRB_EXTENSION));
    if (pAE->DriverState.pSrbExt == NULL) {
        /* Free the allocated buffers before returning */
        NVMeFreeBuffers (pAE);
        return (FALSE);
    }

    /* Allocate memory for LUN extensions */
    pAE->LunExtSize = MAX_NAMESPACES * sizeof(NVME_LUN_EXTENSION);
    pAE->lunExtensionTable[0] =
        (PNVME_LUN_EXTENSION)NVMeAllocateMem(pAE, pAE->LunExtSize, 0);

    if (pAE->lunExtensionTable[0] == NULL) {
        /* Free the allocated buffers before returning */
        NVMeFreeBuffers (pAE);
        return (FALSE);
    }

    /* Populate each LUN extension table with a valid address */
    for (Lun = 1; Lun < MAX_NAMESPACES; Lun++)
        pAE->lunExtensionTable[Lun] = pAE->lunExtensionTable[0] + Lun;

    /*
     * Allocate buffer for data transfer in Start State Machine before State
     * Machine starts
     */
    pAE->DriverState.pDataBuffer = NVMeAllocateMem(pAE, PAGE_SIZE, 0);
    if ( pAE->DriverState.pDataBuffer == NULL ) {
        /* Free the allocated buffers before returning */
        NVMeFreeBuffers(pAE);
        return (FALSE);
    }

    /* Initialize a DPC for command completions that need to free memory */
    StorPortInitializeDpc(pAE, &pAE->SntiDpc, SntiDpcRoutine);
    StorPortInitializeDpc(pAE, &pAE->AerDpc, NVMeAERDpcRoutine);
    StorPortInitializeDpc(pAE, &pAE->RecoveryDpc, RecoveryDpcRoutine);

#ifdef COMPLETE_IN_DPC
    /* Initialize DPC objects for IO completions */
    for (i = 0; i < pAE->NumDpc; i++) {
        StorPortInitializeDpc(pAE,
            (PSTOR_DPC)pAE->pDpcArray + i,
            IoCompletionDpcRoutine);
    }
#endif

    pAE->RecoveryAttemptPossible = FALSE;
    pAE->IoQueuesAllocated = FALSE;
    pAE->ResourceTableMapped = FALSE;
    pAE->LearningCores = 0;

    /*
     * Start off the state machine here, the following commands need to be
     * issued before initialization can be finalized:
     *
     *   Identify (Controller structure)
     *   Identify (Namespace structures)
     *   Asynchronous Event Requests (4 commands by default)
     *   Create IO Completion Queues
     *   Create IO Submission Queues
     *   Go through learning mode to match cores/vestors
     */
     NVMeRunningStartAttempt(pAE, FALSE, NULL);


     /*
      * Check timeout, if we fail to start (or recover from reset) then
      * we leave the controller in this state (NextDriverState) and we
      * won't accept any IO.  We'll also log an error.
      */
     while ((pAE->DriverState.NextDriverState != NVMeStartComplete) &&
            (pAE->DriverState.NextDriverState != NVMeStateFailed)){
        if (++pAE->DriverState.TimeoutCounter == STATE_MACHINE_TIMEOUT_CALLS) {
            NVMeDriverFatalError(pAE,
                                (1 << START_STATE_TIMEOUT_FAILURE));
            break;
        }
        NVMeStallExecution(pAE,STORPORT_TIMER_CB_us);
     }

     return (pAE->DriverState.NextDriverState == NVMeStartComplete) ? TRUE : FALSE;

} /* NVMePassiveInitialize */

/*******************************************************************************
 * NVMeInitialize
 *
 * @brief NVMeInitialize gets called to initialize the following resources after
 *        resetting the adpater. In normal driver loading, enable passive
 *        initialization to handle most of the it. Otherwise, initialization
 *        needs to be finished here.
 *
 *        0. Set up NUMA I/O optimizations
 *        1. Allocate non-paged system memroy queue entries, structures, etc
 *        2. Initialize queues
 *        3. Issue Admin commands to gather more adapter information
 *        4. Construct resource mapping table
 *
 * @param Context - Pointer to hardware device extension.
 *
 * @return VOID
 *     TRUE - If all resources are allocated and initialized properly
 *     FALSE - If anything goes wrong
 ******************************************************************************/
BOOLEAN NVMeInitialize(
    PVOID Context
)
{
    PNVME_DEVICE_EXTENSION pAE = Context;
    PQUEUE_INFO pQI = &pAE->QueueInfo;
    PRES_MAPPING_TBL pRMT = &pAE->ResMapTbl;
    ULONG Status = STOR_STATUS_SUCCESS;
    USHORT QueueID;
    ULONG QEntries;
    ULONG Lun;
    NVMe_CONTROLLER_CONFIGURATION CC = {0};
    NVMe_CONTROLLER_CAPABILITIES CAP;
    PERF_CONFIGURATION_DATA perfData = {0};

    /* Ensure the Context is valid first */
    if (pAE == NULL)
        return (FALSE);

    Status = StorPortInitializePerfOpts(pAE, TRUE, &perfData);
    ASSERT(STOR_STATUS_SUCCESS == Status);
    if (STOR_STATUS_SUCCESS == Status) {
        /* Allow optimization of storport DPCs */
        if (perfData.Flags & STOR_PERF_DPC_REDIRECTION) {
            perfData.Flags = STOR_PERF_DPC_REDIRECTION;
        }
        Status = StorPortInitializePerfOpts(pAE, FALSE, &perfData);
        ASSERT(STOR_STATUS_SUCCESS == Status);
    }
    /* Zero controller config, toggle EN */
    CC.EN = 1;
    StorPortWriteRegisterUlong(pAE,
                               (PULONG)(&pAE->pCtrlRegister->CC),
                               CC.AsUlong);
    CC.EN = 0;
    StorPortWriteRegisterUlong(pAE,
                               (PULONG)(&pAE->pCtrlRegister->CC),
                               CC.AsUlong);

    /*
     * Find out the Timeout value from Controller Capability register, which is
     * in 500 ms. In case the read back unit is 0, make it 1, i.e., 500 ms wait.
     * we'll store it in microseconds.
     */
    CAP.LowPart = StorPortReadRegisterUlong(pAE,
                                            (PULONG)(&pAE->pCtrlRegister->CAP));

    pAE->uSecCrtlTimeout = (ULONG)(CAP.TO * MIN_WAIT_TIMEOUT);
    pAE->uSecCrtlTimeout = (pAE->uSecCrtlTimeout == 0) ?
        MIN_WAIT_TIMEOUT : pAE->uSecCrtlTimeout;
    pAE->uSecCrtlTimeout *= MILLI_TO_MICRO;

    /*
     * NULLify all to-be-allocated buffer pointers. In failure cases we need to
     * free the buffers in NVMeFreeBuffers, it has not yet been allocated. If
     * it's NULL, nothing needs to be done.
     */
    pAE->DriverState.pSrbExt = NULL;
    pAE->lunExtensionTable[0] = NULL;
    pAE->QueueInfo.pSubQueueInfo = NULL;
    pAE->QueueInfo.pCplQueueInfo = NULL;

    /*
     * When Crashdump/Hibernation driver is being loaded, need to complete the
     * entire initialization here. In the case of normal driver loading, enable
     * passive initialization and let NVMePassiveInitialization handle the rest
     * of the initialization
     */
    if (pAE->ntldrDump == FALSE) {
        /* Enumerate granted MSI/MSI-X messages if there is any */
        if (NVMeEnumMsiMessages(pAE) == FALSE) {
            NVMeFreeBuffers(pAE);
            return (FALSE);
        }

        /* Call StorPortPassiveInitialization to enable passive init */
        StorPortEnablePassiveInitialization(pAE, NVMePassiveInitialize);

        return (TRUE);
    } else {
        /* Initialize members of resource mapping table first */
        pRMT->InterruptType = INT_TYPE_INTX;
        pRMT->NumActiveCores = 1;

        /*
         * Allocate sub/cpl queue info structure array first. The total number
         * of structures should be two, one IO queue and one Admin queue.
         */
        pQI->pSubQueueInfo =
            (PSUB_QUEUE_INFO)NVMeAllocatePool(pAE, sizeof(SUB_QUEUE_INFO) *
                                              (pRMT->NumActiveCores + 1));

        if (pQI->pSubQueueInfo == NULL) {
            NVMeFreeBuffers(pAE);
            return (FALSE);
        }

        pQI->pCplQueueInfo =
            (PCPL_QUEUE_INFO)NVMeAllocatePool(pAE, sizeof(CPL_QUEUE_INFO) *
                                              (pRMT->NumActiveCores + 1));

        if (pQI->pCplQueueInfo == NULL) {
            NVMeFreeBuffers(pAE);
            return (FALSE);
        }

        /*
         * Allocate buffers for each queue and initialize them if any failures,
         * free allocated buffers and terminate the initialization
         * unsuccessfully
         */
        for (QueueID = 0; QueueID <= pRMT->NumActiveCores; QueueID++) {
            /*
             * Based on the QueueID (0 means Admin queue, others are IO queues),
             * decide number of queue entries to allocate.  Learning mode is
             * not applicable for INTX
             */
            QEntries = (QueueID == 0) ? pAE->InitInfo.AdQEntries:
                                        pAE->InitInfo.IoQEntries;

            Status = NVMeAllocQueues(pAE,
                                     QueueID,
                                     QEntries,
                                     0);

            if (Status != STOR_STATUS_SUCCESS) {
                /* Free the allocated buffers before returning */
                NVMeFreeBuffers(pAE);
                return (FALSE);
            }

            /* Initialize Submission queue */
            Status = NVMeInitSubQueue(pAE, QueueID);
            if (Status != STOR_STATUS_SUCCESS) {
                /* Free the allocated buffers before returning */
                NVMeFreeBuffers(pAE);
                return (FALSE);
            }

            /* Initialize Completion queue */
            Status = NVMeInitCplQueue(pAE, QueueID);
            if (Status != STOR_STATUS_SUCCESS) {
                /* Free the allocated buffers before returning */
                NVMeFreeBuffers(pAE);
                return (FALSE);
            }

            /* Initialize Command Entries */
            Status = NVMeInitCmdEntries(pAE, QueueID);
            if (Status != STOR_STATUS_SUCCESS) {
                /* Free the allocated buffers before returning */
                NVMeFreeBuffers(pAE);
                return (FALSE);
            }
        }

        /* Now, conclude how many IO queue memory are allocated */
        pQI->NumSubIoQAllocated = pRMT->NumActiveCores;
        pQI->NumCplIoQAllocated = pRMT->NumActiveCores;

        /*
         * Enable adapter after initializing some controller and Admin queue
         * registers. Need to ensure the adapter is ready for processing
         * commands after entering Start State Machine.
         */
        NVMeEnableAdapter(pAE);

        /*
         * Allocate one SRB Extension for Start State Machine command
         * submissions
         */
        pAE->DriverState.pSrbExt =
            NVMeAllocatePool(pAE, sizeof(NVME_SRB_EXTENSION));

        if (pAE->DriverState.pSrbExt == NULL) {
            /* Free the allocated buffers before returning */
            NVMeFreeBuffers (pAE);
            return (FALSE);
        }

        /* Allocate memory for LUN extensions */
        pAE->LunExtSize = MAX_NAMESPACES * sizeof(NVME_LUN_EXTENSION);
        pAE->lunExtensionTable[0] =
            (PNVME_LUN_EXTENSION)NVMeAllocateMem(pAE, pAE->LunExtSize, 0);
        if (pAE->lunExtensionTable[0] == NULL) {
            /* Free the allocated buffers before returning */
            NVMeFreeBuffers(pAE);
            return (FALSE);
        }

        /* Populate each LUN extension table with valid an address */
        for (Lun = 1; Lun < MAX_NAMESPACES; Lun++)
            pAE->lunExtensionTable[Lun] = pAE->lunExtensionTable[0] + Lun;

        /*
         * Allocate buffer for data transfer in Start State Machine before State
         * Machine starts
         */
        pAE->DriverState.pDataBuffer = NVMeAllocateMem(pAE, PAGE_SIZE, 0);
        if (pAE->DriverState.pDataBuffer == NULL) {
            /* Free the allocated buffers before returning */
            NVMeFreeBuffers(pAE);
            return (FALSE);
        }

        /*
         * Start off the state machine here, the following commands need to be
         * issued before initialization can be finalized:
         *
         *   Identify (Controller structure)
         *   Identify (Namespace structures)
         *   Set Features (Feature ID# 7)
         *   Asynchronous Event Requests (4 commands by default)
         *   Create IO Completion Queues
         *   Create IO Submission Queues
         */
         return NVMeRunningStartAttempt(pAE, FALSE, NULL);
    }
} /* NVMeInitialize */

/*******************************************************************************
 * NVMeAdapterControl
 *
 * @brief NVMeAdpaterControl handles calls from the Storport to handle power
 *        type requests
 *
 * @param AdapterExtension - Pointer to device extension
 * @param ControlType - Type of adapter control request
 * @param Parameters - Parameters for the adapter control request
 *
 * @return SCSI_ADAPTER_CONTROL_STATUS
 ******************************************************************************/
SCSI_ADAPTER_CONTROL_STATUS NVMeAdapterControl(
    __in PVOID AdapterExtension,
    __in SCSI_ADAPTER_CONTROL_TYPE ControlType,
    __in PVOID Parameters
)
{
    PNVME_DEVICE_EXTENSION pAE = (PNVME_DEVICE_EXTENSION) AdapterExtension;
    PSCSI_SUPPORTED_CONTROL_TYPE_LIST pList;
    SCSI_ADAPTER_CONTROL_STATUS scsiStatus = ScsiAdapterControlSuccess;
    BOOLEAN status = FALSE;

    switch (ControlType) {
        /* Determine which control types (routines) are supported */
        case ScsiQuerySupportedControlTypes:
            /* Get pointer to control type list */
            pList = (PSCSI_SUPPORTED_CONTROL_TYPE_LIST)Parameters;

            /* Report StopAdapter and RestartAdapter are supported. */
            if (ScsiQuerySupportedControlTypes < pList->MaxControlType) {
                pList->SupportedTypeList[ScsiQuerySupportedControlTypes] = TRUE;
            }

            if (ScsiStopAdapter < pList->MaxControlType) {
                pList->SupportedTypeList[ScsiStopAdapter] = TRUE;
            }

            if (ScsiRestartAdapter < pList->MaxControlType) {
                pList->SupportedTypeList[ScsiRestartAdapter] = TRUE;
            }
        break;
        /* StopAdapter routine called just before power down of adapter */
        case ScsiStopAdapter:
            status = NVMeAdapterControlPowerDown(pAE);
            scsiStatus = (status ? ScsiAdapterControlSuccess :
                                   ScsiAdapterControlUnsuccessful);
            break;
        /*
         * Routine to reinitialize adapter while system in running. Since the
         * adapter DeviceExtension is maintained through a power management
         * cycle, we can just restore the scripts and reinitialize the chip.
         */
        case ScsiRestartAdapter:
            status = NVMeAdapterControlPowerUp(pAE);
            scsiStatus = (status ? ScsiAdapterControlSuccess :
                                    ScsiAdapterControlUnsuccessful);
        break;
        default:
            scsiStatus = ScsiAdapterControlUnsuccessful;
        break;
    } /* end of switch */

    return scsiStatus;
} /* NVMeAdapterControl */

/*******************************************************************************
 * NVMeBuildIo
 *
 * @brief NVMeBuildIo is the Storport entry function for HwStorBuildIo. All pre
 *        work that can be done on an I/O request will be done in this routine
 *        including SCSI to NVMe translation.
 *
 * @param AdapterExtension - Pointer to device extension
 * @param Srb - Pointer to SRB
 *
 * @return BOOLEAN
 *     TRUE - Indicates that HwStorStartIo shall be called on this SRB
 *     FALSE - HwStorStartIo shall not be called on this SRB
 ******************************************************************************/
BOOLEAN NVMeBuildIo(
    __in PVOID AdapterExtension,
    __in PSCSI_REQUEST_BLOCK Srb
)
{
    PNVME_DEVICE_EXTENSION pAdapterExtension =
        (PNVME_DEVICE_EXTENSION)AdapterExtension;
    PSCSI_PNP_REQUEST_BLOCK pPnpSrb = NULL;
    UCHAR Function = Srb->Function;
    SNTI_TRANSLATION_STATUS sntiStatus;
    BOOLEAN ret = FALSE;

    /*
     * Need to ensure the PathId, TargetId and Lun is supported by the
     * controller before processing the request.
     */
    if ((Srb->PathId != 0)   ||
        (Srb->TargetId != 0) ||
        (Srb->Lun >= pAdapterExtension->controllerIdentifyData.NN)) {
        Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        IO_StorPortNotification(RequestComplete, AdapterExtension, Srb);
        return FALSE;
    }

    /* Check to see if the controller has started yet */
    if ((pAdapterExtension != NULL) &&
         (pAdapterExtension->DriverState.NextDriverState != NVMeStartComplete)) {
        Srb->SrbStatus = SRB_STATUS_BUSY;
        IO_StorPortNotification(RequestComplete, AdapterExtension, Srb);
        return FALSE;
    }

    switch (Function) {
        case SRB_FUNCTION_RESET_DEVICE:
        case SRB_FUNCTION_RESET_BUS:
        case SRB_FUNCTION_RESET_LOGICAL_UNIT:
            /* Handle in startio */
            return TRUE;
        break;
        case SRB_FUNCTION_POWER:
            StorPortDebugPrint(INFO, "BuildIo: SRB_FUNCTION_POWER\n");
            /* At this point we will just return and handle it in StartIo */
            return TRUE;
        break;
        case SRB_FUNCTION_FLUSH:
            StorPortDebugPrint(INFO, "BuildIo: SRB_FUNCTION_FLUSH\n");

            Srb->SrbStatus = SRB_STATUS_SUCCESS;
            IO_StorPortNotification(RequestComplete, AdapterExtension, Srb);
            return FALSE;
        break;
        case SRB_FUNCTION_SHUTDOWN:
            StorPortDebugPrint(0, "BuildIo: SRB_FUNCTION_SHUTDOWN\n");

            /*
             * We just return here and will handle the actual shutdown in the
             * SRB_FUNCTION_POWER. When user does shutdown, Storport sends down
             * SRB_FUNCTION_SHUTDOWN and then sends down SRB_FUNCTION_POWER.
             * Whereas for the hibernate, it sends down sync cache and then
             * sends down SRB_FUNCTION_POWER.
             */
            pAdapterExtension->ShutdownInProgress = TRUE;
            Srb->SrbStatus = SRB_STATUS_SUCCESS;
            IO_StorPortNotification(RequestComplete, AdapterExtension, Srb);
            return FALSE;
        break;
        case SRB_FUNCTION_PNP:
            pPnpSrb = (PSCSI_PNP_REQUEST_BLOCK)Srb;

            StorPortDebugPrint(INFO, "BuildIo: SRB_FUNCTION_PNP\n");

            if (((pPnpSrb->SrbPnPFlags & SRB_PNP_FLAGS_ADAPTER_REQUEST) == 0) &&
                (pPnpSrb->PnPAction == StorQueryCapabilities)                 &&
                (pPnpSrb->DataTransferLength >=
                 sizeof(STOR_DEVICE_CAPABILITIES))) {
                /*
                 * Process StorQueryCapabilities request for device, not
                 * adapter. Fill in fields of STOR_DEVICE_CAPABILITIES_EX.
                 */
                PSTOR_DEVICE_CAPABILITIES pDevCapabilities =
                    (PSTOR_DEVICE_CAPABILITIES)pPnpSrb->DataBuffer;
                pDevCapabilities->Version           = 0;
                pDevCapabilities->DeviceD1          = 0;
                pDevCapabilities->DeviceD2          = 0;
                pDevCapabilities->LockSupported     = 0;
                pDevCapabilities->EjectSupported    = 0;
                pDevCapabilities->Removable         = 0;
                pDevCapabilities->DockDevice        = 0;
                pDevCapabilities->UniqueID          = 0;
                pDevCapabilities->SilentInstall     = 0;
                pDevCapabilities->SurpriseRemovalOK = 0;
                pDevCapabilities->NoDisplayInUI     = 0;

                pPnpSrb->SrbStatus = SRB_STATUS_SUCCESS;
                StorPortNotification(RequestComplete,
                                     AdapterExtension,
                                     pPnpSrb);
            } else if (((pPnpSrb->SrbPnPFlags &
                         SRB_PNP_FLAGS_ADAPTER_REQUEST) != 0)   &&
                       ((pPnpSrb->PnPAction == StorRemoveDevice) ||
                        (pPnpSrb->PnPAction == StorSurpriseRemoval))) {
                /*
                 * The adapter is going to be removed, release all resources
                 * allocated for it.
                 */
                if (pAdapterExtension->ntldrDump == FALSE) {
                    ULONG i;
                    PVOID bufferToFree = NULL;
                    /*
                    for (i = 0; i <= adapterExtension->HighestBuffer; i++) {
                        bufferToFree = adapterExtension->Buffer[i];
                        if (bufferToFree != NULL) {
                            StorPortFreePool(AdapterExtension, bufferToFree);
                        }
                    }
                    */
                }

                Srb->SrbStatus = SRB_STATUS_SUCCESS;
                IO_StorPortNotification(RequestComplete, AdapterExtension, Srb);
            } else {
                Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
                IO_StorPortNotification(RequestComplete, AdapterExtension, Srb);
            }

            /* Return FALSE so that StartIo is not called */
            return FALSE;
        break;
        case SRB_FUNCTION_IO_CONTROL:
            /* Confirm SRB data buffer is valid first */
            if (Srb->DataBuffer == NULL) {
                Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
                IO_StorPortNotification(RequestComplete, AdapterExtension, Srb);
                return FALSE;
            }

            /*
             * Call NVMeProcessIoctl to process the request. When it returns
             * IOCTL_COMPLETED, indicates complete the request back to Storport
             * right away.
             */
            if (NVMeProcessIoctl(pAdapterExtension, Srb) == IOCTL_COMPLETED) {
                IO_StorPortNotification(RequestComplete, AdapterExtension, Srb);
                return FALSE;
            }

            /* Return TRUE if more processing in StartIo needed */
            return TRUE;
        break;
        case SRB_FUNCTION_EXECUTE_SCSI:
#if DBG
            /* Debug print only - turn off for main I/O */
            StorPortDebugPrint(INFO, "BuildIo: SRB_FUNCTION_EXECUTE_SCSI\n");
#endif /* DBG */

            /*
             * An SRB that makes it to this point needs to be processed and
             * have a valid SRB Extension... initialize its contents.
             */
            NVMeInitSrbExtension((PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(Srb),
                                 pAdapterExtension,
                                 Srb);

            /* Perform SCSI to NVMe translation */
            sntiStatus = SntiTranslateCommand(pAdapterExtension,
                                              Srb);

            switch (sntiStatus) {
                case SNTI_COMMAND_COMPLETED:
                    /*
                     * Command completed in build phase, return FALSE so start
                     * I/O is not called for this command. The appropriate SRB
                     * status is already set.
                     */
                    IO_StorPortNotification(RequestComplete,
                                            AdapterExtension,
                                            Srb);

                    return FALSE;
                break;
                case SNTI_TRANSLATION_SUCCESS:
                    /*
                     * Command translation completed successfully, return TRUE
                     * so start I/O is called for this command.
                     */
                    return TRUE;
                break;
                case SNTI_FAILURE_CHECK_RESPONSE_DATA:
                case SNTI_UNSUPPORTED_SCSI_REQUEST:
                case SNTI_UNSUPPORTED_SCSI_TM_REQUEST:
                case SNTI_INVALID_SCSI_REQUEST_PARM:
                case SNTI_INVALID_SCSI_PATH_ID:
                case SNTI_INVALID_SCSI_TARGET_ID:
                case SNTI_UNRECOVERABLE_ERROR:
                    /*
                     * SNTI encountered an error during command translation, do
                     * not call start I/O for this command. The appropriate SRB
                     * status and SCSI sense data will aleady be set.
                     */
                    IO_StorPortNotification(RequestComplete,
                                            AdapterExtension,
                                            Srb);

                    return FALSE;
                break;
                default:
                    /* Invalid status returned */
                    Srb->ScsiStatus = SCSISTAT_CHECK_CONDITION;
                    Srb->SrbStatus = SRB_STATUS_ERROR;
                    IO_StorPortNotification(RequestComplete,
                                            AdapterExtension,
                                            Srb);

                    return FALSE;
                break;
            } /* switch */
        break;
        case SRB_FUNCTION_WMI:
            /* For WMI requests, just turn around and complete successfully */
            StorPortDebugPrint(INFO, "BuildIo: SRB_FUNCTION_WMI\n");

            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            IO_StorPortNotification(RequestComplete, AdapterExtension, Srb);
            return FALSE;
        break;
        default:
            /*
             * For unsupported SRB_FUNCTIONs, complete with status:
             * SRB_STATUS_INVALID_REQUEST
             */
            StorPortDebugPrint(INFO,
                               "BuildIo: Unsupported SRB Function = 0x%x\n",
                               Function);

            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            IO_StorPortNotification(RequestComplete, AdapterExtension, Srb);
            return FALSE;
        break;
    } /* end switch */

    return TRUE;
} /* NVMeBuildIo */

/*******************************************************************************
 * NVMeStartIo
 *
 * @brief NVMeStartIo is the Storport entry function for HwStorStartIo. This
 *        function will be used to process and start any I/O requests.
 *
 * @param AdapterExtension - Pointer to device extension
 * @param Srb - Pointer to SRB
 *
 * @return BOOLEAN
 *     TRUE - Required to return TRUE per MSDN
 ******************************************************************************/
BOOLEAN NVMeStartIo(
    __in PVOID AdapterExtension,
    __in PSCSI_REQUEST_BLOCK Srb
    )
{
    PNVME_DEVICE_EXTENSION pAdapterExtension =
        (PNVME_DEVICE_EXTENSION)AdapterExtension;
    PSCSI_POWER_REQUEST_BLOCK pPowerSrb = NULL;
    UCHAR Function = Srb->Function;
    PNVME_SRB_EXTENSION pSrbExtension;
    BOOLEAN status;
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl = NULL;
    PSRB_IO_CONTROL pSrbIoCtrl = NULL;
    PNVMe_COMMAND pNvmeCmd = NULL;
    PNVMe_COMMAND_DWORD_0 pNvmeCmdDW0 = NULL;
    PFORMAT_NVM_INFO pFormatNvmInfo = NULL;
    ULONG Wait = 5;

    /*
     * Initialize Variables. Determine if the request requires controller
     * resources, slot and command history. Check if command processing should
     * happen.
     */
    if (pAdapterExtension->DriverState.NextDriverState != NVMeStartComplete) {
        Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
        IO_StorPortNotification(RequestComplete, pAdapterExtension, Srb);
        return TRUE;
    }

    pSrbExtension = (PNVME_SRB_EXTENSION)Srb->SrbExtension;

    switch (Function) {
        case SRB_FUNCTION_ABORT_COMMAND:
        case SRB_FUNCTION_TERMINATE_IO:
        case SRB_FUNCTION_RESET_DEVICE:
        case SRB_FUNCTION_RESET_LOGICAL_UNIT:
        case SRB_FUNCTION_RESET_BUS:
            status = NVMeResetController(pAdapterExtension, Srb);
            if (status == FALSE) {
                Srb->SrbStatus = SRB_STATUS_ERROR;
                IO_StorPortNotification(RequestComplete, AdapterExtension, Srb);
            }
        break;
        case SRB_FUNCTION_IO_CONTROL:
            pNvmePtIoctl = (PNVME_PASS_THROUGH_IOCTL)(Srb->DataBuffer);
            pSrbIoCtrl = (PSRB_IO_CONTROL)pNvmePtIoctl;
            pNvmeCmd = (PNVMe_COMMAND)pNvmePtIoctl->NVMeCmd;
            pNvmeCmdDW0 = (PNVMe_COMMAND_DWORD_0)&pNvmePtIoctl->NVMeCmd[0];

            if (pSrbIoCtrl->ControlCode == NVME_PASS_THROUGH_SRB_IO_CODE) {
                /* More processing required for Format NVM */
                if (pNvmeCmdDW0->OPC == ADMIN_FORMAT_NVM) {
                    pFormatNvmInfo = &pAdapterExtension->FormatNvmInfo;
                    if (pFormatNvmInfo->State == FORMAT_NVM_NO_ACTIVITY) {
                        pFormatNvmInfo->State = FORMAT_NVM_RECEIVED;
                        pFormatNvmInfo->AddNamespaceNeeded = TRUE;
                    }

                    /* Save the original SRB for later completion */
                    pFormatNvmInfo->pOrgSrb = Srb;
                    if (NVMeIoctlFormatNVM(pAdapterExtension,
                                           Srb,
                                           pNvmePtIoctl) == IOCTL_COMPLETED) {
                        Srb->SrbStatus = SRB_STATUS_SUCCESS;
                        IO_StorPortNotification(RequestComplete,
                                                pAdapterExtension,
                                                Srb);
                        return TRUE;
                    }
                    /* 500ms wait for the namespace is removed completely */
                    while (Wait != 0){
                        NVMeStallExecution(pAdapterExtension, 1);
                        Wait--;
                    }
                    /*
                     * Set Format NVM State Machine as FORMAT_NVM_CMD_ISSUED
                     */
                    pFormatNvmInfo->State = FORMAT_NVM_CMD_ISSUED;
                }
            } else if (pSrbIoCtrl->ControlCode == NVME_HOT_ADD_NAMESPACE) {
                Srb->SrbStatus = SRB_STATUS_SUCCESS;
                /* Call NVMeIoctlHotAddNamespace to add the namespace */
                NVMeIoctlHotAddNamespace(pSrbExtension);
                IO_StorPortNotification(RequestComplete, pAdapterExtension, Srb);
                return TRUE;
            } else if (pSrbIoCtrl->ControlCode == NVME_HOT_REMOVE_NAMESPACE) {
                Srb->SrbStatus = SRB_STATUS_SUCCESS;
                /* Call NVMeIoctlHotRemoveNamespace to remove the namespace */
                NVMeIoctlHotRemoveNamespace(pSrbExtension);
                IO_StorPortNotification(RequestComplete, pAdapterExtension, Srb);
                return TRUE;
            } else {
                Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
                IO_StorPortNotification(RequestComplete, pAdapterExtension, Srb);
                return TRUE;
            }
            /* No processing required, just issue the command */
            if (pSrbExtension->forAdminQueue == TRUE) {
                status = ProcessIo(pAdapterExtension,
                                   pSrbExtension,
                                   NVME_QUEUE_TYPE_ADMIN);
            } else {
                status = ProcessIo(pAdapterExtension,
                                   pSrbExtension,
                                   NVME_QUEUE_TYPE_IO);
            }
        break;
        case SRB_FUNCTION_EXECUTE_SCSI:
            if (pSrbExtension->forAdminQueue == TRUE) {
                status = ProcessIo(pAdapterExtension,
                                   pSrbExtension,
                                   NVME_QUEUE_TYPE_ADMIN);
            } else {
                status = ProcessIo(pAdapterExtension,
                                   pSrbExtension,
                                   NVME_QUEUE_TYPE_IO);
            }
        break;
        case SRB_FUNCTION_POWER:
            pPowerSrb = (PSCSI_POWER_REQUEST_BLOCK)Srb;
            Srb->SrbStatus = SRB_STATUS_SUCCESS;
            status = NVMePowerControl(pAdapterExtension, pPowerSrb);
            IO_StorPortNotification(RequestComplete, pAdapterExtension, Srb);
        break;
        default:
            /*
             * For unsupported SRB, complete with status:
             * SRB_STATUS_INVALID_REQUEST
             */
            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            IO_StorPortNotification(RequestComplete, pAdapterExtension, Srb);
        break;
    }

    return TRUE;
} /* NVMeStartIo */

/*******************************************************************************
 * NVMeIsrIntx
 *
 * @brief Legacy interupt routine
 *
 * @param AdapterExtension - Pointer to device extension
 *
 * @return BOOLEAN
 *     TRUE - Indiciates successful completion
 *     FALSE - Unsuccessful completion or error
 ******************************************************************************/
BOOLEAN NVMeIsrIntx(
    __in PVOID AdapterExtension
)
{
    PNVME_DEVICE_EXTENSION pAE = (PNVME_DEVICE_EXTENSION)AdapterExtension;
    BOOLEAN InterruptClaimed = FALSE;

    pAE->IntxMasked = FALSE;

    /*
     * When no IO queue is created yet, check Admin queue only otherwise, loop
     * thru all queues in NVMeIsrMsix by specifying MsgID as 0
     */
    InterruptClaimed = NVMeIsrMsix(pAE, 0);

    /* Un-mask interrupt if it had been masked */
#ifndef CHATHAM
    if (pAE->IntxMasked == TRUE) {
        StorPortWriteRegisterUlong(pAE, &pAE->pCtrlRegister->INTMC, 1);
        pAE->IntxMasked = FALSE;
    }
#endif

    return InterruptClaimed;
}

#ifdef COMPLETE_IN_DPC
/*******************************************************************************
 * IoCompletionDpcRoutine
 *
 * @brief IO DPC completion routine; called by all IO completion objects
 *
 * @param pHwDeviceExtension - Pointer to device extension
 * @param pSystemArgument1 - MSI-X message Id
 *
 * @return void
 ******************************************************************************/
VOID
IoCompletionDpcRoutine(
    IN PSTOR_DPC  pDpc,
    IN PVOID  pHwDeviceExtension,
    IN PVOID  pSystemArgument1,
    IN PVOID  pSystemArgument2
    )
{
    PNVME_DEVICE_EXTENSION pAE = (PNVME_DEVICE_EXTENSION)pHwDeviceExtension;
    ULONG MsgID = (ULONG)pSystemArgument1;
    PNVMe_COMPLETION_QUEUE_ENTRY pCplEntry = NULL;
    PNVME_SRB_EXTENSION pSrbExtension = NULL;
    SNTI_TRANSLATION_STATUS sntiStatus = SNTI_TRANSLATION_SUCCESS;
    ULONG entryStatus = STOR_STATUS_SUCCESS;
    PMSI_MESSAGE_TBL pMMT = NULL;
    PQUEUE_INFO pQI = &pAE->QueueInfo;
    PRES_MAPPING_TBL pRMT = &pAE->ResMapTbl;
    PSUB_QUEUE_INFO pSQI = NULL;
    PCPL_QUEUE_INFO pCQI = NULL;
    USHORT firstCheckQueue = 0;
    USHORT lastCheckQueue = 0;
    USHORT indexCheckQueue;
    BOOLEAN InterruptClaimed = FALSE;
    ULONG oldIrql = 0;
    STOR_LOCK_HANDLE DpcLockhandle = { 0 };
    BOOLEAN learning;

    if (pRMT->InterruptType == INT_TYPE_INTX) {
        StorPortAcquireSpinLock(pAE, DpcLock, pDpc, &DpcLockhandle);
    } else {
        StorPortAcquireMSISpinLock(pAE, (UINT32)MsgID, (PULONG)&oldIrql);
    }

    /* Use the message id to find the correct entry in the MSI_MESSAGE_TBL */
    pMMT = pRMT->pMsiMsgTbl + MsgID;

    if (pMMT->Shared == TRUE) {
        /*
         * when Qs share an MSI, the we don't learn anything about core
         * mapping, etc., we just look through all of the queues
         */
        learning = FALSE;
        firstCheckQueue = 0;
        lastCheckQueue = (USHORT)pQI->NumCplIoQCreated;
    } else {

        /*
         * if we're done learning, lookup in our table .  Otherwise,
         * the queue #'s were initialized to match the MsgId
         */
        learning = ((pAE->LearningCores < pRMT->NumActiveCores) &&
                      (MsgID > 0)) ? TRUE : FALSE;
        if (!learning) {
            firstCheckQueue = lastCheckQueue = pMMT->CplQueueNum;
        } else {
            firstCheckQueue = lastCheckQueue = (USHORT)MsgID;
        }
    }

    for (indexCheckQueue = firstCheckQueue;
          indexCheckQueue <= lastCheckQueue;
          indexCheckQueue++) {
        pCQI = pQI->pCplQueueInfo + indexCheckQueue;
        pSQI = pQI->pSubQueueInfo + indexCheckQueue;

        do {
            entryStatus = NVMeGetCplEntry(pAE, pCQI, &pCplEntry);
            if (entryStatus == STOR_STATUS_SUCCESS) {
#ifndef CHATHAM
                /*
                 * Mask the interrupt only when first pending completed entry
                 * found.
                 */
                if ((pRMT->InterruptType == INT_TYPE_INTX) &&
                    (pAE->IntxMasked == FALSE)) {
                    StorPortWriteRegisterUlong(pAE,
                                               &pAE->pCtrlRegister->IVMS,
                                               1);

                    pAE->IntxMasked = TRUE;
                }
#endif /* CHATHAM */

                InterruptClaimed = TRUE;

#pragma prefast(suppress:6011,"This pointer is not NULL")
                NVMeCompleteCmd(pAE,
                                pCplEntry->DW2.SQID,
                                pCplEntry->DW2.SQHD,
                                pCplEntry->DW3.CID,
                                (PVOID)&pSrbExtension);

                if (pSrbExtension != NULL) {
                    BOOLEAN callStorportNotification = FALSE;

                    pSrbExtension->pCplEntry = pCplEntry;

                    /*
                     * If we're learning and this is an IO queue then update
                     * the PCT to note which QP to start using for this core
                     */
                    if (learning) {
                        PCORE_TBL pCT = NULL;
                        PQUEUE_INFO pQI = &pAE->QueueInfo;
                        PCPL_QUEUE_INFO pCQI = NULL;
                        PROCESSOR_NUMBER procNum;

                        StorPortGetCurrentProcessorNumber((PVOID)pAE,
                                 &procNum);

                        /* reference appropriate tables */
                        pCT = pRMT->pCoreTbl + procNum.Number;
                        pMMT = pRMT->pMsiMsgTbl + MsgID;
                        pCQI = pQI->pCplQueueInfo + pCT->CplQueue;

                        /* update based on current completion info */
                        pCT->MsiMsgID = (USHORT)MsgID;
                        pCQI->MsiMsgID = pCT->MsiMsgID;
                        pMMT->CplQueueNum = pCT->CplQueue;
                        pMMT->CoreNum = procNum.Number;

                        /* increment our learning counter */
                        pAE->LearningCores++;

                        /* free the read buffer for learning IO */
                        ASSERT(pSrbExtension->pDataBuffer);
                        if (NULL != pSrbExtension->pDataBuffer) {
                            StorPortFreePool((PVOID)pAE, pSrbExtension->pDataBuffer);
                        }
                    }
                    /*
                     * Only call IO_StorPortNotification if:
                     * 1) there's no comp routine, xlation status complted
                     * 2) there is a comp routine returns OK, there's
                     *    an Srb
                     * Otherwise we cont call storport's completion
                     */
                    if ((pSrbExtension->pNvmeCompletionRoutine == NULL) &&
                        (SntiMapCompletionStatus(pSrbExtension) == TRUE)) {
                        callStorportNotification = TRUE;
                    } else if ((pSrbExtension->pNvmeCompletionRoutine(pAE,
                                (PVOID)pSrbExtension) == TRUE) &&
                                (pSrbExtension->pSrb != NULL)) {
                        callStorportNotification = TRUE;
                    } else {
                        callStorportNotification = FALSE;
                    }

                    /* for async calls, call storport if needed */
                    if (callStorportNotification) {
                        IO_StorPortNotification(RequestComplete,
                                                pAE,
                                                pSrbExtension->pSrb);
                    }
                } /* If there was an SRB Extension */
            } /* If a completed command was collected */
        } while (entryStatus == STOR_STATUS_SUCCESS);

        if (InterruptClaimed == TRUE) {
            /* Now update the Completion Head Pointer via Doorbell register */
            StorPortWriteRegisterUlong(pAE,
                                       pCQI->pCplHDBL,
                                       (ULONG)pCQI->CplQHeadPtr);
            InterruptClaimed = FALSE;
        }
    } /* end for loop: for every queue to be checked */

    if (pRMT->InterruptType == INT_TYPE_INTX) {
        StorPortReleaseSpinLock(pAE, &DpcLockhandle);
    } else {
        StorPortReleaseMSISpinLock(pAE,(UINT32)MsgID, oldIrql);
    }
} /* IoCompletionDpcRoutine */


/*******************************************************************************
 * NVMeIsrMsix
 *
 * @brief MSI-X interupt routine
 *
 * @param AdapterExtension - Pointer to device extension
 * @param MsgId - MSI-X message Id to be parsed
 *
 * @return BOOLEAN
 *     TRUE - Indiciates successful completion
 *     FALSE - Unsuccessful completion or error
 ******************************************************************************/
BOOLEAN
NVMeIsrMsix (
    __in PVOID AdapterExtension,
    __in ULONG MsgID )
{
    PNVME_DEVICE_EXTENSION        pAE = (PNVME_DEVICE_EXTENSION)AdapterExtension;
    PRES_MAPPING_TBL              pRMT = &pAE->ResMapTbl;
    PMSI_MESSAGE_TBL              pMMT = pRMT->pMsiMsgTbl;
    ULONG                         qNum = 0;
    BOOLEAN                       status;

    /*
     * For shared mode, we'll use the DPC for queue 0,
     * otherwise we'll use the DPC assoiated with the known
     * queue number
     */
    if (pMMT->Shared == FALSE) {
        pMMT = pRMT->pMsiMsgTbl + MsgID;
        qNum = pMMT->CplQueueNum;
    }

    StorPortIssueDpc(pAE,
                (PSTOR_DPC)pAE->pDpcArray + qNum,
                (PVOID)MsgID,
                NULL);

    return TRUE;
}

#else /* COMPLETE_IN_DPC or in ISR */

BOOLEAN
NVMeIsrMsix (
    __in PVOID AdapterExtension,
    __in ULONG MsgID )
{
    PNVME_DEVICE_EXTENSION pAE = (PNVME_DEVICE_EXTENSION)AdapterExtension;
    PNVMe_COMPLETION_QUEUE_ENTRY pCplEntry = NULL;
    PNVME_SRB_EXTENSION pSrbExtension = NULL;
    SNTI_TRANSLATION_STATUS sntiStatus = SNTI_TRANSLATION_SUCCESS;
    ULONG entryStatus = STOR_STATUS_SUCCESS;
    PMSI_MESSAGE_TBL pMMT = NULL;
    PQUEUE_INFO pQI = &pAE->QueueInfo;
    PRES_MAPPING_TBL pRMT = &pAE->ResMapTbl;
    PSUB_QUEUE_INFO pSQI = NULL;
    PCPL_QUEUE_INFO pCQI = NULL;
    USHORT firstCheckQueue = 0;
    USHORT lastCheckQueue = 0;
    USHORT indexCheckQueue;
    BOOLEAN InterruptClaimed = FALSE;
    BOOLEAN learning;

    /* Use the message id to find the correct entry in the MSI_MESSAGE_TBL */
    pMMT = pRMT->pMsiMsgTbl + MsgID;

    if (pMMT->Shared == TRUE) {
        /*
         * when Qs share an MSI, the we don't learn anything about core
         * mapping, etc., we just look through all of the queues
         */
        learning = FALSE;
        firstCheckQueue = 0;
        lastCheckQueue = (USHORT)pQI->NumCplIoQCreated;
    } else {

        /*
         * if we're done learning, lookup in our table .  Otherwise,
         * the queue #'s were initialized to match the MsgId
         */
        learning = ((pAE->LearningCores < pRMT->NumActiveCores) &&
                      (MsgID > 0)) ? TRUE : FALSE;
        if (!learning) {
            firstCheckQueue = lastCheckQueue = pMMT->CplQueueNum;
        } else {
            firstCheckQueue = lastCheckQueue = (USHORT)MsgID;
        }
    }

    for (indexCheckQueue = firstCheckQueue;
          indexCheckQueue <= lastCheckQueue;
          indexCheckQueue++) {
        pCQI = pQI->pCplQueueInfo + indexCheckQueue;
        pSQI = pQI->pSubQueueInfo + indexCheckQueue;

        do {
            entryStatus = NVMeGetCplEntry(pAE, pCQI, &pCplEntry);
            if (entryStatus == STOR_STATUS_SUCCESS) {
#ifndef CHATHAM
                /*
                 * Mask the interrupt only when first pending completed entry
                 * found.
                 */
                if ((pRMT->InterruptType == INT_TYPE_INTX) &&
                    (pAE->IntxMasked == FALSE)) {
                    StorPortWriteRegisterUlong(pAE,
                                               &pAE->pCtrlRegister->IVMS,
                                               1);

                    pAE->IntxMasked = TRUE;
                }
#endif /* CHATHAM */

                InterruptClaimed = TRUE;

#pragma prefast(suppress:6011,"This pointer is not NULL")
                NVMeCompleteCmd(pAE,
                                pCplEntry->DW2.SQID,
                                pCplEntry->DW2.SQHD,
                                pCplEntry->DW3.CID,
                                (PVOID)&pSrbExtension);

                if (pSrbExtension != NULL) {
                    BOOLEAN callStorportNotification = FALSE;

                    pSrbExtension->pCplEntry = pCplEntry;

                    /*
                     * If we're learning and this is an IO queue then update
                     * the PCT to note which QP to start using for this core
                     */
                    if (learning) {
                        PCORE_TBL pCT = NULL;
                        PQUEUE_INFO pQI = &pAE->QueueInfo;
                        PCPL_QUEUE_INFO pCQI = NULL;
                        PROCESSOR_NUMBER procNum;

                        StorPortGetCurrentProcessorNumber((PVOID)pAE,
                                 &procNum);

                        /* reference appropriate tables */
                        pCT = pRMT->pCoreTbl + procNum.Number;
                        pMMT = pRMT->pMsiMsgTbl + MsgID;
                        pCQI = pQI->pCplQueueInfo + pCT->CplQueue;

                        /* update based on current completion info */
                        pCT->MsiMsgID = (USHORT)MsgID;
                        pCQI->MsiMsgID = pCT->MsiMsgID;
                        pMMT->CplQueueNum = pCT->CplQueue;
                        pMMT->CoreNum = procNum.Number;

                        /* increment our learning counter */
                        pAE->LearningCores++;

                        /* free the read buffer for learning IO */
                        ASSERT(pSrbExtension->pDataBuffer);
                        if (NULL != pSrbExtension->pDataBuffer) {
                            StorPortFreePool((PVOID)pAE, pSrbExtension->pDataBuffer);
                        }
                    }

                    /*
                     * Only call IO_StorPortNotification if:
                     * 1) there's no comp routine, xlation status complted
                     * 2) there is a comp routine returns OK, there's
                     *    an Srb
                     * Otherwise we cont call storport's completion
                    */
                    if ((pSrbExtension->pNvmeCompletionRoutine == NULL) &&
                        (SntiMapCompletionStatus(pSrbExtension) == TRUE)) {
                        callStorportNotification = TRUE;
                    } else if ((pSrbExtension->pNvmeCompletionRoutine(pAE,
                                (PVOID)pSrbExtension) == TRUE) &&
                                (pSrbExtension->pSrb != NULL)) {
                        callStorportNotification = TRUE;
                    } else {
                        callStorportNotification = FALSE;
                    }

                    /* for async calls, call storport if needed */
                    if (callStorportNotification) {
                        IO_StorPortNotification(RequestComplete,
                                                pAE,
                                                pSrbExtension->pSrb);
                    }
                } /* If there was an SRB Extension */
            } /* If a completed command was collected */
        } while (entryStatus == STOR_STATUS_SUCCESS);

        if (InterruptClaimed == TRUE) {
            /* Now update the Completion Head Pointer via Doorbell register */
            StorPortWriteRegisterUlong(pAE,
                                       pCQI->pCplHDBL,
                                       (ULONG)pCQI->CplQHeadPtr);
            InterruptClaimed = FALSE;
        }
    } /* end for loop: for every queue to be checked */

    return InterruptClaimed;

} /* NVMeIsrMsix */

#endif /* COMPLETE_IN_DPC or in ISR */

/*******************************************************************************
 * RecoveryDpcRoutine
 *
 * @brief DPC routine for recovery and resets
 *
 * @param pDpc - Pointer to DPC
 * @param pHwDeviceExtension - Pointer to device extension
 * @param pSystemArgument1
 * @param pSystemArgument2
 *
 * @return VOID
 ******************************************************************************/
VOID RecoveryDpcRoutine(
    IN PSTOR_DPC pDpc,
    IN PVOID pHwDeviceExtension,
    IN PVOID pSystemArgument1,
    IN PVOID pSystemArgument2
)
{
    PNVME_DEVICE_EXTENSION pAE = (PNVME_DEVICE_EXTENSION)pHwDeviceExtension;
    PSCSI_REQUEST_BLOCK pSrb = (PSCSI_REQUEST_BLOCK)pSystemArgument1;
    STOR_LOCK_HANDLE startLockhandle = { 0 };
    PQUEUE_INFO pQI = &pAE->QueueInfo;
    PSUB_QUEUE_INFO pSQI = NULL;
    ULONG index = 0;
    PNVMe_COMMAND pNVMeCmd = NULL;
    PNVME_SRB_EXTENSION pSrbExtension = NULL;

    /*
     * Get spinlocks in order, this assures we don't have submission or
     * completion threads happening before or during reset
     */
    StorPortAcquireSpinLock(pAE, StartIoLock, NULL, &startLockhandle);

    /*
     * Reset the controller; if any steps fail we just quit which
     * will leave the controller un-usable(storport queues frozen)
     * on purpose to prevent possible data corruption
     */
    if (NVMeResetAdapter(pAE) == TRUE) {
        /* 10 msec "settle" delay post reset */
        NVMeStallExecution(pAE, 10000);

        /* Complete outstanding commands on submission queues */
        StorPortNotification(ResetDetected, pAE, 0);

        for (index = 0; index <= pQI->NumSubIoQCreated; index++) {
            /* Start at tail and walk up to current head */
            pSQI = pQI->pSubQueueInfo + index;

            pNVMeCmd = (PNVMe_COMMAND)pSQI->pSubQStart;
            pNVMeCmd += ((pSQI->SubQTailPtr) == 0)
                ? (pSQI->SubQEntries - 1) : (pSQI->SubQTailPtr - 1);

            while (pSQI->SubQTailPtr != pSQI->SubQHeadPtr) {
#pragma prefast(suppress:6011,"This pointer is not NULL")
#if DBG
                DbgPrint("Complete CID 0x%x on 0x%x\n",
                         pNVMeCmd->CDW0.CID,
                         pSQI->SubQueueID);
#endif
                NVMeCompleteCmd(pAE,
                                pSQI->SubQueueID,
                                NO_SQ_HEAD_CHANGE,
                                pNVMeCmd->CDW0.CID,
                                (PVOID)&pSrbExtension);

                if ((pSrbExtension != NULL) &&
                    (pSrbExtension->pSrb != NULL)) {
                    pSrbExtension->pSrb->SrbStatus = SRB_STATUS_BUS_RESET;
                    IO_StorPortNotification(RequestComplete,
                                            pAE,
                                            pSrbExtension->pSrb);
                }

                pSQI->SubQTailPtr = ((pSQI->SubQTailPtr) == 0)
                    ? (pSQI->SubQEntries - 1) : (pSQI->SubQTailPtr - 1);

                /*
                 * Decrement tail and point to new command (one behind the
                 * tail).
                 */
                pNVMeCmd = (PNVMe_COMMAND)pSQI->pSubQStart;
                pNVMeCmd += ((pSQI->SubQTailPtr) == 0)
                    ? (pSQI->SubQEntries - 1) : (pSQI->SubQTailPtr - 1);
            } /* end while */
        } /* end for: for all submission queues */

        /*
         * Don't need to hold this anymore, we won't accept new IOs until the
         * init state machine has completed.
         */
        StorPortReleaseSpinLock(pAE, &startLockhandle);

        /* Prepare for new commands */
        if (NVMeInitAdminQueues(pAE) == STOR_STATUS_SUCCESS) {
            /*
             * Start the state mahcine, if all goes well we'll complete the
             * reset Srb when the machine is done.
             */
            NVMeRunningStartAttempt(pAE, TRUE, pSrb);
        } /* init the admin queues */
    } else {
        StorPortReleaseSpinLock(pAE, &startLockhandle);
    }  /* reset the controller */
} /* RecoveryDpcRoutine */

/*******************************************************************************
 * NVMeResetController
 *
 * @brief Main routine entry for resetting the controller
 *
 * @param pHwDeviceExtension - Pointer to device extension
 * @param pSrb - Pointer to SRB
 *
 * @return BOOLEAN
 *    TRUE - DPC was successufully issued
 *    FALSE - DPC was unsuccessufully issued or recovery not possible
 ******************************************************************************/
BOOLEAN NVMeResetController(
    __in PNVME_DEVICE_EXTENSION pAdapterExtension,
    __in PSCSI_REQUEST_BLOCK pSrb
)
{
    BOOLEAN storStatus = FALSE;

    /**
     * We only allow one recovery attempt at a time
     * if the DPC is sceduled then one has started,
     * when completed and we're ready for IOs again
     * we'll set the flag to allow recovery again.
     * Recoery runs at DPC level and grabs the startio
     * and INT spinlocks to assure no submission or
     * completion threads are in progress during reset
     */
    if (pAdapterExtension->RecoveryAttemptPossible == TRUE) {
        /* We don't want any new stoport reqeusts during reset */
        StorPortBusy(pAdapterExtension, STOR_ALL_REQUESTS);

        storStatus = StorPortIssueDpc(pAdapterExtension,
                                      &pAdapterExtension->RecoveryDpc,
                                      pSrb,
                                      NULL);

        if (storStatus == TRUE) {
            pAdapterExtension->RecoveryAttemptPossible = FALSE;
        }
    }

    return storStatus;
} /* NVMeResetController */

/*******************************************************************************
 * NVMeResetBus
 *
 * @brief Main routine entry for resetting the bus
 *
 * @param pHwDeviceExtension - Pointer to device extension
 * @param PathId - SCSI Path Id (bus)
 *
 * @return BOOLEAN
 ******************************************************************************/
BOOLEAN NVMeResetBus(
    __in PVOID AdapterExtension,
    __in ULONG PathId
)
{
    UNREFERENCED_PARAMETER(PathId);

    return NVMeResetController((PNVME_DEVICE_EXTENSION)AdapterExtension, NULL);
} /* NVMeResetBus */

/*******************************************************************************
 * NVMeAERCompletion
 *
 * @brief Asynchronous Event Request Completion routine... responsible for
 *        issuing the DPC routine to handle AERs.
 *
 * @param pDevExt - Pointer to device extension
 * @param pSrbExt - Pointer to SRB extension
 *
 * @return BOOLEAN
 *    TRUE - DPC was successufully issued
 *    FALSE - DPC was unsuccessufully issued
 ******************************************************************************/
BOOLEAN NVMeAERCompletion(
    PNVME_DEVICE_EXTENSION pDevExt,
    PNVME_SRB_EXTENSION pSrbExt
    )
{
    PNVMe_COMPLETION_QUEUE_ENTRY pCqEntry = pSrbExt->pCplEntry;
    ULONG storStatus;

    storStatus = StorPortIssueDpc(pDevExt,
                                  &pDevExt->AerDpc,
                                  pSrbExt,
                                  pCqEntry);

    /* Return false, so the ISR doesn't complete this command to StorPort */
    return FALSE;
} /* NVMeAERCompletion */

/*******************************************************************************
 * NVMeAERDpcRoutine
 *
 * @brief DPC routine for Asynchronous Event Request handling
 *
 * @param pDpc - Pointer to DPC
 * @param pHwDeviceExtension - Pointer to device extension
 * @param pSystemArgument1 - First generic argument
 * @param pSystemArgument2 - Second generic argument
 *
 * @return VOID
 ******************************************************************************/
VOID NVMeAERDpcRoutine(
    IN PSTOR_DPC pDpc,
    IN PVOID pHwDeviceExtension,
    IN PVOID pSystemArgument1,
    IN PVOID pSystemArgument2
)
{
    PNVME_SRB_EXTENSION pSrbExt = (PNVME_SRB_EXTENSION)pSystemArgument1;
    PNVMe_COMPLETION_QUEUE_ENTRY pCqEntry =
        (PNVMe_COMPLETION_QUEUE_ENTRY)pSystemArgument2;

    /*
     * If this is an AER completion, call the AER completion routine. If this
     * is a GET LOG PAGE completion, then call the GET LOG PAGE completion
     * routine.
     */
    if (pSrbExt->nvmeSqeUnit.CDW0.OPC == ADMIN_ASYNCHRONOUS_EVENT_REQUEST)
        NVMeAERCompletionRoutine(pSrbExt, pCqEntry);
    else if (pSrbExt->nvmeSqeUnit.CDW0.OPC == ADMIN_GET_LOG_PAGE)
        NVMeAERGetLogPageCompletionRoutine(pSrbExt, pCqEntry);
    else
        ASSERT(FALSE);
} /* NVMeAERDpcRoutine */

/*******************************************************************************
 * NVMeAERCompletionRoutine
 *
 * @brief Asynchronous Event Request Completion routine... responsible for
 *        handling AER requests (running at DPC level)
 *
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pCqEntry - Pointer to completion queue entry
 *
 * @return VOID
 ******************************************************************************/
VOID NVMeAERCompletionRoutine(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVMe_COMPLETION_QUEUE_ENTRY pCqEntry
)
{
    PNVME_DEVICE_EXTENSION pDevExt = pSrbExt->pNvmeDevExt;
    PADMIN_ASYNCHRONOUS_EVENT_REQUEST_COMPLETION_DW0 pCqeDword0;
    STOR_PHYSICAL_ADDRESS physAddr;
    PHYSICAL_ADDRESS lowestAddr;
    PHYSICAL_ADDRESS highestAddr;
    PHYSICAL_ADDRESS boundaryAddr;
    MEMORY_CACHING_TYPE cacheType;
    NODE_REQUIREMENT preferredNode;
    ULONG allocStatus;
    ULONG paLength;
    UCHAR logPage;
    UCHAR logPageSize = 0;

    /* DEBUG */
    ASSERT((pSrbExt != NULL) && (pCqEntry != NULL));

    /* Extract AER Info */
    pCqeDword0 =
        (PADMIN_ASYNCHRONOUS_EVENT_REQUEST_COMPLETION_DW0)pCqEntry->DW0;
    logPage = (UCHAR)pCqeDword0->AssociatedLogPage;

    if (logPage == ERROR_INFORMATION) {
        logPageSize =
            (UCHAR)sizeof(ADMIN_GET_LOG_PAGE_ERROR_INFORMATION_LOG_ENTRY);
    } else if (logPage == SMART_HEALTH_INFORMATION) {
        logPageSize = (UCHAR)
            sizeof(ADMIN_GET_LOG_PAGE_SMART_HEALTH_INFORMATION_LOG_ENTRY);
    } else if (logPage == FIRMWARE_SLOT_INFORMATION) {
        logPageSize = (UCHAR)
            sizeof(ADMIN_GET_LOG_PAGE_FIRMWARE_SLOT_INFORMATION_LOG_ENTRY);
    } else {
        ASSERT(FALSE);
    }

    /* Log the Error Status from CQE - DWORD 0 */
    StorPortDebugPrint(INFO,
                       "AER Notification: Event Type - %d, Event Info - %d\n",
                       pCqeDword0->AsynchronousEventType,
                       pCqeDword0->AsynchronousEventInformation);

    /* Check to see if there is a persistant internal device error */
    if ((pCqeDword0->AsynchronousEventType == ERROR_INFORMATION) &&
        (pCqeDword0->AsynchronousEventInformation ==
         PERSISTENT_INTERNAL_DEVICE_ERROR)) {
        /*
         * A failure occurred within the device that is persistent or the device
         * is unable to isolate to a specific set of commands. If this error is
         * indicated, then the CSTS.CFS bit may be set to '1' and the host
         * should perform a rest as described in 7.3.
         */
    }

    /* We can re-use the same AER SRB Extension for the NVME GET LOG PAGE */
    memset(pSrbExt, 0, sizeof(NVME_SRB_EXTENSION));

    /* Build NVME GET LOG PAGE command - Using the AER SRB Extension */
    lowestAddr.QuadPart = 0;                   /* Full physical address range */
    highestAddr.QuadPart = 0xFFFFFFFFFFFFFFFF; /* Full physical address range */
    cacheType = MmNonCached;                   /* Non-Cached memory for DMA */
    preferredNode = 0;                         /* Default to Node 0 */
    boundaryAddr.QuadPart = 0;                 /* No aligned boundaries */

    /* Get physically contigous memory for the PRP entry (GET LOG PAGE cmd) */
    allocStatus = StorPortAllocateContiguousMemorySpecifyCacheNode(
                      pDevExt, (size_t)logPageSize, lowestAddr,
                      highestAddr, boundaryAddr, cacheType, preferredNode,
                      pSrbExt->pDataBuffer);

    if (allocStatus == STOR_STATUS_SUCCESS) {
        pSrbExt->pParentIo = NULL;
        pSrbExt->pChildIo = NULL;

        /* Set up the GET LOG PAGE command */
        memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));
        pSrbExt->nvmeSqeUnit.CDW0.OPC = ADMIN_GET_LOG_PAGE;
        pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
        pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;

        /* DWORD 10 */
        pSrbExt->nvmeSqeUnit.CDW10 = 0;
        pSrbExt->nvmeSqeUnit.CDW10 |= (logPageSize << BYTE_SHIFT_2) &
                                       LOG_PAGE_NUM_DWORDS_MASK;
        pSrbExt->nvmeSqeUnit.CDW10 |= (logPage & DWORD_MASK_LOW_WORD);

        /* Set the completion routine */
        pSrbExt->pNvmeCompletionRoutine =
            (PNVME_COMPLETION_ROUTINE)NVMeAERCompletion;

        physAddr = StorPortGetPhysicalAddress(pDevExt,
                                              NULL,
                                              pSrbExt->pDataBuffer,
                                              &paLength);

        if (physAddr.QuadPart != 0) {
            pSrbExt->nvmeSqeUnit.PRP1 = physAddr.QuadPart;
            pSrbExt->nvmeSqeUnit.PRP2 = 0;

            /* Issue the Get Log Page command */
            if (ProcessIo(pDevExt, pSrbExt, NVME_QUEUE_TYPE_ADMIN) == FALSE) {
                StorPortDebugPrint(
                    INFO,
                    "AER DPC: ProcessIo GET LOG PAGE failed (pSrbExt = 0x%x)\n",
                    pSrbExt);
            }
        } else {
            StorPortDebugPrint(
                INFO,
                "AER DPC: Get Phy Addr GET LOG PAGE failed (pSrbExt = 0x%x)\n",
                pSrbExt);
        }
    } else {
        StorPortDebugPrint(INFO,
                           "StorPortAllocateContiguousMemorySpecifyCacheNode ");
        StorPortDebugPrint(INFO,
                           "for GET LOG PAGE failed (pSrbExt = 0x%x)\n",
                           pSrbExt);
    }
} /* NVMeAERCompletionRoutine */

/*******************************************************************************
 * NVMeAERGetLogPageCompletionRoutine
 *
 * @brief Asynchronous Event Request Get Log Page completion routine
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pCqEntry - Pointer to completion queue entry
 *
 * @return VOID
 ******************************************************************************/
VOID NVMeAERGetLogPageCompletionRoutine(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVMe_COMPLETION_QUEUE_ENTRY pCqEntry
)
{
    PNVME_DEVICE_EXTENSION pDevExt = pSrbExt->pNvmeDevExt;
    USHORT logPageId;
    USHORT logPageSize;

    PADMIN_GET_LOG_PAGE_ERROR_INFORMATION_LOG_ENTRY
        pErrorInfoLogPage = NULL;
    PADMIN_GET_LOG_PAGE_SMART_HEALTH_INFORMATION_LOG_ENTRY
        pSmartHealthLogPage = NULL;
    PADMIN_GET_LOG_PAGE_FIRMWARE_SLOT_INFORMATION_LOG_ENTRY
        pFwSlotLogPage = NULL;

    logPageId = (USHORT)pSrbExt->nvmeSqeUnit.CDW10 & DWORD_MASK_LOW_WORD;
    logPageSize = (USHORT)((pSrbExt->nvmeSqeUnit.CDW10 >> BYTE_SHIFT_2) &
                           LOG_PAGE_NUM_DWORDS_MASK);

    if (logPageId == ERROR_INFORMATION) {
        pErrorInfoLogPage =
            (PADMIN_GET_LOG_PAGE_ERROR_INFORMATION_LOG_ENTRY)
                pSrbExt->pDataBuffer;
    } else if (logPageId == SMART_HEALTH_INFORMATION) {
        pSmartHealthLogPage =
            (PADMIN_GET_LOG_PAGE_SMART_HEALTH_INFORMATION_LOG_ENTRY)
                pSrbExt->pDataBuffer;
    } else if (logPageId == FIRMWARE_SLOT_INFORMATION) {
        pFwSlotLogPage =
            (PADMIN_GET_LOG_PAGE_FIRMWARE_SLOT_INFORMATION_LOG_ENTRY)
                pSrbExt->pDataBuffer;
    } else {
        ASSERT(FALSE);
    }

    /* Release the buffer in the SRB Extension */
    StorPortFreeContiguousMemorySpecifyCache(pDevExt,
                                             pSrbExt->pDataBuffer,
                                             logPageSize,
                                             MmNonCached);

    /* Now, reissue the AER command again (use the same SRB Extension) */
    memset(pSrbExt, 0, sizeof(NVME_SRB_EXTENSION));
    pSrbExt->pParentIo = NULL;
    pSrbExt->pChildIo = NULL;

    /* Set up the AER command */
    memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));
    pSrbExt->nvmeSqeUnit.CDW0.OPC = ADMIN_ASYNCHRONOUS_EVENT_REQUEST;
    pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
    pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;

    /* Set the completion routine */
    pSrbExt->pNvmeCompletionRoutine =
        (PNVME_COMPLETION_ROUTINE)NVMeAERCompletion;

    pSrbExt->nvmeSqeUnit.PRP1 = 0;
    pSrbExt->nvmeSqeUnit.PRP2 = 0;

    /* Re-Issue the AER command */
    if (ProcessIo(pDevExt, pSrbExt, NVME_QUEUE_TYPE_ADMIN) == FALSE) {
        StorPortDebugPrint(
            INFO,
            "AER DPC Routine: ProcessIo for AER failed (pSrbExt = 0x%x)\n",
            pSrbExt);
    }
} /* NVMeAERGetLogPageCompletionRoutine */

/*******************************************************************************
 * NVMeInitSrbExtension
 *
 * @brief Helper function to initialize the SRB extension
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pDevExt - Pointer to device extension
 * @param pSrb - Pointer to SRB
 *
 * @return VOID
 ******************************************************************************/
VOID NVMeInitSrbExtension(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_DEVICE_EXTENSION pDevExt,
    PSCSI_REQUEST_BLOCK pSrb
)
{
    memset(pSrbExt, 0, sizeof(NVME_SRB_EXTENSION));

    pSrbExt->pNvmeDevExt = pDevExt;
    pSrbExt->pSrb = pSrb;

    /* Any future initializations go here... */
} /* NVMeInitSrbExtension */

/*******************************************************************************
 * NVMeIoctlCallback
 *
 * @brief NVMeIoctlCallback is the callback function used to complete IOCTL
 *        requests. This routine needs to finish the following before returning
 *        to Storport:
 *
 *        - Modify DataTransferLength of SRB as the sum of ReturnBufferLen and
 *          sizeof(NVME_PASS_THROUGH_IOCTL) if transferring data from device to
 *          host.
 *        - Modify DataTransferLength of SRB as the
 *          sizeof(NVME_PASS_THROUGH_IOCTL) if transferring data from host to
 *          device.
 *        - Fill CplEntry of NVME_PASS_THROUGH_IOCTL with the entire completion
 *          entry before completing the request back to Storport.
 *        - Always mark SrbStatus and ReturnCode of SRB_IO_CONTROL as SUCCESS to
 *          hint user applications to exam the completion entry.
 *
 * @param pAE - Pointer to hardware device extension.
 * @param pSrbExtension - Pointer to SRB extension
 *
 * @return BOOLEAN
 *     TRUE - If all resources are allocated and initialized properly
 *     FALSE - If anything goes wrong
 ******************************************************************************/
BOOLEAN NVMeIoctlCallback(
    PVOID pNVMeDevExt,
    PVOID pSrbExtension
)
{
    PNVME_SRB_EXTENSION pSrbExt = (PNVME_SRB_EXTENSION)pSrbExtension;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl = NULL;

    pNvmePtIoctl = (PNVME_PASS_THROUGH_IOCTL)(pSrb->DataBuffer);

    /* Adjust the SRB transfer length */
    if (pNvmePtIoctl->Direction == NVME_FROM_HOST_TO_DEV) {
        pSrb->DataTransferLength = sizeof(NVME_PASS_THROUGH_IOCTL);
    } else if (pNvmePtIoctl->Direction == NVME_FROM_DEV_TO_HOST) {
        pSrb->DataTransferLength = pNvmePtIoctl->ReturnBufferLen;
    }

    /* Copy the completion entry to NVME_PASS_THROUGH_IOCTL structure */
    StorPortMoveMemory((PVOID)pNvmePtIoctl->CplEntry,
                       (PVOID)pSrbExt->pCplEntry,
                       sizeof(NVMe_COMPLETION_QUEUE_ENTRY));

    /*
     * Mark down the ReturnCode in SRB_IO_CONTROL as NVME_IOCTL_SUCCESS
     * and SrbStatus as SRB_STATUS_SUCCESS before returning.
     */
    pNvmePtIoctl->SrbIoCtrl.ReturnCode = NVME_IOCTL_SUCCESS;
    pSrb->SrbStatus = SRB_STATUS_SUCCESS;

    return TRUE;
} /* NVMeIoctlCallback */

/*******************************************************************************
 * NVMeIoctlGetLogPage
 *
 * @brief NVMeIoctlGetLogPage handles the GET LOG PAGE command
 *
 * @param pDevExt - Pointer to hardware device extension.
 * @param pSrb - Pointer to SRB
 * @param pNvmePtIoctl - Pointer to pass through IOCTL
 *
 * @return BOOLEAN
 *     TRUE - If all resources are allocated and initialized properly
 *     FALSE - If anything goes wrong
 ******************************************************************************/
BOOLEAN NVMeIoctlGetLogPage(
    PNVME_DEVICE_EXTENSION pDevExt,
    PSCSI_REQUEST_BLOCK pSrb,
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl
)
{
    PSRB_IO_CONTROL pSrbIoCtrl = (PSRB_IO_CONTROL)pNvmePtIoctl;
    PADMIN_GET_LOG_PAGE_COMMAND_DW10 pGetLogPageDW10 = NULL;
    ULONG DataBufferSize = 0;
    ULONG IoctlHdrSize = sizeof(NVME_PASS_THROUGH_IOCTL);

    pGetLogPageDW10 =
        (PADMIN_GET_LOG_PAGE_COMMAND_DW10)&pNvmePtIoctl->NVMeCmd[10];
    DataBufferSize = pGetLogPageDW10->NUMD * sizeof(ULONG);

    /*
     * Ensure the size of return buffer is big enough to accommodate the header
     * and log.
     */
    if (pNvmePtIoctl->ReturnBufferLen < (IoctlHdrSize + DataBufferSize - 1)) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_INSUFFICIENT_OUT_BUFFER;
        return IOCTL_COMPLETED;
    }

    /* Prepare the PRP entries for the transfer */
    if (NVMePreparePRPs(pDevExt,
                        (PNVMe_COMMAND)pNvmePtIoctl->NVMeCmd,
                        pNvmePtIoctl->DataBuffer,
                        DataBufferSize) == FALSE) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_PRP_TRANSLATION_ERROR;
        return IOCTL_COMPLETED;
    }

    pSrbIoCtrl->ReturnCode = NVME_IOCTL_SUCCESS;

    return IOCTL_PENDING;
} /* NVMeIoctlGetLogPage */

/*******************************************************************************
 * NVMeIoctlIdentify
 *
 * @brief NVMeIoctlIdentify handles the Identify command
 *
 * @param pDevExt - Pointer to hardware device extension.
 * @param pSrb - Pointer to SRB
 * @param pNvmePtIoctl - Pointer to pass through IOCTL
 *
 * @return BOOLEAN
 *     TRUE - If all resources are allocated and initialized properly
 *     FALSE - If anything goes wrong
 ******************************************************************************/
BOOLEAN NVMeIoctlIdentify(
    PNVME_DEVICE_EXTENSION pDevExt,
    PSCSI_REQUEST_BLOCK pSrb,
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl
)
{
    PSRB_IO_CONTROL pSrbIoCtrl = (PSRB_IO_CONTROL)pNvmePtIoctl;
    PADMIN_IDENTIFY_COMMAND_DW10 pIdentifyDW10 = NULL;
    ULONG DataBufferSize = 0;
    ULONG IoctlHdrSize = sizeof(NVME_PASS_THROUGH_IOCTL);

    pIdentifyDW10 = (PADMIN_IDENTIFY_COMMAND_DW10)&pNvmePtIoctl->NVMeCmd[10];
    if (pIdentifyDW10->CNS == 0)
        DataBufferSize = sizeof(ADMIN_IDENTIFY_NAMESPACE);
    else
        DataBufferSize = sizeof(ADMIN_IDENTIFY_CONTROLLER);

    /*
     * Ensure the size of return buffer is big enough to accommodate the header
     * and log.
     */
    if (pNvmePtIoctl->ReturnBufferLen < (IoctlHdrSize + DataBufferSize - 1)) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_INSUFFICIENT_OUT_BUFFER;
        return IOCTL_COMPLETED;
    }

    /* Prepare the PRP entries for the transfer */
    if (NVMePreparePRPs(pDevExt,
                        (PNVMe_COMMAND)pNvmePtIoctl->NVMeCmd,
                        pNvmePtIoctl->DataBuffer,
                        DataBufferSize) == FALSE) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_PRP_TRANSLATION_ERROR;
        return IOCTL_COMPLETED;
    }

    pSrbIoCtrl->ReturnCode = NVME_IOCTL_SUCCESS;

    return IOCTL_PENDING;
} /* NVMeIoctlIdentify */

/*******************************************************************************
 * NVMeIoctlFwDownload
 *
 * @brief NVMeIoctlFwDownload handles the FW Download command
 *
 * @param pDevExt - Pointer to hardware device extension.
 * @param pSrb - Pointer to SRB
 * @param pNvmePtIoctl - Pointer to pass through IOCTL
 *
 * @return BOOLEAN
 *     TRUE - If all resources are allocated and initialized properly
 *     FALSE - If anything goes wrong
 ******************************************************************************/
BOOLEAN NVMeIoctlFwDownload(
    PNVME_DEVICE_EXTENSION pDevExt,
    PSCSI_REQUEST_BLOCK pSrb,
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl
)
{
    PSRB_IO_CONTROL pSrbIoCtrl = (PSRB_IO_CONTROL)pNvmePtIoctl;
    PADMIN_FIRMWARE_IMAGE_DOWNLOAD_COMMAND_DW10 pFirmwareImageDW10 = NULL;
    ULONG DataBufferSize = 0;
    ULONG IoctlHdrSize = sizeof(NVME_PASS_THROUGH_IOCTL);
    PADMIN_IDENTIFY_CONTROLLER pAdminCntlrData =
        &pDevExt->controllerIdentifyData;
    USHORT supportsFwActFwDwnld = 0;

    supportsFwActFwDwnld =
        pAdminCntlrData->OACS.SupportsFirmwareActivateFirmwareDownload;

    pFirmwareImageDW10 = (PADMIN_FIRMWARE_IMAGE_DOWNLOAD_COMMAND_DW10)
                         &pNvmePtIoctl->NVMeCmd[10];

    DataBufferSize = pFirmwareImageDW10->NUMD * sizeof(ULONG);

    /* Ensure the command is supported */
    if (supportsFwActFwDwnld == 0) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_UNSUPPORTED_ADMIN_CMD;

        return IOCTL_COMPLETED;
    }

    /* Ensure the size of input buffer is big enough to accommodate the header
     * and image.
     */
    if (pNvmePtIoctl->DataBufferLen < DataBufferSize) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_INSUFFICIENT_IN_BUFFER;

        return IOCTL_COMPLETED;
    }

    /* Prepare the PRP entries for the transfer */
    if (NVMePreparePRPs(pDevExt,
                        (PNVMe_COMMAND)pNvmePtIoctl->NVMeCmd,
                        pNvmePtIoctl->DataBuffer,
                        DataBufferSize) == FALSE) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_PRP_TRANSLATION_ERROR;
        return IOCTL_COMPLETED;
    }

    pSrbIoCtrl->ReturnCode = NVME_IOCTL_SUCCESS;

    return IOCTL_PENDING;
} /* NVMeIoctlFwDownload */

/*******************************************************************************
 * NVMeIoctlSetGetFeatures
 *
 * @brief NVMeIoctlSetGetFeatures handles the Get/Set Features command
 *
 * @param pDevExt - Pointer to hardware device extension.
 * @param pSrb - Pointer to SRB
 * @param pNvmePtIoctl - Pointer to pass through IOCTL
 * @param OPC - opcode
 *
 * @return BOOLEAN
 *     TRUE - If all resources are allocated and initialized properly
 *     FALSE - If anything goes wrong
 ******************************************************************************/
BOOLEAN NVMeIoctlSetGetFeatures(
    PNVME_DEVICE_EXTENSION pDevExt,
    PSCSI_REQUEST_BLOCK pSrb,
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl,
    UCHAR OPC
)
{
    PSRB_IO_CONTROL pSrbIoCtrl = (PSRB_IO_CONTROL)pNvmePtIoctl;
    PADMIN_SET_FEATURES_COMMAND_DW10 pSetFeaturesDW10 = NULL;
    ULONG DataBufferSize = 0;
    ULONG IoctlHdrSize = sizeof(NVME_PASS_THROUGH_IOCTL);

    pSetFeaturesDW10 =
        (PADMIN_SET_FEATURES_COMMAND_DW10)&pNvmePtIoctl->NVMeCmd[10];

    /* Reject Number of Queues via Set Features command */
    if ((OPC == ADMIN_SET_FEATURES) &&
        (pSetFeaturesDW10->FID == NUMBER_OF_QUEUES)) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_UNSUPPORTED_ADMIN_CMD;

        return IOCTL_COMPLETED;
    }

    /* Only LBA_RANGE_TYPE feature needs pre-processing here */
    if (pSetFeaturesDW10->FID == LBA_RANGE_TYPE) {
        DataBufferSize =
            sizeof(ADMIN_SET_FEATURES_LBA_COMMAND_RANGE_TYPE_ENTRY);

        /* Ensure the size of input/output buffer is big enough */
        if (OPC == ADMIN_SET_FEATURES) {
            if (pNvmePtIoctl->DataBufferLen < DataBufferSize) {
                pSrbIoCtrl->ReturnCode = NVME_IOCTL_INSUFFICIENT_IN_BUFFER;

                return IOCTL_COMPLETED;
            }
        } else if (OPC == ADMIN_GET_FEATURES) {
            if (pNvmePtIoctl->ReturnBufferLen <
                (DataBufferSize + IoctlHdrSize - 1)) {
                pSrbIoCtrl->ReturnCode = NVME_IOCTL_INSUFFICIENT_OUT_BUFFER;

                return IOCTL_COMPLETED;
            }
        } else {
            pSrbIoCtrl->ReturnCode = NVME_IOCTL_UNSUPPORTED_ADMIN_CMD;
            return IOCTL_COMPLETED;
        }

        /* Prepare the PRP entries for the transfer */
        if (NVMePreparePRPs(pDevExt,
                            (PNVMe_COMMAND)pNvmePtIoctl->NVMeCmd,
                            pNvmePtIoctl->DataBuffer,
                            DataBufferSize) == FALSE) {
            pSrbIoCtrl->ReturnCode = NVME_IOCTL_PRP_TRANSLATION_ERROR;
            return IOCTL_COMPLETED;
        }
    }

    pSrbIoCtrl->ReturnCode = NVME_IOCTL_SUCCESS;

    return IOCTL_PENDING;
} /* NVMeIoctlSetGetFeatures */

/*******************************************************************************
 * NVMeIoctlSecuritySendRcv
 *
 * @brief NVMeIoctlSecuritySendRcv handles the Secuirty Send/Rcv commands
 *
 * @param pDevExt - Pointer to hardware device extension.
 * @param pSrb - Pointer to SRB
 * @param pNvmePtIoctl - Pointer to pass through IOCTL
 * @param OPC - opcode
 *
 * @return BOOLEAN
 *     TRUE - If all resources are allocated and initialized properly
 *     FALSE - If anything goes wrong
 ******************************************************************************/
BOOLEAN NVMeIoctlSecuritySendRcv(
    PNVME_DEVICE_EXTENSION pDevExt,
    PSCSI_REQUEST_BLOCK pSrb,
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl,
    UCHAR OPC
)
{
    PSRB_IO_CONTROL pSrbIoCtrl = (PSRB_IO_CONTROL)pNvmePtIoctl;
    PNVM_SECURITY_SEND_COMMAND_DW11 pSecuritySendDW11 = NULL;
    ULONG IoctlHdrSize = sizeof(NVME_PASS_THROUGH_IOCTL);
    ULONG DataBufferSize = 0;
    USHORT supportsSecSendRcv = 0;
    PADMIN_IDENTIFY_CONTROLLER pAdminCntlrData =
        &pDevExt->controllerIdentifyData;

    supportsSecSendRcv =
        pAdminCntlrData->OACS.SupportsSecuritySendSecurityReceive;

    /* Ensure the command is supported */
    if (supportsSecSendRcv == 0) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_UNSUPPORTED_ADMIN_CMD;
        return IOCTL_COMPLETED;
    }

    pSecuritySendDW11 =
        (PNVM_SECURITY_SEND_COMMAND_DW11)&pNvmePtIoctl->NVMeCmd[11];

    DataBufferSize = pSecuritySendDW11->TL;

    /* Ensure the size of input/output buffer is big enough */
    if (OPC == ADMIN_SECURITY_SEND) {
        if (pNvmePtIoctl->DataBufferLen < DataBufferSize) {
            pSrbIoCtrl->ReturnCode = NVME_IOCTL_INSUFFICIENT_IN_BUFFER;

            return IOCTL_COMPLETED;
        }
    } else if (OPC == ADMIN_SECURITY_RECEIVE) {
        if (pNvmePtIoctl->ReturnBufferLen <
            (DataBufferSize + IoctlHdrSize - 1)) {
            pSrbIoCtrl->ReturnCode = NVME_IOCTL_INSUFFICIENT_OUT_BUFFER;

            return IOCTL_COMPLETED;
        }
    } else {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_UNSUPPORTED_ADMIN_CMD;

        return IOCTL_COMPLETED;
    }

    /* Prepare the PRP entries for the transfer */
    if (NVMePreparePRPs(pDevExt,
                        (PNVMe_COMMAND)pNvmePtIoctl->NVMeCmd,
                        pNvmePtIoctl->DataBuffer,
                        DataBufferSize) == FALSE) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_PRP_TRANSLATION_ERROR;

        return IOCTL_COMPLETED;
    }

    pSrbIoCtrl->ReturnCode = NVME_IOCTL_SUCCESS;

    return IOCTL_PENDING;
} /* NVMeIoctlSecuritySendRcv */

/*******************************************************************************
 * NVMeIoctlCompare
 *
 * @brief NVMeIoctlCompare handles the Compare command
 *
 * @param pDevExt - Pointer to hardware device extension.
 * @param pSrb - Pointer to SRB
 * @param pNvmePtIoctl - Pointer to pass through IOCTL
 *
 * @return BOOLEAN
 *     TRUE - If all resources are allocated and initialized properly
 *     FALSE - If anything goes wrong
 ******************************************************************************/
BOOLEAN NVMeIoctlCompare(
    PNVME_DEVICE_EXTENSION pDevExt,
    PSCSI_REQUEST_BLOCK pSrb,
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl
)
{
    PNVME_SRB_EXTENSION pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    PNVME_LUN_EXTENSION pLunExt = NULL;
    PSRB_IO_CONTROL pSrbIoCtrl = (PSRB_IO_CONTROL)pNvmePtIoctl;
    PNVM_COMPARE_COMMAND_DW12 pCompareDW12 = NULL;
    ULONG DataBufferSize = 0;
    ULONG IoctlHdrSize = sizeof(NVME_PASS_THROUGH_IOCTL);
    UINT8 flbas = 0;
    ULONG lbaLength = 0, lbaLengthPower = 0;

    if (GetLunExtension(pSrbExt, &pLunExt) != SNTI_SUCCESS) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_INVALID_PATH_TARGET_ID;
        return IOCTL_COMPLETED;
    } else {
        pCompareDW12 = (PNVM_COMPARE_COMMAND_DW12)&pNvmePtIoctl->NVMeCmd[12];

        /* Need to figure out the byte size of LBA first */
        flbas = pLunExt->identifyData.FLBAS.SupportedCombination;
        lbaLengthPower = pLunExt->identifyData.LBAFx[flbas].LBADS;
        lbaLength = 1 << lbaLengthPower;

        /* Figure out the data size */
        DataBufferSize = lbaLength * (pCompareDW12->NLB + 1);

        /**
         * Ensure the size of input buffer is big enough to accommodate the
         * header and image.
         */
        if (pNvmePtIoctl->DataBufferLen < DataBufferSize) {
            pSrbIoCtrl->ReturnCode = NVME_IOCTL_INSUFFICIENT_IN_BUFFER;

            return IOCTL_COMPLETED;
        }

        /* Prepare the PRP entries for the transfer */
        if (NVMePreparePRPs(pDevExt,
                            (PNVMe_COMMAND)pNvmePtIoctl->NVMeCmd,
                            pNvmePtIoctl->DataBuffer,
                            DataBufferSize) == FALSE) {
            pSrbIoCtrl->ReturnCode = NVME_IOCTL_PRP_TRANSLATION_ERROR;
            return IOCTL_COMPLETED;
        }
    }

    pSrbIoCtrl->ReturnCode = NVME_IOCTL_SUCCESS;

    return IOCTL_PENDING;
} /* NVMeIoctlCompare */

/*******************************************************************************
 * NVMeIoctlDataSetManagement
 *
 * @brief NVMeIoctlDataSetManagement handles the DSM command
 *
 * @param pDevExt - Pointer to hardware device extension.
 * @param pSrb - Pointer to SRB
 * @param pNvmePtIoctl - Pointer to pass through IOCTL
 *
 * @return BOOLEAN
 *     TRUE - If all resources are allocated and initialized properly
 *     FALSE - If anything goes wrong
 ******************************************************************************/
BOOLEAN NVMeIoctlDataSetManagement(
    PNVME_DEVICE_EXTENSION pDevExt,
    PSCSI_REQUEST_BLOCK pSrb,
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl
)
{
    PSRB_IO_CONTROL pSrbIoCtrl = (PSRB_IO_CONTROL)pNvmePtIoctl;
    PNVM_DATASET_MANAGEMENT_COMMAND_DW10 pDatasetManagementDW10 = NULL;
    ULONG DataBufferSize = 0;
    ULONG IoctlHdrSize = sizeof(NVME_PASS_THROUGH_IOCTL);

    pDatasetManagementDW10 = (PNVM_DATASET_MANAGEMENT_COMMAND_DW10)
                             &pNvmePtIoctl->NVMeCmd[12];

    /* Need to figure out the byte size data */
    DataBufferSize = (pDatasetManagementDW10->NR + 1) *
                      sizeof(NVM_DATASET_MANAGEMENT_RANGE);

    /*
     * Ensure the size of input buffer is big enough to accommodate the header
     * and image.
     */
    if (pNvmePtIoctl->DataBufferLen < DataBufferSize) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_INSUFFICIENT_IN_BUFFER;

        return IOCTL_COMPLETED;
    }

    /* Prepare the PRP entries for the transfer */
    if (NVMePreparePRPs(pDevExt,
                        (PNVMe_COMMAND)pNvmePtIoctl->NVMeCmd,
                        pNvmePtIoctl->DataBuffer,
                        DataBufferSize) == FALSE) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_PRP_TRANSLATION_ERROR;

        return IOCTL_COMPLETED;
    }

    pSrbIoCtrl->ReturnCode = NVME_IOCTL_SUCCESS;

    return IOCTL_PENDING;
} /* NVMeIoctlDataSetManagement */

/******************************************************************************
 * NVMeIoctlHotRemoveNamespace
 *
 * @brief This function calls StorPortNotification with BusChangeDetected to
 *        force a bus re-enumeration after removing a namespace which used to
 *        be seen by Windows system.
 *
 * @param pSrbExt - Pointer to Srb Extension allocated in SRB.
 *
 * @return None
 ******************************************************************************/
VOID NVMeIoctlHotRemoveNamespace (
    PNVME_SRB_EXTENSION pSrbExt
)
{
    PNVME_DEVICE_EXTENSION pDevExt = pSrbExt->pNvmeDevExt;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl = NULL;
    PNVMe_COMMAND pNvmeCmd = NULL;
    ULONG NS = 0;

    pNvmePtIoctl = (PNVME_PASS_THROUGH_IOCTL)pSrb->DataBuffer;
    pNvmeCmd = (PNVMe_COMMAND)pNvmePtIoctl->NVMeCmd;

    /*
     * If we're in the middle of formatting NVM,
     * simply reject current request:
     * SrbStatus = SRB_STATUS_INVALID_REQUEST
     * ReturCode = NVME_IOCTL_UNSUPPORTED_OPERATION
     */
    if ((pDevExt->FormatNvmInfo.State != FORMAT_NVM_RECEIVED) &&
        (pDevExt->FormatNvmInfo.State != FORMAT_NVM_NO_ACTIVITY)) {
        pNvmePtIoctl = (PNVME_PASS_THROUGH_IOCTL)(pSrb->DataBuffer);
        pNvmePtIoctl->SrbIoCtrl.ReturnCode = NVME_IOCTL_UNSUPPORTED_OPERATION;
        pSrb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        return;
    }

    /* Set the namespace(s) as unavailable now */
    if (pNvmeCmd->NSID == ALL_NAMESPACES_APPLIED) {
        /*
         * Set all supported namespace(s) as unavailable
         * by setting TargetLun bits
         */
        pDevExt->FormatNvmInfo.TargetLun = 0;
        for (NS = 0; NS < pDevExt->controllerIdentifyData.NN; NS++)
            pDevExt->FormatNvmInfo.TargetLun |= (1 << NS);

        /*
         * Set the specific bits of ObsoleteNS as 1
         * to indicate which Namespace structure is obsolete
         */
        pDevExt->FormatNvmInfo.ObsoleteNS = pDevExt->FormatNvmInfo.TargetLun;

    } else {
        /*
         * Set the specified namespace as unavailable
         * by setting the associated TargetLun bit
         */
        pDevExt->FormatNvmInfo.TargetLun = (1 << (pNvmeCmd->NSID - 1));
        /*
         * Set the specific bit of ObsoleteNS as 1
         * to indicate which Namespace structure is obsolete
         */
        pDevExt->FormatNvmInfo.ObsoleteNS = pDevExt->FormatNvmInfo.TargetLun;
    }

    /* Now we are in namespace hot remove state. */
    pDevExt->FormatNvmInfo.State = FORMAT_NVM_NS_REMOVED;
    /*
     * Force bus re-enumeration,
     * then, Windows won't see the target namespace(s)
     */
    StorPortNotification(BusChangeDetected, pDevExt);
} /* NVMeIoctlHotRemoveNamespace */

/******************************************************************************
 * NVMeIoctlHotAddNamespace
 *
 * @brief This function calls StorPortNotification with BusChangeDetected to
 *        force a bus re-enumeration to let Windows discover the newly added
 *        namespace.
 *
 *
 * @param pSrbExt - Pointer to Srb Extension allocated in SRB.
 *
 * @return None
 ******************************************************************************/
VOID NVMeIoctlHotAddNamespace (
    PNVME_SRB_EXTENSION pSrbExt
)
{
    PNVME_DEVICE_EXTENSION pDevExt = pSrbExt->pNvmeDevExt;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl = NULL;

    /*
     * If haven't received NVME_PASS_THROUGH_SRB_IO_CODE(Format NVM) or
     * NVME_HOT_REMOVE_NAMESPACE request, reject current request:
     * SrbStatus = SRB_STATUS_SUCCESS
     * ReturCode = NVME_IOCTL_UNSUPPORTED_OPERATION
     */
    if ((pDevExt->FormatNvmInfo.State == FORMAT_NVM_NO_ACTIVITY) ||
        (pDevExt->FormatNvmInfo.TargetLun == 0)) {
        pNvmePtIoctl = (PNVME_PASS_THROUGH_IOCTL)(pSrb->DataBuffer);
        pNvmePtIoctl->SrbIoCtrl.ReturnCode = NVME_IOCTL_UNSUPPORTED_OPERATION;
        pSrb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        return;
    }
    /*
     * Clear the Pending bit(s) as zero after noting down the formatted Lun(s).
     * Clear bit(s) of FormattedLun when bus is re-enumerated via Inquiry cmds.
     * Complete the request when FormattedLun is zero.
     */
    pDevExt->FormatNvmInfo.FormattedLun = pDevExt->FormatNvmInfo.TargetLun;
    pDevExt->FormatNvmInfo.TargetLun = 0;

    /* Now we are in namespace hot add state. */
    pDevExt->FormatNvmInfo.State = FORMAT_NVM_NS_ADDED;

    /*
     * Force bus re-enumeration
     * and report the available namespace(s) via Inquiry commands
     */
    StorPortNotification(BusChangeDetected, pDevExt);
} /* NVMeIoctlHotAddNamespace */



/******************************************************************************
 * FormatNVMGetIdentify
 *
 * @brief This helper function re-uses the submission entry in Srb Extension
 *        to issue Identify commands by calling ProcessIo.
 *
 *
 * @param pSrbExt - Pointer to Srb Extension allocated in SRB.
 * @param NamespaceID - Specifies which structure to retrieve.
 *
 * @return TRUE - Indicates the request should be completed due to certain errors.
 *         FALSE - Indicates more processing required and no error detected.
 ******************************************************************************/
BOOLEAN FormatNVMGetIdentify(
    PNVME_SRB_EXTENSION pSrbExt,
    ULONG NamespaceID
)
{
    PNVME_DEVICE_EXTENSION pAE = pSrbExt->pNvmeDevExt;
    PNVMe_COMMAND pIdentify = NULL;
    PADMIN_IDENTIFY_CONTROLLER pIdenCtrl = &pAE->controllerIdentifyData;
    PADMIN_IDENTIFY_NAMESPACE pIdenNS = NULL;
    PADMIN_IDENTIFY_COMMAND_DW10 pIdentifyCDW10 = NULL;
    BOOLEAN Status = TRUE;

    /* Populate submission entry fields */
    pIdentify = &pSrbExt->nvmeSqeUnit;
    pIdentify->CDW0.OPC = ADMIN_IDENTIFY;

    if (NamespaceID == IDEN_CONTROLLER) {
        /* Indicate it's for Controller structure */
        pIdentifyCDW10 = (PADMIN_IDENTIFY_COMMAND_DW10) &pIdentify->CDW10;
        pIdentifyCDW10->CNS = 1;
        /* Prepare PRP entries, need at least one PRP entry */
        if (NVMePreparePRPs(pAE, pIdentify,
                            (PVOID)pIdenCtrl,
                            sizeof(ADMIN_IDENTIFY_CONTROLLER)) == FALSE)
            Status = FALSE;
    } else {
        if (pIdenCtrl == NULL) {
            Status = FALSE;
        } else {
            /* Assign the destination buffer for retrieved structure */
            pIdenNS = &pAE->lunExtensionTable[NamespaceID - 1]->identifyData;
            /* Namespace ID is 1-based. */
            pIdentify->NSID = NamespaceID;
            /* Prepare PRP entries, need at least one PRP entry */
            if (NVMePreparePRPs(pAE, pIdentify,
                                (PVOID)pIdenNS,
                                sizeof(ADMIN_IDENTIFY_CONTROLLER)) == FALSE)
                Status = FALSE;
        }
    }
    /* Complete the request now if something goes wrong */
    if (Status == FALSE) {
        pSrbExt->pSrb->SrbStatus = SRB_STATUS_ERROR;
        return Status;
    }
    /* Now issue the command via Admin Doorbell register */
    return ProcessIo(pAE, pSrbExt, NVME_QUEUE_TYPE_ADMIN);
} /* FormatNVMGetIdentify */


/******************************************************************************
 * FormatNVMFailure
 *
 * @brief This helper function handles the failure of completions and
 *        preparations Format NVM and Identify commands in
 *        NVMeIoctlFormatNVMCallback:
 *        Depending on the value of AddNamespaceNeeded:
 *        If TRUE, call NVMeIoctlHotAddNamespace to force bus re-enumeration
 *        and add back the formatted namespace(s).
 *        If FALSE, simply complete the request. User applications need to
 *        issue a NVME_HOT_ADD_NAMESPACE request to add back namespace(s).
 *
 * @param pNVMeDevExt - Pointer to hardware device extension.
 * @param pSrbExten - Pointer to Srb Extension allocated in SRB.
 *
 * @return None.
 *
 ******************************************************************************/
BOOLEAN FormatNVMFailure(
    PNVME_DEVICE_EXTENSION pDevExt,
    PNVME_SRB_EXTENSION pSrbExt
)
{
    PFORMAT_NVM_INFO pFormatNvmInfo = &pDevExt->FormatNvmInfo;

    /*
     * Depends on AddNamespaceNeeded:
     * If TRUE, add back the namespace(s) via calling NVMeIoctlHotAddNamespace.
     *          and return FALSE since the request will be completed later.
     * If FALSE, Clear the FORMAT_NVM_INFO structure and return TRUE
     *          to let caller complete the request.
     */
    if (pFormatNvmInfo->AddNamespaceNeeded == TRUE) {
        /* Need to add back namespace(s) first */
        NVMeIoctlHotAddNamespace(pSrbExt);
        return FALSE;
    } else {
        /*
         * Reset FORMAT_NVM_INFO structure to zero
         * since the request is completed
         */
        memset((PVOID)pFormatNvmInfo, 0, sizeof(FORMAT_NVM_INFO));
        return TRUE;
    }
}


/******************************************************************************
 * NVMeIoctlFormatNVMCallback
 *
 * @brief This function handles the Format NVM State Machine when commands are
 *        completed and ISR is called. It also re-use the Srb Extension to
 *        issue Identify commands to re-fetch Controller and specific structures
 *        after Format NVM command had been completed successfully.
 *
 * @param pNVMeDevExt - Pointer to hardware device extension.
 * @param pSrbExtension - Pointer to Srb Extension allocated in SRB.
 *
 * @return TRUE - Indicates the request should be completed due to errors.
 *         FALSE - Indicates more processing required and no error detected.
 ******************************************************************************/
BOOLEAN NVMeIoctlFormatNVMCallback(
    PVOID pNVMeDevExt,
    PVOID pSrbExtension
)
{
    PNVME_DEVICE_EXTENSION pDevExt = (PNVME_DEVICE_EXTENSION)pNVMeDevExt;
    PNVME_SRB_EXTENSION pSrbExt = (PNVME_SRB_EXTENSION)pSrbExtension;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl = NULL;
    PFORMAT_NVM_INFO pFormatNvmInfo = &pDevExt->FormatNvmInfo;
    ULONG NS;

    pNvmePtIoctl = (PNVME_PASS_THROUGH_IOCTL)(pSrb->DataBuffer);

    switch (pFormatNvmInfo->State) {
        case FORMAT_NVM_CMD_ISSUED:
            /*
             * If it's not completed successfully,
             * complete it and notify Storport
             */
            if ((pSrbExt->pCplEntry->DW3.SF.SC != 0) ||
                (pSrbExt->pCplEntry->DW3.SF.SCT != 0)) {
                /*
                 * Copy the completion entry to
                 * NVME_PASS_THROUGH_IOCTL structure
                 */
                StorPortMoveMemory((PVOID)pNvmePtIoctl->CplEntry,
                                   (PVOID)pSrbExt->pCplEntry,
                                   sizeof(NVMe_COMPLETION_QUEUE_ENTRY));
                /*
                 * Format NVM fails, need to complete the request now.
                 */
                return FormatNVMFailure(pDevExt, pSrbExt);
            } else {
                /* Re-use the SrbExt to fetch Identify Controller structure */
                memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));
                if (FormatNVMGetIdentify(pSrbExt, IDEN_CONTROLLER) == FALSE) {
                    /*
                     * Can't issue Identify command, complete the request now
                     */
                    return FormatNVMFailure(pDevExt, pSrbExt);
                }
                /* Now, Identify/Controller command is issued successfully. */
                pFormatNvmInfo->State = FORMAT_NVM_IDEN_CONTROLLER_ISSUED;
                return FALSE;
            }
        break;
        case FORMAT_NVM_IDEN_CONTROLLER_ISSUED:
            /**
             * If it's not completed successfully,
             * complete it and notify Storport
             */
            if ((pSrbExt->pCplEntry->DW3.SF.SC != 0) ||
                (pSrbExt->pCplEntry->DW3.SF.SCT != 0)) {
                /*
                 * Copy the completion entry to
                 * NVME_PASS_THROUGH_IOCTL structure
                 */
                StorPortMoveMemory((PVOID)pNvmePtIoctl->CplEntry,
                                   (PVOID)pSrbExt->pCplEntry,
                                   sizeof(NVMe_COMPLETION_QUEUE_ENTRY));
                /*
                 * Identify command fails, need to complete the request now.
                 */
                return FormatNVMFailure(pDevExt, pSrbExt);
            } else {
                /*
                 * Identify Controller structure is ready now.
                 * Re-use the SrbExt to fetch Identify Namespace structure(s)
                 * one at a time.
                 * Need to figure out which namespace structure to fetch.
                 */
                for (NS = 0; NS < pDevExt->controllerIdentifyData.NN; NS++) {
                    if ((pFormatNvmInfo->ObsoleteNS & (1 << NS)) == (1 << NS)) {
                        /* Clear the pending bit */
                        pFormatNvmInfo->ObsoleteNS &= ~(1 << NS);
                        break;
                    }
                }
                if (NS >= pDevExt->controllerIdentifyData.NN) {
                    /*
                     * Succeeded !
                     * No more Namespace struture to fetch.
                     * Depending on AddNamespaceNeeded,
                     * if TRUE, call NVMeIoctlHotAddNamespace to force
                     * bus re-enumeration and the request will be completed
                     * afterwards...
                     * If FALSE, complete the request now.
                     */
                    pNvmePtIoctl->SrbIoCtrl.ReturnCode = NVME_IOCTL_SUCCESS;
                    if (pDevExt->FormatNvmInfo.AddNamespaceNeeded == TRUE){
                        NVMeIoctlHotAddNamespace(pSrbExt);
                        return FALSE;
                    } else {
                        pSrb->SrbStatus = SRB_STATUS_SUCCESS;
                        /*
                         * Maintain the state and
                         * let caller complete the request
                         */
                        return TRUE;
                    }
                }
                memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));
                if (FormatNVMGetIdentify(pSrbExt, NS+1) == FALSE) {
                    /*
                     * Can't issue Identify command, complete the request now
                     */
                    return FormatNVMFailure(pDevExt, pSrbExt);
                }
                return FALSE;
            }
        break;
        case FORMAT_NVM_NS_REMOVED:
        case FORMAT_NVM_NS_ADDED:
        default:
        break;
    } /* state switch */
    /* Don't complete the request back to Storport just yet */
    return FALSE;
} /* NVMeIoctlFormatNVMCallback */

/******************************************************************************
 * NVMeIoctlFormatNVM
 *
 * @brief This function is the entry point handling Format NVM command:
 *        1. It does some checkings and initial settings
 *        2. Calls NVMeIoctlHotRemoveNamespace if no NVME_HOT_REMOVE_NAMESPACE
 *           previously received.
 *        3. NVMeStartIo will issue Format NVM command later.
 *
 * @param pDevext - Pointer to hardware device extension.
 * @param pSrb - This parameter specifies the SCSI I/O request.
 * @param pNvmePtIoctl - Pointer to NVME_PASS_THROUGH_IOCTL Structure
 *
 * @return IOCTL_COMPLETED - Indicates the request should be completed due to
 *                           certain errors.
 *         IOCTL_PEDNING - Indicates more processing required.
 ******************************************************************************/
BOOLEAN NVMeIoctlFormatNVM (
    PNVME_DEVICE_EXTENSION pDevExt,
    PSCSI_REQUEST_BLOCK pSrb,
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl
)
{
    PSRB_IO_CONTROL pSrbIoCtrl = (PSRB_IO_CONTROL)pNvmePtIoctl;
    PNVMe_COMMAND pNvmeCmd = (PNVMe_COMMAND)pNvmePtIoctl->NVMeCmd;
    ULONG IoctlHdrSize = sizeof(NVME_PASS_THROUGH_IOCTL);
    PNVME_SRB_EXTENSION pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    ULONG TargetLun = 0;
    ULONG NS = 0;

    /* Ensure the command is supported */
    if (pDevExt->controllerIdentifyData.OACS.SupportsFormatNVM == 0) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_UNSUPPORTED_ADMIN_CMD;
        return IOCTL_COMPLETED;
    }
    /*
     * Ensure the Namespace ID is valid:
     * It's 1-based.
     * It can be 0xFFFFFFFF for all namespaces
     */
    if ((pNvmeCmd->NSID == 0) ||
        ((pNvmeCmd->NSID != ALL_NAMESPACES_APPLIED) &&
         (pNvmeCmd->NSID > pDevExt->controllerIdentifyData.NN))) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_INVALID_NAMESPACE_ID;
        return IOCTL_COMPLETED;
    }

    /*
     * When no namespace had been removed yet,
     * call NVMeIoctlHotRemoveNamespace to remove the namespace(s) first
     */
    if (pDevExt->FormatNvmInfo.State == FORMAT_NVM_RECEIVED) {
        NVMeIoctlHotRemoveNamespace(pSrbExt);
    } else if (pDevExt->FormatNvmInfo.State == FORMAT_NVM_NS_REMOVED) {
        /*
         * The namespace(s) already removed, need to check if NamespaceID
         * is matched. If not, return error
         */
        if (pNvmeCmd->NSID == ALL_NAMESPACES_APPLIED){
            for (NS = 0; NS < pDevExt->controllerIdentifyData.NN; NS++)
                TargetLun |= (1 << NS);
            if (pDevExt->FormatNvmInfo.TargetLun != TargetLun) {
                pSrbIoCtrl->ReturnCode = NVME_IOCTL_INVALID_NAMESPACE_ID;
                return IOCTL_COMPLETED;
            }
        } else {
            if (pDevExt->FormatNvmInfo.TargetLun != (1 << (pNvmeCmd->NSID - 1))) {
                pSrbIoCtrl->ReturnCode = NVME_IOCTL_INVALID_NAMESPACE_ID;
                return IOCTL_COMPLETED;
            }
        }
    } else {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_FORMAT_NVM_FAILED;
        return IOCTL_COMPLETED;
    }
    /*
     * Pre-set ReturnCode as NVME_IOCTL_FORMAT_NVM_FAILED in case
     * the request gets completed too earlier (error case).
     */
    pSrbIoCtrl->ReturnCode = NVME_IOCTL_FORMAT_NVM_FAILED;

    return IOCTL_PENDING;
} /* NVMeIoctlFormatNVM */


/*******************************************************************************
 * NVMeProcessIoctl
 *
 * @brief Processing function for pass through IOCTLs
 *
 * @param pDevExt - Pointer to hardware device extension.
 * @param pSrb - Pointer to SRB
 *
 * @return BOOLEAN
 *     TRUE - If all resources are allocated and initialized properly
 *     FALSE - If anything goes wrong
 ******************************************************************************/
BOOLEAN NVMeProcessIoctl(
    PNVME_DEVICE_EXTENSION pDevExt,
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PNVME_PASS_THROUGH_IOCTL pNvmePtIoctl = NULL;
    PSRB_IO_CONTROL pSrbIoCtrl = NULL;
    PNVMe_COMMAND pNvmeCmd = NULL;
    PNVMe_COMMAND_DWORD_0 pNvmeCmdDW0 = NULL;
    ULONG IoctlHdrSize = sizeof(NVME_PASS_THROUGH_IOCTL);
    PADMIN_IDENTIFY_CONTROLLER pCntrlIdData;
    USHORT supportFwActFwDl;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    pNvmePtIoctl = (PNVME_PASS_THROUGH_IOCTL)(pSrb->DataBuffer);
    pSrbIoCtrl = (PSRB_IO_CONTROL)pNvmePtIoctl;

    /*
     * If the Signature is invalid, note it in ReturnCode of SRB_IO_CONTROL and
     * return.
     */
    if (strncmp((const char*)pSrbIoCtrl->Signature,
                NVME_SIG_STR,
                NVME_SIG_STR_LEN) != 0) {
        pSrb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_INVALID_SIGNATURE;
        return IOCTL_COMPLETED;
    }

    StorPortDebugPrint(INFO,
                       "NVMeProcessIoctl: Code = 0x%x, Signature = 0x%s\n",
                       pSrbIoCtrl->ControlCode,
                       &pSrbIoCtrl->Signature[0]);

    /* Initialize SRB Extension */
    NVMeInitSrbExtension(pSrbExt, pDevExt, pSrb);

    switch (pSrbIoCtrl->ControlCode) {
        case NVME_PASS_THROUGH_SRB_IO_CODE:

    /*
     * If the input buffer length is not big enough, note it in ReturnCode of
     * SRB_IO_CONTROL and return.
     */
    if (pSrb->DataTransferLength < IoctlHdrSize) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_INSUFFICIENT_IN_BUFFER;
        return IOCTL_COMPLETED;
    }

    /*
     * If return buffer length is less than size of NVME_PASS_THROUGH_IOCTL,
     * note it in ReturnCode of SRB_IO_CONTROL and return
     */
    if (pNvmePtIoctl->ReturnBufferLen < IoctlHdrSize) {
        pSrbIoCtrl->ReturnCode = NVME_IOCTL_INSUFFICIENT_OUT_BUFFER;
        return IOCTL_COMPLETED;
    }

    /* Process the request based on the Control code */
    pNvmeCmd = (PNVMe_COMMAND)pNvmePtIoctl->NVMeCmd;
    pNvmeCmdDW0 = (PNVMe_COMMAND_DWORD_0)&pNvmePtIoctl->NVMeCmd[0];

            /* Separate Admin and NVM commands via QueueId */
            if (pNvmePtIoctl->QueueId == 0) {
                /*
                 * Process Admin commands here. In case of errors, complete the
                 * request and return IOCTL_COMPLETED. Otherwise, return
                 * IOCTL_PENDING.
                 */
                pSrbExt->forAdminQueue = TRUE;
                switch (pNvmeCmdDW0->OPC) {
                    case ADMIN_CREATE_IO_SUBMISSION_QUEUE:
                    case ADMIN_CREATE_IO_COMPLETION_QUEUE:
                    case ADMIN_DELETE_IO_SUBMISSION_QUEUE:
                    case ADMIN_DELETE_IO_COMPLETION_QUEUE:
                    case ADMIN_ABORT:
                    case ADMIN_ASYNCHRONOUS_EVENT_REQUEST:
                        /* Reject unsupported commands */
                        pSrbIoCtrl->ReturnCode =
                            NVME_IOCTL_UNSUPPORTED_ADMIN_CMD;

                        return IOCTL_COMPLETED;
                    break;
                    case ADMIN_GET_LOG_PAGE:
                        if (NVMeIoctlGetLogPage(pDevExt, pSrb, pNvmePtIoctl) ==
                                                IOCTL_COMPLETED) {
                            return IOCTL_COMPLETED;
                        }
                    break;
                    case ADMIN_IDENTIFY:
                        if (NVMeIoctlIdentify(pDevExt, pSrb, pNvmePtIoctl) ==
                                              IOCTL_COMPLETED) {
                            return IOCTL_COMPLETED;
                        }
                    break;
                    case ADMIN_SET_FEATURES:
                    case ADMIN_GET_FEATURES:
                        if (NVMeIoctlSetGetFeatures(
                                pDevExt, pSrb, pNvmePtIoctl, pNvmeCmdDW0->OPC)
                                == IOCTL_COMPLETED) {
                            return IOCTL_COMPLETED;
                        }
                    break;
                    case ADMIN_FIRMWARE_IMAGE_DOWNLOAD:
                        if (NVMeIoctlFwDownload(pDevExt, pSrb, pNvmePtIoctl) ==
                                                IOCTL_COMPLETED) {
                            return IOCTL_COMPLETED;
                        }
                    break;
                    case ADMIN_SECURITY_SEND:
                    case ADMIN_SECURITY_RECEIVE:
                        if (NVMeIoctlSecuritySendRcv(
                                pDevExt, pSrb, pNvmePtIoctl, pNvmeCmdDW0->OPC)
                                == IOCTL_COMPLETED) {
                            return IOCTL_COMPLETED;
                        }
                    break;
                    case ADMIN_FORMAT_NVM:
                        /*
                         * There are two scenarios in handling this request:
                         * 1. If no NVME_HOT_REMOVE_NAMESPACE request received,
                         *    Format NVM command will be issued after removing
                         *    the target namespace(s) from the system.
                         * 2. If NVME_HOT_REMOVE_NAMESPACE had been received,
                         *    Format NVM command will be issued and
                         *    applications need to call NVME_HOT_ADD_NAMESPACE
                         *    to add back the formatted namespace(s).
                         */
                        if ((pDevExt->FormatNvmInfo.State !=
                                FORMAT_NVM_NS_REMOVED) &&
                            (pDevExt->FormatNvmInfo.State !=
                                FORMAT_NVM_NO_ACTIVITY)) {
                            pSrb->SrbStatus = SRB_STATUS_SUCCESS;
                            pSrbIoCtrl->ReturnCode =
                                NVME_IOCTL_FORMAT_NVM_FAILED;
                            return IOCTL_COMPLETED;
                        }
                        /* Set status as pending and handle it in StartIo */
                        pSrb->SrbStatus = SRB_STATUS_PENDING;
                    break;
                    case ADMIN_FIRMWARE_ACTIVATE:
                        /*
                         * No pre-processing required. Ensure the command is
                         * supported first.
                         */

                        pCntrlIdData = &pDevExt->controllerIdentifyData;
                        supportFwActFwDl =
                            pCntrlIdData->OACS.SupportsFirmwareActivateFirmwareDownload;

                        if (supportFwActFwDl == 0) {
                            pSrbIoCtrl->ReturnCode =
                                NVME_IOCTL_UNSUPPORTED_ADMIN_CMD;
                            return IOCTL_COMPLETED;
                        }
                    break;
                    default:
                        /*
                         * Supported Opcodes of Admin vendor specific commands
                         * are from 0xC0 to 0xFF.
                         */
                        if (pNvmeCmdDW0->OPC < ADMIN_VENDOR_SPECIFIC_START) {
                            pSrbIoCtrl->ReturnCode =
                                NVME_IOCTL_INVALID_ADMIN_VENDOR_SPECIFIC_OPCODE;

                            return IOCTL_COMPLETED;
                        }

                        /*
                         * Check if AVSCC bit is set as 1, indicating support of
                         * ADMIN vendor specific command handling via Pass
                         * Through.
                         */
                        if (pDevExt->controllerIdentifyData.AVSCC == 0) {
                            pSrbIoCtrl->ReturnCode =
                                NVME_IOCTL_ADMIN_VENDOR_SPECIFIC_NOT_SUPPORTED;

                            return IOCTL_COMPLETED;
                        }
                    break;
                } /* end switch */
                /* Process ADMIN commands ends */
            } else {
                /* Process NVM commands here */
                pSrbExt->forAdminQueue = FALSE;

                switch (pNvmeCmdDW0->OPC) {
                    case NVM_WRITE:
                    case NVM_READ:
                        /* Reject unsupported commands */
                        pSrbIoCtrl->ReturnCode = NVME_IOCTL_UNSUPPORTED_NVM_CMD;

                        return IOCTL_COMPLETED;
                    break;
                    case NVM_COMPARE:
                        if (NVMeIoctlCompare(pDevExt,
                                             pSrb,
                                             pNvmePtIoctl) == IOCTL_COMPLETED)

                            return IOCTL_COMPLETED;
                    break;
                    case NVM_DATASET_MANAGEMENT:
                        if (NVMeIoctlDataSetManagement(pDevExt,
                                             pSrb,
                                             pNvmePtIoctl) == IOCTL_COMPLETED)

                            return IOCTL_COMPLETED;
                    break;
                    case NVM_FLUSH:
                    case NVM_WRITE_UNCORRECTABLE:
                        /* No pre-processing required */
                    break;
                    default:
                        /*
                         * Supported Opcodes of NVM vendor specific commands
                         * are from 0x80 to 0xFF
                         */
                        if (pNvmeCmdDW0->OPC < NVM_VENDOR_SPECIFIC_START) {
                            pSrbIoCtrl->ReturnCode =
                                NVME_IOCTL_INVALID_NVM_VENDOR_SPECIFIC_OPCODE;

                            return IOCTL_COMPLETED;
                        }

                        /*
                         * Check if NVSCC bit is set as 1, indicating support of
                         * NVM vendor specific command handling via Pass Through
                         */
                        if (pDevExt->controllerIdentifyData.NVSCC == 0) {
                            pSrbIoCtrl->ReturnCode =
                                NVME_IOCTL_NVM_VENDOR_SPECIFIC_NOT_SUPPORTED;

                            return IOCTL_COMPLETED;
                        }
                    break;
                }
            } /* Process NVM commands ends */

            /*
             * Need more processing in StartIo before sending down to the controller.
             * Set up callback routine and temp submission entry,
             * return IOCTL_PENDING to indicate StartIo processing is required.
             */
            if (pNvmeCmdDW0->OPC == ADMIN_FORMAT_NVM)
                pSrbExt->pNvmeCompletionRoutine = NVMeIoctlFormatNVMCallback;
            else
                pSrbExt->pNvmeCompletionRoutine = NVMeIoctlCallback;

            StorPortMoveMemory((PVOID)&pSrbExt->nvmeSqeUnit,
                               (PVOID)pNvmePtIoctl->NVMeCmd,
                               sizeof(NVMe_COMMAND));

        break; /* NVME_PASS_THROUGH_SRB_IO_CODE ends */
        case NVME_GET_NAMESPACE_ID:
            /*
             * If return buffer length is less than size of
             * NVME_PASS_THROUGH_IOCTL, plus size of a DWORD, note it in
             * ReturnCode of SRB_IO_CONTROL and return
             */
            if (pNvmePtIoctl->ReturnBufferLen <
                (IoctlHdrSize + sizeof(ULONG) - 1)) {
                pSrbIoCtrl->ReturnCode = NVME_IOCTL_INSUFFICIENT_OUT_BUFFER;

                return IOCTL_COMPLETED;
            }

            /*
             * Lun of SRB can be interpreted as Namespace ID, which is 1-based.
             */
            pNvmePtIoctl->DataBuffer[0] = pSrb->Lun + 1;

            return IOCTL_COMPLETED;
        break;
        case NVME_HOT_REMOVE_NAMESPACE:
        case NVME_HOT_ADD_NAMESPACE:
            /* Set status as pending and handle it in StartIo */
            pSrb->SrbStatus = SRB_STATUS_PENDING;
            return IOCTL_PENDING;
        break;
        default:
            StorPortDebugPrint(
                INFO,
                "NVMeProcessIoctl: UNSUPPORTED Ctrl Code (0x%x)\n",
                pSrbIoCtrl->ControlCode);

            pSrb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            pSrbIoCtrl->ReturnCode = NVME_IOCTL_INVALID_IOCTL_CODE;

            return IOCTL_COMPLETED;
        break;
    } /* end switch */

    return IOCTL_PENDING;
} /* NVMeProcessIoctl */

/*******************************************************************************
 * NVMeInitAdminQueues
 *
 * @brief This function initializes the admin queues (SQ/CQ pair)
 *
 * @param pAE - Pointer to device extension
 *
 * @return ULONG
 *     Returns status based upon results of init'ing the admin queues
 ******************************************************************************/
ULONG NVMeInitAdminQueues(
    PNVME_DEVICE_EXTENSION pAE
)
{
    ULONG Status;

    /* Initialize Admin Submission queue */
    Status = NVMeInitSubQueue(pAE, 0);
    if (Status != STOR_STATUS_SUCCESS) {
        return (FALSE);
    }

    /* Initialize Admin Completion queue */
    Status = NVMeInitCplQueue(pAE, 0);
    if (Status != STOR_STATUS_SUCCESS) {
        return (FALSE);
    }

    /* Initialize Command Entries */
    Status = NVMeInitCmdEntries(pAE, 0);
    if (Status != STOR_STATUS_SUCCESS) {
        return (FALSE);
    }

    /*
     * Enable adapter after initializing some controller and Admin queue
     * registers. Need to determine if the adapter is ready for
     * processing commands after entering Start State Machine
     */
    NVMeEnableAdapter(pAE);

#ifdef CHATHAM
    NVMeChathamSetup(pAE);
#endif

    return (STOR_STATUS_SUCCESS);
} /* NVMeInitAdminQueues */

/*******************************************************************************
 * NVMeLogError
 *
 * @brief Logs error to storport.
 *
 * @param pAE - Pointer to device extension
 * @param ErrorNum - Code to log
 *
 * @return VOID
 ******************************************************************************/
VOID NVMeLogError(
    __in PNVME_DEVICE_EXTENSION pAE,
    __in ULONG ErrorNum
)
{

    StorPortDebugPrint(INFO,
                   "NvmeLogError: logging error (0x%x)\n",
                   ErrorNum);

    StorPortLogError(pAE,
                     NULL,
                     0,
                     0,
                     0,
                     SP_INTERNAL_ADAPTER_ERROR,
                     ErrorNum);
}

#if DBG
/*******************************************************************************
 * IO_StorPortNotification
 *
 * @brief Debug routine for completing IO through one function to Storport.
 *
 * @param NotificationType - Type of StorPort notification
 * @param pHwDeviceExtension - Pointer to device extension
 * @param pSrb - Pointer to SRB
 *
 * @return VOID
 ******************************************************************************/
VOID IO_StorPortNotification(
    __in SCSI_NOTIFICATION_TYPE NotificationType,
    __in PVOID pHwDeviceExtension,
    __in PSCSI_REQUEST_BLOCK pSrb
)
{

    if ((pSrb->SrbStatus != SRB_STATUS_SUCCESS) ||
        (pSrb->ScsiStatus != SCSISTAT_GOOD)){

        /* DbgBreakPoint(); */

        StorPortDebugPrint(INFO,
                           "FYI: SRB status 0x%x scsi 0x%x for CDB 0x%x\n",
                           pSrb->SrbStatus,
                           pSrb->ScsiStatus,
                           pSrb->Cdb[0]);
    }

    StorPortNotification(NotificationType,
                         pHwDeviceExtension,
                         pSrb);
}
#endif

#ifdef CHATHAM
/*******************************************************************************
 * RegisterWriteAddrU64
 *
 * @brief Helper routine for writing a 64 bit register value (Chatham)
 *
 * @param pContext - Context
 * @param pRegAddr - Register address
 * @param Value - value to be written
 *
 * @return VOID
 ******************************************************************************/
VOID RegisterWriteAddrU64(
    PVOID pContext,
    PVOID pRegAddr,
    UINT64 Value
)
{
    UINT32 TmpVal = (UINT32)Value;
    PULONG pAddress = (PULONG)pRegAddr;

    UNREFERENCED_PARAMETER(pContext);
    ASSERT(pContext);

    StorPortWriteRegisterUlong(pContext,
                               (PULONG)pRegAddr,
                               TmpVal);
    StorPortWriteRegisterUlong(pContext,
                               (PULONG)(pAddress + 1),
                               (UINT32)(Value >> 32));
}

/*******************************************************************************
 * NVMeChathamSetup
 *
 * @brief Setup routine for Chatham
 *
 * @param pAE - Pointer to device extension
 *
 * @return VOID
 ******************************************************************************/
VOID NVMeChathamSetup(
    PNVME_DEVICE_EXTENSION pAE
)
{
    PUCHAR pBar = (PUCHAR)pAE->pChathamRegs;
    ULONG ver = 0;
    ULONGLONG reg1 = 0;
    ULONGLONG reg2 = 0;
    ULONGLONG reg3 = 0;
    ULONGLONG temp1 = 0;
    ULONGLONG temp2 = 0;
    ULONG temp3 = 0;

    NVMeStallExecution(pAE,10000);
    ver = StorPortReadRegisterUlong(pAE, (PULONG)(pBar + 0x8080));

    DbgPrint("Chatham Version: 0x%x\n", ver);

    ChathamNlb = StorPortReadRegisterUlong(pAE, (PULONG)(pBar + 0x8068));
    ChathamNlb = ChathamNlb - 0x100;
    ChathamSize = ChathamNlb * 512;

    DbgPrint("ChathamSize: 0x%x\n", ChathamSize);

    reg1 = ChathamSize - 1;
    reg2 = reg3 = reg1;

 //   temp1 = ((ULONGLONG)0x1b58 << 32) | 0x7d0;
 //   temp2 = ((ULONGLONG)0xcb << 32) | 0x131;

    RegisterWriteAddrU64(pAE, (PULONG)(pBar + 0x8000), reg1);
    RegisterWriteAddrU64(pAE, (PULONG)(pBar + 0x8008), reg2);
    RegisterWriteAddrU64(pAE, (PULONG)(pBar + 0x8010), reg3);

    RegisterWriteAddrU64(pAE, (PULONG)(pBar + 0x8020), temp1);
    temp3 = StorPortReadRegisterUlong(pAE, (PULONG)(pBar + 0x8020));
    RegisterWriteAddrU64(pAE, (PULONG)(pBar + 0x8028), temp2);
    temp3 = StorPortReadRegisterUlong(pAE, (PULONG)(pBar + 0x8028));
    RegisterWriteAddrU64(pAE, (PULONG)(pBar + 0x8030), temp1);
    RegisterWriteAddrU64(pAE, (PULONG)(pBar + 0x8038), temp2);
    RegisterWriteAddrU64(pAE, (PULONG)(pBar + 0x8040), temp1);
    RegisterWriteAddrU64(pAE, (PULONG)(pBar + 0x8048), temp2);
    RegisterWriteAddrU64(pAE, (PULONG)(pBar + 0x8050), temp1);
    RegisterWriteAddrU64(pAE, (PULONG)(pBar + 0x8058), temp2);

    NVMeStallExecution(pAE,10000);
}
#endif /* CHATHAM */
