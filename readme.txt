NVMe Windows Driver Community Release, April 2014

Description:
This is the release as revision 1.3 from NVMe Windows Driver Community. The release
includes installation package and the associated source code. In order to install 
on the supported 64-bit Operating Systems, the 64-bit installation packages had been
test signed via Visual Studio 2012. You may find more details below.

Revision: 
1.3.0.0

Compliant NVMe Specification: 
1.0e

SVN source code revision number: 
96

Contents:
- readme.txt
- Visual Studio 2012 solution/project files (under nvme directory) 
- installation packages (under installations directory):
  - 64-bit\Windows7 (for Windows 7 and Server 2008 R2)
  - 64-bit\Windows8 (for Windows 8 and Server 2012, TRIM enabled)
  - 32-bit\Windows7 (for Windows 7, 32-bit)
  - 32-bit\Windows8 (for Windows 8, 32-bit)
- source code (under source directory)

Prerequisites:
- This driver supports the following 64-bit Windows Operating Systems:
  Windows Server 2008 R2, Windows 7, Windows Server 2012 and Windows 8.
- The target system to install the driver on has to enable test signing:
  bcdedit.exe -set TESTSIGNING ON
- The target system to load the driver on has to disable integrity check:
  bcedit.exe -set loadoptions DDISABLE_INTEGRITY_CHECKS
- The Class Codes of PCI Header of the target device need to be compliant with:
  Base Class Code: Mass Storage Controller (01h)
  Sub-Class Code: Non-Volatile Memory Controller (08h)
  Programming Interface: NVM Express (02h)

Driver building:
- For Windows 7, Server 2008 R2, Server 2012(TRIM disabled) and Windows 8(TRIM disabled),
  You may build the driver within WDK 7600, Windows 7 64-bit build environment or Visual
  Studio 2012 when configured for Window7 in Project Property.
- For Windows Server 2012(TRIM enabled) and Windows 8(TRIM enabled)
  You should build the driver within Visual Studio 2012 when configured for Windows8
  in Project Property.
- To open nvme solution within Visual Studio 2012 build environment, you may simply
  double-click the released solution file(nvme.sln) under nvme directory if you have
  Visual Studio 2012 and Windows 8 WDK installed properly.
- After nvme solution is built successfully, the resulted 64-bit binary is saved 
  nvme\x64 directory and 32-bit binary in nvme\Win7(8)Release(Debug).
- If you need check build driver, after opening the solution file within Visual
  Studio 2012, right click the solution and select "Properties" to bring up
  Configuration Manager and specify the desired configuration/platform.

Change Logs:
Revision 1.3 added:
- Hibernation support.
- NUMA Group support.
- SRB Extension support.
- Proper NVMe Reset handling/checking.
- CPU-MSI map learning rework.
- Logical core enumeration rework.
- 32-bit driver support for Windows 7/8
- Treating LBA Range Type as optional in driver initialization.
- Reset handling rework.
- PRP list construction fix for driver-initiated requests.
- FreeQList access synchronization fix.
- Removal of CHATHAM related codes.

Limitations:
- The driver supports Windows 7/8, Server 2008 R2/2012.
- Due to some compatibility issues, it's not recommended to install Windows 7 driver
  on Windows 8 or Server 2012, vice versa.

Contact e-mail:
nvmewin@lists.openfabrics.org
