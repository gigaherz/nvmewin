NVMe Windows Driver Community Release, July 2013

Description:
This is the first release from NVMe Windows Driver Community. The release includes
installation package and the associated source code. In order to install on the
supported 64-bit Operating Systems, the installation package had been test signed
via Visual Studio 2012. You may find more details below.

Revision: 
1.2.0.0

NVMe Specification compliant: 
1.0e

SVN source code revision number: 
73

Contents:
- readme.txt
- Visual Studio 2012 solution/project files (under nvme directory) 
- installation package (under installations directory), you may find directories:
  - Windows7 (installation package for Windows 7 and Server 2008 R2)
  - Windows8 (installation package for Windows 8 and Server 2012, TRIM enabled)
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
- After nvme solution is built, you may find the test signed packages under nvme\x64
  directory.

Limitations:
- The driver supports Windows 7/8, Server 2008 R2/2012.
- The driver does not support 32-bit Operating Systems.
- The driver does not support Crashdump and hibernation.
- Due to some compatibility issues, it's not recommended to install Windows 7 driver
  on Windows 8 or Server 2012, vice versa.

Contact e-mail:
nvmewin@lists.openfabrics.org
