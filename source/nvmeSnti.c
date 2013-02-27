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
 * File: nvmeSnti.c
 */

#include "precomp.h"

HW_INITIALIZATION_DATA gHwInitializationData;

/* NVMe Vendor String */
const char vendorIdString[] = "NVME    ";

/* Current Mode Parameter Block Descriptor Values */
MODE_PARAMETER_BLOCK g_modeParamBlock;

/* Generic Command Status Lookup Table */
SNTI_RESPONSE_BLOCK genericCommandStatusTable[] = {
    /* SUCCESSFUL_COMPLETION - 0x0 */
    {SRB_STATUS_SUCCESS,
        SCSISTAT_GOOD,
        SCSI_SENSE_NO_SENSE,
        SCSI_ADSENSE_NO_SENSE,
        SCSI_ADSENSE_NO_SENSE},

    /* INVALID_COMMAND_OPCODE - 0x1 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_ILLEGAL_REQUEST,
        SCSI_ADSENSE_ILLEGAL_COMMAND,
        SCSI_ADSENSE_NO_SENSE},

    /* INVALID_FIELD_IN_COMMAND - 0x2 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_ILLEGAL_REQUEST,
        SCSI_ADSENSE_INVALID_CDB,
        SCSI_ADSENSE_NO_SENSE},

    /* COMMAND_ID_CONFLICT - 0x3 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_ILLEGAL_REQUEST,
        SCSI_ADSENSE_INVALID_CDB,
        SCSI_ADSENSE_NO_SENSE},

    /* DATA_TRANSFER_ERROR - 0x4 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_MEDIUM_ERROR,
        SCSI_ADSENSE_NO_SENSE,
        SCSI_ADSENSE_NO_SENSE},

    /* COMMANDS_ABORTED_DUE_TO_POWER_LOSS_NOTIFICATION - 0x5 */
    {SRB_STATUS_ABORTED,
        SCSISTAT_TASK_ABORTED,
        SCSI_SENSE_ABORTED_COMMAND,
        SCSI_ADSENSE_NO_SENSE,
        SCSI_ADSENSE_NO_SENSE},

    /* INTERNAL_DEVICE_ERROR - 0x6 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_HARDWARE_ERROR,
        SCSI_ADSENSE_INTERNAL_TARGET_FAILURE,
        SCSI_ADSENSE_NO_SENSE},

    /* COMMAND_ABORT_REQUESTED - 0x7 */
    {SRB_STATUS_ABORTED,
        SCSISTAT_TASK_ABORTED,
        SCSI_SENSE_ABORTED_COMMAND,
        SCSI_ADSENSE_NO_SENSE,
        SCSI_ADSENSE_NO_SENSE},

    /* COMMAND_ABORTED_DUE_TO_SQ_DELETION - 0x8 */
    {SRB_STATUS_ABORTED,
        SCSISTAT_TASK_ABORTED,
        SCSI_SENSE_ABORTED_COMMAND,
        SCSI_ADSENSE_NO_SENSE,
        SCSI_ADSENSE_NO_SENSE},

    /* COMMAND_ABORTED_DUE_TO_FAILED_FUSED_COMMAND - 0x9 */
    {SRB_STATUS_ABORTED,
        SCSISTAT_TASK_ABORTED,
        SCSI_SENSE_ABORTED_COMMAND,
        SCSI_ADSENSE_NO_SENSE,
        SCSI_ADSENSE_NO_SENSE},

    /* COMMAND_ABORTED_DUE_TO_MISSING_FUSED_COMMAND - 0xA */
    {SRB_STATUS_ABORTED,
        SCSISTAT_TASK_ABORTED,
        SCSI_SENSE_ABORTED_COMMAND,
        SCSI_ADSENSE_NO_SENSE,
        SCSI_ADSENSE_NO_SENSE},

    /* INVALID_NAMESPACE_OR_FORMAT - 0xB */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_ILLEGAL_REQUEST,
        SCSI_ADSENSE_ILLEGAL_COMMAND,
        SCSI_SENSEQ_INVALID_LUN_ID},

    /* Generic Command Status (NVM Command Set) Lookup Table */

    /* LBA_OUT_OF_RANGE - 0x80 (0xC) */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_ILLEGAL_REQUEST,
        SCSI_ADSENSE_ILLEGAL_BLOCK,
        SCSI_ADSENSE_NO_SENSE},

    /* CAPACITY_EXCEEDED - 0x81 (0xD) */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_MEDIUM_ERROR,
        SCSI_ADSENSE_NO_SENSE,
        SCSI_ADSENSE_NO_SENSE},

    /* NAMESPACE_NOT_READY - 0x82 (0xE) */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_NOT_READY,
        SCSI_ADSENSE_LUN_NOT_READY,
        SCSI_ADSENSE_NO_SENSE}
};

/* Command Specific Status Lookup Table */
SNTI_RESPONSE_BLOCK commandSpecificStatusTable[] = {
    /* COMPLETION_QUEUE_INVALID - 0x0 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_ILLEGAL_REQUEST,
        SCSI_ADSENSE_NO_SENSE,
        SCSI_ADSENSE_NO_SENSE},

    /* INVALID_QUEUE_IDENTIFIER - 0x1 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_UNIT_ATTENTION,
        SCSI_ADSENSE_INTERNAL_TARGET_FAILURE,
        SCSI_ADSENSE_NO_SENSE},

    /* MAXIMUM_QUEUE_SIZE_EXCEEDED - 0x2 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_UNIT_ATTENTION,
        SCSI_ADSENSE_INTERNAL_TARGET_FAILURE,
        SCSI_ADSENSE_NO_SENSE},

    /* ABORT_COMMAND_LIMIT_EXCEEDED - 0x3 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_ILLEGAL_REQUEST,
        SCSI_ADSENSE_NO_SENSE,
        SCSI_ADSENSE_NO_SENSE},

    /* RESERVED (REQUESTED_COMMAND_TO_ABORT_NOT_FOUND) - 0x4 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_UNIT_ATTENTION,
        SCSI_ADSENSE_INTERNAL_TARGET_FAILURE,
        SCSI_ADSENSE_NO_SENSE},

    /* ASYNCHRONOUS_EVENT_REQUEST_LIMIT_EXCEEDED - 0x5 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_UNIT_ATTENTION,
        SCSI_ADSENSE_INTERNAL_TARGET_FAILURE,
        SCSI_ADSENSE_NO_SENSE},

    /* INVALID_FIRMWARE_SLOT - 0x6 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_UNIT_ATTENTION,
        SCSI_ADSENSE_INTERNAL_TARGET_FAILURE,
        SCSI_ADSENSE_NO_SENSE},

    /* INVALID_FIRMWARE_IMAGE - 0x7 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_UNIT_ATTENTION,
        SCSI_ADSENSE_INTERNAL_TARGET_FAILURE,
        SCSI_ADSENSE_NO_SENSE},

    /* INVALID_INTERRUPT_VECTOR - 0x8 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_UNIT_ATTENTION,
        SCSI_ADSENSE_INTERNAL_TARGET_FAILURE,
        SCSI_ADSENSE_NO_SENSE},

    /* INVALID_LOG_PAGE - 0x9 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_UNIT_ATTENTION,
        SCSI_ADSENSE_INTERNAL_TARGET_FAILURE,
        SCSI_ADSENSE_NO_SENSE},

    /* INVALID_FORMAT - 0xA */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_ILLEGAL_REQUEST,
        SCSI_ADSENSE_FORMAT_COMMAND_FAILED,
        SCSI_SENSEQ_FORMAT_COMMAND_FAILED},

    /* Command Specific Status (NVM Command Set) Lookup Table */

    /* CONFLICTING_ATTRIBUTES - 0x80 (0xB) */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_ILLEGAL_REQUEST,
        SCSI_ADSENSE_INVALID_CDB,
        SCSI_ADSENSE_NO_SENSE}
};

/* Media Error Lookup Table */
SNTI_RESPONSE_BLOCK mediaErrorTable[] = {
    /* WRITE_FAULT - 0x80 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_MEDIUM_ERROR,
        SCSI_ADSENSE_PERIPHERAL_DEV_WRITE_FAULT,
        SCSI_ADSENSE_NO_SENSE},

    /* UNRECOVERED_READ_ERROR - 0x81 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_MEDIUM_ERROR,
        SCSI_ADSENSE_UNRECOVERED_READ_ERROR,
        SCSI_ADSENSE_NO_SENSE},

    /* END_TO_END_GUARD_CHECK_ERROR - 0x82 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_MEDIUM_ERROR,
        SCSI_ADSENSE_LOG_BLOCK_GUARD_CHECK_FAILED,
        SCSI_SENSEQ_LOG_BLOCK_GUARD_CHECK_FAILED},

    /* END_TO_END_APPLICATION_TAG_CHECK_ERROR - 0x83 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_MEDIUM_ERROR,
        SCSI_ADSENSE_LOG_BLOCK_APPTAG_CHECK_FAILED,
        SCSI_SENSEQ_LOG_BLOCK_APPTAG_CHECK_FAILED},

    /* END_TO_END_REFERENCE_TAG_CHECK_ERROR - 0x84 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_MEDIUM_ERROR,
        SCSI_ADSENSE_LOG_BLOCK_REFTAG_CHECK_FAILED,
        SCSI_SENSEQ_LOG_BLOCK_REFTAG_CHECK_FAILED},

    /* COMPARE_FAILURE - 0x85 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_MISCOMPARE,
        SCSI_ADSENSE_MISCOMPARE_DURING_VERIFY,
        SCSI_ADSENSE_NO_SENSE},

    /* ACCESS_DENIED - 0x86 */
    {SRB_STATUS_ERROR,
        SCSISTAT_CHECK_CONDITION,
        SCSI_SENSE_ILLEGAL_REQUEST,
        SCSI_ADSENSE_ACCESS_DENIED_INVALID_LUN_ID,
        SCSI_SENSEQ_ACCESS_DENIED_INVALID_LUN_ID}
};

/******************************************************************************
 * SntiTranslateCommand
 *
 * @brief This method parses the supplied SCSI request and then invokes the SCSI
 *        to NVMe translation function for that particular request. Unsupported
 *        SCSI commands in the open source NVMe driver:
 *
 *        - COMPARE AND WRITE (NVME_COMPARE is required for translation and this
 *          command will not be supported in the Windows open source driver)
 *
 *        - WRITE LONG 10/16 (NVME_WRITE_UNCORRECTABLE is required for
 *          translation and this command will not be supported in the Windows
 *          open source driver)
 *
 *        - UNMAP (NVME_DATASET_MANAGEMENT is required for translation and this
 *          command will not be supported in the Windows open source driver)
 *
 *
 * @param pAdapterExtension - pointer to the adapter device extension
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateCommand(
    PNVME_DEVICE_EXTENSION pAdapterExtension,
    PSCSI_REQUEST_BLOCK pSrb
)
{
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_SUCCESS;
    BOOLEAN supportsVwc = pAdapterExtension->controllerIdentifyData.VWC.Present;

    /* DEBUG ONLY - Turn off for main I/O */
#if DBG
    StorPortDebugPrint(INFO,
                       "SNTI: Translating opcode - 0x%02x BTL (%d %d %d)\n",
                       GET_OPCODE(pSrb),
                       pSrb->PathId,
                       pSrb->TargetId,
                       pSrb->Lun);
#endif /* DBG */

    switch (GET_OPCODE(pSrb)) {
        case SCSIOP_READ6:
        case SCSIOP_READ:
        case SCSIOP_READ12:
        case SCSIOP_READ16:
            returnStatus = SntiTranslateRead(pSrb);
        break;
        case SCSIOP_WRITE6:
        case SCSIOP_WRITE:
        case SCSIOP_WRITE12:
        case SCSIOP_WRITE16:
            returnStatus = SntiTranslateWrite(pSrb);
        break;
        case SCSIOP_INQUIRY:
            returnStatus = SntiTranslateInquiry(pSrb);
            /* set the maximum queue depth per LUN */
            if (pSrb->SrbStatus == SRB_STATUS_SUCCESS) {
                StorPortSetDeviceQueueDepth(pAdapterExtension,
                            pSrb->PathId,
                            pSrb->TargetId,
                            pSrb->Lun,
                            SNTI_STORPORT_QUEUE_DEPTH
                            );
            }
        break;
        case SCSIOP_LOG_SENSE:
            SntiSetScsiSenseData(pSrb,
                                 SCSISTAT_CHECK_CONDITION,
                                 SCSI_SENSE_ILLEGAL_REQUEST,
                                 SCSI_ADSENSE_ILLEGAL_COMMAND,
                                 SCSI_ADSENSE_NO_SENSE);
            pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
            pSrb->DataTransferLength = 0;
            returnStatus = SNTI_UNSUPPORTED_SCSI_REQUEST;
        break;
        case SCSIOP_MODE_SELECT:
        case SCSIOP_MODE_SELECT10:
            returnStatus = SntiTranslateModeSelect(pSrb, supportsVwc);
        break;
        case SCSIOP_MODE_SENSE:
        case SCSIOP_MODE_SENSE10:
            returnStatus = SntiTranslateModeSense(pSrb, supportsVwc);
        break;
        case SCSIOP_READ_CAPACITY:
        case SCSIOP_READ_CAPACITY16:
            returnStatus = SntiTranslateReadCapacity(pSrb);
        break;
        case SCSIOP_REPORT_LUNS:
            returnStatus = SntiTranslateReportLuns(pSrb);
        break;
        case SCSIOP_REQUEST_SENSE:
            returnStatus = SntiTranslateRequestSense(pSrb);
        break;
        case SCSIOP_SECURITY_PROTOCOL_IN:
        case SCSIOP_SECURITY_PROTOCOL_OUT:
            /* Need support added QEMU for NVME SECURITY SEND/RECEIVE */
            /* returnStatus = SntiTranslateSecurityProtocol(pSrb); */
            StorPortDebugPrint(INFO,
                "SNTI: SCSIOP_SECURITY_PROTOCOL_IN_OUT unsupported - 0x%02x\n",
                GET_OPCODE(pSrb));

            SntiSetScsiSenseData(pSrb,
                                 SCSISTAT_CHECK_CONDITION,
                                 SCSI_SENSE_ILLEGAL_REQUEST,
                                 SCSI_ADSENSE_ILLEGAL_COMMAND,
                                 SCSI_ADSENSE_NO_SENSE);

            pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
            pSrb->DataTransferLength = 0;

            returnStatus = SNTI_UNSUPPORTED_SCSI_REQUEST;
        break;
        case SCSIOP_START_STOP_UNIT:
            returnStatus = SntiTranslateStartStopUnit(pSrb);
        break;
        case SCSIOP_SYNCHRONIZE_CACHE:
        case SCSIOP_SYNCHRONIZE_CACHE16:
            if (supportsVwc == TRUE) {
            returnStatus = SntiTranslateSynchronizeCache(pSrb);
            } else {
                pSrb->SrbStatus = SRB_STATUS_SUCCESS;
                pSrb->DataTransferLength = 0;
                returnStatus = SNTI_COMMAND_COMPLETED;
            }
        break;
        case SCSIOP_FORMAT_UNIT:
            /* Will never get this request from OS */
            /* returnStatus = SntiTranslateFormatUnit(pSrb); */
            StorPortDebugPrint(INFO,
                "SNTI: SCSIOP_FORMAT_UNIT unsupported - 0x%02x\n",
                GET_OPCODE(pSrb));

            SntiSetScsiSenseData(pSrb,
                                 SCSISTAT_CHECK_CONDITION,
                                 SCSI_SENSE_ILLEGAL_REQUEST,
                                 SCSI_ADSENSE_ILLEGAL_COMMAND,
                                 SCSI_ADSENSE_NO_SENSE);

            pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
            pSrb->DataTransferLength = 0;

            returnStatus = SNTI_UNSUPPORTED_SCSI_REQUEST;
        break;
        case SCSIOP_TEST_UNIT_READY:
            returnStatus = SntiTranslateTestUnitReady(pSrb);
        break;
        case SCSIOP_WRITE_DATA_BUFF:
            returnStatus = SntiTranslateWriteBuffer(pSrb);
        break;
        default:
            StorPortDebugPrint(INFO,
                               "SNTI: UNSUPPORTED opcode - 0x%02x\n",
                               GET_OPCODE(pSrb));

            SntiSetScsiSenseData(pSrb,
                                 SCSISTAT_CHECK_CONDITION,
                                 SCSI_SENSE_ILLEGAL_REQUEST,
                                 SCSI_ADSENSE_ILLEGAL_COMMAND,
                                 SCSI_ADSENSE_NO_SENSE);

            pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
            pSrb->DataTransferLength = 0;

            returnStatus = SNTI_UNSUPPORTED_SCSI_REQUEST;
        break;
   } /* end switch */

    return returnStatus;
} /* SntiTranslateCommand */

/******************************************************************************
 * SntiTranslateInquiry
 *
 * @brief Translates the SCSI Inquiry command and populate the appropriate SCSI
 *        Inquiry fields based on the NVMe Translation spec. Do not need to
 *        create an SQE here as we just complete the command in the build phase
 *        (by returning FALSE to StorPort with SRB status of SUCCESS).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateInquiry(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    UINT8 pageCode;
    BOOLEAN evpd;
    PNVME_LUN_EXTENSION pLunExt = NULL;
    SNTI_STATUS status;

    /* Default the translation status to command completed */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_COMMAND_COMPLETED;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    evpd = GET_INQ_EVPD_BIT(pSrb);
    pageCode = GET_INQ_PAGE_CODE(pSrb);

    ASSERT(pSrbExt != NULL);

    status = GetLunExtension(pSrbExt, &pLunExt);
    if (status != SNTI_SUCCESS) {
        SntiMapInternalErrorStatus(pSrb, status);
        return SNTI_FAILURE_CHECK_RESPONSE_DATA;
    }

    /*
     * Set SRB status to success to indicate the command will complete
     * successfully (assuming no errors occur during translation) and reset the
     * status value to use below.
     */
    pSrb->SrbStatus = SRB_STATUS_SUCCESS;

    /* Set the completion routine - no translation necessary on completion */
    pSrbExt->pNvmeCompletionRoutine = NULL;

    /* EVPD = 0: Standard Inquiry Page; EVPD = 1: VPD Inquiry Page */
    if (evpd != 0) {
        switch (pageCode) {
            case VPD_SUPPORTED_PAGES:
                SntiTranslateSupportedVpdPages(pSrb);
            break;
            case VPD_SERIAL_NUMBER:
                SntiTranslateUnitSerialPage(pSrb);
            break;
            case VPD_DEVICE_IDENTIFIERS:
                SntiTranslateDeviceIdentificationPage(pSrb);
            break;
            default:
                SntiSetScsiSenseData(pSrb,
                                     SCSISTAT_CHECK_CONDITION,
                                     SCSI_SENSE_ILLEGAL_REQUEST,
                                     SCSI_ADSENSE_INVALID_CDB,
                                     SCSI_ADSENSE_NO_SENSE);

                pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
                pSrb->DataTransferLength = 0;
                returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
            break;
        } /* end switch */
    } else {
        /* For Standard Inquiry, the page code must be 0 */
        if (pageCode == INQ_STANDARD_INQUIRY_PAGE) {
            SntiTranslateStandardInquiryPage(pSrb);
        } else {
            /* Ensure correct sense data for SCSI compliance test case 1.4 */
           SntiSetScsiSenseData(pSrb,
                                SCSISTAT_CHECK_CONDITION,
                                SCSI_SENSE_ILLEGAL_REQUEST,
                                SCSI_ADSENSE_INVALID_CDB,
                                SCSI_ADSENSE_NO_SENSE);

           pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
           pSrb->DataTransferLength = 0;
           returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
        }
    }

    /*
     * For Inquiry commands, we shall always used cached data from the HW and
     * therefore shall always return SNTI_COMMAND_COMPLETED unless and error
     * occurred during the translation phase.
     */
    return returnStatus;
} /* SntiTranslateInquiry */

/******************************************************************************
 * SntiTranslateSupportedVpdPages
 *
 * @brief Translates the SCSI Inquiry VPD page - Supported VPD Pages. Populates
 *        the appropriate SCSI Inqiry response fields based on the NVMe
 *        Translation spec. Do not need to create SQE here as we just complete
 *        the command in the build phase (by returning FALSE to StorPort with
 *        SRB status of SUCCESS).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return VOID
 ******************************************************************************/
VOID SntiTranslateSupportedVpdPages(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PVPD_SUPPORTED_PAGES_PAGE pSupportedVpdPages = NULL;
    UINT16 allocLength;

    pSupportedVpdPages = (PVPD_SUPPORTED_PAGES_PAGE)GET_DATA_BUFFER(pSrb);
    allocLength = GET_INQ_ALLOC_LENGTH(pSrb);

    memset(pSupportedVpdPages, 0, allocLength);
    pSupportedVpdPages->DeviceType           = DIRECT_ACCESS_DEVICE;
    pSupportedVpdPages->DeviceTypeQualifier  = DEVICE_CONNECTED;
    pSupportedVpdPages->PageCode             = VPD_SUPPORTED_PAGES;
    pSupportedVpdPages->Reserved             = INQ_RESERVED;
    pSupportedVpdPages->PageLength           = INQ_NUM_SUPPORTED_VPD_PAGES;
    pSupportedVpdPages->SupportedPageList[BYTE_0] = VPD_SUPPORTED_PAGES;
    pSupportedVpdPages->SupportedPageList[BYTE_1] = VPD_SERIAL_NUMBER;
    pSupportedVpdPages->SupportedPageList[BYTE_2] = VPD_DEVICE_IDENTIFIERS;

    pSrb->DataTransferLength = min(allocLength,
        (FIELD_OFFSET(VPD_SUPPORTED_PAGES_PAGE, SupportedPageList) +
            INQ_NUM_SUPPORTED_VPD_PAGES));
} /* SntiTranslateSupportedVpdPages */

/******************************************************************************
 * SntiTranslateUnitSerialPage
 *
 * @brief Translates the SCSI Inquiry VPD page - Unit Serial Number Page.
 *        Populates the appropriate SCSI Inqiry response fields based on the
 *        NVMe Translation spec. Do not need to create SQE here as we just
 *        complete the command in the build phase (by returning FALSE to
 *        StorPort with SRB status of SUCCESS).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return VOID
 ******************************************************************************/
VOID SntiTranslateUnitSerialPage(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_DEVICE_EXTENSION pDevExt = NULL;
    PVPD_SERIAL_NUMBER_PAGE pSerialNumberPage = NULL;
    UINT16 allocLength;

    pDevExt = ((PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb))->pNvmeDevExt;
    pSerialNumberPage = (PVPD_SERIAL_NUMBER_PAGE)GET_DATA_BUFFER(pSrb);
    allocLength = GET_INQ_ALLOC_LENGTH(pSrb);

    memset(pSerialNumberPage, 0, allocLength);
    pSerialNumberPage->DeviceType          = DIRECT_ACCESS_DEVICE;
    pSerialNumberPage->DeviceTypeQualifier = DEVICE_CONNECTED;
    pSerialNumberPage->PageCode            = VPD_SERIAL_NUMBER;
    pSerialNumberPage->Reserved            = INQ_RESERVED;
    pSerialNumberPage->PageLength          = INQ_SERIAL_NUMBER_LENGTH;

    StorPortCopyMemory(pSerialNumberPage->SerialNumber,
           pDevExt->controllerIdentifyData.SN,
           INQ_SERIAL_NUMBER_LENGTH);

    pSrb->DataTransferLength =
        min(allocLength, (FIELD_OFFSET(VPD_SERIAL_NUMBER_PAGE, SerialNumber) +
                          INQ_SERIAL_NUMBER_LENGTH));
} /* SntiTranslateUnitSerialPage */

/******************************************************************************
 * SntiTranslateDeviceIdentificationPage
 *
 * @brief Translates the SCSI Inquiry VPD page - Device Identification Page.
 *        Populates the appropriate SCSI Inqiry response fields based on the
 *        NVMe Translation spec. Do not need to create SQE here as we just
 *        complete the command in the build phase (by returning FALSE to
 *        StorPort with SRB status of SUCCESS).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return VOID
 ******************************************************************************/
VOID SntiTranslateDeviceIdentificationPage(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PVPD_IDENTIFICATION_PAGE pDeviceIdPage = NULL;
    PVPD_IDENTIFICATION_DESCRIPTOR pIdDescriptor = NULL;
    UINT16 allocLength = 0;

    INT64 Eui64Id = EUI64_ID;

    pDeviceIdPage = (PVPD_IDENTIFICATION_PAGE)GET_DATA_BUFFER(pSrb);
    allocLength = GET_INQ_ALLOC_LENGTH(pSrb);

    memset(pDeviceIdPage, 0, allocLength);
    pDeviceIdPage->DeviceType          = DIRECT_ACCESS_DEVICE;
    pDeviceIdPage->DeviceTypeQualifier = DEVICE_CONNECTED;
    pDeviceIdPage->PageCode            = VPD_DEVICE_IDENTIFIERS;
    pDeviceIdPage->PageLength          = VPD_ID_DESCRIPTOR_LENGTH +
                                         EUI64_16_ID_SZ;

    pIdDescriptor = (PVPD_IDENTIFICATION_DESCRIPTOR)pDeviceIdPage->Descriptors;

    memset(pIdDescriptor, 0, sizeof(VPD_IDENTIFICATION_DESCRIPTOR));
    pIdDescriptor->CodeSet             = VpdCodeSetBinary;

    pIdDescriptor->Reserved            = INQ_DEV_ID_DESCRIPTOR_RESERVED;
    pIdDescriptor->Association         = VpdAssocDevice;
    pIdDescriptor->IdentifierType      = VpdIdentifierTypeEUI64;
    pIdDescriptor->IdentifierLength    = EUI64_16_ID_SZ;

    StorPortCopyMemory(pIdDescriptor->Identifier + INQ_DEV_ID_DESCRIPTOR_OFFSET,
           &Eui64Id,
           INQ_DEV_ID_DESCRIPTOR_OFFSET);

    pSrb->DataTransferLength =
        min(DEVICE_IDENTIFICATION_PAGE_SIZE, allocLength);
} /* SntiTranslateDeviceIdentificationPage */

/******************************************************************************
 * SntiTranslateExtendedInquiryDataPage
 *
 * @brief Translates the SCSI Inquiry VPD page - Extended Inquiry Data Page.
 *        Populates the appropriate SCSI Inqiry response fields based on the
 *        NVMe Translation spec. Do not need to create SQE here as we just
 *        complete the command in the build phase (by returning FALSE to
 *        StorPort with SRB status of SUCCESS).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return VOID
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateExtendedInquiryDataPage(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_DEVICE_EXTENSION pDevExt = NULL;
    PEXTENDED_INQUIRY_DATA pExtInqData = NULL;
    PNVME_LUN_EXTENSION pLunExt = NULL;
    PNVME_SRB_EXTENSION pSrbExt = NULL;

    UINT16 allocLength;
    UINT8 dataProtectionCapabilities;
    UINT8 dataProtectionSettings;

    SNTI_STATUS status;

    /* Default the translation status to command completed */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_COMMAND_COMPLETED;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    pExtInqData = (PEXTENDED_INQUIRY_DATA)GET_DATA_BUFFER(pSrb);
    pDevExt = pSrbExt->pNvmeDevExt;
    allocLength = GET_INQ_ALLOC_LENGTH(pSrb);

    status = GetLunExtension(pSrbExt, &pLunExt);
    if (status != SNTI_SUCCESS) {
        /* Map the translation error to a SCSI error */
        SntiMapInternalErrorStatus(pSrb, status);
        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
    } else {
        /* DPC field check from Namepspace Identify struct */
        if (pLunExt->identifyData.DPC.SupportsProtectionType1 ||
            pLunExt->identifyData.DPC.SupportsProtectionType2 ||
            pLunExt->identifyData.DPC.SupportsProtectionType3 ||
            pLunExt->identifyData.DPC.SupportsProtectionFirst8 ||
            pLunExt->identifyData.DPC.SupportsProtectionLast8) {
            dataProtectionCapabilities = PROTECTION_ENABLED;
        } else {
            dataProtectionCapabilities = PROTECTION_DISABLED;
        }

        /* DPS field check from Namepspace Identify struct */
        if (pLunExt->identifyData.DPS.ProtectionEnabled)
            dataProtectionSettings = PROTECTION_ENABLED;
        else
            dataProtectionSettings = PROTECTION_DISABLED;

        memset(pExtInqData, 0, allocLength);
        pExtInqData->DeviceType             = DIRECT_ACCESS_DEVICE;
        pExtInqData->DeviceTypeQualifier    = DEVICE_CONNECTED;
        pExtInqData->PageCode               = VPD_EXTENDED_INQUIRY_DATA;
        pExtInqData->Reserved1              = RESERVED_FIELDS;
        pExtInqData->PageLength             = EXTENDED_INQUIRY_DATA_PAGE_LENGTH;
        pExtInqData->ActivateMicrocode      = ACTIVATE_AFTER_HARD_RESET;

        pExtInqData->SupportedProtectionType   = dataProtectionCapabilities;
        pExtInqData->GuardCheck                = dataProtectionSettings;
        pExtInqData->ApplicationTagCheck       = dataProtectionSettings;
        pExtInqData->ReferenceTagCheck         = dataProtectionSettings;
        pExtInqData->Reserved2                 = RESERVED_FIELDS;
        pExtInqData->UACSKDataSupported        = SENSE_KEY_SPECIFIC_DATA;
        pExtInqData->GroupingFunctionSupported = GROUPING_FUNCTION_UNSUPPORTED;

        pExtInqData->PrioritySupported    = COMMAND_PRIORITY_UNSUPPORTED;
        pExtInqData->HeadOfQueueSupported = HEAD_OF_QUEUE_TASK_ATTR_UNSUPPORTED;
        pExtInqData->OrderedSupported     = ORDERED_TASK_ATTR_UNSUPPORTED;
        pExtInqData->SimpleSupported      = SIMPLE_TASK_ATTR_UNSUPPORTED;
        pExtInqData->Reserved3            = RESERVED_FIELDS;

        pExtInqData->WriteUncorrectableSupported =
            WRITE_UNCORRECTABLE_UNSUPPORTED;
        pExtInqData->CorrectionDisableSupported  =
            CORRECTION_DISABLE_UNSUPPORTED;
        pExtInqData->NonVolatileCacheSupported   =
            NON_VOLATILE_CACHE_UNSUPPORTED;
        pExtInqData->VolatileCacheSupported      =
            pDevExt->controllerIdentifyData.VWC.Present;
        pExtInqData->Reserved4                   =
            RESERVED_FIELDS;

        pExtInqData->ProtectionInfoIntervalSupported =
            PROTECTION_INFO_INTERNALS_UNSUPPORTED;

        pExtInqData->Reserved5               = RESERVED_FIELDS;
        pExtInqData->LogicalUnitITNexusClear = LUN_UNIT_ATTENTIONS_CLEARED;
        pExtInqData->Reserved6               = RESERVED_FIELDS;
        pExtInqData->ReferralsSupported      = REFERRALS_UNSUPPORTED;
        pExtInqData->Reserved6               = RESERVED_FIELDS;

        pExtInqData->CapabilityBasedCommandSecurity =
            CAPABILITY_BASED_SECURITY_UNSUPPORTED;
        pExtInqData->Reserved7                      =
            RESERVED_FIELDS;
        pExtInqData->MultiITNexusMicrodeDownload    =
            MICROCODE_DOWNLOAD_VENDOR_SPECIFIC;

        pSrb->DataTransferLength =
            min(EXTENDED_INQUIRY_DATA_PAGE_SIZE, allocLength);
    }

    return returnStatus;
} /* SntiTranslateExtendedInquiryDataPage*/

/******************************************************************************
 * SntiTranslateStandardInquiryPage
 *
 * @brief Translates the SCSI Inquiry page - Standard Inquiry Page. Populates
 *        the appropriate SCSI Inqiry response fields based on the NVMe
 *        Translation spec. Do not need to create SQE here as we just complete
 *        the command in the build phase (by returning FALSE to StorPort with
 *        SRB status of SUCCESS).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return VOID
 ******************************************************************************/
VOID SntiTranslateStandardInquiryPage(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_DEVICE_EXTENSION pDevExt = NULL;
    PINQUIRYDATA pStdInquiry = NULL;
    UINT16 allocLength;

    pStdInquiry = (PINQUIRYDATA)GET_DATA_BUFFER(pSrb);
    allocLength = GET_INQ_ALLOC_LENGTH(pSrb);
    pDevExt = ((PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb))->pNvmeDevExt;

    memset(pStdInquiry, 0, allocLength);
    pStdInquiry->DeviceType          = DIRECT_ACCESS_DEVICE;
    pStdInquiry->DeviceTypeQualifier = DEVICE_CONNECTED;
    pStdInquiry->RemovableMedia      = UNREMOVABLE_MEDIA;
    pStdInquiry->Versions            = VERSION_SPC_4;
    pStdInquiry->NormACA             = ACA_UNSUPPORTED;
    pStdInquiry->HiSupport           = HIERARCHAL_ADDR_UNSUPPORTED;
    pStdInquiry->ResponseDataFormat  = RESPONSE_DATA_FORMAT_SPC_4;
    pStdInquiry->AdditionalLength    = ADDITIONAL_STD_INQ_LENGTH;
    pStdInquiry->EnclosureServices   = EMBEDDED_ENCLOSURE_SERVICES_UNSUPPORTED;
    pStdInquiry->MediumChanger       = MEDIUM_CHANGER_UNSUPPORTED;
    pStdInquiry->CommandQueue        = COMMAND_MANAGEMENT_MODEL;
    pStdInquiry->Wide16Bit           = WIDE_16_BIT_XFERS_UNSUPPORTED;
    pStdInquiry->Addr16              = WIDE_16_BIT_ADDRESES_UNSUPPORTED;
    pStdInquiry->Synchronous         = SYNCHRONOUS_DATA_XFERS_UNSUPPORTED;
    pStdInquiry->Reserved3[0]        = RESERVED_FIELD;

    /*
     *  Fields not defined in Standard Inquiry page from storport.h
     *
     *    - SCCS:    Embedded Storage Arrays
     *    - ACC:     Access Control Coordinator
     *    - TPGS:    Target Port Groupo Suppport
     *    - 3PC:     3rd Party Copy
     *    - Protect: LUN Protection Information
     *    - SPT:     Type of protection LUN supports
     */

    /* Vendor Id */
    StorPortCopyMemory(pStdInquiry->VendorId, "NVMe    ", VENDOR_ID_SIZE);

    /* Product Id - First 16 bytes of model # in Controller Identify structure*/
    StorPortCopyMemory(pStdInquiry->ProductId,
                       pDevExt->controllerIdentifyData.MN,
                       PRODUCT_ID_SIZE);

    /* Product Revision Level */
    StorPortCopyMemory(pStdInquiry->ProductRevisionLevel,
                       pDevExt->controllerIdentifyData.FR,
                       PRODUCT_REVISION_LEVEL_SIZE);

    pSrb->DataTransferLength = min(STANDARD_INQUIRY_LENGTH, allocLength);
} /* SntiTranslateStandardInquiryPage */

/******************************************************************************
 * SntiTranslateReportLuns
 *
 * @brief Translates the SCSI Report LUNs command. Populates the appropriate
 *        SCSI Report LUNs response data based on the NVMe Translation spec. Do
 *        not need to create SQE here as we just complete the command in the
 *        build phase (by returning FALSE to StorPort with SRB status of
 *        SUCCESS).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateReportLuns(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_DEVICE_EXTENSION pDevExt = NULL;
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PUCHAR pResponseBuffer = NULL;
    UINT32 numberOfLuns = 0;
    UINT32 numberOfLunsFound = 0;
    UINT32 lunListLength = 0;
    UINT32 allocLength = 0;
    UINT8 lunIdDataOffset = 0;
    UINT8 selectReport = 0;
    UCHAR lunExtIdx = 0;
    PNVME_LUN_EXTENSION pLunExt = NULL;

    /* Default to a successful command completion */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_COMMAND_COMPLETED;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    pDevExt = pSrbExt->pNvmeDevExt;
    pResponseBuffer = (PUCHAR)GET_DATA_BUFFER(pSrb);
    allocLength = GET_REPORT_LUNS_ALLOC_LENGTH(pSrb);
    selectReport = GET_U8_FROM_CDB(pSrb, REPORT_LUNS_SELECT_REPORT_OFFSET);

    /*
     * Set SRB status to success to indicate the command will complete
     * successfully (assuming no errors occur during translation) and
     * reset the status value to use below.
     */
    pSrb->SrbStatus = SRB_STATUS_SUCCESS;

    /* Set the completion routine - no translation necessary on completion */
    pSrbExt->pNvmeCompletionRoutine = NULL;

    if ((selectReport != ALL_LUNS_RETURNED)            &&
        (selectReport != ALL_WELL_KNOWN_LUNS_RETURNED) &&
        (selectReport != RESTRICTED_LUNS_RETURNED)) {
        SntiSetScsiSenseData(pSrb,
                             SCSISTAT_CHECK_CONDITION,
                             SCSI_SENSE_ILLEGAL_REQUEST,
                             SCSI_ADSENSE_INVALID_CDB,
                             SCSI_ADSENSE_NO_SENSE);

        pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        pSrb->DataTransferLength = 0;

        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
    } else {
        /*
         * Per the NVM Express spec, namespaces ids shall be allocated in order
         * (starting with 0) and packed sequentially.
         */
        numberOfLuns = pDevExt->visibleLuns;

        lunListLength = numberOfLuns * LUN_ENTRY_SIZE;

        memset(pResponseBuffer, 0, allocLength);
        lunIdDataOffset = REPORT_LUNS_FIRST_LUN_OFFSET;

        /* The first LUN Id will always be 0 per the SAM spec */
        for (lunExtIdx = 0; lunExtIdx < MAX_NAMESPACES; lunExtIdx++) {
             pLunExt = pDevExt->pLunExtensionTable[lunExtIdx];
             if ((pLunExt->slotStatus == ONLINE) &&
                 (++numberOfLunsFound <= numberOfLuns)) {
            /*
             * Set the LUN Id and then increment to the next LUN location in
             * the paramter data (8 byte offset each time),
             * lunNum position is at byte 1 in the resp buffer per SAM3
             */
            #define SINGLE_LVL_LUN_OFFSET (1)
                    pResponseBuffer[lunIdDataOffset + SINGLE_LVL_LUN_OFFSET] = lunExtIdx;
            lunIdDataOffset += LUN_ENTRY_SIZE;
            if (lunIdDataOffset >= allocLength) {
                break;
            }
        }
        }

        /* Set the LUN LIST LENGTH field */
        pResponseBuffer[BYTE_0] =
            (UCHAR)((lunListLength & DWORD_MASK_BYTE_3) >> BYTE_SHIFT_3);
        pResponseBuffer[BYTE_1] =
            (UCHAR)((lunListLength & DWORD_MASK_BYTE_2) >> BYTE_SHIFT_2);
        pResponseBuffer[BYTE_2] =
            (UCHAR)((lunListLength & DWORD_MASK_BYTE_1) >> BYTE_SHIFT_1);
        pResponseBuffer[BYTE_3] =
            (UCHAR)((lunListLength & DWORD_MASK_BYTE_0));

        pSrb->DataTransferLength =
            min((lunListLength + LUN_DATA_HEADER_SIZE), allocLength);
    }

    return returnStatus;
} /* SntiTranslateReportLuns */
/******************************************************************************
 * SntiTranslateReadCapacity
 *
 * @brief Translates the SCSI Read Capacity command. Populates the appropriate
 *        SCSI Read Capacity parameter data response fields based on the NVMe
 *        Translation spec. Do not need to create SQE here as we just complete
 *        the command in the build phase (by returning FALSE to StorPort with
 *        SRB status of SUCCESS).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateReadCapacity(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_LUN_EXTENSION pLunExt = NULL;
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PUCHAR pResponseBuffer = NULL;
    SNTI_STATUS status;
    UINT8 opcode;

    /* Default to a successful command completion */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_COMMAND_COMPLETED;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    pResponseBuffer = (PUCHAR)GET_DATA_BUFFER(pSrb);

    /*
     * Set SRB status to success to indicate the command will complete
     * successfully (assuming no errors occur during translation) and
     * reset the status value to use below.
     */
    pSrb->SrbStatus = SRB_STATUS_SUCCESS;

    /* Set the completion routine - no translation necessary on completion */
    pSrbExt->pNvmeCompletionRoutine = NULL;

    status = GetLunExtension(pSrbExt, &pLunExt);
    if (status != SNTI_SUCCESS) {
        SntiMapInternalErrorStatus(pSrb, status);
        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
    } else {
        opcode = GET_OPCODE(pSrb);
        if (opcode == SCSIOP_READ_CAPACITY) {
            returnStatus = SntiTranslateReadCapacity10(pSrb,
                                                       pResponseBuffer,
                                                       pLunExt);
        } else if (opcode == SCSIOP_READ_CAPACITY16) {
            returnStatus = SntiTranslateReadCapacity16(pSrb,
                                                       pResponseBuffer,
                                                       pLunExt);
        } else {
            ASSERT(FALSE);
        }
    }

    return returnStatus;
} /* SntiTranslateReadCapacity */

/******************************************************************************
 * SntiTranslateReadCapacity10
 *
 * @brief Translates the SCSI Read Capacity 10 command.
 *
 *        NOTE 1: SBC-3 r27 does not define Allocation Length for READ CAP 10
 *        NOTE 2: NVMe/SCSI Translation spec - Returned LBA is 0xFFFFFFFF
 *        NOTE 3: LBA Length in Bytes - Set to LBA Data Size (LBADS) field of
 *                the LBA Format Data structure indicated by the Formatted LBA
 *                Size (FLBAS) field within the Identify Namespace Data
 *                Structure. Bits 0-3 of the FLBAS field indicate is used as an
 *                index into the array at the end of the namespace identify
 *                structure... the array is the types of LBA formats... LBA Data
 *                Size (LBADS) is in terms of power of two (2^n).
 *
 *        SCSI Compliance Testing Note:
 *
 *        For the Microsoft SCSI Compliance Test 2.0, there will be one failed
 *        test case due to revision implementation. This NVM Express driver is
 *        coded to SBC-3 revision 27, but the Microsoft SCSI Compliance Test
 *        suite 2.0 follows the SBC-2 revision 16 spec for a negative test for
 *        READ CAP 10. Because the SCSI complaince test case is testing against
 *        an older revision of the SBC spec, there is a failure that occurs when
 *        the test sends a READ CAP 10 with a PMI bit set to 0 and the LBA is
 *        non zero. The test expects a check condition in this case. However, in
 *        the SBC-3 revision 27 spec, this test scenario is perfecly allowable.
 *        Therefore this driver has been coded according the definition of the
 *        SBC-3 spec. Please note that this failure is a false negative failure
 *        and any future testing may show a failure when, in fact, the code is
 *        correct.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param pResponseBuffer - This parameter is the buffer to which the Read
 *                          Capacity 10 paramter data is written for this
 *                          namespace/LUN.
 * @param pLunExt - This parameter is the LUN Extension pointer that contains
 *                  the namespace identify data structure.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateReadCapacity10(
    PSCSI_REQUEST_BLOCK pSrb,
    PUCHAR pResponseBuffer,
    PNVME_LUN_EXTENSION pLunExt
)
{
    PREAD_CAPACITY_DATA pReadCapacityData = NULL;
    UINT64 namespaceSize;
    UINT32 lastLba;
    UINT32 lbaLength;
    UINT32 lbaLengthPower;
    UINT8 flbas;

    /* Default to a successful command completion */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_COMMAND_COMPLETED;

    /* Use default READ CAP 10 struct provided by StorPort */
    pReadCapacityData = (PREAD_CAPACITY_DATA)pResponseBuffer;

    /* LBA Length */
    flbas = pLunExt->identifyData.FLBAS.SupportedCombination;
    lbaLengthPower = pLunExt->identifyData.LBAFx[flbas].LBADS;
    lbaLength = 1 << lbaLengthPower;

    if (lbaLength < DEFAULT_SECTOR_SIZE) {
        SntiMapInternalErrorStatus(pSrb, SNTI_FAILURE);
        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
    } else {
        memset(pReadCapacityData, 0, sizeof(READ_CAPACITY_DATA));

        /*
         * NOTE: SCSI Compliance Suite 2.0 incorrect failure happens since
         *       there is no checking of the PMI bit and LBA. Older revisions
         *       of the SBC spec (i.e. SBC-2 r16) support PMI, however, SBC-3
         *       r27, obsoletes PMI and checking for PMI. Refer to comments.
         */

        /*
         * Last LBA - If the NSZE is greater than a DWORD, set to all F's...
         *
         * From SBC-3 r27:
         *
         *   If the RETURNED LOGICAL BLOCK ADDRESS field is set to FFFF_FFFFh,
         *   then the application client should issue a READ CAPACITY (16)
         *   command (see 5.16) to request that the device server transfer the
         *   READ CAPACITY (16) parameter data to the data-in buffer.
         */
        namespaceSize = pLunExt->identifyData.NSZE;

        if ((namespaceSize & UPPER_DWORD_BIT_MASK) != 0)
            lastLba = LBA_MASK_LOWER_32_BITS;
        else
            lastLba = (UINT32)namespaceSize;

        /* NSZE is not zero based */
        lastLba--;

        /* Must byte swap these as they are returned in big endian */
        REVERSE_BYTES(&pReadCapacityData->LogicalBlockAddress, &lastLba);
        REVERSE_BYTES(&pReadCapacityData->BytesPerBlock, &lbaLength);

        pSrb->DataTransferLength = READ_CAP_10_PARM_DATA_SIZE;
    }

    return returnStatus;
} /* SntiTranslateReadCapacity10 */

/******************************************************************************
 * SntiTranslateReadCapacity16
 *
 * @brief Translates the SCSI Read Capacity 16 command.
 *
 *        NOTE 1: NVMe/SCSI Translation spec - Returned LBA is 0xFFFFFFFF
 *        NOTE 2: LBA Length in Bytes - Set to LBA Data Size (LBADS) field of
 *                the LBA Format Data structure indicated by the Formatted LBA
 *                Size (FLBAS) field within the Identify Namespace Data
 *                Structure. Bits 0-3 of the FLBAS field indicate is used as an
 *                index into the array at the end of the namespace identify
 *                structure... the array is the types of LBA formats... LBA Data
 *                Size (LBADS) is in terms of power of two (2^n).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param pResponseBuffer - This parameter is the buffer to which the Read
 *                          Capacity 10 paramter data is written for this
 *                          namespace/LUN.
 * @param pLunExt - This parameter is the LUN Extension pointer that
 *                  contains the namespace identify data structure.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateReadCapacity16(
    PSCSI_REQUEST_BLOCK pSrb,
    PUCHAR pResponseBuffer,
    PNVME_LUN_EXTENSION pLunExt
)
{
    PREAD_CAPACITY_16_DATA pReadCapacityData = NULL;
    UINT64 lastLba;
    UINT32 lbaLength;
    UINT32 allocLength;
    UINT32 lbaLengthPower;
    UINT8  flbas;
    UINT8  dps;
    UINT8  protectionType;
    UINT8  protectionEnabled;

    /* Default to a successful command completion */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_COMMAND_COMPLETED;

    pReadCapacityData = (PREAD_CAPACITY_16_DATA)pResponseBuffer;
    allocLength = GET_READ_CAP_16_ALLOC_LENGTH(pSrb);

    flbas = pLunExt->identifyData.FLBAS.SupportedCombination;
    lbaLengthPower = pLunExt->identifyData.LBAFx[flbas].LBADS;
    lbaLength = 1 << lbaLengthPower;

    if (lbaLength < DEFAULT_SECTOR_SIZE) {
        SntiMapInternalErrorStatus(pSrb, SNTI_FAILURE);
        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
    } else {
        memset(pReadCapacityData, 0, sizeof(READ_CAPACITY_16_DATA));

        /* Get the Data Protection Settings (DPS) */
        dps = pLunExt->identifyData.DPS.ProtectionEnabled;
        lastLba = pLunExt->identifyData.NSZE;

        /* NSZE is not zero based */
        lastLba--;

        if (!dps) {
            /* If the DPS settings are 0, then protection is disabled */
            protectionEnabled = PROTECTION_DISABLED;
            protectionType = UNSPECIFIED;
        } else {
            protectionEnabled = PROTECTION_ENABLED;
            switch(dps) {
                case 1:
                    /* 000b - NVMe translation spec (6.4 - Table 6-16) */
                    protectionType = 0;
                break;
                case 2:
                    /* 001b - NVMe translation spec (6.4 - Table 6-16) */
                    protectionType = 1;
                break;
                case 3:
                    /* 010b - NVMe translation spec (6.4 - Table 6-16) */
                    protectionType = 2;
                break;
                default:
                    /* Undefined - NVMe translation spec (6.4 - Table 6-16) */
                    protectionType = 0;
                break;
            }; /* end switch */
        }

        /* Must byte swap these as they are returned in big endian */
        REVERSE_BYTES_QUAD(&pReadCapacityData->LogicalBlockAddress, &lastLba);
        REVERSE_BYTES(&pReadCapacityData->BytesPerBlock, &lbaLength);

        pReadCapacityData->ProtectionType          = protectionType;
        pReadCapacityData->ProtectionEnable        = protectionEnabled;
        pReadCapacityData->ProtectionInfoIntervals = UNSPECIFIED;

        pReadCapacityData->LogicalBlocksPerPhysicalBlockExponent =
            ONE_OR_MORE_PHYSICAL_BLOCKS;

        pReadCapacityData->LogicalBlockProvisioningMgmtEnabled = UNSPECIFIED;
        pReadCapacityData->LogicalBlockProvisioningReadZeros   = UNSPECIFIED;
        pReadCapacityData->LowestAlignedLbaMsb                 = LBA_0;
        pReadCapacityData->LowestAlignedLbaLsb                 = LBA_0;

        pSrb->DataTransferLength = min(allocLength, READ_CAP_16_PARM_DATA_SIZE);
    }

    return returnStatus;
} /* SntiTranslateReadCapacity16 */

/******************************************************************************
 * SntiTranslateWrite
 *
 * @brief Translates the SCSI Write command based on the NVMe Translation spec
 *        and populates a temporary SQE stored in the SRB Extension.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateWrite(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_DEVICE_EXTENSION pDevExt = NULL;
    PNVME_LUN_EXTENSION pLunExt = NULL;
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PSTOR_SCATTER_GATHER_LIST pSgl = NULL;
    SNTI_STATUS status;
    UINT8 opcode;

    /* Default to successful translation */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_TRANSLATION_SUCCESS;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    pDevExt = pSrbExt->pNvmeDevExt;

    status = GetLunExtension(pSrbExt, &pLunExt);
    if (status != SNTI_SUCCESS) {
        /* Map the translation error to a SCSI error */
        SntiMapInternalErrorStatus(pSrb, status);
        return SNTI_FAILURE_CHECK_RESPONSE_DATA;
    }

    pSgl = StorPortGetScatterGatherList(pSrbExt->pNvmeDevExt, pSrb);
    ASSERT(pSgl != NULL);

    /* Set the SRB status to pending - controller communication necessary */
    pSrb->SrbStatus = SRB_STATUS_PENDING;

    /* Set the completion routine - no translation necessary on completion */
    pSrbExt->pNvmeCompletionRoutine = NULL;

    /* Set up common portions of the NVMe WRITE command */
    memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));

    pSrbExt->nvmeSqeUnit.CDW0.OPC = NVME_WRITE;
    pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
    pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;
    pSrbExt->nvmeSqeUnit.NSID = pLunExt->namespaceId;

    /* TBD: Contiguous physical buffer of metadata (DWORD alinged) */
    pSrbExt->nvmeSqeUnit.MPTR = 0;

    /* PRP Entry/List */
    SntiTranslateSglToPrp(pSrbExt, pSgl);

    /* Complete the non-common translation fields for the command */
    opcode = GET_OPCODE(pSrb);
    switch (opcode) {
        case SCSIOP_WRITE6:
            status = SntiTranslateWrite6(pSrbExt, pLunExt);
        break;
        case SCSIOP_WRITE:
            status = SntiTranslateWrite10(pSrbExt, pLunExt);
        break;
        case SCSIOP_WRITE12:
            status = SntiTranslateWrite12(pSrbExt, pLunExt);
        break;
        case SCSIOP_WRITE16:
            status = SntiTranslateWrite16(pSrbExt, pLunExt);
        break;
        default:
            ASSERT(FALSE);
            status = SNTI_FAILURE;
        break;
    }; /* end switch */

    if (status != SNTI_SUCCESS)
        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;

    return returnStatus;
} /* SntiTranslateWrite */

/******************************************************************************
 * SntiTranslateWrite6
 *
 * @brief Translates the SCSI Write 6 command to an NVMe WRITE command.
 *        NOTE: FUA is not defined for WRITE 6
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 *
 * @return SNTI_STATUS
 *     Indicates internal translation status
 ******************************************************************************/
SNTI_STATUS SntiTranslateWrite6(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
)
{
    SNTI_STATUS status = SNTI_SUCCESS;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;

    UINT32 lba = 0;
    UINT8 length = 0;

    lba = GET_U24_FROM_CDB(pSrb, WRITE_6_CDB_LBA_OFFSET);
    length = GET_U8_FROM_CDB(pSrb, WRITE_6_CDB_TX_LEN_OFFSET);

    /* Mask off the unnecessary bits and validate the LBA range */
    lba &= WRITE_6_CDB_LBA_MASK;

    status = SntiValidateLbaAndLength(pLunExt,
                                      pSrbExt,
                                      (UINT64)lba,
                                      (UINT32)length);

    if (status == SNTI_SUCCESS) {
        /* Command DWORD 10/11 - Starting LBA */
        pSrbExt->nvmeSqeUnit.CDW10 = lba;
        pSrbExt->nvmeSqeUnit.CDW11 = 0;

        /* Command DWORD 12 - LR/FUA/PRINFO/NLB */
        pSrbExt->nvmeSqeUnit.CDW12 |= FUA_ENABLED;
        pSrbExt->nvmeSqeUnit.CDW12 |= length - 1; /* 0's based */
    }

    return status;
} /* SntiTranslateWrite6 */

/******************************************************************************
 * SntiTranslateWrite10
 *
 * @brief Translates the SCSI Write 10 command to an NVMe WRITE command.
 *        NOTE: The following Write CDB fields are defined as "unspecified" or
 *        "unsupported" in the SCSI/NVMe Translation specification:
 *
 *        - DPO (Disable Page Out)
 *        - FUA_NV (Force Unit Access from Non-Volatile Memory)
 *        - Group Number
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 *
 * @return SNTI_STATUS
 *     Indicates internal translation status
 ******************************************************************************/
SNTI_STATUS SntiTranslateWrite10(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
)
{
    SNTI_STATUS status = SNTI_SUCCESS;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;

    UINT32 lba = 0;
    UINT16 length = 0;
    UINT8 fua = 0;

    lba = GET_U32_FROM_CDB(pSrb, WRITE_10_CDB_LBA_OFFSET);
    length = GET_U16_FROM_CDB(pSrb, WRITE_10_CDB_TX_LEN_OFFSET);
    fua = GET_U8_FROM_CDB(pSrb, WRITE_CDB_FUA_OFFSET);
    fua &= WRITE_CDB_FUA_MASK;

    status = SntiValidateLbaAndLength(pLunExt,
                                      pSrbExt,
                                      (UINT64)lba,
                                      (UINT32)length);

    if (status == SNTI_SUCCESS) {
        /* Command DWORD 10/11 - Starting LBA */
        pSrbExt->nvmeSqeUnit.CDW10 = lba;
        pSrbExt->nvmeSqeUnit.CDW11 = 0;

        /* Command DWORD 12 - LR/FUA/PRINFO/NLB */
        pSrbExt->nvmeSqeUnit.CDW12 |= (fua ? FUA_ENABLED : FUA_DISABLED);
        pSrbExt->nvmeSqeUnit.CDW12 |= length - 1; /* 0's based */
    }

    return status;
} /* SntiTranslateWrite10 */

/******************************************************************************
 * SntiTranslateWrite12
 *
 * @brief Translates the SCSI Write 12 command to an NVMe WRITE command.
 *        NOTE: The following Write CDB fields are defined as "unspecified" or
 *        "unsupported" in the SCSI/NVMe Translation specification:
 *
 *        - DPO (Disable Page Out)
 *        - FUA_NV (Force Unit Access from Non-Volatile Memory)
 *        - Group Number
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 *
 * @return SNTI_STATUS
 *     Indicates internal translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateWrite12(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
)
{
    SNTI_STATUS status = SNTI_SUCCESS;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;

    UINT32 lba = 0;
    UINT32 length = 0;
    UINT8 fua = 0;

    lba = GET_U32_FROM_CDB(pSrb, WRITE_12_CDB_LBA_OFFSET);
    length = GET_U32_FROM_CDB(pSrb, WRITE_12_CDB_TX_LEN_OFFSET);
    fua = GET_U8_FROM_CDB(pSrb, WRITE_CDB_FUA_OFFSET);
    fua &= WRITE_CDB_FUA_MASK;

    status = SntiValidateLbaAndLength(pLunExt,
                                      pSrbExt,
                                      (UINT64)lba,
                                      (UINT32)length);

    if (status == SNTI_SUCCESS) {
        /* Command DWORD 10/11 - Starting LBA */
        pSrbExt->nvmeSqeUnit.CDW10 = lba;
        pSrbExt->nvmeSqeUnit.CDW11 = 0;

        /* Command DWORD 12 - LR/FUA/PRINFO/NLB */
        pSrbExt->nvmeSqeUnit.CDW12 |= (fua ? FUA_ENABLED : FUA_DISABLED);
        pSrbExt->nvmeSqeUnit.CDW12 |= (length & DWORD_BIT_MASK) - 1; /* 0's based */
    }

    return status;
} /* SntiTranslateWrite12 */

/******************************************************************************
 * SntiTranslateWrite16
 *
 * @brief Translates the SCSI Write 16 command to an NVMe WRITE command.
 *        NOTE: The following Write CDB fields are defined as "unspecified" or
 *        "unsupported" in the SCSI/NVMe Translation specification:
 *
 *        - DPO (Disable Page Out)
 *        - FUA_NV (Force Unit Access from Non-Volatile Memory)
 *        - Group Number
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 *
 * @return SNTI_STATUS
 *     Indicates internal translation status.
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateWrite16(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
)
{
    SNTI_STATUS status = SNTI_SUCCESS;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;

    UINT64 lba = 0;
    UINT32 length = 0;
    UINT8 fua = 0;

    lba = (UINT64)
        ((((UINT64)(GET_U32_FROM_CDB(pSrb, WRITE_16_CDB_LBA_OFFSET + 0)))
          << DWORD_SHIFT_MASK) |
         (((UINT64)(GET_U32_FROM_CDB(pSrb, WRITE_16_CDB_LBA_OFFSET + 4)))
          & DWORD_BIT_MASK));

    length = GET_U32_FROM_CDB(pSrb, WRITE_16_CDB_TX_LEN_OFFSET);
    fua = GET_U8_FROM_CDB(pSrb, WRITE_CDB_FUA_OFFSET);
    fua &= WRITE_CDB_FUA_MASK;

    status = SntiValidateLbaAndLength(pLunExt, pSrbExt, lba, length);

    if (status == SNTI_SUCCESS) {
        /* Command DWORD 10/11 - Starting LBA */
        pSrbExt->nvmeSqeUnit.CDW10 = (UINT32)(lba & DWORD_BIT_MASK);
        pSrbExt->nvmeSqeUnit.CDW11 = (UINT32)(lba >> DWORD_SHIFT_MASK);

        /* Command DWORD 12 - LR/FUA/PRINFO/NLB */
        pSrbExt->nvmeSqeUnit.CDW12 |= (fua ? FUA_ENABLED : FUA_DISABLED);
        pSrbExt->nvmeSqeUnit.CDW12 |= (length & DWORD_BIT_MASK) - 1; /* 0's based */
    }

    return status;
} /* SntiTranslateWrite16 */

/******************************************************************************
 * SntiTranslateRead
 *
 * @brief Translates the SCSI Read command based on the NVMe Translation spec
 *        and populates a temporary SQE stored in the SRB Extension.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateRead(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_DEVICE_EXTENSION pDevExt = NULL;
    PNVME_LUN_EXTENSION pLunExt = NULL;
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PSTOR_SCATTER_GATHER_LIST pSgl = NULL;
    SNTI_STATUS status;
    UINT8 opcode;

    /* Default to successful translation */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_TRANSLATION_SUCCESS;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    pDevExt = pSrbExt->pNvmeDevExt;

    status = GetLunExtension(pSrbExt, &pLunExt);
    if (status != SNTI_SUCCESS) {
        /* Map the translation error to a SCSI error */
        SntiMapInternalErrorStatus(pSrb, status);
        return SNTI_FAILURE_CHECK_RESPONSE_DATA;
    }

    pSgl = StorPortGetScatterGatherList(pSrbExt->pNvmeDevExt, pSrb);
    ASSERT(pSgl != NULL);

    /* Set the SRB status to pending - controller communication necessary */
    pSrb->SrbStatus = SRB_STATUS_PENDING;
    pSrbExt->pNvmeCompletionRoutine = NULL;

    /* Set up common portions of the NVMe WRITE command */
    memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));

    pSrbExt->nvmeSqeUnit.CDW0.OPC = NVME_READ;
    pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
    pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;
    pSrbExt->nvmeSqeUnit.NSID = pLunExt->namespaceId;

    /* TBD: Contiguous physical buffer of metadata (DWORD alinged) */
    pSrbExt->nvmeSqeUnit.MPTR = 0;

    /* PRP Entry/List */
    SntiTranslateSglToPrp(pSrbExt, pSgl);

    /* Complete the non-common translation fields for the command */
    opcode = GET_OPCODE(pSrb);
    switch (opcode) {
        case SCSIOP_READ6:
            status = SntiTranslateRead6(pSrbExt, pLunExt);
        break;
        case SCSIOP_READ:
            status = SntiTranslateRead10(pSrbExt, pLunExt);
        break;
        case SCSIOP_READ12:
            status = SntiTranslateRead12(pSrbExt, pLunExt);
        break;
        case SCSIOP_READ16:
            status = SntiTranslateRead16(pSrbExt, pLunExt);
        break;
        default:
            ASSERT(FALSE);
            status = SNTI_FAILURE;
        break;
    }; /* end switch */

    if (status != SNTI_SUCCESS)
        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;

    return returnStatus;
} /* SntiTranslateRead */

/******************************************************************************
 * SntiTranslateRead6
 *
 * @brief Translates the SCSI READ 6 command to an NVMe READ command.
 *        NOTE: FUA is not defined for READ 6
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 *
 * @return SNTI_STATUS
 *     Indicates internal translation status
 ******************************************************************************/
SNTI_STATUS SntiTranslateRead6(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
)
{
    SNTI_STATUS status = SNTI_SUCCESS;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;

    UINT32 lba = 0;
    UINT8 length = 0;

    lba = GET_U24_FROM_CDB(pSrb, READ_6_CDB_LBA_OFFSET);
    length = GET_U8_FROM_CDB(pSrb, READ_6_CDB_TX_LEN_OFFSET);

    /* Mask off the unnecessary bits and validate the LBA range */
    lba &= READ_6_CDB_LBA_MASK;

    status = SntiValidateLbaAndLength(pLunExt,
                                      pSrbExt,
                                      (UINT64)lba,
                                      (UINT32)length);

    if (status == SNTI_SUCCESS) {
        /* Command DWORD 10/11 - Starting LBA */
        pSrbExt->nvmeSqeUnit.CDW10 = lba;
        pSrbExt->nvmeSqeUnit.CDW11 = 0;

        /* Command DWORD 12 - LR/FUA/PRINFO/NLB */
        pSrbExt->nvmeSqeUnit.CDW12 |= FUA_ENABLED;
        pSrbExt->nvmeSqeUnit.CDW12 |= length - 1; /* 0's based */
    }

    return status;
} /* SntiTranslateRead6 */

/******************************************************************************
 * SntiTranslateRead10
 *
 * @brief Translates the SCSI Read 10 command to an NVMe READ command.
 *        NOTE: The following Read CDB fields are defined as "unspecified" or
 *        "unsupported" in the SCSI/NVMe Translation specification:
 *
 *        - DPO (Disable Page Out)
 *        - FUA_NV (Force Unit Access from Non-Volatile Memory)
 *        - Group Number
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 *
 * @return SNTI_STATUS
 *     Indicates internal translation status
 ******************************************************************************/
SNTI_STATUS SntiTranslateRead10(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
)
{
    SNTI_STATUS status = SNTI_SUCCESS;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;

    UINT32 lba = 0;
    UINT16 length = 0;
    UINT8 fua = 0;

    lba = GET_U32_FROM_CDB(pSrb, READ_10_CDB_LBA_OFFSET);
    length = GET_U16_FROM_CDB(pSrb, READ_10_CDB_TX_LEN_OFFSET);
    fua = GET_U8_FROM_CDB(pSrb, READ_CDB_FUA_OFFSET);
    fua &= READ_CDB_FUA_MASK;

    status = SntiValidateLbaAndLength(pLunExt,
                                      pSrbExt,
                                      (UINT64)lba,
                                      (UINT32)length);

    if (status == SNTI_SUCCESS) {
        /* Command DWORD 10/11 - Starting LBA */
        pSrbExt->nvmeSqeUnit.CDW10 = lba;
        pSrbExt->nvmeSqeUnit.CDW11 = 0;

        /* Command DWORD 12 - LR/FUA/PRINFO/NLB */
        pSrbExt->nvmeSqeUnit.CDW12 |= (fua ? FUA_ENABLED : FUA_DISABLED);
        pSrbExt->nvmeSqeUnit.CDW12 |= length - 1; /* 0's based */
    }

    return status;
} /* SntiTranslateRead10 */

/******************************************************************************
 * SntiTranslateRead12
 *
 * @brief Translates the SCSI Read 12 command to an NVMe READ command.
 *        NOTE: The following Read CDB fields are defined as "unspecified" or
 *        "unsupported" in the SCSI/NVMe Translation specification:
 *
 *        - DPO (Disable Page Out)
 *        - FUA_NV (Force Unit Access from Non-Volatile Memory)
 *        - Group Number
 *
 * @param pSrbExt - Pointer to SRB extension
 *
 * @param pLunExt - Pointer to LUN extension
 *
 * @return SNTI_STATUS
 *     Indicates internal translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateRead12(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
)
{
    SNTI_STATUS status = SNTI_SUCCESS;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;

    UINT32 lba = 0;
    UINT32 length = 0;
    UINT8 fua = 0;

    lba = GET_U32_FROM_CDB(pSrb, READ_12_CDB_LBA_OFFSET);
    length = GET_U32_FROM_CDB(pSrb, READ_12_CDB_TX_LEN_OFFSET);
    fua = GET_U8_FROM_CDB(pSrb, READ_CDB_FUA_OFFSET);
    fua &= READ_CDB_FUA_MASK;

    status = SntiValidateLbaAndLength(pLunExt,
                                      pSrbExt,
                                      (UINT64)lba,
                                      (UINT32)length);

    if (status == SNTI_SUCCESS) {
        /* Command DWORD 10/11 - Starting LBA */
        pSrbExt->nvmeSqeUnit.CDW10 = lba;
        pSrbExt->nvmeSqeUnit.CDW11 = 0;

        /* Command DWORD 12 - LR/FUA/PRINFO/NLB */
        pSrbExt->nvmeSqeUnit.CDW12 |= (fua ? FUA_ENABLED : FUA_DISABLED);

        /* NVMe Translation Spec ERRATA... NLB is only 16 bits!!! */
        pSrbExt->nvmeSqeUnit.CDW12 |= (length & DWORD_BIT_MASK) - 1; /* 0's based */

    }

    return status;
} /* SntiTranslateRead12 */

/******************************************************************************
 * SntiTranslateRead16
 *
 * @brief Translates the SCSI Read 16 command to an NVMe READ command.
 *        NOTE: The following Read CDB fields are defined as "unspecified" or
 *        "unsupported" in the SCSI/NVMe Translation specification:
 *
 *        - DPO (Disable Page Out)
 *        - FUA_NV (Force Unit Access from Non-Volatile Memory)
 *        - Group Number
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 *
 * @return SNTI_STATUS
 *     Indicates internal translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateRead16(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
)
{
    SNTI_STATUS status = SNTI_SUCCESS;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;

    UINT64 lba = 0;
    UINT32 length = 0;
    UINT8 fua = 0;

    lba = (UINT64)
        ((((UINT64)(GET_U32_FROM_CDB(pSrb, READ_16_CDB_LBA_OFFSET + 0)))
          << DWORD_SHIFT_MASK) |
         (((UINT64)(GET_U32_FROM_CDB(pSrb, READ_16_CDB_LBA_OFFSET + 4)))
          & DWORD_BIT_MASK));

    length = GET_U32_FROM_CDB(pSrb, READ_16_CDB_TX_LEN_OFFSET);
    fua = GET_U8_FROM_CDB(pSrb, READ_CDB_FUA_OFFSET);
    fua &= READ_CDB_FUA_MASK;

    status = SntiValidateLbaAndLength(pLunExt, pSrbExt, lba, length);

    if (status == SNTI_SUCCESS) {
        /* Command DWORD 10/11 - Starting LBA */
        pSrbExt->nvmeSqeUnit.CDW10 = (UINT32)(lba & DWORD_BIT_MASK);
        pSrbExt->nvmeSqeUnit.CDW11 = (UINT32)(lba >> DWORD_SHIFT_MASK);

        /* Command DWORD 12 - LR/FUA/PRINFO/NLB */
        pSrbExt->nvmeSqeUnit.CDW12 |= (fua ? FUA_ENABLED : FUA_DISABLED);
        pSrbExt->nvmeSqeUnit.CDW12 |= (length & DWORD_BIT_MASK) - 1; /* 0's based */
    }

    return status;
} /* SntiTranslateRead16 */

/******************************************************************************
 * SntiTranslateRequestSense
 *
 * @brief Translates the SCSI Request Sense command. Populates the appropriate
 *        SCSI Request Sense response data based on the NVMe Translation spec.
 *        Do not need to create an SQE here as we just complete the command in
 *        the build phase (by returning FALSE to StorPort with SRB status of
 *        SUCCESS).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateRequestSense(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_DEVICE_EXTENSION pDevExt = NULL;
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    UINT32 allocLength;
    UINT8 descFormat;

    /* Default to command completed */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_COMMAND_COMPLETED;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    pDevExt = pSrbExt->pNvmeDevExt;
    allocLength = GET_REQUEST_SENSE_ALLOC_LENGTH(pSrb);
    descFormat = GET_U8_FROM_CDB(pSrb, REQUEST_SENSE_DESCRIPTOR_FORMAT_OFFSET);

    descFormat &= REQUEST_SENSE_DESCRIPTOR_FORMAT_MASK;

    /*
     * Set SRB status to success to indicate the command will complete
     * successfully (assuming no errors occur during translation) and
     * reset the status value to use below.
     */
    pSrb->SrbStatus = SRB_STATUS_SUCCESS;

    /* Set the completion routine - no translation necessary on completion */
    pSrbExt->pNvmeCompletionRoutine = NULL;

    /*
     * NOTE: SCSI to NVMe translation spec statest that DESC bit must only be
     *       set to 1. MS SCSI Compliance Suite will send a Request Sense cmd
     *       with this field set to 0 and expect a good status.
     */
    if (descFormat == DESCRIPTOR_FORMAT_SENSE_DATA_TYPE) {
        /* Descriptor Format Sense Data */
        PDESCRIPTOR_FORMAT_SENSE_DATA pSenseData = NULL;
        pSenseData = (PDESCRIPTOR_FORMAT_SENSE_DATA)pSrb->SenseInfoBuffer;

        memset(pSenseData, 0, allocLength);
        pSenseData->ResponseCode                 = DESC_FORMAT_SENSE_DATA;
        pSenseData->SenseKey                     = SCSI_SENSE_NO_SENSE;
        pSenseData->AdditionalSenseCode          = SCSI_ADSENSE_NO_SENSE;
        pSenseData->AdditionalSenseCodeQualifier = 0;
        pSenseData->AdditionalSenseLength        = 0;

        pSrb->DataTransferLength =
            min(sizeof(DESCRIPTOR_FORMAT_SENSE_DATA), allocLength);
    } else {
        /* Fixed Format Sense Data */
        PSENSE_DATA pSenseData = NULL;
        pSenseData = (PSENSE_DATA)pSrb->SenseInfoBuffer;

        memset(pSenseData, 0, allocLength);
        pSenseData->ErrorCode                    = FIXED_SENSE_DATA;
        pSenseData->SenseKey                     = SCSI_SENSE_NO_SENSE;
        pSenseData->AdditionalSenseLength        = FIXED_SENSE_DATA_ADD_LENGTH;
        pSenseData->AdditionalSenseCode          = SCSI_ADSENSE_NO_SENSE;
        pSenseData->AdditionalSenseCodeQualifier = 0;

        pSrb->DataTransferLength = min(sizeof(SENSE_DATA), allocLength);
    }

    return returnStatus;
} /* SntiTranslateRequestSense */

/******************************************************************************
 * SntiTranslateSecurityProtocol
 *
 * @brief Translates the SCSI Security Protocol In/Out commands based on the
 *        NVMe Translation spec to a NVMe Security Receive/Send command and
 *        populates a temporary SQE stored in the SRB Extension.
 *
 *        NOTE: From the SPC-4 r.24 specification:
 *
 *        The device server shall retain data resulting from a SECURITY PROTOCOL
 *        OUT command, if any, until one of the following events is processed:
 *
 *           a) transfer of the data via a SECURITY PROTOCOL IN command from
 *              the same I_T_L nexus as defined by the protocol specified by
 *              the SECURITY PROTOCOL field (see table 259);
 *           b) logical unit reset (See SAM-4); or
 *           c) I_T nexus loss (See SAM-4) associated with the I_T nexus that
 *              sent the SECURITY PROTOCOL OUT command.
 *
 *        If the data is lost due to one of these events the application client
 *        may send a new SECURITY PROTOCOL OUT command to retry the operation.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateSecurityProtocol(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PNVME_LUN_EXTENSION pLunExt = NULL;
    UINT32 length;
    UINT16 secProtocolSp;
    UINT8 secProtocol;
    UINT8 inc_512;
    UINT8 opcode;
    SNTI_STATUS status;

    /* Default to successful translation */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_TRANSLATION_SUCCESS;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    length = GET_U32_FROM_CDB(pSrb, SECURITY_PROTOCOL_CDB_LENGTH_OFFSET);
    secProtocol = GET_U8_FROM_CDB(pSrb, SECURITY_PROTOCOL_CDB_SEC_PROT_OFFSET);
    secProtocolSp =
        GET_U16_FROM_CDB(pSrb, SECURITY_PROTOCOL_CDB_SEC_PROT_SP_OFFSET);
    inc_512 = GET_U8_FROM_CDB(pSrb, SECURITY_PROTOCOL_CDB_INC_512_OFFSET);
    inc_512 &= SECURITY_PROTOCOL_CDB_INC_512_MASK;

    if ((inc_512 >> SECURITY_PROTOCOL_CDB_INC_512_SHIFT) != 0) {
        /* Ensure correct sense data for SCSI compliance test case 1.4 */
        SntiSetScsiSenseData(pSrb,
                             SCSISTAT_CHECK_CONDITION,
                             SCSI_SENSE_ILLEGAL_REQUEST,
                             SCSI_ADSENSE_INVALID_CDB,
                             SCSI_ADSENSE_NO_SENSE);

        pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        pSrb->DataTransferLength = 0;

        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
    } else {
        status = GetLunExtension(pSrbExt, &pLunExt);
        if (status != SNTI_SUCCESS) {
            SntiMapInternalErrorStatus(pSrb, status);
            returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
        } else {
            /* Set SRB status to pending - controller communication necessary */
            pSrb->SrbStatus = SRB_STATUS_PENDING;

            /* Set completion routine, no translation necessary on completion */
            pSrbExt->pNvmeCompletionRoutine = NULL;

            if (GET_OPCODE(pSrb) == SCSIOP_SECURITY_PROTOCOL_IN)
                opcode = ADMIN_SECURITY_RECEIVE;
            else
                opcode = ADMIN_SECURITY_SEND;

            SntiBuildSecuritySendReceiveCmd(pSrbExt,
                                            pLunExt,
                                            opcode,
                                            length,
                                            secProtocolSp,
                                            secProtocol);
        }
    }

    return returnStatus;
} /* SntiTranslateSecurityProtocol */

/******************************************************************************
 * SntiTranslateStartStopUnit
 *
 * @brief Translates the SCSI Start Stop Unit command based on the NVMe
 *        Translation spec to a NVMe Set Features command and populates a
 *        temporary SQE stored in the SRB Extension.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicate translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateStartStopUnit(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    UINT32 dword11 = 0;
    UINT8 immed = 0;
    UINT8 powerCondMod = 0;
    UINT8 powerCond = 0;
    UINT8 noFlush = 0;
    UINT8 loadEject = 0; /* Unspecified */
    UINT8 start = 0;

    /* Default to successful translation */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_TRANSLATION_SUCCESS;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    immed = GET_U8_FROM_CDB(pSrb, START_STOP_UNIT_CDB_IMMED_OFFSET);
    powerCondMod =
        GET_U8_FROM_CDB(pSrb, START_STOP_UNIT_CDB_POWER_COND_MOD_OFFSET);
    powerCond = GET_U8_FROM_CDB(pSrb, START_STOP_UNIT_CDB_POWER_COND_OFFSET);
    noFlush = GET_U8_FROM_CDB(pSrb, START_STOP_UNIT_CDB_NO_FLUSH_OFFSET);
    start = GET_U8_FROM_CDB(pSrb, START_STOP_UNIT_CDB_START_OFFSET);

    immed &= START_STOP_UNIT_CDB_IMMED_MASK;
    powerCondMod &= START_STOP_UNIT_CDB_POWER_COND_MOD_MASK;
    powerCond &= START_STOP_UNIT_CDB_POWER_COND_MASK;
    noFlush &= START_STOP_UNIT_CDB_NO_FLUSH_MASK;
    start &= START_STOP_UNIT_CDB_START_MASK;

    /* Default the SRB Status to PENDING */
    pSrb->SrbStatus = SRB_STATUS_PENDING;

    /*
     * Default the completion routine to NULL - no translation will be
     * necessary on completion unless we have to issue a FLUSH first
     */
    pSrbExt->pNvmeCompletionRoutine = NULL;

    if (immed != 0) {
        /* Ensure correct sense data for SCSI compliance test case 1.4 */
        SntiSetScsiSenseData(pSrb,
                             SCSISTAT_CHECK_CONDITION,
                             SCSI_SENSE_ILLEGAL_REQUEST,
                             SCSI_ADSENSE_INVALID_CDB,
                             SCSI_ADSENSE_NO_SENSE);

        pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        pSrb->DataTransferLength = 0;

        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
    } else {
        if (noFlush == 0) {
            /* Issue NVME FLUSH command prior to translating START STOP UNIT */
            SntiBuildFlushCmd(pSrbExt);

            /*
             * START STOP UNIT translation shall continue on FLUSH completion.
             * Override the completion routine.
             */
            pSrbExt->pNvmeCompletionRoutine = SntiCompletionCallbackRoutine;

            returnStatus = SNTI_TRANSLATION_SUCCESS;
            pSrb->SrbStatus = SRB_STATUS_PENDING;
        } else {
            /* Setup the expected power state transition */
            returnStatus = SntiTransitionPowerState(pSrbExt,
                                                    powerCond,
                                                    powerCondMod,
                                                    start);
        }
    }

    return returnStatus;
} /* SntiTranslateStartStopUnit */

/******************************************************************************
 * SntiTransitionPowerState
 *
 * @brief Transitions power state based on SCSI START/STOP unit command.
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param powerCond - Power condition from START/STOP UNIT command
 * @param powerCondMod - Power condition modifier from START/STOP UNIT command
 * @param start - Start value
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTransitionPowerState(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT8 powerCond,
    UINT8 powerCondMod,
    UINT8 start
)
{
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;

    UINT32 dword11 = 0;
    UINT8 numPowerStatesSupported = 0;
    UINT8 lowestPowerStateSupported = 0;
    UINT8 lastPowerStateSupported = 0;

    /* Default to successful translation */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_TRANSLATION_SUCCESS;

    /* Determine lowest and last power state supported */
    numPowerStatesSupported = pSrbExt->pNvmeDevExt->controllerIdentifyData.NPSS;
    lowestPowerStateSupported = 0;

    switch ((powerCond >> NIBBLE_SHIFT)) {
        case NVME_POWER_STATE_START_VALID:
            /* Action unspecified - POWER CONDITION MODIFIER != 1 */
            if (powerCondMod == 0) {
                if (start == 0x1) {
                    /* Issue Set Features - Power State 0 */
                    dword11 &= POWER_STATE_0;
                    SntiBuildSetFeaturesCmd(pSrbExt, POWER_MANAGEMENT, dword11);
                } else {
                    /* Issue Set Features - Lowest Power State supported */
                    dword11 &= lowestPowerStateSupported;
                    SntiBuildSetFeaturesCmd(pSrbExt, POWER_MANAGEMENT, dword11);
                }
            }
        break;
        case NVME_POWER_STATE_ACTIVE:
            /* Action unspecified - POWER CONDITION MODIFIER = 1 */
            if (powerCondMod == 0) {
                /* Issue Set Features - Power State 0 */
                dword11 &= POWER_STATE_0;
                SntiBuildSetFeaturesCmd(pSrbExt, POWER_MANAGEMENT, dword11);
            }
        break;
        case NVME_POWER_STATE_IDLE:
            /* Action unspecified - POWER CONDITION MODIFIER != 0, 1, or 2 */
            if (powerCondMod == 0x0) {
                /* Issue Set Features - Power State 1 */
                dword11 &= POWER_STATE_1;
                SntiBuildSetFeaturesCmd(pSrbExt, POWER_MANAGEMENT, dword11);
            } else if (powerCondMod == 0x1 ) {
                /* Issue Set Features - Power State 2 */
                dword11 &= POWER_STATE_2;
                SntiBuildSetFeaturesCmd(pSrbExt, POWER_MANAGEMENT, dword11);
            } else if (powerCondMod == 0x2 ) {
                /* Issue Set Features - Power State 3 */
                dword11 &= POWER_STATE_3;
                SntiBuildSetFeaturesCmd(pSrbExt, POWER_MANAGEMENT, dword11);
            }
        break;
        case NVME_POWER_STATE_STANDBY:
            /* Action unspecified - POWER CONDITION MODIFIER != 0 or 1 */
            if (powerCondMod == 0x0) {
                /*
                 * Issue Set Features - Power State N-2, where N = last
                 * power state supported
                 */
                dword11 &= (numPowerStatesSupported - 2);
                SntiBuildSetFeaturesCmd(pSrbExt, POWER_MANAGEMENT, dword11);
            } else if (powerCondMod == 0x1 ) {
                /*
                 * Issue Set Features - Power State N-1, where N = last
                 * power state supported
                 */
                dword11 &= (numPowerStatesSupported - 1);
                SntiBuildSetFeaturesCmd(pSrbExt, POWER_MANAGEMENT, dword11);
            }
        break;
        case NVME_POWER_STATE_LU_CONTROL:
            /* Action unspecified */
            pSrb->SrbStatus = SRB_STATUS_SUCCESS;
        break;
        default:
            /* Ensure correct sense data for SCSI compliance test case 1.4 */
            SntiSetScsiSenseData(pSrb,
                                 SCSISTAT_CHECK_CONDITION,
                                 SCSI_SENSE_ILLEGAL_REQUEST,
                                 SCSI_ADSENSE_INVALID_CDB,
                                 SCSI_ADSENSE_NO_SENSE);

            pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
            pSrb->DataTransferLength = 0;

            returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
        break;
    } /* end switch */

    return returnStatus;
} /* SntiTransitionPowerState */

/******************************************************************************
 * SntiTranslateWriteBuffer
 *
 * @bried Translates the SCSI Write Buffer command based on the NVMe Translation
 *        spec to a NVMe Firmware Image Download and/or Firmware Activate
 *        command and populates a temporary SQE stored in the SRB Extension.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateWriteBuffer(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PSTOR_SCATTER_GATHER_LIST pSgl = NULL;
    UINT32 bufferOffset = 0; /* 3 byte field */
    UINT32 paramListLength = 0; /* 3 byte field */
    UINT32 dword10 = 0;
    UINT32 dword11 = 0;
    UINT8 bufferId = 0;
    UINT8 mode = 0;

    /* Default to successful translation */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_TRANSLATION_SUCCESS;
    SNTI_STATUS status = SNTI_SUCCESS;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    mode = GET_U8_FROM_CDB(pSrb, WRITE_BUFFER_CDB_MODE_OFFSET);
    bufferId = GET_U8_FROM_CDB(pSrb, WRITE_BUFFER_CDB_BUFFER_ID_OFFSET);

    bufferOffset =
        GET_U24_FROM_CDB(pSrb, WRITE_BUFFER_CDB_BUFFER_OFFSET_OFFSET);
    paramListLength =
        GET_U24_FROM_CDB(pSrb, WRITE_BUFFER_CDB_PARAM_LIST_LENGTH_OFFSET);

    /* Set the completion routine - translation necessary on completion */
    pSrbExt->pNvmeCompletionRoutine = SntiCompletionCallbackRoutine;

    switch (mode & WRITE_BUFFER_CDB_MODE_MASK) {
        case DOWNLOAD_SAVE_ACTIVATE:
            /* Issue NVME FIRMWARE IMAGE DOWNLOAD command */
            dword10 |= paramListLength;
            dword11 |= bufferOffset;

            SntiBuildFirmwareImageDownloadCmd(pSrbExt, dword10, dword11);

            /* Activate microcode upon completion of Firmware Image Download */
        break;
        case DOWNLOAD_SAVE_DEFER_ACTIVATE:
            /* Issue NVME FIRMWARE IMAGE DOWNLOAD command */
            dword10 |= paramListLength;
            dword11 |= bufferOffset;

            SntiBuildFirmwareImageDownloadCmd(pSrbExt, dword10, dword11);

            /* Do not activate - must receive a separate activate command */
        break;
        case ACTIVATE_DEFERRED_MICROCODE:
            /* Issue NVME FIRMWARE ACTIVATE command */
            /* Spec Errata - Truncation and Activate Action (AA) */
            dword10 |= bufferId;
            dword10 |= 0x00000008;

            SntiBuildFirmwareActivateCmd(pSrbExt, dword10);

            /* In this case there is no translation on the completion */
            pSrbExt->pNvmeCompletionRoutine = NULL;
        break;
        default:
            status = SNTI_INVALID_PARAMETER;
        break;
    } /* end switch */

    if (status != SNTI_SUCCESS) {
        /* Ensure correct sense data for SCSI compliance test case 1.4 */
        SntiSetScsiSenseData(pSrb,
                             SCSISTAT_CHECK_CONDITION,
                             SCSI_SENSE_ILLEGAL_REQUEST,
                             SCSI_ADSENSE_INVALID_CDB,
                             SCSI_ADSENSE_NO_SENSE);

        pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        pSrb->DataTransferLength = 0;

        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
    }

    return returnStatus;
} /* SntiTranslateWriteBuffer */

/******************************************************************************
 * SntiTranslateSynchronizeCache
 *
 * @brief Translates the SCSI Synchrozize Cache 10/16 commands based on the NVMe
 *        Translation spec to a NVMe Flush command and populates a temporary SQE
 *        stored in the SRB Extension.
 *
 *        NOTE: The NVME FLUSH command is defined:
 *
 *        6.7 Flush command
 *          The Flush command is used by the host to indicate that any data in
 *          volatile storage should be flushed to non-volatile memory.
 *
 *        All command specific fields are reserved.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateSynchronizeCache(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PNVME_LUN_EXTENSION pLunExt = NULL;
    SNTI_STATUS status;

    /* Default to successful translation */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_TRANSLATION_SUCCESS;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);

    status = GetLunExtension(pSrbExt, &pLunExt);
    if (status != SNTI_SUCCESS) {
        /* Map the translation error to a SCSI error */
        SntiMapInternalErrorStatus(pSrb, status);
        return SNTI_FAILURE_CHECK_RESPONSE_DATA;
    }

    /* Set the SRB status to pending - controller communication necessary */
    pSrb->SrbStatus = SRB_STATUS_PENDING;

    /* Set the completion routine - no translation necessary on completion */
    pSrbExt->pNvmeCompletionRoutine = NULL;

    /* Set up common portions of the NVMe WRITE command */
    memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));

    pSrbExt->nvmeSqeUnit.CDW0.OPC = NVME_FLUSH;
    pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
    pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;
    pSrbExt->nvmeSqeUnit.NSID = pLunExt->namespaceId;

    return returnStatus;
} /* SntiTranslateSynchronizeCache */

/******************************************************************************
 * SntiTranslateTestUnitReady
 *
 * @brief Translates the SCSI Test Unit Ready command. Populates the appropriate
 *        SCSI fixed sense response data based on the NVMe Translation spec. Do
 *        not need to create SQE here as we just complete the command in the
 *        build phase (by returning FALSE to StorPort with SRB status of
 *        SUCCESS).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateTestUnitReady(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_SRB_EXTENSION pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    PNVME_DEVICE_EXTENSION pDevExt = pSrbExt->pNvmeDevExt;

    /* Default to command completed */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_COMMAND_COMPLETED;

    /* Set the completion routine - no translation necessary on completion */
    pSrbExt->pNvmeCompletionRoutine = NULL;

    /*
     * Look at GenNextStartStat and ensure that the asynchronous state machine
     * has been successfully started.
     */
    if (pDevExt->DriverState.NextDriverState != NVMeStartComplete) {
        SntiSetScsiSenseData(pSrb,
                             SCSISTAT_CHECK_CONDITION,
                             SCSI_SENSE_NOT_READY,
                             SCSI_ADSENSE_LUN_NOT_READY,
                             SCSI_ADSENSE_NO_SENSE);

        pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
    } else {
        pSrb->SrbStatus = SRB_STATUS_SUCCESS;
    }

    pSrb->DataTransferLength = 0;

    return returnStatus;
} /* SntiTranslateTestUnitReady */

/******************************************************************************
 * SntiTranslateFormatUnit
 *
 * @brief Translates the SCSI Format Unit command.
 *        NOTE: Before performing the operation specified by this command, the
 *        device server shall stop all:
 *
 *           a) enabled power condition timers (see SPC-4):
 *           b) timers for enabled background scan operations (see 4.20); and
 *           c) timers or counters enabled for device-specific background
 *              functions.
 *
 *        After the operation is complete, the device server shall reinitialize
 *        and restart all enabled timers and counters for power conditions and
 *        background functions.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateFormatUnit(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PNVME_LUN_EXTENSION pLunExt = NULL;
    PUINT8 pDataBuf = NULL;
    UINT8 protectionType = 0;
    UINT8 formatProtInfo = 0;
    UINT8 longList = 0;
    UINT8 formatData = 0;
    UINT8 completeList = 0;
    UINT8 defectListFormat = 0;
    UINT8 protFieldUsage = 0;
    UINT8 immed = 0;
    UINT8 protIntervalExp = 0;
    BOOLEAN shortParamList = FALSE;

    SNTI_STATUS status = SNTI_SUCCESS;
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_COMMAND_COMPLETED;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    formatProtInfo =
        GET_U8_FROM_CDB(pSrb, FORMAT_UNIT_CDB_FORMAT_PROT_INFO_OFFSET);
    longList = GET_U8_FROM_CDB(pSrb, FORMAT_UNIT_CDB_LONG_LIST_OFFSET);
    formatData = GET_U8_FROM_CDB(pSrb, FORMAT_UNIT_CDB_FORMAT_DATA_OFFSET);
    completeList = GET_U8_FROM_CDB(pSrb,FORMAT_UNIT_CDB_COMPLETE_LIST_OFFSET);
    defectListFormat =
        GET_U8_FROM_CDB(pSrb,FORMAT_UNIT_CDB_DEFECT_LIST_FORMAT_OFFSET);

    formatProtInfo &= FORMAT_UNIT_CDB_FORMAT_PROT_INFO_MASK;
    longList &= FORMAT_UNIT_CDB_LONG_LIST_MASK;
    formatData &= FORMAT_UNIT_CDB_FORMAT_DATA_MASK;
    completeList &= FORMAT_UNIT_CDB_COMPLETE_LIST_MASK;
    defectListFormat &= FORMAT_UNIT_CDB_DEFECT_LIST_FORMAT_MASK;

    /* Set the completion routine - no translation necessary on completion */
    pSrbExt->pNvmeCompletionRoutine = NULL;

    /*
     * FMTDATA: Unspecified. When set to 1b DEFECT LIST FORMAT is used.
     * CMPLIST: Unspecified. Indicates type of defect list (complete list).
     *          Ignored when FMTDATA is set to 1.
     * DEFECT LIST FORMAT: Unspecifed. If FMTDATA is 1, specifies format of
     *                     address descriptors in defect list.
     */
    if (formatProtInfo == 0) {
        /* Shall be supported by formatting unit w/o protection */

        /* Ignore LONGLIST */
    } else {
        if (longList == 0)
            shortParamList = TRUE;
        else
            shortParamList = FALSE;

        /* Ignore CMPLIST */

        /* DEFECT LIST FORMAT specifies format of address descriptors */
    }

    pDataBuf = (PUINT8)GET_DATA_BUFFER(pSrb);
    protFieldUsage = (*pDataBuf) & PROTECTION_FIELD_USAGE_MASK;
    immed = (*(++pDataBuf)) & FORMAT_UNIT_IMMED_MASK;

    /*
     * FORMAT UNIT Parameter List contains:
     *   - header
     *   - optional initialization pattern descriptor
     *   - optional defect list
     */
    if (immed != 0) {
        /* Ensure correct sense data for SCSI compliance test case 1.4 */
        SntiSetScsiSenseData(pSrb,
                             SCSISTAT_CHECK_CONDITION,
                             SCSI_SENSE_ILLEGAL_REQUEST,
                             SCSI_ADSENSE_INVALID_CDB,
                             SCSI_ADSENSE_NO_SENSE);

        pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        pSrb->DataTransferLength = 0;

        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
        status = SNTI_INVALID_REQUEST;
    } else {
        if (shortParamList == TRUE) {
            /* Short parameter list header */
        } else {
            /* Long parameter list header */
            protIntervalExp = (*(pDataBuf + 2)) &
                PROTECTION_INTERVAL_EXPONENT_MASK;
            if (protIntervalExp != 0) {
                SntiSetScsiSenseData(pSrb,
                                     SCSISTAT_CHECK_CONDITION,
                                     SCSI_SENSE_ILLEGAL_REQUEST,
                                     SCSI_ADSENSE_INVALID_CDB,
                                     SCSI_ADSENSE_NO_SENSE);

                pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
                pSrb->DataTransferLength = 0;

                returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
                status = SNTI_INVALID_REQUEST;
            }
        }
    }

    if (status == SNTI_SUCCESS) {
        if (protFieldUsage == 0) {
            if ((formatProtInfo >> FORMAT_UNIT_CDB_FORMAT_PROT_INFO_SHIFT) == 0)
                protectionType = PROT_TYPE_0;
            else if ((formatProtInfo >>
                      FORMAT_UNIT_CDB_FORMAT_PROT_INFO_SHIFT) == 1)
                protectionType = PROT_TYPE_1;
            else if ((formatProtInfo >>
                      FORMAT_UNIT_CDB_FORMAT_PROT_INFO_SHIFT) == 2)
                protectionType = PROT_TYPE_2;
            else
                ASSERT(FALSE);
        } else if (protFieldUsage == 1) {
            if ((formatProtInfo >> FORMAT_UNIT_CDB_FORMAT_PROT_INFO_SHIFT) == 3)
                protectionType = PROT_TYPE_3;
        } else {
            /* All other values are unspecified */
            ASSERT(FALSE);
        }

        SntiBuildFormatNvmCmd(pSrbExt, protectionType);
    }

    return returnStatus;
} /* SntiTranslateFormatUnit */

/******************************************************************************
 * SntiTranslateLogSense
 *
 * @brief Translates the SCSI Log Sense command. Populates the appropriate SCSI
 *        Log Sense page data based on the NVMe Translation spec. Some log pages
 *        require controller communication and some do not and can be completed
 *        in the build I/O phase.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateLogSense(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    UINT32 bufferSize;
    UINT16 allocLength;
    UINT8 saveParameters;
    UINT8 pageControl;
    UINT8 pageCode;

    SNTI_STATUS status = SNTI_SUCCESS;
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_COMMAND_COMPLETED;

    /* Extract the Log Sense fields to determine the page */
    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    saveParameters = GET_U8_FROM_CDB(pSrb, LOG_SENSE_CDB_SP_OFFSET);
    pageControl = GET_U8_FROM_CDB(pSrb, LOG_SENSE_CDB_PC_OFFSET);
    pageCode = GET_U8_FROM_CDB(pSrb, LOG_SENSE_CDB_PAGE_CODE_OFFSET);

    /* Mask and get SP, PC, and Page code */
    saveParameters &= LOG_SENSE_CDB_SP_MASK;
    pageControl &= LOG_SENSE_CDB_PC_MASK;
    pageCode &= LOG_SENSE_CDB_PAGE_CODE_MASK;

    /*
     * Set SRB status to success to indicate the command will complete
     * successfully (assuming no errors occur during translation) and
     * reset the status value to use below.
     */
    pSrb->SrbStatus = SRB_STATUS_SUCCESS;

    /* Set the completion callback routine to finish the command translation */
    pSrbExt->pNvmeCompletionRoutine = SntiCompletionCallbackRoutine;

    if ((saveParameters != LOG_SENSE_CDB_SP_NOT_ENABLED) ||
        ((pageControl >> LOG_SENSE_CDB_PC_SHIFT) != PC_CUMULATIVE_VALUES)) {
        SntiSetScsiSenseData(pSrb,
                             SCSISTAT_CHECK_CONDITION,
                             SCSI_SENSE_ILLEGAL_REQUEST,
                             SCSI_ADSENSE_INVALID_CDB,
                             SCSI_ADSENSE_NO_SENSE);

        pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        pSrb->DataTransferLength = 0;
        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
    } else {
        switch (pageCode) {
            case LOG_PAGE_SUPPORTED_LOG_PAGES_PAGE:
                SntiTranslateSupportedLogPages(pSrb);

                /* Command is completed in Build I/O phase */
                returnStatus = SNTI_COMMAND_COMPLETED;

                /* Override the completion routine... finished here */
                pSrbExt->pNvmeCompletionRoutine = NULL;
            break;
            case LOG_PAGE_TEMPERATURE_PAGE:
                SntiTranslateTemperature(pSrb);

                /*
                 * Fall through to finish setting up the PRP entries and GET LOG
                 * PAGE command
                 */

            case LOG_PAGE_INFORMATIONAL_EXCEPTIONS_PAGE:
                /*
                 * These log pages requires a Get Log Page (SMART/Health Info
                 * Log) command and uses the field "Temperature" (after
                 * converting from celsius to Kelvin) to set the "Most Recent
                 * Temperature Reading" field in the SCSI log page. A data
                 * buffer must be allocated here since the SMART/Health Info log
                 * page is 512 bytes long. Therefore, the translation must be
                 * performed on the completion side of this command.
                 *
                 * NOTE: Use the SRB Extension from the Log Sense command.
                 * NOTE: The Temp log page requires a parameter from Get
                 *       Features command and a parameter from Get Log Page
                 *       (Tempterature) command.
                 */
                bufferSize =
                  sizeof(ADMIN_GET_LOG_PAGE_SMART_HEALTH_INFORMATION_LOG_ENTRY);

                pSrbExt->pDataBuffer =
                  SntiAllocatePhysicallyContinguousBuffer(pSrbExt, bufferSize);

                if (pSrbExt->pDataBuffer != NULL) {
                    SntiBuildGetLogPageCmd(pSrbExt, SMART_HEALTH_INFORMATION);
                    returnStatus = SNTI_TRANSLATION_SUCCESS;
                } else {
                    SntiSetScsiSenseData(pSrb,
                                         SCSISTAT_CHECK_CONDITION,
                                         SCSI_SENSE_UNIQUE,
                                         SCSI_ADSENSE_INTERNAL_TARGET_FAILURE,
                                         SCSI_ADSENSE_NO_SENSE);

                    pSrb->SrbStatus |= SRB_STATUS_ERROR;
                    pSrb->DataTransferLength = 0;
                    returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
                }
            break;
            default:
                SntiSetScsiSenseData(pSrb,
                                     SCSISTAT_CHECK_CONDITION,
                                     SCSI_SENSE_ILLEGAL_REQUEST,
                                     SCSI_ADSENSE_INVALID_CDB,
                                     SCSI_ADSENSE_NO_SENSE);

                pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
                pSrb->DataTransferLength = 0;

                returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
            break;
        } /* end switch */
    }

    return returnStatus;
} /* SntiTranslateLogSense */

/******************************************************************************
 * SntiTranslateSupportedLogPages
 *
 * @brief Translates the Log Sense page - Supported Log Pages Page. Populates
 *        the appropriate log page response fields based on the NVMe Translation
 *        spec. Do not need to create SQE here as we just complete the command
 *        in the build phase (by returning FALSE to StorPort with SRB status of
 *        SUCCESS).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return VOID
 ******************************************************************************/
VOID SntiTranslateSupportedLogPages(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PSUPPORTED_LOG_PAGES_LOG_PAGE pLogPage = NULL;
    UINT16 allocLength;

    pLogPage = (PSUPPORTED_LOG_PAGES_LOG_PAGE)GET_DATA_BUFFER(pSrb);
    allocLength = GET_U16_FROM_CDB(pSrb, LOG_SENSE_CDB_ALLOC_LENGTH_OFFSET);

    memset(pLogPage, 0, sizeof(SUPPORTED_LOG_PAGES_LOG_PAGE));
    pLogPage->PageCode = LOG_PAGE_SUPPORTED_LOG_PAGES_PAGE;
    pLogPage->SubPageFormat = SUB_PAGE_FORMAT_UNSUPPORTED;
    pLogPage->DisableSave = DISABLE_SAVE_UNSUPPORTED;
    pLogPage->SubPageCode = SUB_PAGE_CODE_UNSUPPORTED;
    pLogPage->PageLength[BYTE_0] = 0;
    pLogPage->PageLength[BYTE_1] = SUPPORTED_LOG_PAGES_PAGE_LENGTH;
    pLogPage->supportedPages[BYTE_0].PageCode =
        LOG_PAGE_SUPPORTED_LOG_PAGES_PAGE;
    pLogPage->supportedPages[BYTE_1].PageCode =
        LOG_PAGE_TEMPERATURE_PAGE;
    pLogPage->supportedPages[BYTE_2].PageCode =
        LOG_PAGE_INFORMATIONAL_EXCEPTIONS_PAGE;

    pSrb->DataTransferLength =
        min(sizeof(SUPPORTED_LOG_PAGES_LOG_PAGE), allocLength);
} /* SntiTranslateSupportedLogPages */

/******************************************************************************
 * SntiTranslateTemperature
 *
 * @brief Translates the Log Sense page - Temperature Log Page. Populates the
 *        appropriate log page response fields based on the NVMe Translation
 *        spec. This log page requires both local data storage and adapter
 *        communication. Create the NVMe Admin command - Get Log Page:
 *        SMART/Health Information and populates the temporary SQE stored in the
 *        SRB Extension.
 *
 *        NOTE: Must free extra buffer used for this command.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return VOID
 ******************************************************************************/
VOID SntiTranslateTemperature(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PNVME_DEVICE_EXTENSION pDevExt = NULL;
    PTEMPERATURE_LOG_PAGE pLogPage = NULL;
    PSTOR_SCATTER_GATHER_LIST pSgl = NULL;
    ULONG memAllocStatus = 0;
    UINT16 logPageIdentifier = SMART_HEALTH_INFORMATION;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);

    /* Set up the SCSI Log Page in the SRB Data Buffer */
    pLogPage = (PTEMPERATURE_LOG_PAGE)pSrb->DataBuffer;

    memset(pLogPage, 0, sizeof(TEMPERATURE_LOG_PAGE));
    pLogPage->PageCode = LOG_PAGE_TEMPERATURE_PAGE;
    pLogPage->SubPageFormat = SUB_PAGE_FORMAT_UNSUPPORTED;
    pLogPage->DisableSave = DISABLE_SAVE_UNSUPPORTED;
    pLogPage->SubPageCode = SUB_PAGE_CODE_UNSUPPORTED;
    pLogPage->PageLength[BYTE_0] = 0;
    pLogPage->PageLength[BYTE_1] = REMAINING_TEMP_PAGE_LENGTH;
    pLogPage->ParameterCode_Temp[BYTE_0] = TEMPERATURE_PARM_CODE;
    pLogPage->ParameterCode_Temp[BYTE_1] = TEMPERATURE_PARM_CODE;
    pLogPage->FormatAndLinking_Temp = BINARY_FORMAT;
    pLogPage->TMC_Temp = TMC_UNSUPPORTED;
    pLogPage->ETC_Temp = ETC_UNSUPPORTED;
    pLogPage->TSD_Temp = TSD_UNSUPPORTED;
    pLogPage->DU_Temp = DU_UNSUPPORTED;
    pLogPage->ParameterLength_Temp = TEMP_PARM_LENGTH;
    pLogPage->Temperature = 0; /* To be filled after Get Log Page */

    pLogPage->ParameterCode_RefTemp[BYTE_0] = 0;
    pLogPage->ParameterCode_RefTemp[BYTE_1] = REFERENCE_TEMPERATURE_PARM_CODE;
    pLogPage->FormatAndLinking_RefTemp = BINARY_FORMAT;
    pLogPage->TMC_RefTemp = TMC_UNSUPPORTED;
    pLogPage->ETC_RefTemp = ETC_UNSUPPORTED;
    pLogPage->TSD_RefTemp = TSD_UNSUPPORTED;
    pLogPage->DU_RefTemp = DU_UNSUPPORTED;
    pLogPage->ParameterLength_RefTemp = REF_TEMP_PARM_LENGTH;
    pLogPage->ReferenceTemperature = 0; /* To be filled after Get Log Page */

    pSrb->SrbStatus = SRB_STATUS_PENDING;
} /* SntiTranslateTemperature */

/******************************************************************************
 * SntiTranslateModeSense
 *
 * @brief Translates the SCSI Mode Sense command. Populates the appropriate SCSI
 *        Mode Sense page data based on the NVMe Translation spec. Some pages
 *        require controller communication and others can be completed in the
 *        build phase.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param supportsVwc - the ID data that indicates if its legal to sent a VWC
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicate translation status.
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateModeSense(
    PSCSI_REQUEST_BLOCK pSrb,
    BOOLEAN supportsVwc
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PNVME_LUN_EXTENSION pLunExt = NULL;
    UINT16 allocLength;
    UINT8 longLbaAccepted;
    UINT8 disableBlockDesc;
    UINT8 pageControl;
    UINT8 pageCode;
    UINT8 subPageCode;
    BOOLEAN modeSense10;

    SNTI_STATUS status = SNTI_SUCCESS;
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_COMMAND_COMPLETED;

    /* Extract the Log Sense fields to determine the page */
    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    longLbaAccepted = GET_U8_FROM_CDB(pSrb, MODE_SENSE_CDB_LLBAA_OFFSET);
    disableBlockDesc = GET_U8_FROM_CDB(pSrb, MODE_SENSE_CDB_DBD_OFFSET);
    pageControl = GET_U8_FROM_CDB(pSrb, MODE_SENSE_CDB_PAGE_CONTROL_OFFSET);
    pageCode = GET_U8_FROM_CDB(pSrb, MODE_SENSE_CDB_PAGE_CODE_OFFSET);
    subPageCode = GET_U8_FROM_CDB(pSrb, MODE_SENSE_CDB_SUBPAGE_CODE_OFFSET);

    if (GET_OPCODE(pSrb) == SCSIOP_MODE_SENSE) {
        /* MODE SENSE 6 */
        allocLength =
            GET_U8_FROM_CDB(pSrb, MODE_SENSE_6_CDB_ALLOC_LENGTH_OFFSET);
        modeSense10 = FALSE;
    } else {
        /* MODE SENSE 10 */
        allocLength =
            GET_U16_FROM_CDB(pSrb, MODE_SENSE_10_CDB_ALLOC_LENGTH_OFFSET);
        modeSense10 = TRUE;
    }

    /* Mask and get LLBAA, DBD, PC, and Page code */
    longLbaAccepted = (longLbaAccepted & MODE_SENSE_CDB_LLBAA_MASK) >>
                       MODE_SENSE_CDB_LLBAA_SHIFT;
    disableBlockDesc = (disableBlockDesc & MODE_SENSE_CDB_DBD_MASK) >>
                        MODE_SENSE_CDB_DBD_SHIFT;
    pageControl = (pageControl & MODE_SENSE_CDB_PAGE_CONTROL_MASK) >>
                   MODE_SENSE_CDB_PAGE_CONTROL_SHIFT;
    pageCode &= MODE_SENSE_CDB_PAGE_CODE_MASK;

    /* Set the completion routine - no translation necessary on completion */
    pSrbExt->pNvmeCompletionRoutine = NULL;

    if (((pageControl != MODE_SENSE_PC_CURRENT_VALUES)    &&
         (pageControl != MODE_SENSE_PC_CHANGEABLE_VALUES) &&
         (pageControl != MODE_SENSE_PC_DEFAULT_VALUES))   ||
        ((pageCode != MODE_SENSE_RETURN_ALL)      &&
         (pageCode != MODE_PAGE_CACHING)          &&
         (pageCode != MODE_PAGE_CONTROL)          &&
         (pageCode != MODE_PAGE_POWER_CONDITION)  &&
         (pageCode != MODE_PAGE_FAULT_REPORTING))) {
        SntiSetScsiSenseData(pSrb,
                             SCSISTAT_CHECK_CONDITION,
                             SCSI_SENSE_ILLEGAL_REQUEST,
                             SCSI_ADSENSE_ILLEGAL_COMMAND,
                             SCSI_ADSENSE_NO_SENSE);

        pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        pSrb->DataTransferLength = 0;
        returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
    } else {
        status = GetLunExtension(pSrbExt, &pLunExt);
        if (status != SNTI_SUCCESS) {
            SntiMapInternalErrorStatus(pSrb, status);
            returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
        } else {
            switch (pageCode) {
                case MODE_PAGE_CACHING:
                    /* Per NVMe we can't send VWC commands if its not supported */
                    if (supportsVwc == FALSE) {
                    SntiHardCodeCacheModePage(pSrbExt,
                                              pLunExt,
                                              allocLength,
                                              longLbaAccepted,
                                              disableBlockDesc,
                                              modeSense10);

                    pSrb->SrbStatus = SRB_STATUS_SUCCESS;
                    returnStatus = SNTI_COMMAND_COMPLETED;
                    } else {
                    /*
                     * This mode page requires a paramter from Get Features.
                     * Must send Get Features w/ Volatile Write Cache Feature
                     * Identifier and set to the CACHE MODE PAGE field of Write
                     * Back Cache Enable (WCE). However, we can still use the
                     * SRB data buffer since the Get Feature command will
                     * return the data in one of the DWORDs in the completion
                     * queue entry. Therefore, the translation will be done on
                     * the completion side after we get the WCE info.
                     */
                    SntiBuildGetFeaturesCmd(pSrbExt, VOLATILE_WRITE_CACHE);

                    returnStatus = SNTI_TRANSLATION_SUCCESS;
                    pSrb->SrbStatus = SRB_STATUS_PENDING;

                    /*
                     * Override the completion routine - translation necessary
                     * on completion
                     */
                    pSrbExt->pNvmeCompletionRoutine =
                        SntiCompletionCallbackRoutine;
                    }
                break;
                case MODE_PAGE_CONTROL:
                    SntiCreateControlModePage(pSrbExt,
                                              pLunExt,
                                              allocLength,
                                              longLbaAccepted,
                                              disableBlockDesc,
                                              modeSense10);

                    /* Command is completed in Build I/O phase */
                    pSrb->SrbStatus = SRB_STATUS_SUCCESS;
                    returnStatus = SNTI_COMMAND_COMPLETED;
                break;
                case MODE_PAGE_POWER_CONDITION:
                    SntiCreatePowerConditionControlModePage(pSrbExt,
                                                            pLunExt,
                                                            allocLength,
                                                            longLbaAccepted,
                                                            disableBlockDesc,
                                                            modeSense10);

                    /* Command is completed in Build I/O phase */
                    pSrb->SrbStatus = SRB_STATUS_SUCCESS;
                    returnStatus = SNTI_COMMAND_COMPLETED;
                break;
                case MODE_PAGE_FAULT_REPORTING:
                    SntiCreateInformationalExceptionsControlModePage(
                        pSrbExt,
                        pLunExt,
                        allocLength,
                        longLbaAccepted,
                        disableBlockDesc,
                        modeSense10);

                    /* Command is completed in Build I/O phase */
                    pSrb->SrbStatus = SRB_STATUS_SUCCESS;
                    returnStatus = SNTI_COMMAND_COMPLETED;
                break;
                case MODE_SENSE_RETURN_ALL:
                    SntiReturnAllModePages(pSrbExt,
                                           pLunExt,
                                           allocLength,
                                           longLbaAccepted,
                                           disableBlockDesc,
                                           modeSense10,
                                           supportsVwc);

                    if (supportsVwc == FALSE) {
                        /* Command now because we don't set the VWC cmd */
                    pSrb->SrbStatus = SRB_STATUS_SUCCESS;
                    returnStatus = SNTI_COMMAND_COMPLETED;
                    } else {
                        /* Command is completed in Build I/O phase */
                        pSrb->SrbStatus = SRB_STATUS_PENDING;
                        returnStatus = SNTI_TRANSLATION_SUCCESS;

                    /*
                     * Override the completion routine - translation necessary
                     * on completion
                     */
                    pSrbExt->pNvmeCompletionRoutine =
                        SntiCompletionCallbackRoutine;
                    }
                break;
                default:
                    SntiSetScsiSenseData(pSrb,
                                         SCSISTAT_CHECK_CONDITION,
                                         SCSI_SENSE_ILLEGAL_REQUEST,
                                         SCSI_ADSENSE_ILLEGAL_COMMAND,
                                         SCSI_ADSENSE_NO_SENSE);

                    pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
                    pSrb->DataTransferLength = 0;
                    returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
                break;
            } /* end switch */

            if (status != SNTI_SUCCESS) {
                pSrb->SrbStatus = SRB_STATUS_ERROR;
                returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
            }
        }
    }

    return returnStatus;
} /* SntiTranslateModeSense */

/******************************************************************************
 * SntiCreateControlModePage
 *
 * @brief Create the Mode Sense page - Caching Mode Page. Populates the
 *        appropriate mode page response fields based on the NVMe Translation
 *        spec. Do not need to create SQE here as we just complete the command
 *        in the build phase (by returning FALSE to StorPort with SRB status of
 *        SUCCESS).
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 * @param allocLength - Allocation length from Mode Sense CDB
 * @param longLbaAccepted - LLBAA bit from Mode Sense CDB
 * @param disableBlockDesc - DBD bit from Mode Sense CDB
 * @param modeSense10 - Boolean to determine Mode Sense 10
 *
 * @return VOID
 ******************************************************************************/
VOID SntiCreateControlModePage(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT16 allocLength,
    UINT8 longLbaAccepted,
    UINT8 disableBlockDesc,
    BOOLEAN modeSense10
)
{
    PMODE_PARAMETER_HEADER pModeHeader6 = NULL;
    PMODE_PARAMETER_HEADER10 pModeHeader10 = NULL;
    PMODE_PARAMETER_BLOCK pModeParamBlock = NULL;
    PCONTROL_MODE_PAGE pControlModePage = NULL;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    UINT16 modeDataLength = 0;
    UINT16 blockDescLength = 0;
    UINT8  flbas;

    /* Determine which Mode Parameter Descriptor Block to use (8 or 16) */
    if (longLbaAccepted == 0)
        blockDescLength = SHORT_DESC_BLOCK;
    else
        blockDescLength = LONG_DESC_BLOCK;

    /* Mode Page Header */
    SntiCreateModeDataHeader(pSrb,
                             &pModeParamBlock,
                             &modeDataLength,
                             (disableBlockDesc ? 0 : blockDescLength),
                             modeSense10);


    /* Check if block descriptors enabled... if not then mode page comes next */
    if (disableBlockDesc == BLOCK_DESCRIPTORS_ENABLED)
    {
        /* Mode Parameter Descriptor Block */
    SntiCreateModeParameterDescBlock(pLunExt,
                                     pModeParamBlock,
                                     &modeDataLength);

        /* Increment pointer to after block descriptor */
        pModeParamBlock++;
    }

    /* Caching Mode Page */
    pControlModePage = (PCONTROL_MODE_PAGE)pModeParamBlock;
    modeDataLength += sizeof(CONTROL_MODE_PAGE);

    memset(pControlModePage, 0, sizeof(CONTROL_MODE_PAGE));
    pControlModePage->PageSaveable   = MODE_PAGE_PARAM_SAVEABLE_DISABLED;
    pControlModePage->SubPageFormat  = 0;
    pControlModePage->PageCode       = MODE_PAGE_CONTROL;
    pControlModePage->PageLength     = sizeof(CONTROL_MODE_PAGE) - 2;
    pControlModePage->TST            = ONE_TASK_SET;
    pControlModePage->TMF_Only       = ACA_UNSUPPORTED;
    pControlModePage->DPICZ          = RESERVED_FIELD;
    pControlModePage->D_Sense        = SENSE_DATA_DESC_FORMAT;
    pControlModePage->GLTSD          = LOG_PARMS_NOT_IMPLICITLY_SAVED_PER_LUN;
    pControlModePage->RLEC           = LOG_EXCP_COND_NOT_REPORTED;
    pControlModePage->QAlgMod        = CMD_REORDERING_SUPPORTED;
    pControlModePage->NUAR           = RESERVED_FIELD;
    pControlModePage->QERR           = 1; /* TBD: No spec defintion??? */
    pControlModePage->RAC            = BUSY_RETURNS_ENABLED;
    pControlModePage->UA_INTLCK_CTRL = UA_CLEARED_AT_CC_STATUS;
    pControlModePage->SWP            = SW_WRITE_PROTECT_UNSUPPORTED;
    pControlModePage->ATO            = LBAT_LBRT_MODIFIABLE;
    pControlModePage->TAS            = TASK_ABORTED_STATUS_FOR_ABORTED_CMDS;
    pControlModePage->AutoLodeMode   = MEDIUM_LOADED_FULL_ACCESS;

    pControlModePage->BusyTimeoutPeriod[BYTE_0]   = UNLIMITED_BUSY_TIMEOUT_HIGH;
    pControlModePage->BusyTimeoutPeriod[BYTE_1]   = UNLIMITED_BUSY_TIMEOUT_LOW;
    pControlModePage->ExtSelfTestCompTime[BYTE_0] =
        SMART_SELF_TEST_UNSUPPORTED_HIGH;
    pControlModePage->ExtSelfTestCompTime[BYTE_1] =
        SMART_SELF_TEST_UNSUPPORTED_LOW;

    /* Now go back and set the Mode Data Length in the header */
    if (modeSense10 == FALSE) {
        /* Get the correct header that starts at the buffer beginning */
        pModeHeader6 = (PMODE_PARAMETER_HEADER)(GET_DATA_BUFFER(pSrb));

        /* Subtract 1 from mode data length - MODE DATA LENGTH field */
        pModeHeader6->ModeDataLength = (UCHAR)(modeDataLength - 1);
    } else {
        /* Get the correct header that starts at the buffer beginning */
        pModeHeader10 = (PMODE_PARAMETER_HEADER10)(GET_DATA_BUFFER(pSrb));

        /* Subtract 2 from mode data length - MODE DATA LENGTH field */
        pModeHeader10->ModeDataLength[BYTE_0] =
            ((modeDataLength - 2) & WORD_HIGH_BYTE_MASK) >> BYTE_SHIFT_1;
        pModeHeader10->ModeDataLength[BYTE_1] =
            ((modeDataLength - 2) & WORD_LOW_BYTE_MASK);
    }

    pSrb->DataTransferLength = min(modeDataLength, allocLength);
} /* SntiCreateControlModePage*/

VOID SntiHardCodeCacheModePage(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT16 allocLength,
    UINT8 longLbaAccepted,
    UINT8 disableBlockDesc,
    BOOLEAN modeSense10
)
{
    PMODE_PARAMETER_HEADER pModeHeader6 = NULL;
    PMODE_PARAMETER_HEADER10 pModeHeader10 = NULL;
    PMODE_PARAMETER_BLOCK pModeParamBlock = NULL;
    PMODE_CACHING_PAGE pCachingModePage = NULL;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    UINT16 modeDataLength = 0;
    UINT16 blockDescLength = 0;

    /* Determine which Mode Parameter Descriptor Block to use (8 or 16) */
    if (longLbaAccepted == 0)
        blockDescLength = SHORT_DESC_BLOCK;
    else
        blockDescLength = LONG_DESC_BLOCK;

    /* Mode Page Header */
    SntiCreateModeDataHeader(pSrb,
                             &pModeParamBlock,
                             &modeDataLength,
                             (disableBlockDesc ? 0 : blockDescLength),
                             modeSense10);

    /* Check if block descriptors enabled, if not then mode page comes next */
    if (disableBlockDesc == BLOCK_DESCRIPTORS_ENABLED)
    {
        /* Mode Parameter Descriptor Block */
        SntiCreateModeParameterDescBlock(pLunExt,
                                         pModeParamBlock,
                                         &modeDataLength);

        /* Increment pointer to after block descriptor */
        pModeParamBlock++;
    }

    /* Cache Mode Page */
    pCachingModePage  = (PMODE_CACHING_PAGE)pModeParamBlock;
    modeDataLength += sizeof(MODE_CACHING_PAGE);

    memset(pCachingModePage, 0, sizeof(MODE_CACHING_PAGE));
    pCachingModePage->PageCode         = MODE_PAGE_CACHING;
    pCachingModePage->PageSavable      = MODE_PAGE_PARAM_SAVEABLE_DISABLED;
    pCachingModePage->PageLength       = CACHING_MODE_PAGE_LENGTH;
    pCachingModePage->WriteCacheEnable = 0;

    /* Now go back and set the Mode Data Length in the header */
    if (modeSense10 == FALSE) {
        pModeHeader6 = (PMODE_PARAMETER_HEADER)(GET_DATA_BUFFER(pSrb));

        /* Subtract 1 from the mode data length - MODE DATA LENGTH field */
        pModeHeader6->ModeDataLength = (UCHAR)(modeDataLength - 1);
    } else {
        pModeHeader10 = (PMODE_PARAMETER_HEADER10)(GET_DATA_BUFFER(pSrb));

        /* Subtract 2 from mode data length - MODE DATA LENGTH field */
        pModeHeader10->ModeDataLength[BYTE_0] = ((modeDataLength - 2) &
            WORD_HIGH_BYTE_MASK) >> BYTE_SHIFT_1;
        pModeHeader10->ModeDataLength[BYTE_1] = ((modeDataLength - 2) &
            WORD_LOW_BYTE_MASK);
    }

    pSrb->DataTransferLength = min(modeDataLength, allocLength);
} /* SntiHardCodeCacheModePage */

/******************************************************************************
 * SntiCreatePowerConditionControlModePage
 *
 * @brief Creates the Mode Sense page - Control Mode Page. Populates the
 *        appropriate mode page response fields based on the NVMe Translation
 *        spec. Do not need to create SQE here as we just complete the command
 *        in the build phase (by returning FALSE to StorPort with SRB status of
 *        SUCCESS).
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 * @param allocLength - Allocation length from Mode Sense CDB
 * @param longLbaAccepted - LLBAA bit from Mode Sense CDB
 * @param disableBlockDesc - DBD bit from Mode Sense CDB
 * @param modeSense10 - Boolean to determine Mode Sense 10
 *
 * @return VOID
 ******************************************************************************/
VOID SntiCreatePowerConditionControlModePage(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT16 allocLength,
    UINT8 longLbaAccepted,
    UINT8 disableBlockDesc,
    BOOLEAN modeSense10
)
{
    PMODE_PARAMETER_HEADER pModeHeader6 = NULL;
    PMODE_PARAMETER_HEADER10 pModeHeader10 = NULL;
    PMODE_PARAMETER_BLOCK pModeParamBlock = NULL;
    PPOWER_CONDITION_MODE_PAGE pPowerConditionModePage = NULL;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    UINT16 modeDataLength = 0;
    UINT16 blockDescLength = 0;

    /* Determine which Mode Parameter Descriptor Block to use (8 or 16) */
    if (longLbaAccepted == 0)
        blockDescLength = SHORT_DESC_BLOCK;
    else
        blockDescLength = LONG_DESC_BLOCK;

    /* Mode Page Header */
    SntiCreateModeDataHeader(pSrb,
                             &pModeParamBlock,
                             &modeDataLength,
                             (disableBlockDesc ? 0 : blockDescLength),
                             modeSense10);

    /* Check if block descriptors enabled, if not then mode page comes next */
    if (disableBlockDesc == BLOCK_DESCRIPTORS_ENABLED)
    {
        /* Mode Parameter Descriptor Block */
        SntiCreateModeParameterDescBlock(pLunExt,
                                         pModeParamBlock,
                                         &modeDataLength);

        /* Increment pointer to after block descriptor */
        pModeParamBlock++;
    }

    /* Power Condition Control Mode Page */
    pPowerConditionModePage = (PPOWER_CONDITION_MODE_PAGE)pModeParamBlock;
    modeDataLength += sizeof(POWER_CONDITION_MODE_PAGE);

    memset(pPowerConditionModePage, 0, sizeof(POWER_CONDITION_MODE_PAGE));
    pPowerConditionModePage->PageCode        = MODE_PAGE_POWER_CONDITION;
    pPowerConditionModePage->PageLength      = POWER_COND_MODE_PAGE_LENGTH;
    pPowerConditionModePage->PmBgInteraction = 0;

    /* Now go back and set the Mode Data Length in the header */
    if (modeSense10 == FALSE) {
        pModeHeader6 = (PMODE_PARAMETER_HEADER)(GET_DATA_BUFFER(pSrb));

        /* Subtract 1 from the mode data length - MODE DATA LENGTH field */
        pModeHeader6->ModeDataLength = (UCHAR)(modeDataLength - 1);
    } else {
        pModeHeader10 = (PMODE_PARAMETER_HEADER10)(GET_DATA_BUFFER(pSrb));

        /* Subtract 2 from mode data length - MODE DATA LENGTH field */
        pModeHeader10->ModeDataLength[BYTE_0] = ((modeDataLength - 2) &
            WORD_HIGH_BYTE_MASK) >> BYTE_SHIFT_1;
        pModeHeader10->ModeDataLength[BYTE_1] = ((modeDataLength - 2) &
            WORD_LOW_BYTE_MASK);
    }

    pSrb->DataTransferLength = min(modeDataLength, allocLength);
} /* SntiCreatePowerConditionControlModePage */

/******************************************************************************
 * SntiCreateInformationalExceptionsControlModePage
 *
 * @brief Creates the Mode Sense page - Information Exceptions Mode Page.
 *        Populates the appropriate mode page response fields based on the NVMe
 *        Translation spec. Do not need to create SQE here as we just complete
 *        the command in the build phase (by returning FALSE to StorPort with
 *        SRB status of SUCCESS).
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 * @param allocLength - Allocation length from Mode Sense CDB
 * @param longLbaAccepted - LLBAA bit from Mode Sense CDB
 * @param disableBlockDesc - DBD bit from Mode Sense CDB
 * @param modeSense10 - Boolean to determine Mode Sense 10
 *
 * @return VOID
 ******************************************************************************/
VOID SntiCreateInformationalExceptionsControlModePage(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT16 allocLength,
    UINT8 longLbaAccepted,
    UINT8 disableBlockDesc,
    BOOLEAN modeSense10
)
{
    PMODE_PARAMETER_HEADER pModeHeader6 = NULL;
    PMODE_PARAMETER_HEADER10 pModeHeader10 = NULL;
    PMODE_PARAMETER_BLOCK pModeParamBlock = NULL;
    PINFO_EXCEPTIONS_MODE_PAGE pInfoExceptionsModePage = NULL;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    UINT16 modeDataLength = 0;
    UINT16 blockDescLength = 0;

    /* Determine which Mode Parameter Descriptor Block to use (8 or 16) */
    if (longLbaAccepted == 0)
        blockDescLength = SHORT_DESC_BLOCK;
    else
        blockDescLength = LONG_DESC_BLOCK;

    /* Mode Page Header */
    SntiCreateModeDataHeader(pSrb,
                             &pModeParamBlock,
                             &modeDataLength,
                             (disableBlockDesc ? 0 : blockDescLength),
                             modeSense10);

    /* Check if block descriptors enabled, if not then mode page comes next */
    if (disableBlockDesc == BLOCK_DESCRIPTORS_ENABLED)
    {
        /* Mode Parameter Descriptor Block */
        SntiCreateModeParameterDescBlock(pLunExt,
                                         pModeParamBlock,
                                         &modeDataLength);

        /* Increment pointer to after block descriptor */
        pModeParamBlock++;
    }

    /* Informational Exceptions Control Mode Page */
    pInfoExceptionsModePage = (PINFO_EXCEPTIONS_MODE_PAGE)pModeParamBlock;
    modeDataLength += sizeof(INFO_EXCEPTIONS_MODE_PAGE);

    memset(pInfoExceptionsModePage, 0, sizeof(INFO_EXCEPTIONS_MODE_PAGE));
    pInfoExceptionsModePage->PageCode   = MODE_PAGE_FAULT_REPORTING;
    pInfoExceptionsModePage->PageLength = INFO_EXCP_MODE_PAGE_LENGTH;

    /* NOTE: All values shall be zero by default */

    /* Now go back and set the Mode Data Length in the header */
    if (modeSense10 == FALSE) {
        pModeHeader6 = (PMODE_PARAMETER_HEADER)(GET_DATA_BUFFER(pSrb));

        /* Subtract 1 from the mode data length - MODE DATA LENGTH field*/
        pModeHeader6->ModeDataLength = (UCHAR)(modeDataLength - 1);
    } else {
        pModeHeader10 = (PMODE_PARAMETER_HEADER10)(GET_DATA_BUFFER(pSrb));

        /* Subtract 2 from mode data length - MODE DATA LENGTH field */
        pModeHeader10->ModeDataLength[BYTE_0] = ((modeDataLength - 2) &
            WORD_HIGH_BYTE_MASK) >> BYTE_SHIFT_1;
        pModeHeader10->ModeDataLength[BYTE_1] = ((modeDataLength - 2) &
            WORD_LOW_BYTE_MASK);
    }

    pSrb->DataTransferLength = min(modeDataLength, allocLength);
} /* SntiCreateInformationalExceptionsControlModePage*/

/******************************************************************************
 * SntiReturnAllModePages
 *
 * @brief Returns all supported mode pages
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 * @param allocLength - Allocation length from Mode Sense CDB
 * @param longLbaAccepted - LLBAA bit from Mode Sense CDB
 * @param disableBlockDesc - DBD bit from Mode Sense CDB
 * @param modeSense10 - Boolean to determine Mode Sense 10
 * @param supportsVwc - From ID data, tells us if we can send VWC cmds or not
 *
 * @return VOID
 ******************************************************************************/
VOID SntiReturnAllModePages(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT16 allocLength,
    UINT8 longLbaAccepted,
    UINT8 disableBlockDesc,
    BOOLEAN modeSense10,
    BOOLEAN supportsVwc
)
{
    PMODE_PARAMETER_HEADER pModeHeader6 = NULL;
    PMODE_PARAMETER_HEADER10 pModeHeader10 = NULL;
    PMODE_PARAMETER_BLOCK pModeParamBlock = NULL;
    PCONTROL_MODE_PAGE pControlModePage = NULL;
    PPOWER_CONDITION_MODE_PAGE pPowerConditionModePage = NULL;
    PCACHING_MODE_PAGE pCachingModePage = NULL;
    PINFO_EXCEPTIONS_MODE_PAGE pInfoExceptionsModePage = NULL;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    UINT16 modeDataLength = 0;
    UINT16 blockDescLength = 0;

    //memset(GET_DATA_BUFFER(pSrb), 0, allocLength);
    memset(GET_DATA_BUFFER(pSrb), 0, MODE_SENSE_ALL_PAGES_LENGTH);

    /* Determine which Mode Parameter Descriptor Block to use (8 or 16) */
    if (longLbaAccepted == 0)
        blockDescLength = SHORT_DESC_BLOCK;
    else
        blockDescLength = LONG_DESC_BLOCK;

    /* Only use the 8 byte mode parameter block desc... spec errata */

    /* Mode Page Header */
    SntiCreateModeDataHeader(pSrb,
                             &pModeParamBlock,
                             &modeDataLength,
                             (disableBlockDesc ? 0 : blockDescLength),
                             modeSense10);

    /* Check if block descriptors enabled, if not then mode pages comes next */
    if (disableBlockDesc == BLOCK_DESCRIPTORS_ENABLED)
    {
        /* Mode Parameter Descriptor Block */
        SntiCreateModeParameterDescBlock(pLunExt,
                                         pModeParamBlock,
                                         &modeDataLength);

        /* Increment pointer to after block descriptor */
        pModeParamBlock++;
    }

    /* Caching Mode Page */
    pCachingModePage = (PCACHING_MODE_PAGE)pModeParamBlock;
    modeDataLength += sizeof(CACHING_MODE_PAGE);

    memset(pCachingModePage, 0, sizeof(CACHING_MODE_PAGE));
    pCachingModePage->PageCode         = MODE_PAGE_CACHING;
    pCachingModePage->PageSavable      = MODE_PAGE_PARAM_SAVEABLE_DISABLED;
    pCachingModePage->PageLength       = CACHING_MODE_PAGE_LENGTH;
    pCachingModePage->WriteCacheEnable = 0; /* Filled in on completion side */

    /* Increment pointer to after the control mode page */
    pCachingModePage++;

    /* Control Mode Page */
    pControlModePage = (PCONTROL_MODE_PAGE)pCachingModePage;
    modeDataLength += sizeof(CONTROL_MODE_PAGE);

    memset(pControlModePage, 0, sizeof(CONTROL_MODE_PAGE));
    pControlModePage->PageCode       = MODE_PAGE_CONTROL;
    pControlModePage->PageSaveable   = MODE_PAGE_PARAM_SAVEABLE_DISABLED;
    pControlModePage->SubPageFormat  = SUB_PAGE_FORMAT_UNSUPPORTED;
    pControlModePage->PageLength     = CONTROL_MODE_PAGE_SIZE;
    pControlModePage->TST            = ONE_TASK_SET;
    pControlModePage->TMF_Only       = ACA_UNSUPPORTED;
    pControlModePage->DPICZ          = RESERVED_FIELD;
    pControlModePage->D_Sense        = SENSE_DATA_DESC_FORMAT;
    pControlModePage->GLTSD          = LOG_PARMS_NOT_IMPLICITLY_SAVED_PER_LUN;
    pControlModePage->RLEC           = LOG_EXCP_COND_NOT_REPORTED;
    pControlModePage->QAlgMod        = CMD_REORDERING_SUPPORTED;
    pControlModePage->NUAR           = RESERVED_FIELD;
    pControlModePage->QERR           = 1; /* TBD: No spec defintion??? */
    pControlModePage->RAC            = BUSY_RETURNS_ENABLED;
    pControlModePage->UA_INTLCK_CTRL = UA_CLEARED_AT_CC_STATUS;
    pControlModePage->SWP            = SW_WRITE_PROTECT_UNSUPPORTED;
    pControlModePage->ATO            = LBAT_LBRT_MODIFIABLE;
    pControlModePage->TAS            = TASK_ABORTED_STATUS_FOR_ABORTED_CMDS;
    pControlModePage->AutoLodeMode   = MEDIUM_LOADED_FULL_ACCESS;
    pControlModePage->BusyTimeoutPeriod[BYTE_0]   = UNLIMITED_BUSY_TIMEOUT_HIGH;
    pControlModePage->BusyTimeoutPeriod[BYTE_1]   = UNLIMITED_BUSY_TIMEOUT_LOW;
    pControlModePage->ExtSelfTestCompTime[BYTE_0] =
        SMART_SELF_TEST_UNSUPPORTED_HIGH;
    pControlModePage->ExtSelfTestCompTime[BYTE_1] =
        SMART_SELF_TEST_UNSUPPORTED_LOW;

    /* Increment pointer to after the control mode page */
    pControlModePage++;

    /* Power Condition Control Mode Page */
    pPowerConditionModePage = (PPOWER_CONDITION_MODE_PAGE)pControlModePage;
    modeDataLength += sizeof(POWER_CONDITION_MODE_PAGE);

    memset(pPowerConditionModePage, 0, sizeof(POWER_CONDITION_MODE_PAGE));
    pPowerConditionModePage->PageCode        = MODE_PAGE_POWER_CONDITION;
    pPowerConditionModePage->PageSaveable  = MODE_PAGE_PARAM_SAVEABLE_DISABLED;
    pPowerConditionModePage->SubPageFormat = SUB_PAGE_FORMAT_UNSUPPORTED;
    pPowerConditionModePage->PageLength    = POWER_COND_MODE_PAGE_LENGTH;

    /* Increment pointer to after the power condition mode page */
    pPowerConditionModePage++;

    /* Informational Exceptions Control Mode Page */
    pInfoExceptionsModePage =
        (PINFO_EXCEPTIONS_MODE_PAGE)pPowerConditionModePage;
    modeDataLength += sizeof(INFO_EXCEPTIONS_MODE_PAGE);

    memset(pInfoExceptionsModePage, 0, sizeof(INFO_EXCEPTIONS_MODE_PAGE));
    pInfoExceptionsModePage->PageCode   = MODE_PAGE_FAULT_REPORTING;
    pInfoExceptionsModePage->PageLength = INFO_EXCP_MODE_PAGE_LENGTH;

    /* Now go back and set the Mode Data Length in the header */
    if (modeSense10 == FALSE) {
        /* Get the correct header that starts at the buffer beginning */
        pModeHeader6 = (PMODE_PARAMETER_HEADER)(GET_DATA_BUFFER(pSrb));

        /* Subtract 1 from the mode data length - MODE DATA LENGTH field */
        pModeHeader6->ModeDataLength = (UCHAR)(modeDataLength - 1);
    } else {
        /* Get the correct header that starts at the buffer beginning */
        pModeHeader10 = (PMODE_PARAMETER_HEADER10)(GET_DATA_BUFFER(pSrb));

        /* Subtract 2 from mode data length - MODE DATA LENGTH field */
        pModeHeader10->ModeDataLength[BYTE_0] = ((modeDataLength - 2) &
            WORD_HIGH_BYTE_MASK) >> BYTE_SHIFT_1;
        pModeHeader10->ModeDataLength[BYTE_1] = ((modeDataLength - 2) &
            WORD_LOW_BYTE_MASK);
    }

    if (supportsVwc == TRUE) {
    /* Finally, make sure we issue the GET FEATURES command */
    SntiBuildGetFeaturesCmd(pSrbExt, VOLATILE_WRITE_CACHE);
    }

    pSrb->DataTransferLength = min(modeDataLength, allocLength);
} /* SntiReturnAllModePages */

/******************************************************************************
 * SntiTranslateModeSelect
 *
 * @brief Translates the SCSI Mode Select command.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param supportsVwc - the ID data that indicates if its legal to sent a VWC
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateModeSelect(
    PSCSI_REQUEST_BLOCK pSrb,
    BOOLEAN supportsVwc
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PNVME_LUN_EXTENSION pLunExt = NULL;
    UINT16 paramListLength;
    UINT8 pageFormat;
    UINT8 savePages;
    BOOLEAN isModeSelect10;

    SNTI_STATUS status = SNTI_SUCCESS;
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_COMMAND_COMPLETED;

    /* Extract the Log Sense fields to determine the page */
    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    pageFormat = GET_U8_FROM_CDB(pSrb, MODE_SELECT_CDB_PAGE_FORMAT_OFFSET);
    savePages = GET_U8_FROM_CDB(pSrb, MODE_SELECT_CDB_SAVE_PAGES_OFFSET);

    /* Mask and get page format, save pages, and parameter list length */
    pageFormat &= MODE_SELECT_CDB_PAGE_FORMAT_MASK;
    savePages &= MODE_SELECT_CDB_SAVE_PAGES_MASK;

    /* Set the completion routine - no translation necessary on completion */
    pSrbExt->pNvmeCompletionRoutine = NULL;

    if (GET_OPCODE(pSrb) == SCSIOP_MODE_SELECT) {
        /* MODE SELECT 6 */
        paramListLength = GET_U8_FROM_CDB(pSrb,
            MODE_SELECT_6_CDB_PARAM_LIST_LENGTH_OFFSET);

        isModeSelect10 = FALSE;
    } else {
         /* MODE SENSE 10 */
        paramListLength = GET_U16_FROM_CDB(pSrb,
            MODE_SELECT_10_CDB_PARAM_LIST_LENGTH_OFFSET);

        isModeSelect10 = TRUE;
    }

    if (paramListLength == 0) {
        /*
         * According to SPC-4 r24, a paramter list length field of 0
         * shall not be considered an error
         */
        pSrb->SenseInfoBufferLength = 0;
        pSrb->DataTransferLength = 0;

        pSrb->ScsiStatus = SCSISTAT_GOOD;
        pSrb->SrbStatus = SRB_STATUS_SUCCESS;
        returnStatus = SNTI_COMMAND_COMPLETED;
    } else {
        status = GetLunExtension(pSrbExt, &pLunExt);
        if (status != SNTI_SUCCESS) {
            SntiMapInternalErrorStatus(pSrb, status);
            returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
        } else {
            returnStatus = SntiTranslateModeData(pSrbExt,
                                                 pLunExt,
                                                 paramListLength,
                                                 isModeSelect10,
                                                 supportsVwc);
        }
    }

    return returnStatus;
} /* SntiTranslateModeSelect */


/******************************************************************************
 * SntiTranslateModeData
 *
 * @brief Parse out the Mode Parameter Header, the Mode Parameter Descriptor
 *        Block, and the Mode Page for the Mode Select 6/10 command.
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 * @param paramListLength - Length of parameter list
 * @param isModeSelect10 - Boolean to determine Mode Select 10
 * @param supportsVwc - From ID data, tells us if we can send VWC cmds or not
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateModeData(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT16 paramListLength,
    BOOLEAN isModeSelect10,
    BOOLEAN supportsVwc
)
{
    PMODE_PARAMETER_HEADER pModeHeader6 = NULL;
    PMODE_PARAMETER_HEADER10 pModeHeader10 = NULL;
    PMODE_PARAMETER_BLOCK pModeParamBlock = NULL;
    PMODE_CACHING_PAGE pCacheModePage = NULL;
    PPOWER_CONDITION_MODE_PAGE pPowerModePage = NULL;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    PUCHAR pModePagePtr = NULL;
    UINT32 numBlockDesc = 0;
    UINT32 dword11 = 0;
    UINT16 blockDescLength = 0;
    UINT8 longLba = 0;
    UINT8 pageCode = 0;

    SNTI_TRANSLATION_STATUS returnStatus;

    if (isModeSelect10 == FALSE) {
        /* Mode Select 6 */
        pModeHeader6 = (PMODE_PARAMETER_HEADER)(GET_DATA_BUFFER(pSrb));
        blockDescLength = pModeHeader6->BlockDescriptorLength;

        /* Set pointer to Mode Param Desc Block - if any */
        pModeParamBlock = (PMODE_PARAMETER_BLOCK)(pModeHeader6 + 1);
    } else {
        /* Mode Select 10 */
        pModeHeader10 = (PMODE_PARAMETER_HEADER10)(GET_DATA_BUFFER(pSrb));

        blockDescLength = (UINT16) ((pModeHeader10->BlockDescriptorLength[BYTE_0] << 8) | 
                                    (pModeHeader10->BlockDescriptorLength[BYTE_1])); 

        /* Get LONGLBA */
        longLba = pModeHeader10->Reserved[BYTE_0] & LONG_LBA_MASK;

        /* Set pointer to Mode Param Desc Block - if any */
        pModeParamBlock = (PMODE_PARAMETER_BLOCK)(pModeHeader10 + 1);
    }

    /*
     * Block Descriptor Length is equal to # of block descriptors times 8 if
     * if LONGLBA bit is set to zero or times 16 if LONGLBA is set to one.
     */
    if (blockDescLength != 0) {
        /* There are block descriptors to parse */
        numBlockDesc = (blockDescLength / ((longLba == 0) ?
                                           SHORT_DESC_BLOCK : LONG_DESC_BLOCK));

        /* Store off the block descriptor info if a FORMAT UNIT comes later */
        g_modeParamBlock.NumberOfBlocks[BYTE_0] =
            pModeParamBlock->NumberOfBlocks[BYTE_0];
        g_modeParamBlock.NumberOfBlocks[BYTE_1] =
            pModeParamBlock->NumberOfBlocks[BYTE_1];
        g_modeParamBlock.NumberOfBlocks[BYTE_2] =
            pModeParamBlock->NumberOfBlocks[BYTE_2];

        g_modeParamBlock.BlockLength[BYTE_0] =
            pModeParamBlock->BlockLength[BYTE_0];
        g_modeParamBlock.BlockLength[BYTE_1] =
            pModeParamBlock->BlockLength[BYTE_1];
        g_modeParamBlock.BlockLength[BYTE_2] =
            pModeParamBlock->BlockLength[BYTE_2];

        pModePagePtr = (PUCHAR)(pModeParamBlock + numBlockDesc);
    } else {
        /* No block descriptors to parse... go straight to Mode Page */
        pModePagePtr = (PUCHAR)pModeParamBlock;
    }

    pageCode = (*pModePagePtr) & MODE_SELECT_PAGE_CODE_MASK;

    switch (pageCode) {
        case MODE_PAGE_CACHING:
            /* Per NVMe we can't send VWC commands if its not supported */
            if (supportsVwc == FALSE) {
                pSrb->SrbStatus = SRB_STATUS_SUCCESS;
                returnStatus = SNTI_COMMAND_COMPLETED;
            } else {
            /* Command requires NVMe Get Features to adapter - build DWORD 11 */
            pCacheModePage = (PMODE_CACHING_PAGE)pModePagePtr;
            dword11 = (pCacheModePage->WriteCacheEnable ? 1 : 0);

            SntiBuildSetFeaturesCmd(pSrbExt, VOLATILE_WRITE_CACHE, dword11);

            returnStatus = SNTI_TRANSLATION_SUCCESS;
            pSrb->SrbStatus = SRB_STATUS_PENDING;
            }
        break;
        case MODE_PAGE_CONTROL:
            /* Command is completed in Build I/O phase */
            pSrb->SrbStatus = SRB_STATUS_SUCCESS;
            returnStatus = SNTI_COMMAND_COMPLETED;
        break;
        case MODE_PAGE_POWER_CONDITION:
            /* Verify the OS is not trying to set timers */
            pPowerModePage = (PPOWER_CONDITION_MODE_PAGE)pModePagePtr;

            /* Command is completed in Build I/O phase */
            pSrb->SrbStatus = SRB_STATUS_SUCCESS;
            returnStatus = SNTI_COMMAND_COMPLETED;
        break;
        default:
            SntiSetScsiSenseData(pSrb,
                                 SCSISTAT_CHECK_CONDITION,
                                 SCSI_SENSE_ILLEGAL_REQUEST,
                                 SCSI_ADSENSE_INVALID_CDB,
                                 SCSI_ADSENSE_NO_SENSE);

            pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
            pSrb->DataTransferLength = 0;

            returnStatus = SNTI_FAILURE_CHECK_RESPONSE_DATA;
        break;
    } /* end switch */

    return returnStatus;
} /* SntiTranslateModeData */

/******************************************************************************
 * SntiCreateModeDataHeader
 *
 * @brief Creates the Mode Data Header and sets the appropriate ModeDataLength
 *        and pointer for the Mode Parameter Descriptor Block
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param ppModeParamBlock - Double pointer to mode param block
 * @param pModeDataLength - Pointer to mode data length var
 * @param blockDescLength - Block descriptor length
 * @param modeSense10 - Boolean to determine Mode Sense 10
 *
 * @return VOID
 ******************************************************************************/
VOID SntiCreateModeDataHeader(
    PSCSI_REQUEST_BLOCK pSrb,
    PMODE_PARAMETER_BLOCK *ppModeParamBlock,
    PUINT16 pModeDataLength,
    UINT16 blockDescLength,
    BOOLEAN modeSense10
)
{
    PMODE_PARAMETER_HEADER pModeHeader6 = NULL;
    PMODE_PARAMETER_HEADER10 pModeHeader10 = NULL;

    if (modeSense10 == FALSE) {
        /* MODE SENSE 6 */
        pModeHeader6 = (PMODE_PARAMETER_HEADER)(GET_DATA_BUFFER(pSrb));

        /* Set necessary fields */
        memset(pModeHeader6, 0, sizeof(MODE_PARAMETER_HEADER));
        pModeHeader6->MediumType              = DIRECT_ACCESS_DEVICE;
        pModeHeader6->DeviceSpecificParameter = 0;
        pModeHeader6->BlockDescriptorLength   = (UCHAR)blockDescLength;

        /* Increment pointer and set pointer to Param Desc Block */
        pModeHeader6++;
        *ppModeParamBlock = (PMODE_PARAMETER_BLOCK)pModeHeader6;

        /*
         * Calculate the mode data length - subtract the number of bytes for the
         * mode data length field itself.
         */
        *pModeDataLength += sizeof(MODE_PARAMETER_HEADER);
    } else {
        /* MODE SENSE 10 */
        pModeHeader10 = (PMODE_PARAMETER_HEADER10)(GET_DATA_BUFFER(pSrb));

        /* Set necessary fields */
        memset(pModeHeader10, 0, sizeof(MODE_PARAMETER_HEADER10));
        pModeHeader10->MediumType               = DIRECT_ACCESS_DEVICE;
        pModeHeader10->DeviceSpecificParameter  = 0;

        pModeHeader10->BlockDescriptorLength[BYTE_0] =
            (blockDescLength & WORD_HIGH_BYTE_MASK) >> BYTE_SHIFT_1;
        pModeHeader10->BlockDescriptorLength[BYTE_1] =
            (blockDescLength & WORD_LOW_BYTE_MASK);

        /* Increment pointer and set pointer to Param Desc Block */
        pModeHeader10++;
        *ppModeParamBlock = (PMODE_PARAMETER_BLOCK)pModeHeader10;

        /*
         * Calculate the mode data length - subtract the number of bytes for the
         * mode data length field itself.
         */
        *pModeDataLength += sizeof(MODE_PARAMETER_HEADER10);
    }
} /* SntiCreateModeDataHeader */

/******************************************************************************
 * SntiCreateModeParameterDescBlock
 *
 * @brief Creates the Mode Parameter Descriptor Block and sets the appropriate
 *        ModeDataLength and pointer for the Mode Parameter Descriptor Block
 *
 * @param pLunExt - Pointer to LUN extension
 * @param pModeParamBlock - Pointer to mode param block
 * @param pModeDataLength - Pointer to mode data length
 *
 * @return VOID
 ******************************************************************************/
VOID SntiCreateModeParameterDescBlock(
    PNVME_LUN_EXTENSION pLunExt,
    PMODE_PARAMETER_BLOCK pModeParamBlock,
    PUINT16 pModeDataLength
)
{
    UINT32 lbaLength;
    UINT32 lbaLengthPower;
    UINT8  flbas;

    *pModeDataLength += sizeof(MODE_PARAMETER_BLOCK);

    memset(pModeParamBlock, 0, sizeof(MODE_PARAMETER_BLOCK));
    if (pLunExt->identifyData.NCAP > MODE_BLOCK_DESC_MAX) {
        pModeParamBlock->NumberOfBlocks[BYTE_0] = MODE_BLOCK_DESC_MAX_BYTE;
        pModeParamBlock->NumberOfBlocks[BYTE_1] = MODE_BLOCK_DESC_MAX_BYTE;
        pModeParamBlock->NumberOfBlocks[BYTE_2] = MODE_BLOCK_DESC_MAX_BYTE;
    } else {
    pModeParamBlock->NumberOfBlocks[BYTE_0] =
        (UCHAR)((pLunExt->identifyData.NCAP &
                 DWORD_MASK_BYTE_2) >> BYTE_SHIFT_2);
    pModeParamBlock->NumberOfBlocks[BYTE_1] =
        (UCHAR)((pLunExt->identifyData.NCAP &
                 DWORD_MASK_BYTE_1) >> BYTE_SHIFT_1);
    pModeParamBlock->NumberOfBlocks[BYTE_2] =
        (UCHAR)((pLunExt->identifyData.NCAP & DWORD_MASK_BYTE_0));
    }

    flbas = pLunExt->identifyData.FLBAS.SupportedCombination;
    lbaLengthPower = pLunExt->identifyData.LBAFx[flbas].LBADS;
    lbaLength = 1 << lbaLengthPower;

    pModeParamBlock->BlockLength[BYTE_0] =
        (UCHAR)((lbaLength & DWORD_MASK_BYTE_2) >> BYTE_SHIFT_2);
    pModeParamBlock->BlockLength[BYTE_1] =
        (UCHAR)((lbaLength & DWORD_MASK_BYTE_1) >> BYTE_SHIFT_1);
    pModeParamBlock->BlockLength[BYTE_2] =
        (UCHAR)((lbaLength & DWORD_MASK_BYTE_0));
} /* SntiCreateModeParameterDescBlock */

/******************************************************************************
 * SntiTranslateSglToPrp
 *
 * @brief Translates the Scatter Gather List (SGL) to PRP Entries/List.
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pSgl - Pointer to Scatter Gather List
 *
 * @return VOID
 ******************************************************************************/
VOID SntiTranslateSglToPrp(
    PNVME_SRB_EXTENSION pSrbExt,
    PSTOR_SCATTER_GATHER_LIST pSgl
)
{
    /* PRP list is needed */
    PHYSICAL_ADDRESS physicalAddress;
    PUINT64 pPrpList = NULL;
    UINT32 numImplicitEntries;
    UINT32 subIndex;
    UINT32 sgElementSize;
    UINT32 index;
    ULONGLONG localPrpEntry = 0;
    ULONG lengthIncrement;
    ULONG offset;
    PULONGLONG pPrp1 = &pSrbExt->nvmeSqeUnit.PRP1;
    PULONGLONG pPrp2 = &pSrbExt->nvmeSqeUnit.PRP2;
    ULONG modulo;

#if DUMB_DRIVER
        return;
#endif

    pSrbExt->numberOfPrpEntries = 0;
    ASSERT(pSgl->NumberOfElements != 0);

    /* There may not always be a 1:1 ratio of SG elements to PRP entries... */
    pPrpList = &pSrbExt->prpList[0];

    for (index = 0; index < pSgl->NumberOfElements; index++) {

        /* NOTE: This size may be more than a PAGE size */
        sgElementSize = pSgl->List[index].Length;

        /* Sub page size entries can only be the first or last SG elements */
        if (((sgElementSize % PAGE_SIZE) != 0)  &&
              (index != (pSgl->NumberOfElements - 1)) &&
             (index != 0 )) {
            /* Future code to break out child I/O's, if necessary */
            ASSERT(FALSE);
        }

        /*
         * Get the number of implicit PRP entries in this SG element and
         * the starting physical address... this can either be one entry
         * or multiple entries (if not the first or last entry). Note that
         * multiple entries per SG element are required to be multiples of
         * the page size.
         */
        physicalAddress.QuadPart = pSgl->List[index].PhysicalAddress.QuadPart;

        /* calc number of whole pages, if any */
        numImplicitEntries = sgElementSize / PAGE_SIZE;
        /* calc the leftover size minus whole pages */
        modulo = sgElementSize % PAGE_SIZE;
        /* determine if our start is aligned or not */
        offset = physicalAddress.LowPart % PAGE_SIZE;

        /*
         * if we have a length modulo or we're not aligned then
         * we treat this like an unaligned < page size case where
         * we we're definately taking up one page and depending
         * on the size of the leftovers (mod + alignment offset)
         * we may need to account for one more page
         */
        if (modulo || offset) {
            numImplicitEntries += 1 + (modulo + offset - 1) / PAGE_SIZE;
        }

        /* For each SG element */
        for (subIndex = 0; subIndex < numImplicitEntries; subIndex++) {
            /* Get first sub element (implied element) */
            localPrpEntry = (UINT64)(physicalAddress.QuadPart);

            /* Keep track of the number of PRP Entries */
            pSrbExt->numberOfPrpEntries++;

            if (pSrbExt->numberOfPrpEntries == PRP_ENTRY_1) {
                *pPrp1 = localPrpEntry;

                /* Check for offset if first entry is not page aligned */
                offset = physicalAddress.LowPart & PAGE_MASK;
                lengthIncrement = PAGE_SIZE - offset;
            } else if (pSrbExt->numberOfPrpEntries == PRP_ENTRY_2) {
                *pPrp2 = localPrpEntry;
            } else if (pSrbExt->numberOfPrpEntries == PRP_ENTRY_3) {
                /*
                 * Copy the second entry and increment the pointer then zero
                 * out the 2nd entry, it will be updated in the StartIo path
                 * wiht pre-allcoated list memory.
                 */

                *pPrpList = *pPrp2;
                 pPrpList++;
                *pPrp2 = 0;

                /* Place next PRP entry in list and increment the ptr */
                *pPrpList = (UINT64)(physicalAddress.QuadPart);
                pPrpList++;
            } else {
                /* Place next PRP entry in list and increment the ptr */
                *pPrpList = (UINT64)(physicalAddress.QuadPart);
                pPrpList++;
            }

            /*
             * Increment to the next implied SG element (physical address).
             * Note that these entries must be contiguous in physical memory
             * so increment to the next page entry.
             */
            if (pSrbExt->numberOfPrpEntries > 1)
                lengthIncrement = PAGE_SIZE;

            physicalAddress.QuadPart += lengthIncrement;
        } /* end for loop */
    } /* end for loop */
} /* SntiTranslateSglToPrp */

/******************************************************************************
 * SntiValidateLbaAndLength
 *
 * @brief Validates the LBA and TransferLength of a SCSI WRITE 6, 10, 12, 16
 *        command. If the LBA + Transler Length exceeds the capacity of the
 *        namespace, a check condition with ILLEGAL REQUEST - LBA Out Of Range
 *        (5/2100) shall be returned.
 *
 *        NOTE: Transfer Length is in logical blocks (per the SBC-3)
 *        NOTE: Namespace Size is in logical blocks (per NVM Express 1.0b)
 *
 * @param: pLunExt - Pointer to LUN extension
 * @param: pSrbExt - Pointer to SRB extension
 * @param: lba - LBA to verify
 * @param: length - Length to verify
 *
 * @return SNTI_STATUS
 *     Indicates internal tranlsation status
 ******************************************************************************/
SNTI_STATUS SntiValidateLbaAndLength(
    PNVME_LUN_EXTENSION pLunExt,
    PNVME_SRB_EXTENSION pSrbExt,
    UINT64 lba,
    UINT32 length
)
{
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    SNTI_STATUS status = SNTI_SUCCESS;

    if ((lba + length) > (pLunExt->identifyData.NSZE + 1)) {
        SntiSetScsiSenseData(pSrb,
                             SCSISTAT_CHECK_CONDITION,
                             SCSI_SENSE_ILLEGAL_REQUEST,
                             SCSI_ADSENSE_ILLEGAL_BLOCK,
                             SCSI_ADSENSE_NO_SENSE);

        pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        pSrb->DataTransferLength = 0;
        status = SNTI_INVALID_PARAMETER;
    }

    if (length > NVME_MAX_NUM_BLOCKS_PER_READ_WRITE) {
        SntiSetScsiSenseData(pSrb,
                             SCSISTAT_CHECK_CONDITION,
                             SCSI_SENSE_ILLEGAL_REQUEST,
                             SCSI_ADSENSE_ILLEGAL_BLOCK,
                             SCSI_ADSENSE_NO_SENSE);

        pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        pSrb->DataTransferLength = 0;
        status = SNTI_INVALID_PARAMETER;
    }

    return status;
} /* SntiValidateLbaAndLength */

/******************************************************************************
 * SntiSetScsiSenseData
 *
 * @brief Sets up the SCSI sense data in the provided SRB.
 *
 *        NOTE: The caller of this func must set the correlating SRB status
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param scsiStatus - SCSI Status to be stored in the Sense Data buffer
 * @param senseKey - Sense Key for the Sense Data
 * @param asc - Additional Sense Code (ASC)
 * @param ascq - Additional Sense Code Qualifier (ASCQ)
 *
 * @return BOOLEAN
 ******************************************************************************/
BOOLEAN SntiSetScsiSenseData(
    PSCSI_REQUEST_BLOCK pSrb,
    UCHAR scsiStatus,
    UCHAR senseKey,
    UCHAR asc,
    UCHAR ascq
)
{
    PSENSE_DATA pSenseData = NULL;
    BOOLEAN status = TRUE;

    pSrb->ScsiStatus = scsiStatus;

    if ((scsiStatus != SCSISTAT_GOOD) &&
        (pSrb->SenseInfoBufferLength >= sizeof(SENSE_DATA))) {
        pSenseData = (PSENSE_DATA)pSrb->SenseInfoBuffer;

        memset(pSenseData, 0, pSrb->SenseInfoBufferLength);
        pSenseData->ErrorCode                    = FIXED_SENSE_DATA;
        pSenseData->SenseKey                     = senseKey;
        pSenseData->AdditionalSenseCode          = asc;
        pSenseData->AdditionalSenseCodeQualifier = ascq;

        pSenseData->AdditionalSenseLength = sizeof(SENSE_DATA) -
            FIELD_OFFSET(SENSE_DATA, CommandSpecificInformation);

        pSrb->SenseInfoBufferLength = sizeof(SENSE_DATA);
        pSrb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;
    } else {
        pSrb->SenseInfoBufferLength = 0;
        status = FALSE;
    }

   return status;
}

/******************************************************************************
 * GetLunExtension
 *
 * @brief Returns the LUN Extension structure for the B/T/L nexus associated
 *        with the SRB Extension and SRB I/O request.
 *
 * @param pSrbExt - Pointer to the SRB Extension
 * @param ppLunExt - Double pointer to the LUN Extension
 *
 * @return SNTI_STATUS
 *     Indicates if the LUN Extension was found successfully.
 ******************************************************************************/
SNTI_STATUS GetLunExtension(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION *ppLunExt
)
{
    PNVME_DEVICE_EXTENSION pDevExt = NULL;
    PSCSI_REQUEST_BLOCK pSrb = NULL;

    SNTI_STATUS returnStatus = SNTI_SUCCESS;

    pDevExt = pSrbExt->pNvmeDevExt;
    pSrb = pSrbExt->pSrb;

    ASSERT(pDevExt != NULL);
    ASSERT(pSrb != NULL);

    if ((pSrb->PathId != VALID_NVME_PATH_ID) ||
        (pSrb->TargetId != VALID_NVME_TARGET_ID) ||
        (pDevExt->pLunExtensionTable[pSrb->Lun]->slotStatus != ONLINE)) {
        *ppLunExt = NULL;
        returnStatus = SNTI_INVALID_PATH_TARGET_ID;
    } else {
        *ppLunExt = pDevExt->pLunExtensionTable[pSrb->Lun];
    }
    return returnStatus;
} /* GetLunExtension */

/******************************************************************************
 * SntiIssueGetFeaturesCmd
 *
 * @brief Builds an internal NVMe GET FEATURES command. Note that the SRB data
 *        buffer will be used for the PRP entry unless indicated by the boolean
 *        value passed in that states a separate buffer is needed.
 *
 * @param pSrbExt - This parameter specifies the SRB Extension and the
 *                  associated SRB with P/T/L nexus.
 * @param featureIdentifier - Identifier for Get Features
 *
 * @return VOID
 ******************************************************************************/
VOID SntiBuildGetFeaturesCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT8 featureIdentifier
)
{
    PNVME_DEVICE_EXTENSION pDevExt = pSrbExt->pNvmeDevExt;
    PSTOR_SCATTER_GATHER_LIST pSgl = NULL;

    /* Make sure we indicate which I/Os are the child and parent */
    pSrbExt->pParentIo = NULL;
    pSrbExt->pChildIo = NULL;

    /* And which queue to use */
    pSrbExt->forAdminQueue = TRUE;

    /* Set up the GET FEATURES command */
    memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));
    pSrbExt->nvmeSqeUnit.CDW0.OPC = ADMIN_GET_FEATURES;
    pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
    pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;

    /* DWORD 10 */
    pSrbExt->nvmeSqeUnit.CDW10 |= featureIdentifier;

    /*
     *  NOTE: The only feature identifer that uses a buffer to return data is
     *        LBA Range Type (0x03). All other feature identifiers will use a
     *        DWORD in the completion queue entry to return the attributes of
     *        a feature requested.
     */
    if (featureIdentifier != LBA_RANGE_TYPE) {
        /* PRP's not used for these requests */
        pSrbExt->nvmeSqeUnit.PRP1 = 0;
        pSrbExt->nvmeSqeUnit.PRP2 = 0;
    } else {
        /* PRP Entry/List - Use the SGL from the original command */
        pSgl = StorPortGetScatterGatherList(pDevExt, pSrbExt->pSrb);
        ASSERT(pSgl != NULL);

        SntiTranslateSglToPrp(pSrbExt, pSgl);
    }

    /* SRB Status must be set to PENDING */
    pSrbExt->pSrb->SrbStatus = SRB_STATUS_PENDING;
} /* SntiBuildGetFeaturesCmd */

/******************************************************************************
 * SntiBuildSetFeaturesCmd
 *
 * @brief Builds an internal NVMe SET FEATURES command.
 *
 * @param pSrbExt - This parameter specifies the SRB Extension and the
 *                  associated SRB with P/T/L nexus.
 * @param featureIdentifier - Identifer for the Set Features command
 * @param dword11 - DWORD 11 of the Set Features command
 *
 * @return VOID
 ******************************************************************************/
VOID SntiBuildSetFeaturesCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT8 featureIdentifier,
    UINT32 dword11
)
{
    PNVME_DEVICE_EXTENSION pDevExt = pSrbExt->pNvmeDevExt;
    PSTOR_SCATTER_GATHER_LIST pSgl = NULL;

    /* Make sure we indicate which I/Os are the child and parent */
    pSrbExt->pParentIo = NULL;
    pSrbExt->pChildIo = NULL;

    /* And which queue to use */
    pSrbExt->forAdminQueue = TRUE;

    /* Set up the GET FEATURES command */
    memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));
    pSrbExt->nvmeSqeUnit.CDW0.OPC = ADMIN_SET_FEATURES;
    pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
    pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;

    /* DWORD 10 and 11 */
    pSrbExt->nvmeSqeUnit.CDW10 |= featureIdentifier;
    pSrbExt->nvmeSqeUnit.CDW11 = dword11;

    /*
     *  NOTE: The only feature identifer that uses a buffer to return data is
     *        LBA Range Type (0x03). All other feature identifiers will use a
     *        DWORD in the completion queue entry to return the attributes of
     *        a feature requested.
     */
    if (featureIdentifier != LBA_RANGE_TYPE) {
        /* PRP's not used for these requests */
        pSrbExt->nvmeSqeUnit.PRP1 = 0;
        pSrbExt->nvmeSqeUnit.PRP2 = 0;
    } else {
        /* PRP Entry/List - Use the SGL from the original command */
        pSgl = StorPortGetScatterGatherList(pDevExt, pSrbExt->pSrb);
        ASSERT(pSgl != NULL);

        SntiTranslateSglToPrp(pSrbExt, pSgl);
    }

    /* SRB Status must be set to PENDING */
    pSrbExt->pSrb->SrbStatus = SRB_STATUS_PENDING;
} /* SntiBuildSetFeaturesCmd */

/******************************************************************************
 * SntiBuildGetLogPageCmd
 *
 * @brief Builds an internal NVMe GET LOG PAGE command. Note that the SRB data
 *        buffer will be used for the PRP entries.
 *
 * @param pSrbExt - This parameter specifies the SRB Extension and the
 *                  associated SRB with P/T/L nexus.
 * @param logIdentifier - Log page identifier
 *
 * @return VOID
 ******************************************************************************/
VOID SntiBuildGetLogPageCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT8 logIdentifier
)
{
    PNVME_DEVICE_EXTENSION pDevExt = pSrbExt->pNvmeDevExt;
    PSTOR_SCATTER_GATHER_LIST pSgl = NULL;
    UINT32 numDwords = 0;

    /* Make sure we indicate which I/Os are the child and parent */
    pSrbExt->pParentIo = NULL;
    pSrbExt->pChildIo = NULL;

    /* And which queue to use */
    pSrbExt->forAdminQueue = TRUE;

    /* Set up the GET LOG PAGE command */
    memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));
    pSrbExt->nvmeSqeUnit.CDW0.OPC = ADMIN_GET_LOG_PAGE;
    pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
    pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;

    /* NOTE: This value must not exceed 4K */
    switch(logIdentifier) {
        case ERROR_INFORMATION:
            numDwords =
                sizeof(ADMIN_GET_LOG_PAGE_ERROR_INFORMATION_LOG_ENTRY);
        break;
        case SMART_HEALTH_INFORMATION:
            numDwords =
                sizeof(ADMIN_GET_LOG_PAGE_SMART_HEALTH_INFORMATION_LOG_ENTRY);
        break;
        case FIRMWARE_SLOT_INFORMATION:
            numDwords =
                sizeof(ADMIN_GET_LOG_PAGE_FIRMWARE_SLOT_INFORMATION_LOG_ENTRY);
        break;
        default:
            ASSERT(FALSE);
        break;
    }

    /* DWORD 10 */
    pSrbExt->nvmeSqeUnit.CDW10 |= (numDwords << BYTE_SHIFT_2);
    pSrbExt->nvmeSqeUnit.CDW10 |= (logIdentifier & DWORD_MASK_LOW_WORD);

    if (pSrbExt->pDataBuffer != NULL) {
        /* Use the allocated data buffer for PRP Entries/List */
        STOR_PHYSICAL_ADDRESS physAddr;
        ULONG paLength;

        physAddr = StorPortGetPhysicalAddress(pDevExt,
                                              NULL,
                                              pSrbExt->pDataBuffer,
                                              &paLength);

        if (physAddr.QuadPart != 0) {
            pSrbExt->nvmeSqeUnit.PRP1 = physAddr.QuadPart;
            pSrbExt->nvmeSqeUnit.PRP2 = 0;
        } else {
            StorPortDebugPrint(
                INFO,
                "SNTI: Get PhysAddr for GET LOG PAGE failed (pSrbExt = 0x%x)\n",
                pSrbExt);

            ASSERT(FALSE);
        }
    } else {
        /* Use the SGL from the original command for PRP Entries/List */
        pSgl = StorPortGetScatterGatherList(pDevExt, pSrbExt->pSrb);

        if (pSgl != NULL) {
            #ifdef DEBUG_CHECK
            if ((pSgl->NumberOfElements * sizeof(STOR_SCATTER_GATHER_ELEMENT))
                >= (numDwords/sizeof(UINT32))) {
                /* In this case, fail the command or create a temp buffer */
                ASSERT(FALSE);
                return;
            }
            #endif /* DEBUG_CHECK */

            /* Translate the SRB SGL to a PRP List */
            SntiTranslateSglToPrp(pSrbExt, pSgl);
        } else {
            StorPortDebugPrint(
                INFO,
                "SNTI: Get SGL for GET LOG PAGE failed (pSrbExt = 0x%x)\n",
                pSrbExt);

            ASSERT(FALSE);
        }
    }

    /* SRB Status must be set to PENDING */
    pSrbExt->pSrb->SrbStatus = SRB_STATUS_PENDING;
} /* SntiBuildGetLogPageCmd */

/******************************************************************************
 * SntiBuildFirmwareImageDownloadCmd
 *
 * Builds an internal NVMe FIRMWARE IMAGE DOWNLOAD command.
 *
 * @param pSrbExt - This parameter specifies the SRB Extension and the
 *                  associated SRB with P/T/L nexus.
 * @param dword10 - DWORD 10 of the FIRMWARE IMAGE DOWNLOAD command
 * @param dword11 - DWORD 11 of the FIRMWARE IMAGE DOWNLOAD command
 *
 * return: VOID
 ******************************************************************************/
VOID SntiBuildFirmwareImageDownloadCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT32 dword10,
    UINT32 dword11
)
{
    PNVME_DEVICE_EXTENSION pDevExt = pSrbExt->pNvmeDevExt;
    PSTOR_SCATTER_GATHER_LIST pSgl = NULL;
    UINT32 numDwords = 0;

    /* Make sure we indicate which I/Os are the child and parent */
    pSrbExt->pParentIo = NULL;
    pSrbExt->pChildIo = NULL;

    /* And which queue to use */
    pSrbExt->forAdminQueue = TRUE;

    /* Set up the GET LOG PAGE command */
    memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));
    pSrbExt->nvmeSqeUnit.CDW0.OPC = ADMIN_FIRMWARE_IMAGE_DOWNLOAD;
    pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
    pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;

    /* DWORD 10/11 */
    pSrbExt->nvmeSqeUnit.CDW10 = dword10;
    pSrbExt->nvmeSqeUnit.CDW11 = dword11;

    /* PRP Entry/List - Use the SGL from the original command */
    pSgl = StorPortGetScatterGatherList(pDevExt, pSrbExt->pSrb);
    ASSERT(pSgl != NULL);

    #define DEBUG_CHECK
    #ifdef DEBUG_CHECK
    if ((pSgl->NumberOfElements * sizeof(STOR_SCATTER_GATHER_ELEMENT)) >=
        (numDwords/sizeof(UINT32))) {
        /* In this case, must fail the command or create a temp buffer... */

        ASSERT(FALSE);
        return;
    }
    #endif /* DEBUG_CHECK */

    SntiTranslateSglToPrp(pSrbExt, pSgl);

    /* SRB Status must be set to PENDING */
    pSrbExt->pSrb->SrbStatus = SRB_STATUS_PENDING;
} /* SntiBuildFirmwareImageDownloadCmd */

/******************************************************************************
 * SntiBuildFirmwareActivateCmd
 *
 * @brief Builds an internal NVMe FIRMWARE ACTIVATE command.
 *
 * @param pSrbExt - This parameter specifies the SRB Extension and the
 *                  associated SRB with P/T/L nexus.
 * @param dword10 - DWORD 10 of the FIRMWARE ACTIVATE command
 *
 * return: VOID
 ******************************************************************************/
VOID SntiBuildFirmwareActivateCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT32 dword10
)
{
    /* Chose which queue to use */
    pSrbExt->forAdminQueue = TRUE;

    /* Set up the GET LOG PAGE command */
    memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));
    pSrbExt->nvmeSqeUnit.CDW0.OPC = ADMIN_FIRMWARE_ACTIVATE;
    pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
    pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;

    /* DWORD 10/11 */
    pSrbExt->nvmeSqeUnit.CDW10 = dword10;

    /* SRB Status must be set to PENDING */
    pSrbExt->pSrb->SrbStatus = SRB_STATUS_PENDING;
} /* SntiBuildFirmwareActivateCmd */

/******************************************************************************
 * SntiBuildFlushCmd
 *
 * @brief Builds an internal NVME FLUSH command. The NVME FLUSH command is
 *        defined:
 *
 *        6.7 Flush command
 *
 *        The Flush command is used by the host to indicate that any data in
 *        volatile storage should be flushed to non-volatile memory.
 *
 *        All command specific fields are reserved.
 *
 * @param pSrbExt - This parameter specifies the SRB Extension and the
 *                  associated SRB with P/T/L nexus.
 *
 * @return VOID
 ******************************************************************************/
VOID SntiBuildFlushCmd(
    PNVME_SRB_EXTENSION pSrbExt
)
{
    /* Set up common portions of the NVMe WRITE command */
    memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));

    pSrbExt->nvmeSqeUnit.CDW0.OPC = NVM_FLUSH;
    pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
    pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;

    /* Set the SRB status to pending - controller communication necessary */
    pSrbExt->pSrb->SrbStatus = SRB_STATUS_PENDING;
} /* SntiBuildFlushCmd */

/******************************************************************************
 * SntiBuildFormatNvmCmd
 *
 * @brief Builds an internal NVME FORMAT NVM command.
 *
 * @param pSrbExt - This parameter specifies the SRB Extension and the
 *                  associated SRB with P/T/L nexus.
 * @param protectionType
 *
 * return: VOID
 ******************************************************************************/
VOID SntiBuildFormatNvmCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT8 protectionType
)
{
    /* Chose which queue to use */
    pSrbExt->forAdminQueue = TRUE;

    /* Set up common portions of the NVMe WRITE command */
    memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));

    pSrbExt->nvmeSqeUnit.CDW0.OPC = ADMIN_FORMAT_NVM;
    pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
    pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;

    /* DWORD 10 */
    pSrbExt->nvmeSqeUnit.CDW10 |=
        (protectionType << FORMAT_NVM_PROTECTION_INFO_SHIFT_MASK);

    /* Specify the block length and # of blocks from last MODE SELECT */

    /* Set the SRB status to pending - controller communication necessary */
    pSrbExt->pSrb->SrbStatus = SRB_STATUS_PENDING;
} /* SntiBuildFormatNvmCmd */

/******************************************************************************
 * SntiBuildSecuritySendReceiveCmd
 *
 * @brief Builds an internal NVME Security Send/Receive command.
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pLunExt - Pointer to LUN extension
 * @param opcode - Opcode (SEND or RECEIVE)
 * @param transferLength - Length of data transfer
 * @param secProtocolSp - Security protocol specific info
 * @param secProtocol - Security protocol info
 *
 * @return VOID
 ******************************************************************************/
VOID SntiBuildSecuritySendReceiveCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT8 opcode,
    UINT32 transferLength,
    UINT16 secProtocolSp,
    UINT8 secProtocol
)
{
    PSTOR_SCATTER_GATHER_LIST pSgl = NULL;
    STOR_PHYSICAL_ADDRESS physAddr;
    ULONG length;

    PADMIN_SECURITY_SEND_COMMAND_DW10 pCdw10 =
        (PADMIN_SECURITY_SEND_COMMAND_DW10)&pSrbExt->nvmeSqeUnit.CDW10;
    PADMIN_SECURITY_SEND_COMMAND_DW11 pCdw11 =
        (PADMIN_SECURITY_SEND_COMMAND_DW11)&pSrbExt->nvmeSqeUnit.CDW11;

    /* Chose which queue to use */
    pSrbExt->forAdminQueue = TRUE;

    /* Set up common portions of the NVMe WRITE command */
    memset(&pSrbExt->nvmeSqeUnit, 0, sizeof(NVMe_COMMAND));

    pSrbExt->nvmeSqeUnit.CDW0.OPC = opcode;
    pSrbExt->nvmeSqeUnit.CDW0.CID = 0;
    pSrbExt->nvmeSqeUnit.CDW0.FUSE = FUSE_NORMAL_OPERATION;

    pSrbExt->nvmeSqeUnit.NSID = pLunExt->namespaceId;

    /* PRP Entry/List */
    pSgl = StorPortGetScatterGatherList(pSrbExt->pNvmeDevExt, pSrbExt->pSrb);
    ASSERT(pSgl != NULL);

    SntiTranslateSglToPrp(pSrbExt, pSgl);

    /* DWORD 10 */
    pCdw10->SPSP = secProtocolSp;
    pCdw10->SECP = secProtocol;

    /* DWORD 11 */
    pCdw11->AL = transferLength;
} /* SntiBuildSecuritySendReceiveCmd */

/******************************************************************************
 * SntiCompletionCallbackRoutine
 *
 * @brief This method handles the completion callback from the I/O engine for
 *        commands that need to have a status translated, a NVMe response
 *        translated into data, or that need to send additional commands.
 *
 *        Commands that require additional work on completion side:
 *
 *        - Log Sense
 *        - Mode Sense
 *        - Start Stop Unit (with implicit NVMe Flush cmd)
 *        - Write Buffer
 *
 * @param param1 - Pointer to device extension
 * @param param2 - Pointer to SRB extension
 *
 * @return BOOLEAN
 *     TRUE - successful completion of callback
 *     FALSE - unsuccessful completion of callback or error
 ******************************************************************************/
BOOLEAN SntiCompletionCallbackRoutine(
    PVOID param1,
    PVOID param2
)
{
    PNVME_DEVICE_EXTENSION pDevExt = (PNVME_DEVICE_EXTENSION)param1;
    PNVME_SRB_EXTENSION pSrbExt = (PNVME_SRB_EXTENSION)param2;

    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    PNVMe_COMPLETION_QUEUE_ENTRY pCQEntry = pSrbExt->pCplEntry;
    UINT8 statusCodeType;
    UINT8 statusCode;
    BOOLEAN returnValue = TRUE;

    /* Default to successful command sequence completion */
    SNTI_TRANSLATION_STATUS translationStatus = SNTI_SEQUENCE_COMPLETED;

    /* Before calling commpletion code, insure NVMe command succeeded */
    statusCodeType = (UINT8)pSrbExt->pCplEntry->DW3.SF.SCT;
    statusCode = (UINT8)pSrbExt->pCplEntry->DW3.SF.SC;

    if ((statusCodeType == GENERIC_COMMAND_STATUS) &&
        (statusCode == SUCCESSFUL_COMPLETION)) {
        if (pSrb != NULL) {
            switch (GET_OPCODE(pSrb)) {
                case SCSIOP_LOG_SENSE:
                    translationStatus = SntiTranslateLogSenseResponse(
                        pSrb, pCQEntry);

                    if (translationStatus == SNTI_SEQUENCE_IN_PROGRESS)
                        returnValue = FALSE;
                break;
                case SCSIOP_MODE_SENSE:
                case SCSIOP_MODE_SENSE10:
                    translationStatus = SntiTranslateModeSenseResponse(
                        pSrb, pCQEntry);
                break;
                case SCSIOP_START_STOP_UNIT:
                    /* NOTE: This is an implicit NVM Flush command completing */
                    translationStatus =
                        SntiTranslateStartStopUnitResponse(pSrb);

                    if (translationStatus == SNTI_SEQUENCE_IN_PROGRESS)
                        returnValue = FALSE;
                break;
                case SCSIOP_WRITE_DATA_BUFF:
                    translationStatus = SntiTranslateWriteBufferResponse(pSrb);
                break;
                default:
                    /* Invalid Condition */
                    ASSERT(FALSE);
                    translationStatus = SNTI_SEQUENCE_ERROR;
                break;
            } /* end switch */
        } else {
            /* SRB is NULL... this function should not be called */
            translationStatus = SNTI_SEQUENCE_ERROR;
        }
    } else {
        /* NVME command status failure */
        translationStatus = SNTI_SEQUENCE_ERROR;
    }

    if ((pSrb != NULL) &&
        (translationStatus != SNTI_SEQUENCE_COMPLETED) &&
        (translationStatus != SNTI_SEQUENCE_IN_PROGRESS) &&
        (pSrb->SrbStatus != SRB_STATUS_PENDING)) {
        SntiMapCompletionStatus(pSrbExt);
    }

    return returnValue;
} /* SntiCompletionCallbackRoutine */

/******************************************************************************
 * SntiTranslateLogSenseResponse
 *
 * @brief Translates the SCSI Log Sense command. Populates the appropriate SCSI
 *        Log Sense page data based on the NVMe Translation spec. Do not need to
 *        create SQE here as we just complete the command in the build phase (by
 *        returning FALSE to StorPort with SRB status of SUCCESS).
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param pCQEntry - Pointer to completion queue entry
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateLogSenseResponse(
    PSCSI_REQUEST_BLOCK pSrb,
    PNVMe_COMPLETION_QUEUE_ENTRY pCQEntry
)
{
    UINT16 allocLength;
    UINT8 pageCode;

    /* Default to successful command sequence completion */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_SEQUENCE_COMPLETED;

    pageCode = GET_U8_FROM_CDB(pSrb, LOG_SENSE_CDB_PAGE_CODE_OFFSET);
    pageCode &= LOG_SENSE_CDB_PAGE_CODE_MASK;
    allocLength = GET_U16_FROM_CDB(pSrb, LOG_SENSE_CDB_ALLOC_LENGTH_OFFSET);

    /* No parameter checking is necessary... it was done on the submission */

    if (pageCode == LOG_PAGE_INFORMATIONAL_EXCEPTIONS_PAGE) {
        returnStatus =
            SntiTranslateInformationalExceptionsResponse(pSrb, allocLength);
    } else if (pageCode == LOG_PAGE_TEMPERATURE_PAGE) {
        returnStatus =
            SntiTranslateTemperatureResponse(pSrb, pCQEntry, allocLength);
    } else {
        returnStatus = SNTI_SEQUENCE_ERROR;
    }

    return returnStatus;
} /* SntiTranslateLogSenseResponse*/

/******************************************************************************
 * SntiTranslateInformationalExceptionsResponse
 *
 * @brief Translates the Log Sense page - Informational Exceptions Page.
 *        Populates the appropriate log page response fields based on the NVMe
 *        Translation spec. This log page requires both local data storage and
 *        adapter communication. Create the NVMe Admin command - Get Log Page:
 *        SMART/Health Information and populates the temporary SQE stored in the
 *        SRB Extension.
 *
 *        NOTE: Must free extra buffer used for this command.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param allocLength - Allocation Length from Log Sense command
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateInformationalExceptionsResponse(
    PSCSI_REQUEST_BLOCK pSrb,
    UINT16 allocLength
)
{
    PNVME_SRB_EXTENSION pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    PNVME_DEVICE_EXTENSION pDevExt = pSrbExt->pNvmeDevExt;
    PADMIN_GET_LOG_PAGE_SMART_HEALTH_INFORMATION_LOG_ENTRY pNvmeLogPage = NULL;
    PINFORMATIONAL_EXCEPTIONS_LOG_PAGE pScsiLogPage = NULL;
    UINT16 temperature = 0;
    BOOLEAN storStatus;
    PVOID pBuf = pSrbExt->pDataBuffer;

    /* Default to successful command sequence completion */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_SEQUENCE_COMPLETED;

    /* Parse the SMART/Health Information Log Page */

    /* Must convert temperature from Kelvin to Celsius */
    pNvmeLogPage =
        (PADMIN_GET_LOG_PAGE_SMART_HEALTH_INFORMATION_LOG_ENTRY)pBuf;
    temperature = pNvmeLogPage->Temperature - KELVIN_TEMP_FACTOR;

    /* Issue DPC to free the data buffer memory in the SRB Extension */
    storStatus = StorPortIssueDpc(
        pDevExt,
        &pDevExt->SntiDpc,
        (PVOID)pSrbExt,
        (PVOID)sizeof(ADMIN_GET_LOG_PAGE_SMART_HEALTH_INFORMATION_LOG_ENTRY));

    if (storStatus != TRUE) {
        returnStatus = SNTI_SEQUENCE_ERROR;
        ASSERT(FALSE);
    } else {
        pScsiLogPage = (PINFORMATIONAL_EXCEPTIONS_LOG_PAGE)pSrb->DataBuffer;

        memset(pScsiLogPage, 0, sizeof(INFORMATIONAL_EXCEPTIONS_LOG_PAGE));
        pScsiLogPage->PageCode         = LOG_PAGE_INFORMATIONAL_EXCEPTIONS_PAGE;
        pScsiLogPage->SubPageFormat    = SUB_PAGE_FORMAT_UNSUPPORTED;
        pScsiLogPage->DisableSave      = DISABLE_SAVE_UNSUPPORTED;
        pScsiLogPage->SubPageCode      = SUB_PAGE_CODE_UNSUPPORTED;
        pScsiLogPage->PageLength[0]    = 0;
        pScsiLogPage->PageLength[1]    = REMAINING_INFO_EXCP_PAGE_LENGTH;
        pScsiLogPage->ParameterCode[0] = GENERAL_PARAMETER_DATA;
        pScsiLogPage->ParameterCode[1] = GENERAL_PARAMETER_DATA;
        pScsiLogPage->FormatAndLinking = BINARY_FORMAT_LIST;
        pScsiLogPage->TMC              = TMC_UNSUPPORTED;
        pScsiLogPage->ETC              = ETC_UNSUPPORTED;
        pScsiLogPage->TSD              = LOG_PARAMETER_DISABLED;
        pScsiLogPage->DU               = DU_UNSUPPORTED;
        pScsiLogPage->ParameterLength  = INFO_EXCP_PARM_LENGTH;
        pScsiLogPage->InfoExcpAsc      = INFO_EXCP_ASC_NONE;
        pScsiLogPage->InfoExcpAscq     = INFO_EXCP_ASCQ_NONE;
        pScsiLogPage->MostRecentTempReading = (UINT8)temperature;

        pSrb->DataTransferLength =
            min(sizeof(INFORMATIONAL_EXCEPTIONS_LOG_PAGE), allocLength);

        pSrb->SrbStatus = SRB_STATUS_SUCCESS;
        returnStatus = SNTI_SEQUENCE_COMPLETED;
    }

    return returnStatus;
} /* SntiTranslateInformationalExceptionsResponse */

/******************************************************************************
 * SntiTranslateTemperatureResponse
 *
 * @brief Translates the NVM Express command(s) for the SCSI Log Sense page -
 *        Temperature Log Page. Populates the appropriate log page response
 *        fields based on the NVMe Translation spec. This log page requires both
 *        local data storage and adapter communication.
 *
 *        NOTE: Must free extra buffer used for this command.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param pCQEntry - Pointer to completion queue entry
 * @param allocLength - Allocation length from Log Sense CDB
 *
 * return: VOID
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateTemperatureResponse(
    PSCSI_REQUEST_BLOCK pSrb,
    PNVMe_COMPLETION_QUEUE_ENTRY pCQEntry,
    UINT16 allocLength
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PNVME_DEVICE_EXTENSION pDevExt = NULL;
    PTEMPERATURE_LOG_PAGE pScsiLogPage = NULL;
    PADMIN_GET_LOG_PAGE_SMART_HEALTH_INFORMATION_LOG_ENTRY pNvmeLogPage = NULL;
    BOOLEAN storStatus;
    UINT16 logTemp;
    PVOID pBuf;

    /* Default to in-progress command sequence */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_SEQUENCE_IN_PROGRESS;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    pDevExt = (PNVME_DEVICE_EXTENSION)pSrbExt->pNvmeDevExt;
    pBuf = pSrbExt->pDataBuffer;

    /*
     * The SCSI Log Page (Temperature page) is being stored in the SRB data
     * buffer
     */
    pScsiLogPage = (PTEMPERATURE_LOG_PAGE)pSrb->DataBuffer;

    /* Check which phase of sequence by checking the SQE unit opcode */
    if (pSrbExt->nvmeSqeUnit.CDW0.OPC == ADMIN_GET_LOG_PAGE) {
        /*
         * Parse the SMART/Health Information Log Page. Must convert temperature
         * from Kelvin to Celsius.
         */
        pNvmeLogPage =
            (PADMIN_GET_LOG_PAGE_SMART_HEALTH_INFORMATION_LOG_ENTRY)pBuf;
        pScsiLogPage->Temperature = pNvmeLogPage->Temperature -
                                    KELVIN_TEMP_FACTOR;

        /*
         * Do not need the extra buffer any longer since the Get Features
         * response will come in DWORD 0 of the completion queue entry. Issue
         * the DPC to free the data buffer memory in the SRB Extension.
         */
        storStatus = StorPortIssueDpc(
          pDevExt,
          &pDevExt->SntiDpc,
          (PVOID)pSrbExt,
          (PVOID)sizeof(ADMIN_GET_LOG_PAGE_SMART_HEALTH_INFORMATION_LOG_ENTRY));

        /* TBD: What do we do if we can't queue the DPC??? */
        if (storStatus != TRUE) {
            returnStatus = SNTI_SEQUENCE_ERROR;
            ASSERT(FALSE);
        } else {
            BOOLEAN ioStarted = FALSE;

            /* Issue the GET FEATURES command (Temperature Threshold Feature) */
            SntiBuildGetFeaturesCmd(pSrbExt, TEMPERATURE_THRESHOLD);

            /* Commmand sequence is still in progress */
            returnStatus = SNTI_SEQUENCE_IN_PROGRESS;
            pSrb->SrbStatus = SRB_STATUS_PENDING;

            /* Issue the command internally */
            ioStarted = ProcessIo(pDevExt, pSrbExt, NVME_QUEUE_TYPE_ADMIN, TRUE);
            if (ioStarted == FALSE)
                ASSERT(FALSE);
        }
    } else if (pSrbExt->nvmeSqeUnit.CDW0.OPC == ADMIN_GET_FEATURES) {
        /* Parse the Get Features DWORD 0 resposne */
        pScsiLogPage->ReferenceTemperature = pCQEntry->DW0 & WORD_LOW_BYTE_MASK;

        /* Commmand sequence is complete */
        returnStatus = SNTI_SEQUENCE_COMPLETED;
        pSrb->SrbStatus = SRB_STATUS_SUCCESS;
        pSrb->DataTransferLength = min(sizeof(TEMPERATURE_LOG_PAGE),
                                       allocLength);
    } else {
        returnStatus = SNTI_SEQUENCE_ERROR;
        pSrb->SrbStatus = SRB_STATUS_ERROR;
        pSrb->DataTransferLength = 0;

        ASSERT(FALSE);
    }

    return returnStatus;
} /* SntiTranslateTemperatureResponse */

/******************************************************************************
 * SntiTranslateModeSenseResponse
 *
 * @brief Translates the NVM Express command(s) response for the indicated SCSI
 *        Mode Sense command. Parses the appropriate SCSI Mode Sense page data
 *        based on the NVMe Translation spec.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param pCQEntry - Pointer to completion queue entry
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicate translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateModeSenseResponse(
    PSCSI_REQUEST_BLOCK pSrb,
    PNVMe_COMPLETION_QUEUE_ENTRY pCQEntry
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PNVME_LUN_EXTENSION pLunExt = NULL;
    UINT16 allocLength;
    UINT8 longLbaAccepted;
    UINT8 disableBlockDesc;
    UINT8 pageCode;
    BOOLEAN modeSense10;

    /* Default to successful command sequence completion */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_SEQUENCE_COMPLETED;
    SNTI_STATUS status = SNTI_SUCCESS;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    longLbaAccepted = GET_U8_FROM_CDB(pSrb, MODE_SENSE_CDB_LLBAA_OFFSET);
    disableBlockDesc = GET_U8_FROM_CDB(pSrb, MODE_SENSE_CDB_DBD_OFFSET);
    pageCode = GET_U8_FROM_CDB(pSrb, MODE_SENSE_CDB_PAGE_CODE_OFFSET);

    if (GET_OPCODE(pSrb) == SCSIOP_MODE_SENSE) {
        /* MODE SENSE 6 */
        allocLength =
            GET_U8_FROM_CDB(pSrb, MODE_SENSE_6_CDB_ALLOC_LENGTH_OFFSET);
        modeSense10 = FALSE;
    } else {
        /* MODE SENSE 10 */
        allocLength =
            GET_U16_FROM_CDB(pSrb, MODE_SENSE_10_CDB_ALLOC_LENGTH_OFFSET);
        modeSense10 = TRUE;
    }

    /* Mask and get LLBAA, DBD, PC, and Page code */
    longLbaAccepted = (longLbaAccepted & MODE_SENSE_CDB_LLBAA_MASK) >>
                       MODE_SENSE_CDB_LLBAA_SHIFT;
    disableBlockDesc = (disableBlockDesc & MODE_SENSE_CDB_DBD_MASK) >>
                        MODE_SENSE_CDB_DBD_SHIFT;
    pageCode &= MODE_SENSE_CDB_PAGE_CODE_MASK;

    status = GetLunExtension(pSrbExt, &pLunExt);
    if (status != SNTI_SUCCESS) {
        /* Map the translation error to a SCSI error */
        SntiMapInternalErrorStatus(pSrb, status);
        returnStatus = SNTI_SEQUENCE_ERROR;
    } else {
        switch (pageCode) {
            case MODE_PAGE_CACHING:
                SntiTranslateCachingModePageResponse(pSrbExt,
                                                     pLunExt,
                                                     pCQEntry,
                                                     allocLength,
                                                     longLbaAccepted,
                                                     disableBlockDesc,
                                                     modeSense10);
            break;
            case MODE_SENSE_RETURN_ALL:
                SntiTranslateReturnAllModePagesResponse(pSrbExt,
                                                        pCQEntry,
                                                        modeSense10);
            break;
            default:
                ASSERT(FALSE);
                SntiSetScsiSenseData(pSrb,
                                     SCSISTAT_CHECK_CONDITION,
                                     SCSI_SENSE_ILLEGAL_REQUEST,
                                     SCSI_ADSENSE_NO_SENSE,
                                     SCSI_ADSENSE_NO_SENSE);

                pSrb->SrbStatus |= SRB_STATUS_ERROR;
                pSrb->DataTransferLength = 0;
                returnStatus = SNTI_SEQUENCE_ERROR;
            break;
        } /* end switch */
    }

    return returnStatus;
} /* SntiTranslateModeSenseResponse */

/******************************************************************************
 * SntiTranslateCachingModePageResponse
 *
 * @brief Translates the NVM Express command(s) to support the Mode Sense page -
 *        Caching Mode Page. Populates the appropriate mode page response fields
 *        based on the NVMe Translation spec.
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param pSrbExt - Pointer to LUN extension
 * @param pCQEntry - Pointer to completion queue entry
 * @param allocLength - Allocation length from Mode Sense CDB
 * @param longLbaAccepted - Boolean for long LBA
 * @param disableBlockDesc - Boolean for DBD form Mose Sense CDB
 * @param modeSense10 - Boolean to determine if Mode Sense 10
 *
 * @return VOID
 ******************************************************************************/
VOID SntiTranslateCachingModePageResponse(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    PNVMe_COMPLETION_QUEUE_ENTRY pCQEntry,
    UINT16 allocLength,
    UINT8 longLbaAccepted,
    UINT8 disableBlockDesc,
    BOOLEAN modeSense10
)
{
    PMODE_PARAMETER_HEADER pModeHeader6 = NULL;
    PMODE_PARAMETER_HEADER10 pModeHeader10 = NULL;
    PMODE_PARAMETER_BLOCK pModeParamBlock = NULL;
    PCACHING_MODE_PAGE pCachingModePage = NULL;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    UINT32 volatileWriteCache = 0;
    UINT16 modeDataLength = 0;
    UINT16 blockDescLength = 0;

    /* The Volatile Write Cache info will be stored in DWORD 0 of the CQE */
    volatileWriteCache = pCQEntry->DW0;

    memset(pSrb->DataBuffer, 0, allocLength);

    /* Determine which Mode Parameter Descriptor Block to use (8 or 16) */
    if (longLbaAccepted == 0)
        blockDescLength = SHORT_DESC_BLOCK;
    else
        blockDescLength = LONG_DESC_BLOCK;

    /* Mode Page Header */
    SntiCreateModeDataHeader(pSrb,
                             &pModeParamBlock,
                             &modeDataLength,
                             (disableBlockDesc ? 0 : blockDescLength),
                             modeSense10);

    /* Check if block descriptors enabled, if not, then mode pages comes next */
    if (disableBlockDesc == BLOCK_DESCRIPTORS_ENABLED) {
        /* Mode Parameter Descriptor Block */
    SntiCreateModeParameterDescBlock(pLunExt,
                                     pModeParamBlock,
                                     &modeDataLength);

        /* Increment pointer to after block descriptor */
        pModeParamBlock++;
    }

    /* Caching Mode Page */
    pCachingModePage = (PCACHING_MODE_PAGE)pModeParamBlock;
    modeDataLength += sizeof(CACHING_MODE_PAGE);

    memset(pCachingModePage, 0, sizeof(CACHING_MODE_PAGE));
    pCachingModePage->PageCode    = MODE_PAGE_CACHING;
    pCachingModePage->PageSavable = MODE_PAGE_PARAM_SAVEABLE_DISABLED;
    pCachingModePage->PageLength  = CACHING_MODE_PAGE_LENGTH;
    pCachingModePage->WriteCacheEnable = volatileWriteCache &
                                         VOLATILE_WRITE_CACHE_MASK;

    /* Now go back and set the Mode Data Length in the header */
    if (modeSense10 == FALSE) {
        /* Get the correct header that starts at the buffer beginning */
        pModeHeader6 = (PMODE_PARAMETER_HEADER)(GET_DATA_BUFFER(pSrb));
        pModeHeader6->ModeDataLength = (UCHAR)(modeDataLength - 1);
    } else {
        /* Get the correct header that starts at the buffer beginning */
        pModeHeader10 = (PMODE_PARAMETER_HEADER10)(GET_DATA_BUFFER(pSrb));
        pModeHeader10->ModeDataLength[0] =
            ((modeDataLength - 2) & WORD_HIGH_BYTE_MASK) >> BYTE_SHIFT_1;
        pModeHeader10->ModeDataLength[1] =
            ((modeDataLength - 2) & WORD_LOW_BYTE_MASK);
    }

    pSrb->DataTransferLength = min(modeDataLength, allocLength);

    pSrbExt->pSrb->ScsiStatus = SCSISTAT_GOOD;
    pSrbExt->pSrb->SrbStatus = SRB_STATUS_SUCCESS;
} /* SntiTranslateCachingModePageResponse */

/******************************************************************************
 * SntiTranslateReturnAllModePagesResponse
 *
 * @brief Translates the final NVM Express command to complet the Mode Sense
 *        page request - Return All Mode Pages. Populates the appropriate mode
 *        page response fields based on the NVMe Translation spec.
 *
 * @param pSrbExt - Pointer to the SRB extension for this command
 * @param pCQEntry - Pointer to completion queue entry
 * @param modeSense10 - Boolean to determine if Mode Sense 10 request
 *
 * @return VOID
 ******************************************************************************/
VOID SntiTranslateReturnAllModePagesResponse(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVMe_COMPLETION_QUEUE_ENTRY pCQEntry,
    BOOLEAN modeSense10
)
{
    PUCHAR pBuffPtr = NULL;
    PMODE_CACHING_PAGE pCachingModePage = NULL;
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    UINT32 volatileWriteCache = 0;

    /* The Volatile Write Cache info will be stored in DWORD 0 of the CQE */
    volatileWriteCache = pCQEntry->DW0;

    /*
     * Find the offset to the caching mode page by using the data transfer
     * length field set in the build I/O phase and then backing up by the size
     * of the caching mode page.
     */
    pBuffPtr = (PUCHAR)GET_DATA_BUFFER(pSrb);
    pBuffPtr += pSrb->DataTransferLength;

    /* Subtract the size of the header */
    if (modeSense10 == FALSE)
        pBuffPtr -= sizeof(MODE_PARAMETER_HEADER);
    else
        pBuffPtr -= sizeof(MODE_PARAMETER_HEADER10);

    pCachingModePage = (PMODE_CACHING_PAGE)pBuffPtr;
    pCachingModePage--;

    pCachingModePage->WriteCacheEnable =
        volatileWriteCache & VOLATILE_WRITE_CACHE_MASK;

    pSrbExt->pSrb->ScsiStatus = SCSISTAT_GOOD;
    pSrbExt->pSrb->SrbStatus = SRB_STATUS_SUCCESS;
} /* SntiTranslateReturnAllModePagesResponse */

/******************************************************************************
 * SntiTranslateStartStopUnitResponse
 *
 * @brief Translates the appropriate NVM Express command response that will
 *        complete the SCSI START/STOP UNIT request.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicate translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateStartStopUnitResponse(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    UINT8 powerCondMod = 0;
    UINT8 powerCond = 0;
    UINT8 start = 0;

    /* Default to successful translation */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_SEQUENCE_IN_PROGRESS;

    /* NOTE: Parameter checking was done on submission side */

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    powerCondMod = GET_U8_FROM_CDB(pSrb,
                                   START_STOP_UNIT_CDB_POWER_COND_MOD_OFFSET);
    powerCond = GET_U8_FROM_CDB(pSrb, START_STOP_UNIT_CDB_POWER_COND_OFFSET);
    start = GET_U8_FROM_CDB(pSrb, START_STOP_UNIT_CDB_START_OFFSET);

    powerCondMod &= START_STOP_UNIT_CDB_POWER_COND_MOD_MASK;
    powerCond &= START_STOP_UNIT_CDB_POWER_COND_MASK;
    start &= START_STOP_UNIT_CDB_START_MASK;

    /* Now perform the expected power state transition */
    returnStatus = SntiTransitionPowerState(pSrbExt,
                                            powerCond,
                                            powerCondMod,
                                            start);

    if (returnStatus == SNTI_TRANSLATION_SUCCESS) {
        BOOLEAN ioStarted = FALSE;

        /*
         * If the power state transtion function is successful, then the
         * command sequence is still in process... make sure we don't complete
         * the START STOP UNIT command yet.
         *
         * Issue the Get/Set Features command internally.
         */
        ioStarted = ProcessIo(pSrbExt->pNvmeDevExt,
                              pSrbExt,
                              NVME_QUEUE_TYPE_ADMIN,
                              TRUE);

        if (ioStarted == FALSE)
            ASSERT(FALSE);

        pSrb->SrbStatus = SRB_STATUS_PENDING;
        returnStatus = SNTI_SEQUENCE_IN_PROGRESS;
    } else {
        /*
         * Otherwise, the command sequence has somehow failed...  reset the SRB
         * status to indicate an error (sense data will already be set).
         */
        pSrb->SrbStatus = 0;
        pSrb->SrbStatus |= SRB_STATUS_ERROR;
        pSrb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;
        returnStatus = SNTI_SEQUENCE_ERROR;
    }

    /*
     * Clear the completion callback routine so we don't get back here after
     * the power transition.
     */
    pSrbExt->pNvmeCompletionRoutine = NULL;

    return returnStatus;
} /* SntiTranslateStartStopUnitResponse */

/******************************************************************************
 * SntiTranslateWriteBufferResponse
 *
 * @brief Translates the appropriate NVM Express command for completing the
 *        SCSI Write Buffer requests.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return SNTI_TRANSLATION_STATUS
 *     Indicates translation status
 ******************************************************************************/
SNTI_TRANSLATION_STATUS SntiTranslateWriteBufferResponse(
    PSCSI_REQUEST_BLOCK pSrb
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PSTOR_SCATTER_GATHER_LIST pSgl = NULL;
    UINT32 bufferOffset = 0; /* 3 byte field */
    UINT32 paramListLength = 0; /* 3 byte field */
    UINT32 dword10 = 0;
    UINT32 dword11 = 0;
    UINT8 bufferId = 0;
    UINT8 mode = 0;

    /* Default to successful translation */
    SNTI_TRANSLATION_STATUS returnStatus = SNTI_SEQUENCE_IN_PROGRESS;

    pSrbExt = (PNVME_SRB_EXTENSION)GET_SRB_EXTENSION(pSrb);
    mode = GET_U8_FROM_CDB(pSrb, WRITE_BUFFER_CDB_MODE_OFFSET);

    switch (mode & WRITE_BUFFER_CDB_MODE_MASK) {
        case DOWNLOAD_SAVE_ACTIVATE:
            if (pSrbExt->nvmeSqeUnit.CDW0.OPC ==
                ADMIN_FIRMWARE_IMAGE_DOWNLOAD) {
                /* Activate microcode upon completion of FW Image Download */
                SntiBuildFirmwareActivateCmd(pSrbExt, dword10);

                returnStatus = SNTI_SEQUENCE_IN_PROGRESS;
                pSrb->SrbStatus = SRB_STATUS_PENDING;
            } else if (pSrbExt->nvmeSqeUnit.CDW0.OPC ==
                       ADMIN_FIRMWARE_ACTIVATE) {
                /* Command is complete */
                returnStatus = SNTI_SEQUENCE_COMPLETED;
                pSrb->SrbStatus = SRB_STATUS_SUCCESS;
            } else {
                returnStatus = SNTI_SEQUENCE_ERROR;
                pSrb->SrbStatus = SRB_STATUS_ERROR;
                ASSERT(FALSE);
            }
        break;
        case DOWNLOAD_SAVE_DEFER_ACTIVATE:
        case ACTIVATE_DEFERRED_MICROCODE:
            /* NVME completion status already checked, commands are complete */
            returnStatus = SNTI_SEQUENCE_COMPLETED;
            pSrb->SrbStatus = SRB_STATUS_SUCCESS;
        break;
        default:
            returnStatus = SNTI_SEQUENCE_ERROR;
            pSrb->SrbStatus = SRB_STATUS_ERROR;
        break;
    } /* end switch */

    return returnStatus;
} /* SntiTranslateWriteBufferResponse */

/******************************************************************************
 * SntiMapCompletionStatus
 *
 * @brief Entry function to perform the mapping of the NVM Express Command
 *        Status to a SCSI Status Code, sense key, and Additional Sense
 *        Code/Qualifier (ASC/ASCQ) where applicable.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 *
 * @return BOOLEAN
 *     Status to indicate if the status translation was successful.
 ******************************************************************************/
BOOLEAN SntiMapCompletionStatus(
    PNVME_SRB_EXTENSION pSrbExt
)
{
    PSCSI_REQUEST_BLOCK pSrb = pSrbExt->pSrb;
    UINT8 statusCodeType = (UINT8)pSrbExt->pCplEntry->DW3.SF.SCT;
    UINT8 statusCode = (UINT8)pSrbExt->pCplEntry->DW3.SF.SC;
    BOOLEAN returnValue = TRUE;

    if (pSrb != NULL) {
        switch(statusCodeType) {
            case GENERIC_COMMAND_STATUS:
                SntiMapGenericCommandStatus(pSrb, statusCode);
            break;
            case COMMAND_SPECIFIC_ERRORS:
                SntiMapCommandSpecificStatus(pSrb, statusCode);
            break;
            case MEDIA_ERRORS:
                SntiMapMediaErrors(pSrb, statusCode);
            break;
            default:
                returnValue = FALSE;
            break;
        }
    } else {
        returnValue = FALSE;
    }
    ASSERT(returnValue == TRUE);
    return returnValue;
} /* SntiMapCompletionStatus */

/******************************************************************************
 * SntiMapGenericCommandStatus
 *
 * @brief Maps the NVM Express Generic Command Status to a SCSI Status Code,
 *        sense key, and Additional Sense Code/Qualifier (ASC/ASCQ) where
 *        applicable.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param genericCommandStatus - NVMe Generic Command Status to translate
 *
 * @return VOID
 ******************************************************************************/
VOID SntiMapGenericCommandStatus(
    PSCSI_REQUEST_BLOCK pSrb,
    UINT8 genericCommandStatus
)
{
    PSENSE_DATA pSenseData = NULL;
    SNTI_RESPONSE_BLOCK responseData;
    BOOLEAN status = TRUE;

    memset(&responseData, 0, sizeof(SNTI_RESPONSE_BLOCK));

    /**
     * Perform table lookup for Generic Command Status translation
     *
     * Generic Status Code Values:
     *   0x00 - 0x0B and
     *   0x80 - 0x82 (0x0C - 0xE)
     *
     * Check bit 7 to see if this is a NVM Command Set status, if so then
     * start at 0xC to index into lookup table
     */
    if ((genericCommandStatus & NVM_CMD_SET_STATUS) != NVM_CMD_SET_STATUS)
        responseData = genericCommandStatusTable[genericCommandStatus];
    else
        responseData = genericCommandStatusTable[genericCommandStatus -
                       NVM_CMD_SET_GENERIC_STATUS_OFFSET];

    SntiSetScsiSenseData(pSrb,
                         responseData.StatusCode,
                         responseData.SenseKey,
                         responseData.ASC,
                         responseData.ASCQ);

    /* Override the SRB Status */
    pSrb->SrbStatus |= responseData.SrbStatus;
} /* SntiMapGenericCommandStatus */

/******************************************************************************
 * SntiMapCommandSpecificStatus
 *
 * @brief Maps the NVM Express Command Specific Status to a SCSI Status Code,
 *        sense key, and Additional Sense Code/Qualifier (ASC/ASCQ) where
 *        applicable.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param commandSpecificStatus - NVMe Command Specific Status to translate
 *
 * @return VOID
 ******************************************************************************/
VOID SntiMapCommandSpecificStatus(
    PSCSI_REQUEST_BLOCK pSrb,
    UINT8 commandSpecificStatus
)
{
    PSENSE_DATA pSenseData = NULL;
    SNTI_RESPONSE_BLOCK responseData;
    BOOLEAN status = TRUE;

    memset(&responseData, 0, sizeof(SNTI_RESPONSE_BLOCK));

    /**
     * Perform table lookup for Generic Command Status translation
     *
     * Command Specific Status Code Values:
     *   0x00 - 0x0A and
     *   0x80 (0x0B)
     *
     * Check bit 7 to see if this is a NVM Command Set status, if so then
     * start at 0xB to index into lookup table
     */
    if ((commandSpecificStatus & NVM_CMD_SET_STATUS) != NVM_CMD_SET_STATUS)
        responseData = commandSpecificStatusTable[commandSpecificStatus];
    else
        responseData = commandSpecificStatusTable[commandSpecificStatus -
                       NVM_CMD_SET_SPECIFIC_STATUS_OFFSET];

    SntiSetScsiSenseData(pSrb,
                         responseData.StatusCode,
                         responseData.SenseKey,
                         responseData.ASC,
                         responseData.ASCQ);

    /* Override the SRB Status */
    pSrb->SrbStatus |= responseData.SrbStatus;
} /* SntiMapCommandSpecificStatus */

/******************************************************************************
 * SntiMapMediaErrors
 *
 * @brief Maps the NVM Express Media Error Status to a SCSI Status Code, sense
 *        key, and Additional Sense Code/Qualifier (ASC/ASCQ) where applicable.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param mediaError - NVMe Media Error Status to translate
 *
 * @return VOID
 ******************************************************************************/
VOID SntiMapMediaErrors(
    PSCSI_REQUEST_BLOCK pSrb,
    UINT8 mediaError
)
{
    PSENSE_DATA pSenseData = NULL;
    SNTI_RESPONSE_BLOCK responseData;
    BOOLEAN status = TRUE;

    memset(&responseData, 0, sizeof(SNTI_RESPONSE_BLOCK));

    /*
     * Perform table lookup for Generic Command Status translation
     *
     * Media Error Status Code Values: 0x80 - 0x86
     */
    responseData = mediaErrorTable[mediaError - NVM_MEDIA_ERROR_STATUS_OFFSET];

    SntiSetScsiSenseData(pSrb,
                         responseData.StatusCode,
                         responseData.SenseKey,
                         responseData.ASC,
                         responseData.ASCQ);

    /* Override the SRB Status */
    pSrb->SrbStatus |= responseData.SrbStatus;
} /* SntiMapMediaErrors */

/******************************************************************************
 * SntiMapInternalErrorStatus
 *
 * @brief Maps an Internal Error Status to a SCSI Status Code, sense key, and
 *        Additional Sense Code/Qualifier (ASC/ASCQ) where applicable.
 *
 * @param pSrb - This parameter specifies the SCSI I/O request. SNTI expects
 *               that the user can access the SCSI CDB, response, and data from
 *               this pointer. For example, if there is a failure in translation
 *               resulting in sense data, then SNTI will call the appropriate
 *               internal error handling code and set the status info/data and
 *               pass the pSrb pointer as a parameter.
 * @param status - NVMe Error Status to translate
 *
 * @return VOID
 ******************************************************************************/
VOID SntiMapInternalErrorStatus(
    PSCSI_REQUEST_BLOCK pSrb,
    SNTI_STATUS status
)
{
    /* Switch on internal SNTI error */
    switch (status) {
        case SNTI_FAILURE:
            SntiSetScsiSenseData(pSrb,
                                 SCSISTAT_CHECK_CONDITION,
                                 SCSI_SENSE_UNIQUE,
                                 SCSI_ADSENSE_INTERNAL_TARGET_FAILURE,
                                 SCSI_ADSENSE_NO_SENSE);

            pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        break;
        case SNTI_INVALID_REQUEST:
        case SNTI_INVALID_PATH_TARGET_ID:
            SntiSetScsiSenseData(pSrb,
                                 SCSISTAT_CHECK_CONDITION,
                                 SCSI_SENSE_ILLEGAL_REQUEST,
                                 SCSI_ADSENSE_INVALID_CDB,
                                 SCSI_ADSENSE_NO_SENSE);

            pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        break;
        case SNTI_INVALID_PARAMETER:
            SntiSetScsiSenseData(pSrb,
                                 SCSISTAT_CHECK_CONDITION,
                                 SCSI_SENSE_ILLEGAL_REQUEST,
                                 SCSI_ADSENSE_INVALID_FIELD_PARAMETER_LIST,
                                 SCSI_ADSENSE_NO_SENSE);

            pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        break;
        case SNTI_NO_MEMORY:
            SntiSetScsiSenseData(pSrb,
                                 SCSISTAT_CHECK_CONDITION,
                                 SCSI_SENSE_ILLEGAL_REQUEST,
                                 SCSI_ADSENSE_INVALID_CDB,
                                 SCSI_ADSENSE_NO_SENSE);

            pSrb->SrbStatus |= SRB_STATUS_INVALID_REQUEST;
        break;
        default:
            ASSERT(FALSE);
        break;
    }
} /* SntiMapInternalErrorStatus */

/******************************************************************************
 * SntiDpcRoutine
 *
 * @brief SNTI DPC routine to feee memory that was allocated (since we cannot
 *        call the Storport API to free memory at DIRQL).
 *
 * @param pDpc - Pointer to SNTI DPC
 * @param pHwDeviceExtension - Pointer to device extension
 * @param pSystemArgument1 - Arg 1
 * @param pSystemArgument2 - Arg 2
 *
 * @return VOID
 ******************************************************************************/
VOID SntiDpcRoutine(
    IN PSTOR_DPC  pDpc,
    IN PVOID  pHwDeviceExtension,
    IN PVOID  pSystemArgument1,
    IN PVOID  pSystemArgument2
)
{
    PNVME_SRB_EXTENSION pSrbExt = NULL;
    PNVME_DEVICE_EXTENSION pDevExt = NULL;
    UINT32 bufferSize;
    ULONG status;

    pDevExt = (PNVME_DEVICE_EXTENSION)pHwDeviceExtension;
    pSrbExt = (PNVME_SRB_EXTENSION)pSystemArgument1;
    bufferSize = (UINT32)pSystemArgument2;

    status = StorPortFreeContiguousMemorySpecifyCache(pDevExt,
                                                      pSrbExt->pDataBuffer,
                                                      bufferSize,
                                                      MmCached);

    if (status != STOR_STATUS_SUCCESS) {
        ASSERT(FALSE);
    }
} /* SntiDpcRoutine */

/******************************************************************************
 * SntiAllocatePhysicallyContinguousBuffer
 *
 * @brief Allocates a physically contiguous data buffer for use w/ PRP lists.
 *
 * @param pSrbExt - Pointer to SRB extension
 * @param bufferSize - Size of buffer to allcoate
 *
 * @return BOOLEAN
 *     Indicates if the memory allocation was successful.
 ******************************************************************************/
PVOID SntiAllocatePhysicallyContinguousBuffer(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT32 bufferSize
)
{
    PHYSICAL_ADDRESS low;
    PHYSICAL_ADDRESS high;
    PHYSICAL_ADDRESS align;
    PVOID pBuffer = NULL;
    ULONG status = 0;

    /* Set up preferred range and alignment before allocating */
    low.QuadPart = 0;
    high.QuadPart = (-1);
    align.QuadPart = 0;

    status = StorPortAllocateContiguousMemorySpecifyCacheNode(
                 pSrbExt->pNvmeDevExt, bufferSize, low, high,
                 align, MmCached, 0, (PVOID)&pBuffer);

    StorPortDebugPrint(INFO,
                       "NVMeAllocateMem: Size=0x%x\n",
                       bufferSize );

    /* It fails, log the error and return 0 */
    if ((status != 0) || (pBuffer == NULL)) {
        StorPortDebugPrint(ERROR,
                           "NVMeAllocateMem:<Error> Failure, sts=0x%x\n",
                           status);

        return NULL;
    }

    /* Zero out the buffer before return */
    memset (pBuffer, 0, bufferSize);

    return pBuffer;
} /* SntiAllocatePhysicallyContinguousBuffer */
