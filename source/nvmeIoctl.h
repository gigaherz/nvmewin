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

/**
 * File: nvmeIoctl.h 
 */ 

#ifndef __NVME_IOCTL_H__
#define __NVME_IOCTl_H__

#define NVME_STORPORT_DRIVER 0xE000

#define NVME_PASS_THROUGH_SRB_IO_CODE \
    CTL_CODE(NVME_STORPORT_DRIVER, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NVME_GET_NAMESPACE_ID \
    CTL_CODE(NVME_STORPORT_DRIVER, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NVME_HOT_ADD_NAMESPACE \
    CTL_CODE(NVME_STORPORT_DRIVER, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NVME_HOT_REMOVE_NAMESPACE \
    CTL_CODE(NVME_STORPORT_DRIVER, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define NVME_SIG_STR          "NvmeMini"
#define NVME_SIG_STR_LEN      8
#define NVME_NO_DATA_TX       0 /* No data transfer involved */
#define NVME_FROM_HOST_TO_DEV 1 /* Transfer data from host to device */
#define NVME_FROM_DEV_TO_HOST 2 /* Transfer data from device to host */
#define NVME_BI_DIRECTION     3 /* Tx data from host to device and back */

#define NVME_IOCTL_VENDOR_SPECIFIC_DW_SIZE 6  /* Vendor sp qualifier (DWORDs) */
#define NVME_IOCTL_CMD_DW_SIZE             16 /* NVMe cmd entry size (DWORDs) */
#define NVME_IOCTL_COMPLETE_DW_SIZE        4  /* NVMe cpl entry size (DWORDs) */

/*******************************************************************************
 * NVMe Pass Through IOCTL return codes from Miniport driver.
 *
 * These numbers are returned in ReturnCode of SRB_IO_CONTROL structure. 
 * When driver receives an IOCTL request, before issuing the associated NVMe
 * command, it examines all fields of NVME_PASS_THROUGH_IOCTL structure. 
 *
 * Once certain error found, the error is interpreted and noted in ReturnCode 
 * of SRB_IO_CONTROL structure. User applications can find out the specific
 * status after driver processes the request.
 ******************************************************************************/
enum _IOCTL_STATUS
{
     NVME_IOCTL_SUCCESS,
     NVME_IOCTL_INVALID_IOCTL_CODE,                     
     NVME_IOCTL_INVALID_SIGNATURE,                      
     NVME_IOCTL_INSUFFICIENT_IN_BUFFER,                 
     NVME_IOCTL_INSUFFICIENT_OUT_BUFFER,                 
     NVME_IOCTL_UNSUPPORTED_ADMIN_CMD,                  
     NVME_IOCTL_UNSUPPORTED_NVM_CMD,
     NVME_IOCTL_UNSUPPORTED_OPERATION,
     NVME_IOCTL_INVALID_ADMIN_VENDOR_SPECIFIC_OPCODE,   
     NVME_IOCTL_INVALID_NVM_VENDOR_SPECIFIC_OPCODE,    
     NVME_IOCTL_ADMIN_VENDOR_SPECIFIC_NOT_SUPPORTED,  // i.e., AVSCC = 0
     NVME_IOCTL_NVM_VENDOR_SPECIFIC_NOT_SUPPORTED,    // i.e., NVSCC = 0
     NVME_IOCTL_INVALID_DIRECTION_SPECIFIED,          // Direction > 3
     NVME_IOCTL_INVALID_META_BUFFER_LENGTH,
     NVME_IOCTL_PRP_TRANSLATION_ERROR,
     NVME_IOCTL_INVALID_PATH_TARGET_ID,
     NVME_IOCTL_FORMAT_NVM_PENDING,      // Only one Format NVM at a time
     NVME_IOCTL_FORMAT_NVM_FAILED,
     NVME_IOCTL_INVALID_NAMESPACE_ID
};

#pragma pack(1)
/******************************************************************************
 * NVMe Pass Through IOCTL data structure.
 *
 * This structure contains WDK defined SRB_IO_CONTROL structure, 64-byte 
 * NVMe command entry and 16-byte completion entry, and other important fields
 * that driver needs to reference when processing the requests.
 *
 * User applications need to allocate proper size of buffer(s) and populate the
 * fields to ensure the requests are being processed correctly after issuing.
 ******************************************************************************/
typedef struct _NVME_PASS_THROUGH_IOCTL
{
    /* WDK defined SRB_IO_CONTROL structure */
    SRB_IO_CONTROL SrbIoCtrl;

    /* Vendor unique qualifiers for vendor unique commands */
    ULONG          VendorSpecific[NVME_IOCTL_VENDOR_SPECIFIC_DW_SIZE];

    /* 64-byte submission entry defined in NVMe Specification */
    ULONG          NVMeCmd[NVME_IOCTL_CMD_DW_SIZE];

    /* DW[0..3] of completion entry */
    ULONG          CplEntry[NVME_IOCTL_COMPLETE_DW_SIZE];

    /* Data transfer direction, from host to device or vice versa */
    ULONG          Direction; 

    /* 0 means using Admin queue, otherwise, IO queue is used */
    ULONG          QueueId;

    /* Transfer byte length, including Metadata, starting at DataBuffer */
    ULONG          DataBufferLen;

    /* Set to 0 if not supported or interleaved with data  */
    ULONG          MetaDataLen; 

    /*
     * Returned byte length from device to host, at least the length of this 
     * structure. When data transfer required, add the length of the data. 
     */
    ULONG          ReturnBufferLen;

    /* Start with Metadata if present, and then regular data */
    UCHAR          DataBuffer[1]; 
} NVME_PASS_THROUGH_IOCTL, *PNVME_PASS_THROUGH_IOCTL;
#pragma pack() 

#endif // __NVME_IOCTL_H__