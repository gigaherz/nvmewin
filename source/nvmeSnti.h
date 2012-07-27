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
 * File: nvmeSnti.h
 */

#ifndef __NVME_SNTI_H__
#define __NVME_SNTI_H__

#define VALID_NVME_PATH_ID   0
#define VALID_NVME_TARGET_ID 0
#define SNTI_STORPORT_QUEUE_DEPTH (254)
#define MODE_BLOCK_DESC_MAX (0xFFFFFF)
#define MODE_BLOCK_DESC_MAX_BYTE (0xFF)
#define NVME_MAX_NUM_BLOCKS_PER_READ_WRITE (0xFFFF)

/*******************************************************************************
 * VPD_EXTENDED_INQUIRY_DATA
 *
 * This structure defines Inquiry VPD page - VPD_EXTENDED_INQUIRY_DATA as is
 * defined in SPC-4 rev. 24.
 ******************************************************************************/
typedef struct _extended_inquiry_data
{
   UCHAR DeviceType                      : 5;
   UCHAR DeviceTypeQualifier             : 3;
   UCHAR PageCode;
   UCHAR Reserved1;
   UCHAR PageLength;
   UCHAR ActivateMicrocode               : 2;
   UCHAR SupportedProtectionType         : 3;
   UCHAR GuardCheck                      : 1;
   UCHAR ApplicationTagCheck             : 1;
   UCHAR ReferenceTagCheck               : 1; 
   UCHAR Reserved2                       : 2;
   UCHAR UACSKDataSupported              : 1;
   UCHAR GroupingFunctionSupported       : 1;
   UCHAR PrioritySupported               : 1; 
   UCHAR HeadOfQueueSupported            : 1;
   UCHAR OrderedSupported                : 1;
   UCHAR SimpleSupported                 : 1;
   UCHAR Reserved3                       : 4;
   UCHAR WriteUncorrectableSupported     : 1;
   UCHAR CorrectionDisableSupported      : 1;
   UCHAR NonVolatileCacheSupported       : 1;
   UCHAR VolatileCacheSupported          : 1;
   UCHAR Reserved4                       : 3;
   UCHAR ProtectionInfoIntervalSupported : 1; 
   UCHAR Reserved5                       : 3;
   UCHAR LogicalUnitITNexusClear         : 1;
   UCHAR Reserved6                       : 3;
   UCHAR ReferralsSupported              : 1;
   UCHAR Reserved7                       : 3;
   UCHAR CapabilityBasedCommandSecurity  : 1;
   UCHAR Reserved8                       : 4;
   UCHAR MultiITNexusMicrodeDownload     : 4;
   UCHAR Reserved9[54];
} EXTENDED_INQUIRY_DATA, *PEXTENDED_INQUIRY_DATA;

/*******************************************************************************
 * READ_CAPACITY_16_DATA
 *
 * This structure defines READ CAP 16 Data Page as is defined in SPC-4 rev. 24.
 ******************************************************************************/
typedef struct _read_capacity_16_data
{
    UINT64 LogicalBlockAddress;
    UINT32 BytesPerBlock;
    UINT8  Reserved1                             : 4;
    UINT8  ProtectionType                        : 3;
    UINT8  ProtectionEnable                      : 1;
    UINT8  ProtectionInfoIntervals               : 4;
    UINT8  LogicalBlocksPerPhysicalBlockExponent : 4;
    UINT8  LogicalBlockProvisioningMgmtEnabled   : 1;
    UINT8  LogicalBlockProvisioningReadZeros     : 1;
    UINT8  LowestAlignedLbaMsb                   : 6;
    UINT8  LowestAlignedLbaLsb;
    UINT8  Reserved2[16];
} READ_CAPACITY_16_DATA, *PREAD_CAPACITY_16_DATA;

/*******************************************************************************
 * DESCRIPTOR_FORMAT_SENSE_DATA
 *
 * This structure defines Descriptor Formate Sense Data as defined in SPC-4
 * rev. 24.
 ******************************************************************************/
typedef struct _descriptor_format_sense_data
{
    UINT8 ResponseCode                  : 7;
    UINT8 Reserved1                     : 1;
    UINT8 SenseKey                      : 4;
    UINT8 Reserved2                     : 4;
    UINT8 AdditionalSenseCode;
    UINT8 AdditionalSenseCodeQualifier;
    UINT8 Reserved3[3];
    UINT8 AdditionalSenseLength;

    /* Determine what Descriptor Types will be needed, if any */
    PUCHAR SenseDataDescriptor[1];
} DESCRIPTOR_FORMAT_SENSE_DATA, *PDESCRIPTOR_FORMAT_SENSE_DATA;

/*******************************************************************************
 * SUPPORTED_PAGE_DESCRIPTOR
 ******************************************************************************/
typedef struct _supported_page_descriptor
{
    UINT8 PageCode : 6;
    UINT8 Reserved : 2;
} SUPPORTED_PAGE_DESCRIPTOR, *PSUPPORTED_PAGE_DESCRIPTOR;

/*******************************************************************************
 * SUPPORTED_LOG_PAGES_LOG_PAGE
 ******************************************************************************/
typedef struct _supported_log_pages_log_page
{
    UINT8  PageCode      : 6;
    UINT8  SubPageFormat : 1;
    UINT8  DisableSave   : 1;
    UINT8  SubPageCode;
    UINT8 PageLength[2];
    SUPPORTED_PAGE_DESCRIPTOR supportedPages[3];
} SUPPORTED_LOG_PAGES_LOG_PAGE, *PSUPPORTED_LOG_PAGES_LOG_PAGE;

/*******************************************************************************
 * INFORMATIONAL_EXCEPTIONS_LOG_PAGE
 ******************************************************************************/
typedef struct _informational_exceptions_log_page
{
    UINT8  PageCode         : 6;
    UINT8  SubPageFormat    : 1;
    UINT8  DisableSave      : 1;
    UINT8  SubPageCode;
    UINT8 PageLength[2];
    UINT8 ParameterCode[2];
    UINT8  FormatAndLinking : 2;
    UINT8  TMC              : 2;
    UINT8  ETC              : 1;
    UINT8  TSD              : 1;
    UINT8  Reserved1        : 1;
    UINT8  DU               : 1;
    UINT8  ParameterLength;
    UINT8  InfoExcpAsc;
    UINT8  InfoExcpAscq;
    UINT8  MostRecentTempReading; /* Data from SMART/Health NVMe Log Page */
    UINT8  Reserved2;
} INFORMATIONAL_EXCEPTIONS_LOG_PAGE, *PINFORMATIONAL_EXCEPTIONS_LOG_PAGE;

/*******************************************************************************
 * TEMPERATURE_LOG_PAGE
 ******************************************************************************/
typedef struct _temperature_log_page
{
    UINT8  PageCode                 : 6;
    UINT8  SubPageFormat            : 1;
    UINT8  DisableSave              : 1;
    UINT8  SubPageCode;
    UINT8 PageLength[2];
    UINT8 ParameterCode_Temp[2];
    UINT8  FormatAndLinking_Temp    : 2;
    UINT8  TMC_Temp                 : 2;
    UINT8  ETC_Temp                 : 1;
    UINT8  TSD_Temp                 : 1;
    UINT8  Reserved1_Temp           : 1;
    UINT8  DU_Temp                  : 1;
    UINT8  ParameterLength_Temp;
    UINT8  Reserved2_Temp; 
    UINT8  Temperature;
    UINT8 ParameterCode_RefTemp[2];
    UINT8  FormatAndLinking_RefTemp : 2;
    UINT8  TMC_RefTemp              : 2;
    UINT8  ETC_RefTemp              : 1;
    UINT8  TSD_RefTemp              : 1;
    UINT8  Reserved1_RefTemp        : 1;
    UINT8  DU_RefTemp               : 1;
    UINT8  ParameterLength_RefTemp;
    UINT8  Reserved2_RefTemp;
    UINT8  ReferenceTemperature; /* Data from SMART/Health NVMe Log Page */
} TEMPERATURE_LOG_PAGE, *PTEMPERATURE_LOG_PAGE;

/*******************************************************************************
 * SMART_HEALTH_INFO_LOG
 ******************************************************************************/
typedef struct _smart_health_info_log
{

    UINT8  CriticalWarning;            /* Bytes:       0 */
    UINT16 Temperature;                /* Bytes:     1:2 */
    UINT8  AvailableSpare;             /* Bytes:       3 */
    UINT8  AvailableSpareThreshold;    /* Bytes:       4 */
    UINT8  PercentageUsed;             /* Bytes:       5 */
    UINT8  Reserved1[26];              /* Bytes:    6:31 */
    UINT8  DataUnitsRead[16];          /* Bytes:   32:47 */
    UINT8  DataUnitsWritten[16];       /* Bytes:   48:63 */
    UINT8  HostReadCommands[16];       /* Bytes:   64:79 */
    UINT8  HostWriteCommands[16];      /* Bytes:   80:95 */
    UINT8  ControllerBusyTime[16];     /* Bytes:  96:111 */
    UINT8  PowerCycles[16];            /* Bytes: 112:127 */
    UINT8  PowerOnHours[16];           /* Bytes: 128:143 */
    UINT8  UnsafeShutdowns[16];        /* Bytes: 144:159 */
    UINT8  MediaErrors[16];            /* Bytes: 160:175 */
    UINT8  NumErrorInfoLogEntries[16]; /* Bytes: 176:191 */
    UINT8  Reserved2[321];             /* Bytes: 192:512 */
} SMART_HEALTH_INFO_LOG, *PSMART_HEALTH_INFO_LOG;

/*******************************************************************************
 * CACHING_MODE_PAGE - 0x08
 ******************************************************************************/
typedef struct _caching_mode_page
{
    UINT8 PageCode                    : 6; /* 0x08 */
    UINT8 Reserved1                   : 1;
    UINT8 PageSavable                 : 1;
    UINT8 PageLength;                      /* 0x12 */
    UINT8 ReadDisableCache            : 1;
    UINT8 MultiplicationFactor        : 1;
    UINT8 WriteCacheEnable            : 1;
    UINT8 Reserved2                   : 5;
    UINT8 WriteRetensionPriority      : 4;
    UINT8 ReadRetensionPriority       : 4;
    UINT8 DisablePrefetchTransfer[2];
    UINT8 MinimumPrefetch[2];
    UINT8 MaximumPrefetch[2];
    UINT8 MaximumPrefetchCeiling[2];
    UINT8 NV_DIS                      : 1;
    UINT8 Reserved3                   : 2;
    UINT8 VendorSpecific              : 2;
    UINT8 DRA                         : 1;
    UINT8 LBCSS                       : 1;
    UINT8 FSW                         : 1;
    UINT8 NumberOfCacheSegments;
    UINT8 CacheSegmentSize[2];
    UINT8 Reserved4[4];
} CACHING_MODE_PAGE, *PCACHING_MODE_PAGE;

/*******************************************************************************
 * CONTROL_MODE_PAGE - 0x0A
 ******************************************************************************/
typedef struct _control_mode_page
{
    UINT8 PageCode       : 6; /* 0x0A */
    UINT8 SubPageFormat  : 1;
    UINT8 PageSaveable   : 1;
    UINT8 PageLength;         /* 0x0A */
    UINT8 RLEC           : 1;
    UINT8 GLTSD          : 1;
    UINT8 D_Sense        : 1;
    UINT8 DPICZ          : 1;
    UINT8 TMF_Only       : 1;
    UINT8 TST            : 3;
    UINT8 Reserved1      : 1;
    UINT8 QERR           : 2;
    UINT8 NUAR           : 1;
    UINT8 QAlgMod        : 4;
    UINT8 Reserved2      : 3;
    UINT8 SWP            : 1;
    UINT8 UA_INTLCK_CTRL : 2;
    UINT8 RAC            : 1;
    UINT8 VS             : 1;
    UINT8 AutoLodeMode   : 3;
    UINT8 Reserved3      : 3;
    UINT8 TAS            : 1;
    UINT8 ATO            : 1;
    UINT8 Reserved4[2];
    UINT8 BusyTimeoutPeriod[2];
    UINT8 ExtSelfTestCompTime[2];
} CONTROL_MODE_PAGE, *PCONTROL_MODE_PAGE;

/*******************************************************************************
 * POWER_CONDITION_MODE_PAGE - 0x1A
 ******************************************************************************/
typedef struct _power_condition_mode_page
{
    UINT8 PageCode          : 6; /* 0x1A */
    UINT8 SubPageFormat     : 1;
    UINT8 PageSaveable      : 1;
    UINT8 PageLength;            /* 0x26 */
    UINT8 Standby_Y         : 1;
    UINT8 Reserved1         : 5;
    UINT8 PmBgInteraction   : 2;
    UINT8 Standby_Z         : 1;
    UINT8 Idle_A            : 1;
    UINT8 Idle_B            : 1;
    UINT8 Idle_C            : 1;
    UINT8 Reserved2         : 4;
    UINT8 Idle_A_ConditionTimer[4];
    UINT8 Standby_Z_ConditionTimer[4];
    UINT8 Idle_B_ConditionTimer[4];
    UINT8 Idle_C_ConditionTimer[4];
    UINT8 Standby_Y_ConditionTimer[4];
    UINT8 Reserved3[16];
} POWER_CONDITION_MODE_PAGE, *PPOWER_CONDITION_MODE_PAGE;

/*******************************************************************************
 * INFO_EXCEPTIONS_MODE_PAGE - 0x1C
 ******************************************************************************/
typedef struct _info_exceptions_mode_page
{
    UINT8 PageCode      : 6; /* 0x1C */
    UINT8 SubPageFormat : 1;
    UINT8 PageSaveable  : 1;
    UINT8 PageLength;        /* 0x0A */
    UINT8 LogErr        : 1;
    UINT8 EbackErr      : 1;
    UINT8 Test          : 1;
    UINT8 DExcpt        : 1;
    UINT8 EWasc         : 1;
    UINT8 Ebf           : 1;
    UINT8 Reserved1     : 1;
    UINT8 Perf          : 1;
    UINT8 Mrie          : 4;
    UINT8 Reserved2     : 4;
    UINT8 IntervalTimer[4];
    UINT8 ReportCount[4];
} INFO_EXCEPTIONS_MODE_PAGE, *PINFO_EXCEPTIONS_MODE_PAGE;

/*******************************************************************************
 * SNTI_TRANSLATION_STATUS
 *
 * The SNTI_TRANSLATION_STATUS enumeration defines all possible status codes
 * for a translation sequence. 
 ******************************************************************************/
typedef enum _snti_translation_status
{
    SNTI_TRANSLATION_SUCCESS = 0,     /* Translation occurred w/o error */
    SNTI_COMMAND_COMPLETED,           /* Command completed in xlation phase */
    SNTI_SEQUENCE_IN_PROGRESS,        /* Command sequence still in progress */
    SNTI_SEQUENCE_COMPLETED,          /* Command sequence completed */
    SNTI_SEQUENCE_ERROR,              /* Error in command sequence */
    SNTI_FAILURE_CHECK_RESPONSE_DATA, /* Check SCSI status, device error */
    SNTI_UNSUPPORTED_SCSI_REQUEST,    /* Unsupported SCSI opcode */
    SNTI_UNSUPPORTED_SCSI_TM_REQUEST, /* Unsupported SCSI TM opcode */
    SNTI_INVALID_SCSI_REQUEST_PARM,   /* An invalid SCSI request parameter */
    SNTI_INVALID_SCSI_PATH_ID,        /* An invalid SCSI path id */
    SNTI_INVALID_SCSI_TARGET_ID,      /* An invalid SCSI target id */
    SNTI_UNRECOVERABLE_ERROR,         /* Unrecoverable error */
    SNTI_RESERVED                     /* Reserved for future use */
     
    /* TBD: Add additional codes as necessary */

} SNTI_TRANSLATION_STATUS;

/*******************************************************************************
 * SNTI_STATUS
 *
 * The SNTI_STATUS enumeration defines all possible "internal" status codes
 * for a translation sequence.
 ******************************************************************************/
typedef enum _snti_status
{
    SNTI_SUCCESS = 0,
    SNTI_FAILURE,
    SNTI_INVALID_REQUEST,
    SNTI_INVALID_PARAMETER,
    SNTI_INVALID_PATH_TARGET_ID,
    SNTI_NO_MEMORY

    /* TBD: Add fields as necessary */

} SNTI_STATUS;

/******************************************************************************
 * SNTI_RESPONSE_BLOCK
 ******************************************************************************/
typedef struct _snti_response_block
{
    UINT8 SrbStatus;
    UINT8 StatusCode;
    UINT8 SenseKey;
    UINT8 ASC;
    UINT8 ASCQ;
} SNTI_RESPONSE_BLOCK, *PSNTI_RESPONSE_BLOCK;

/***  Public Interfaces  ***/

SNTI_TRANSLATION_STATUS SntiTranslateCommand(
    PNVME_DEVICE_EXTENSION pAdapterExtension,
    PSCSI_REQUEST_BLOCK pSrb
);

BOOLEAN SntiCompletionCallbackRoutine(
    PVOID param1,
    PVOID param2
);

BOOLEAN SntiMapCompletionStatus(
    PNVME_SRB_EXTENSION pSrbExt
);

/*** Private Interfaces ***/

SNTI_TRANSLATION_STATUS SntiTranslateInquiry(
    PSCSI_REQUEST_BLOCK pSrb
);

VOID SntiTranslateSupportedVpdPages(
    PSCSI_REQUEST_BLOCK pSrb
);

VOID SntiTranslateUnitSerialPage(
    PSCSI_REQUEST_BLOCK pSrb
);

VOID SntiTranslateDeviceIdentificationPage(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTranslateExtendedInquiryDataPage(
    PSCSI_REQUEST_BLOCK pSrb
);

VOID SntiTranslateStandardInquiryPage(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTranslateReportLuns(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTranslateReadCapacity(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTranslateReadCapacity10(
    PSCSI_REQUEST_BLOCK pSrb,
    PUCHAR pResponseBuffer,
    PNVME_LUN_EXTENSION pLunExtension
);

SNTI_TRANSLATION_STATUS SntiTranslateReadCapacity16(
    PSCSI_REQUEST_BLOCK pSrb,
    PUCHAR pResponseBuffer,
    PNVME_LUN_EXTENSION pLunExtension
);

SNTI_TRANSLATION_STATUS SntiTranslateWrite(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_STATUS SntiTranslateWrite6(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
);

SNTI_STATUS SntiTranslateWrite10(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
);

SNTI_STATUS SntiTranslateWrite12(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
);

SNTI_STATUS SntiTranslateWrite16(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
);

SNTI_TRANSLATION_STATUS SntiTranslateRead(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_STATUS SntiTranslateRead6(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
);

SNTI_STATUS SntiTranslateRead10(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
);

SNTI_STATUS SntiTranslateRead12(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
);

SNTI_STATUS SntiTranslateRead16(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt
);

SNTI_TRANSLATION_STATUS SntiTranslateRequestSense(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTranslateSecurityProtocol(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTranslateStartStopUnit(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTransitionPowerState(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT8 powerCond,
    UINT8 powerCondMod,
    UINT8 start
);

SNTI_TRANSLATION_STATUS SntiTranslateWriteBuffer(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTranslateSynchronizeCache(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTranslateTestUnitReady(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTranslateFormatUnit(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTranslateLogSense(
    PSCSI_REQUEST_BLOCK pSrb
);

VOID SntiTranslateSupportedLogPages(
    PSCSI_REQUEST_BLOCK pSrb
);

VOID SntiTranslateInformationalExceptions(
    PSCSI_REQUEST_BLOCK pSrb
);

VOID SntiTranslateTemperature(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTranslateModeSense(
    PSCSI_REQUEST_BLOCK pSrb,
    BOOLEAN supportsVwc
);

VOID SntiCreateControlModePage(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT16 allocLength,
    UINT8 longLbaAccepted,
    UINT8 disableBlockDesc,
    BOOLEAN modeSense10
);

VOID SntiHardCodeCacheModePage(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT16 allocLength,
    UINT8 longLbaAccepted,
    UINT8 disableBlockDesc,
    BOOLEAN modeSense10
);

VOID SntiCreatePowerConditionControlModePage(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT16 allocLength,
    UINT8 longLbaAccepted,
    UINT8 disableBlockDesc,
    BOOLEAN modeSense10
);

VOID SntiCreateInformationalExceptionsControlModePage(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT16 allocLength,
    UINT8 longLbaAccepted,
    UINT8 disableBlockDesc,
    BOOLEAN modeSense10
);

VOID SntiReturnAllModePages(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT16 allocLength,
    UINT8 longLbaAccepted,
    UINT8 disableBlockDesc,
    BOOLEAN modeSense10,
    BOOLEAN supportsVwc
);

SNTI_TRANSLATION_STATUS SntiTranslateModeSelect(
    PSCSI_REQUEST_BLOCK pSrb,
    BOOLEAN supportsVwc
);

SNTI_TRANSLATION_STATUS SntiTranslateModeData(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT16 paramListLength,
    BOOLEAN isModeSelect10,
    BOOLEAN supportsVwc
);

VOID SntiCreateModeDataHeader(
    PSCSI_REQUEST_BLOCK pSrb,
    PMODE_PARAMETER_BLOCK *ppModeParamBlock,
    PUINT16 pModeDataLength,
    UINT16 blockDescLength,
    BOOLEAN modeSense10
);

VOID SntiCreateModeParameterDescBlock(
    PNVME_LUN_EXTENSION pLunExt,
    PMODE_PARAMETER_BLOCK pModeParamBlock,
    PUINT16 pModeDataLength
);

VOID SntiTranslateSglToPrp(
    PNVME_SRB_EXTENSION pSrbExt,
    PSTOR_SCATTER_GATHER_LIST pSgl
);

SNTI_STATUS SntiValidateLbaAndLength(
    PNVME_LUN_EXTENSION pLunExtension,
    PNVME_SRB_EXTENSION pSrbExtension,
    UINT64 lba,
    UINT32 length
);

BOOLEAN SntiSetScsiSenseData(
    PSCSI_REQUEST_BLOCK pSrb,
    UCHAR scsiStatus,
    UCHAR senseKey,
    UCHAR asc,
    UCHAR ascq 
);

VOID SntiMapGenericCommandStatus(
    PSCSI_REQUEST_BLOCK pSrb,
    UINT8 genericCommandStatus
);

VOID SntiMapCommandSpecificStatus(
    PSCSI_REQUEST_BLOCK pSrb,
    UINT8 commandSpecificStatus
);

VOID SntiMapMediaErrors(
    PSCSI_REQUEST_BLOCK pSrb,
    UINT8 mediaError 
);

SNTI_STATUS GetLunExtension(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION *ppLunExt
);

VOID SntiBuildGetFeaturesCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT8 featureIdentifier
);

VOID SntiBuildSetFeaturesCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT8 featureIdentifier,
    UINT32 dword11
);

VOID SntiBuildGetLogPageCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT8 logIdentifier
);

VOID SntiBuildFirmwareImageDownloadCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT32 dword10,
    UINT32 dword11
);

VOID SntiBuildFirmwareActivateCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT32 dword10
);

VOID SntiBuildFlushCmd(
    PNVME_SRB_EXTENSION pSrbExt
);

VOID SntiBuildFormatNvmCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT8 protectionType
);

VOID SntiBuildSecuritySendReceiveCmd(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    UINT8 opcode,
    UINT32 transferLength,
    UINT16 secProtocolSp,
    UINT8 secProtocol
);

VOID SntiMapInternalErrorStatus(
    PSCSI_REQUEST_BLOCK pSrb,
    SNTI_STATUS status
);

SNTI_TRANSLATION_STATUS SntiTranslateLogSenseResponse(
    PSCSI_REQUEST_BLOCK pSrb,
    PNVMe_COMPLETION_QUEUE_ENTRY pCQEntry
);

SNTI_TRANSLATION_STATUS SntiTranslateInformationalExceptionsResponse(
    PSCSI_REQUEST_BLOCK pSrb,
    UINT16 allocLength
);

SNTI_STATUS SntiTranslateTemperatureResponse(
    PSCSI_REQUEST_BLOCK pSrb,
    PNVMe_COMPLETION_QUEUE_ENTRY pCQEntry,
    UINT16 allocLength
);

SNTI_TRANSLATION_STATUS SntiTranslateModeSenseResponse(
    PSCSI_REQUEST_BLOCK pSrb,
    PNVMe_COMPLETION_QUEUE_ENTRY pCQEntry
);

VOID SntiTranslateCachingModePageResponse(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVME_LUN_EXTENSION pLunExt,
    PNVMe_COMPLETION_QUEUE_ENTRY pCQEntry,
    UINT16 allocLength,
    UINT8 longLbaAccepted,
    UINT8 disableBlockDesc,
    BOOLEAN modeSense10
);

VOID SntiTranslateReturnAllModePagesResponse(
    PNVME_SRB_EXTENSION pSrbExt,
    PNVMe_COMPLETION_QUEUE_ENTRY pCQEntry,
    BOOLEAN modeSense10
);

SNTI_TRANSLATION_STATUS SntiTranslateStartStopUnitResponse(
    PSCSI_REQUEST_BLOCK pSrb
);

SNTI_TRANSLATION_STATUS SntiTranslateWriteBufferResponse(
    PSCSI_REQUEST_BLOCK pSrb
);

VOID SntiDpcRoutine(
    IN PSTOR_DPC  pDpc,
    IN PVOID  pHwDeviceExtension,
    IN PVOID  pSystemArgument1,
    IN PVOID  pSystemArgument2
);

PVOID SntiAllocatePhysicallyContinguousBuffer(
    PNVME_SRB_EXTENSION pSrbExt,
    UINT32 bufferSize
);

#endif /* __NVME_SNTI_H__ */
