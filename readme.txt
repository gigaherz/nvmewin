NVMe Windows Driver Community Release, December 2016

Description:
This is the release as revision 1.5 from NVMe Windows Driver Community. The release
includes installation package and the associated source code. In order to install 
on the supported 64-bit Operating Systems, the 64-bit installation packages had been
test signed via Visual Studio 2015. You may find more details below.

Revision: 
1.5.0.0

Compliant NVMe Specification: 
1.2

SVN source code revision number: 
139

Contents:
- readme.txt
- Visual Studio 2013 solution/project files (under VS2013_sln_proj directory) 
- installation packages (under installations directory):
  - 64-bit\Windows7 (for Windows 7 and Server 2008 R2)
  - 64-bit\Windows8 (for Windows 8 and Server 2012)
  - 64-bit\Windows81 (for Windows 8.1 and Server 2012 R2)
  - 32-bit\Windows7 (for Windows 7, 32-bit)
  - 32-bit\Windows8 (for Windows 8, 32-bit)
  - 32-bit\Windows81 (for Windows 8.1, 32-bit)
- source code (under source directory)

Prerequisites:
- This driver supports the following 64/32 bit Windows Operating Systems:
  Windows Server 2008 R2, 2012, 2012 R2
  Windows 7, 8, 8.1, 10.
- The target system to install the driver on has to enable test signing:
  bcdedit.exe -set TESTSIGNING ON
- The target system to load the driver on has to disable integrity check:
  bcedit.exe -set loadoptions DDISABLE_INTEGRITY_CHECKS
- The Class Codes of PCI Header of the target device need to be compliant with:
  Base Class Code: Mass Storage Controller (01h)
  Sub-Class Code: Non-Volatile Memory Controller (08h)
  Programming Interface: NVM Express (02h)

Driver building:
- To open nvme solution within Visual Studio 2013 build environment, you may simply
  double-click the released solution file(nvme.sln) under VS2013_sln_proj directory
  if you have Visual Studio 2013 installed properly.
- After nvme solution is built successfully, the resulted 64-bit binary is saved 
  under x64 directory and 32-bit binary in VS2013_sln_proj\Win7(8/8.1)Release(Debug).
- If you need check build driver, after opening the solution file within Visual
  Studio 2013, right click the solution and select "Properties" to bring up
  Configuration Manager and specify the desired configuration/platform.

Change Logs:
Revision 1.5 added:
- Namespace Management (Create, Delete, Attach, Detach)
- EOL Read Only Support
- Win 8.1 Timers
- Surprise Removal Support in IOCTL Path
- Disk Initialization Performance Optimization
- Storage Request Block Support
- StorPort Performance Options
- StorPort DPC Redirection
- Security Send/Receive with Zero Data Length
- SNTI updates for SCSI to NVMe Translation
- Misc. Bug Fixes

Limitations:
- The driver supports Windows 7/8/8.1/10, Server 2008 R2/2012/2012 R2.
- Due to some compatibility issues, it's not recommended to install Windows 7 driver
  on Windows 8 or Server 2012, vice versa.

Contact e-mail:
nvmewin@lists.openfabrics.org
