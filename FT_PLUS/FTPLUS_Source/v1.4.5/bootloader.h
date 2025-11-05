///////////////////////////////////////////////////////////////////////////
////                       BOOTLOADER.H                                ////
////                                                                   ////
////  This include file must be included by any application loaded     ////
////  by the example bootloader (ex_bootloader.c).                     ////
////                                                                   ////
////  The directives in this file relocate the reset and interrupt     ////
////  vectors as well as reserving space for the bootloader.           ////
////                                                                   ////
////  LOADER_END may need to be adjusted for a specific chip and       ////
////  bootloader.  LOADER_END must be 1 minus a multiple of            ////
////  FLASH_ERASE_SIZE.                                                ////
///////////////////////////////////////////////////////////////////////////
////        (C) Copyright 1996,2013 Custom Computer Services           ////
//// This source code may only be used by licensed users of the CCS    ////
//// C compiler.  This source code may only be distributed to other    ////
//// licensed users of the CCS C compiler.  No other use,              ////
//// reproduction or distribution is permitted without written         ////
//// permission.  Derivative programs created using this software      ////
//// in object code form are not restricted in any way.                ////
///////////////////////////////////////////////////////////////////////////
#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#ifndef LOADER_END
 #if defined(__PCM__)
  #define LOADER_END 0x2FF
 #elif defined(__PCH__)
  #define FLASH_SIZE getenv("FLASH_ERASE_SIZE")
  #if ((0x500 % FLASH_SIZE) == 0)         //IF 0x500 is even flash boundary
   #define LOADER_END   0x4FF
  #else                                  //ELSE, goto next even boundary
   #define LOADER_END   ((0x500+FLASH_SIZE-(0x500 % FLASH_SIZE))-1)
  #endif
 #else
  #error Bootloader only works with PCM or PCH compiler
 #endif
#endif

#define LOADER_SIZE   LOADER_END

#ifndef BOOTLOADER_AT_START
 #define BOOTLOADER_AT_START
#endif

#ifndef _bootloader
 #if defined(__PCM__)
  #build(reset=LOADER_END+1, interrupt=LOADER_END+5)
 #elif defined(__PCH__)
  #build(reset=LOADER_END+1, interrupt=LOADER_END+9)
 #endif

 #org 0, LOADER_END {}
#else
 #ifdef __PCM__
  #if getenv("PROGRAM_MEMORY") <= 0x800
   #org LOADER_END+3, (getenv("PROGRAM_MEMORY") - 1) {}
  #else
   #org LOADER_END+3, 0x7FF {}
   #if getenv("PROGRAM_MEMORY") <= 0x1000
    #org 0x800, (getenv("PROGRAM_MEMORY") - 1) {}
   #else
    #org 0x800, 0xFFF{}
    #if getenv("PROGRAM_MEMORY") <= 0x1800
     #org 0x1000, (getenv("PROGRAM_MEMORY") - 1) {}
    #else
     #org 0x1000, 0x17FF {}
     #if getenv("PROGRAM_MEMORY") <= 0x2000
      #org 0x1800, (getenv("PROGRAM_MEMORY") - 1) {}
     #else
      #org 0x1800, 0x1FFF {}
      #if getenv("PROGRAM_MEMORY") <= 0x2800
       #org 0x2000, (getenv("PROGRAM_MEMORY") - 1) {}
      #else
       #org 0x2000, 0x27FF {}
       #if getenv("PROGRAM_MEMORY") <= 0x3000
        #org 0x2800, (getenv("PROGRAM_MEMORY") - 1) {}
       #else
        #org 0x2800, 0x2FFF {}
        #if getenv("PROGRAM_MEMORY") <= 0x3800
         #org 0x3000, (getenv("PROGRAM_MEMORY") - 1) {}
        #else
         #org 0x3000, 0x37FF {}
         #org 0x3800, 0x3FFF {}
        #endif
       #endif
      #endif
     #endif
    #endif
   #endif
  #endif
 #else
  #if getenv("PROGRAM_MEMORY") <= 0x10000
   #org LOADER_END+5, (getenv("PROGRAM_MEMORY") - 1) {}
  #else
   #org LOADER_END+5, 0xFFFE {}
   #org 0x10000, (getenv("PROGRAM_MEMORY") - 1) {}
  #endif
 #endif
#endif

#endif
