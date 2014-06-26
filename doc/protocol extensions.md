# Palace protocol extension proposal
The following proposes server-side extensions to the QPalace server
implementation. As such, other third-party clients could potentially be incompatible.
These additions will be implemented to be opt-in at the host's discretion
as to maintain support for other client software.
## Message structures
### AuxRegistrationRec
#### auxFlags
ID | Value | Info
--- | --- | ---
LI_AUXFLAGS_MacIntel | 6 | Mac OS X Intel
LI_AUXFLAGS_Win64 | 7 | Microsoft Windows 64-bit
LI_AUXFLAGS_Linux32 | 8 | Linux 32-bit
LI_AUXFLAGS_Linux64 | 9 | Linux 64-bit
LI_AUXFLAGS_LinuxARM | 10 | Linux on ARM
LI_AUXFLAGS_AndroidARMv5 | 11 | Android on ARMv5
LI_AUXFLAGS_AndroidARMv7 | 12 | Android on ARMv7
LI_AUXFLAGS_AndroidIntel | 13 | Android on Intel
LI_AUXFLAGS_Dalvik | 14 | Dalvik JVM
LI_AUXFLAGS_iOS | 15 | iOS
LI_AUXFLAGS_iOSSim | 16 | iOS Simulator
LI_AUXFLAGS_WinRT | 17 | Microsoft Windows RT
LI_AUXFLAGS_CLR | 18 | Common Language Runtime (Mono/.NET)
LI_AUXFLAGS_Python | 19 | Python
LI_AUXFLAGS_Ruby | 20 | Ruby
LI_AUXFLAGS_Flash | 21 | Adobe Flash/Apache Flex
### ul2DGraphicsCaps
ID | Value | Info
--- | --- | ---
LI_2DGRAPHCAP_PNG | 0x00000040 |
