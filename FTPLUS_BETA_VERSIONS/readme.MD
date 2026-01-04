FTPlus BETA channel.
-
These are BETA / TESTING versions of FTPlus.
-
No supplement is included, instead new notes will be provided here until certified release-ready.
These releases may appear or disappear from time to time. Things may be missing or broken,
though care is taken to ensure everything works.
-
Remember HEX = Pickit version, BLHEX = FTPlus updater version. 
-
FTPlus updater app is found elsewhere in my repo. Requires CAT!
-

v1.5.2 Proposed changes (included in above HEX and BLHEX files)

BUGFIX: Bug found: Occasionally, baud selection would activate when switching radio on.

BUGFIX: Bug found: Custom offset wasn't applied, or applied incorrectly. An int32 in wrong place. Offset being applied AFTER n-code calculation. Matthew is a clown.

Reworked accelerated dial to a stepped-speed approach

Replaced loading nag screen in app

New combined codebase, supporting FTMinus and FTPlus, switchable in source. Source has not been published yet.

New app, allowing configuration of FTPlus and management of memories. (Windows app or Linux in Wine).
When app is running and the "magic packet" has been received from the radio, FTPlus stays at 4800 baud until the app, which I've called "Configurator"
is exited and radio is powered on and off. This has been designed to be seamless and automatic. If not using configurator, startup will be standard. It's all automatic.

Baud rate is now changed by holding down M>VFO at power-on and manually choosing a baud rate, by rotating dial or pressing UP / DOWN.
Confirm your option by pressing M>VFO again.



